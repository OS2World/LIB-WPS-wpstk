/****************************** Module Header ******************************\
*
* Module Name: wtkset.c
*
* Source for the settings and details manager for WPS classes.
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: wtkset.c,v 1.53 2009-07-03 20:31:38 cla Exp $
*
* ===========================================================================
*
* This file is part of the WPS Toolkit package and is free software.  You can
* redistribute it and/or modify it under the terms of the GNU Library General
* Public License as published by the Free Software Foundation, in version 2
* as it comes in the "COPYING.LIB" file of the WPS Toolkit main distribution.
* This library is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
* License for more details.
*
\***************************************************************************/

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


#define INCL_DOS
#define INCL_WIN
#define INCL_WINWORKPLACE
#define INCL_ERRORS
#include <os2.h>

#include "wtkset.h"
#include "wtkuctl.h"
#include "wtkusys.h"
#include "warp4def.h"
#include "wpstk.ih"

#include <som.h>
#include <wpobject.h>

// helper macros
#define EMPTYSTRING(p) (!p || (!*(PBYTE)p))
#define ENDOFSTRING(s) (s+strlen(s))
#define NEXTSTRING(s)  (s+strlen(s)+1)

#define _c_ ,
#ifdef DEBUG
#define DMARK                  printf( "MARK: %s:%u\n", __FUNCTION__, __LINE__)
#define DEBUGMSG(s,p)          __debugmsg( ulDebugAPIValue, -1,                       s, p)
#define DEBUGMSGDETAIL(s,p)    __debugmsg( ulDebugAPIValue, STM_DEBUG_FUNC_DETAILS,   s, p)
#define DEBUGMSGCALLBACK(s,p)  __debugmsg( ulDebugAPIValue, STM_DEBUG_FUNC_CALLBACKS, s, p)
#else
#define DMARK
#define DEBUGMSG(s,p)
#define DEBUGMSGDETAIL(s,p)
#define DEBUGMSGCALLBACK(s,p)
#endif

// *** masks for debug messages

// global
#define STM_DEBUG_ALL                             ((ULONG) 0xffffffff)
#define STM_DEBUG_NONE                            ((ULONG) 0x00000000)

// mask for debug info details
#define STM_DEBUG_MASK_FUNC_DETAILS               ((ULONG) 0xff000000)
#define STM_DEBUG_FUNC_ENTRYEXIT                  ((ULONG) 0x01000000)
#define STM_DEBUG_FUNC_DETAILS                    ((ULONG) 0x02000000)
#define STM_DEBUG_FUNC_MAIN                       ((ULONG) 0x03000000) /* combination */
#define STM_DEBUG_FUNC_CALLBACKS                  ((ULONG) 0x04000000)

// mask for APIs concerning all provided APIs
#define STM_DEBUG_MASK_API                        ((ULONG) 0x00ffffff)

// masks for APIs concerning the metaclass part
#define STM_DEBUG_MASK_API_METACLASS              ((ULONG) 0x00ff0000)
#define STM_DEBUG_API_CREATECLASSSETTINGSTABLE    ((ULONG) 0x00010000)
#define STM_DEBUG_API_DESTROYCLASSSETTINGSTABLE   ((ULONG) 0x00020000)
#define STM_DEBUG_API_ADDCLASSSETTING             ((ULONG) 0x00040000)
#define STM_DEBUG_API_ADDCLASSDETAIL              ((ULONG) 0x00080000)
#define STM_DEBUG_API_CLOSECLASSSETTINGSTABLE     ((ULONG) 0x00100000)
#define STM_DEBUG_API_QUERYDETAILSINFO            ((ULONG) 0x00200000)
#define STM_DEBUG_API_QUERYICONDATA               ((ULONG) 0x00400000)
#define STM_DEBUG_API_QUERYOBJECTCLASS            ((ULONG) 0x00800000)

// masks for APIs concerning the object instance part
#define STM_DEBUG_MASK_API_OBJECTCLASS            ((ULONG) 0x0000ffff)
#define STM_DEBUG_API_CREATEOBJECTVALUETABLE      ((ULONG) 0x00000001)
#define STM_DEBUG_API_DESTROYOBJECTVALUETABLE     ((ULONG) 0x00000002)
#define STM_DEBUG_API_EVALUATEOBJECTSETTINGS      ((ULONG) 0x00000004)
#define STM_DEBUG_API_QUERYOBJECTSETTINGS         ((ULONG) 0x00000008)
#define STM_DEBUG_API_QUERYGUICONTROLSCHANGED     ((ULONG) 0x00000010)
#define STM_DEBUG_API_REGISTERSETTINGSDIALOG      ((ULONG) 0x00000020)
#define STM_DEBUG_API_DEREGISTERSETTINGSDIALOG    ((ULONG) 0x00000040)
#define STM_DEBUG_API_REGISTERSETTINGSNOTEBOOK    ((ULONG) 0x00000080)
#define STM_DEBUG_API_VALIDATEVALUETABLE          ((ULONG) 0x00000100)
#define STM_DEBUG_API_READVALUETABLE              ((ULONG) 0x00000200)
#define STM_DEBUG_API_WRITEVALUETABLE             ((ULONG) 0x00000400)
#define STM_DEBUG_API_SAVESETTINGS                ((ULONG) 0x00000800)
#define STM_DEBUG_API_RESTORESETTINGS             ((ULONG) 0x00001000)
#define STM_DEBUG_API_QUERYDETAILSDATA            ((ULONG) 0x00002000)
#define STM_DEBUG_API_SAVESTATE                   ((ULONG) 0x00004000)
#define STM_DEBUG_API_RESTORESTATE                ((ULONG) 0x00010000)
#define STM_DEBUG_API_QUERYOBJECTINSTANCE         ((ULONG) 0x00020000)
#define STM_DEBUG_API_QUERYSETTINGTABLE           ((ULONG) 0x00040000)

// memory sig for settings data
#define SIG_SETTINGSMANAGER 0x4D53   /* 'MS' -  stored like 'SM' on intel ;-) */
#define SIG_VALUEMANAGER    0x4D56   /* 'MV'                                  */

// macros for verifying object pointers etc.
#define SETERROR(o,rc)                 if (somIsObj( o)) _wpSetError( o, rc)
#define REFRESHDETAILS(o)              if (somIsObj( o)) _wpCnrRefreshDetails( o)
#define QUERYHANDLE(o)                 _wpQueryHandle( o)

// macros for allocating memory etc.
#ifdef DEBUG
   #define OBJECTVALID(p)                 (p != NULL)
   #define OBJ_ALLOCATEMEMORY(o,s,prc)    (malloc( s))
   #define OBJ_FREEMEMORY(o,p)            if (p) free( p)
   #define CLS_ALLOCATEMEMORY(o,s,prc)    (malloc( s))
   #define CLS_FREEMEMORY(o,p)            if (p) free( p)
#else
   #define OBJECTVALID(p)                 (somIsObj( p))
   #define OBJ_ALLOCATEMEMORY(o,s,prc)    ((PVOID)_wpAllocMem(o,s,prc))
   #define OBJ_FREEMEMORY(o,p)            if (p) _wpFreeMem( o, (PVOID)p)
   #define CLS_ALLOCATEMEMORY(o,s,prc)    (SOMMalloc( s))
   #define CLS_FREEMEMORY(o,p)            if (p) SOMFree( p)
#endif

// macro for calculation of settings id ranges
#define SETTINGSID(id,vx,qx) (id + (vx * qx) - 1)

// ------------------------------------------------------------

#pragma pack(1)

// ----- settings definition

typedef struct _SETTINGSDATA
   {
         PSZ            pszName;                      // pointer to name and values
         ULONG          ulSettingId;                  // id of setting
         ULONG          ulValueCount;                 // number of default setting values
         ULONG          ulQueryCount;                 // how often setting should be queried
         ULONG          ulThisEntryLen;               // length of this structure plus name + tables
         PSZ            pszAllTitles;                 // pointer to all titles

         // --- table items start
         // - these point to memory being allocated with SETTINGSDATA
         //   in one chunk, so no separate free() necessary
         // - modify SETTING_PTRTABLE_COUNT when changing number of table items!
         PULONG         paulValueType;                // points to table of value types
         PUSHORT        pausDialogid;                 // points to table of dialog ids
         PUSHORT        pausControlid;                // points to table of control ids
         PHWND          pahwndControl;                // points to handles of the controls
         PFNWP         *papfnwpSubclass;              // points to subclass procedures per value
         PSZ           *paszDetailsTitle;             // points to table of display flags per value
         PULONG         paulDispStringMaxLen;         // points to table of maxlen of display string of indexed values
         // --- table items end

         PVOID          pvNextSetting;
   } SETTINGSDATA, *PSETTINGSDATA;

// ----- root of all settings for an object class

typedef struct _SETTINGSDATAROOT
   {
         ULONG          ulSig;                        // sig, must be SIG_SETTINGSMANAGER
         ULONG          fTableComplete;               // table may not be used before completion
         PVOID          pvObjectClass;                // somSelf of meta class
         PFNCB          pfnCallbackValue;             // ptr to callback routine
         ULONG          ulDebugMask;                  // debug mask for this setting table
         ULONG          ulDetailsCount;               // count of details in pcfi
         ULONG          ulDetailsSize;                // size of details data
         PCLASSFIELDINFO pcfi;                        // pointer to details info
         PSETTINGSDATA  psdFirstSetting;              // points to first settings definition
   } SETTINGSDATAROOT, *PSETTINGSDATAROOT;

// ----- value definition

typedef struct _VALUEDATA
   {
         PSETTINGSDATA  psd;
         PSZ           *papszValues;                  // points to table of target buffers
         PULONG         paulValueMaxLen;              // points to table of max len values
         PVOID          pvNextValue;                  // number of setting values to be evaluate
   } VALUEDATA, *PVALUEDATA;

// ----- root of all settings for an object class

typedef struct _VALUEDATAROOT
   {

         ULONG          ulSig;                        // sig, must be SIG_VALUEMANAGER
         PSETTINGSDATAROOT psdr;                      // pointer to setting data root
         PVOID          pvObjectInstance;             // somSelf of object instance
         PVOID          pvObjectData;                 // somSelf of object instance
         PVALUEDATA     pvdFirstValue;                // points to first value data
         PSZ            pszObjectDetailsItems;        // pointer to details data
         PFNWP          pfnwpNotebookFrameOrg;        // pointer to original settings notebook proc
         HWND           hwndNotebook;                 // handle to open settings notebook
   } VALUEDATAROOT, *PVALUEDATAROOT;

// ---------------------------------------------------------------------------
// global variables
// ---------------------------------------------------------------------------

static   ULONG         ulGlobalDebugMask  = 0;

// ---------------------------------------------------------------------------
// internal prototypes
// ---------------------------------------------------------------------------

static VOID __SetDebugInfoMask( VOID); /* this function at the end of this file */

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static PSZ __getValueType( ULONG ulValueType)
{
         PSZ            pszValueType;

switch (ulValueType)
   {
   case STM_VALUETYPE_STRING:    pszValueType = "STRING";               break;
   case STM_VALUETYPE_INDEX:     pszValueType = "INDEX";                break;
   case STM_VALUETYPE_INDEXITEM: pszValueType = "INDEXITEM";            break;
   case STM_VALUETYPE_LONG:      pszValueType = "LONG";                 break;
   case STM_VALUETYPE_TRUEFALSE: pszValueType = "TRUEFALSE";            break;
   case STM_VALUETYPE_YESNO:     pszValueType = "YESNO";                break;
   case STM_VALUETYPE_ONOFF:     pszValueType = "ONOFF";                break;
   default:                      pszValueType = "<unsupported type !>"; break;
   } // switch

return pszValueType;
}

// ---------------------------------------------------------------------------

static VOID __getDetailsType( ULONG ulValueType, PULONG pulFieldType, PULONG pulFieldFlags,
                              PULONG pulFieldLen, PSZ *ppszFieldType, PBOOL pfIndexedValue)
{
         ULONG             ulFieldType  = 0;
         ULONG             ulFieldLen   = 0;
         ULONG             ulFieldFlags = 0;
         PSZ               pszFieldType = "<unknown>";
         ULONG             fIndexedValue = FALSE;

switch (ulValueType)
   {
   case STM_VALUETYPE_TRUEFALSE:
   case STM_VALUETYPE_YESNO:
   case STM_VALUETYPE_INDEX:
   case STM_VALUETYPE_ONOFF:
   case STM_VALUETYPE_INDEXITEM:
      fIndexedValue = TRUE;
      // fallthru !!!!

   case STM_VALUETYPE_STRING:
      ulFieldType  = CFA_STRING;
      ulFieldFlags = (fIndexedValue) ? CFA_CENTER : CFA_LEFT;
      ulFieldLen   = sizeof( PSZ);
      pszFieldType = "CFA_STRING";
      break;

   case STM_VALUETYPE_LONG:
      ulFieldType = CFA_ULONG;
      ulFieldFlags = (fIndexedValue) ? CFA_CENTER : CFA_RIGHT;
      ulFieldLen  = sizeof( ULONG);
      pszFieldType = "CFA_ULONG";
      break;

   case STM_VALUETYPE_CDATE:
      ulFieldType = CFA_DATE;
      ulFieldFlags = CFA_RIGHT;
      ulFieldLen  = sizeof( CDATE);
      pszFieldType = "CFA_CDATE";
      break;

   case STM_VALUETYPE_CTIME:
      ulFieldType = CFA_TIME;
      ulFieldFlags = CFA_RIGHT;
      ulFieldLen  = sizeof( CTIME);
      pszFieldType = "CFA_TIME";
      break;

   } // switch

if (pulFieldType)   *pulFieldType   = ulFieldType;
if (pulFieldLen)    *pulFieldLen    = ulFieldLen;
if (pulFieldFlags)  *pulFieldFlags  = ulFieldFlags;
if (ppszFieldType)  *ppszFieldType  = pszFieldType;
if (pfIndexedValue) *pfIndexedValue = fIndexedValue;

return;
}

// ---------------------------------------------------------------------------

static BOOL __isnumeric( PSZ pszString)
{
         BOOL fResult = TRUE;

do
   {
   // check Parms
   if ((pszString == NULL) ||
       (*pszString == 0))
      {
      fResult = FALSE;
      break;
      }

   // check digits
   while (*pszString != 0)
      {
      if (!isdigit( *pszString))
         {
         fResult = FALSE;
         break;
         }
      else
         pszString++;
      }

   } while (FALSE);

return fResult;
}

// ---------------------------------------------------------------------------

static char * __ultohex( unsigned long value, char * str)
{
     char * p = str;

strcpy( p, "0x");
p += 2;
_ltoa( value, p, 16);
return str;
}

// ---------------------------------------------------------------------------

static BOOL __isListboxIndexValid( USHORT usIndex)
{
         BOOL              fNotError;
         BOOL              fNotNone;
         BOOL              fValid;

fNotError = (usIndex != (USHORT) LIT_ERROR);
fNotNone  = (usIndex != (USHORT) LIT_NONE);
fValid    = (fNotError) && (fNotNone);
return fValid;

}

// ---------------------------------------------------------------------------

static BOOL __isRadioButton( HWND hwndControl)
{
         BOOL              fResult = FALSE;
         CHAR              szClassname[ 40];
         PSZ               pszClassIndex;
         ULONG             ulWindowStyle;

do
   {
   // check parms
   if (!hwndControl)
      break;

   pszClassIndex = WtkQueryClassIndex( hwndControl);
   if (pszClassIndex != WC_BUTTON)
      break;

   ulWindowStyle = WinQueryWindowULong( hwndControl, QWL_STYLE) & BS_PRIMARYSTYLES;
   if ((ulWindowStyle != BS_RADIOBUTTON) && (ulWindowStyle != BS_AUTORADIOBUTTON))
      break;

   fResult = TRUE;

   } while (FALSE);

return fResult;
}
// ---------------------------------------------------------------------------

static HINI __openProfile( HAB hab, PSZ pszFilename)

{

         HINI           hiniResult = NULLHANDLE;
do
   {
   // check parm
   if (!pszFilename)
      break;

   // do the job
   if (!strcmp( pszFilename, "USER"))
      hiniResult = HINI_USER;
   else if (!strcmp( pszFilename, "SYSTEM"))
      hiniResult = HINI_SYSTEM;
   else
      hiniResult = PrfOpenProfile( hab, pszFilename);

   } while (FALSE);

return hiniResult;
}

// ---------------------------------------------------------------------------

static VOID __debugmsg( ULONG ulDebugAPIValue, ULONG ulDebugDetailsValue, PSZ pszMessage, ...)
{
         va_list        arg_ptr;
         PSZ            pszBuffer     = NULL;
         ULONG          ulAPIBits     = ulGlobalDebugMask & STM_DEBUG_MASK_API;
         ULONG          ulDetailsBits = ulGlobalDebugMask & STM_DEBUG_MASK_FUNC_DETAILS;

do
   {

#ifdef DEBUG
{
         ULONG          ulSize = 0;

   PrfQueryProfileSize( HINI_USER, "WPSTK", "wtkset_NoDebugMessage", &ulSize);
   if (ulSize)
      break;
}
#endif

   // output this message ?
   if ((ulDebugAPIValue & ulAPIBits) == 0)
      break;
   if ((ulDebugDetailsValue & ulDetailsBits) == 0)
      break;

   // get temporary memory
   pszBuffer = malloc( 1024);
   if (!pszBuffer)
      break;

   // get the pointer to the argument list
   va_start( arg_ptr, pszMessage);
   vsprintf( pszBuffer, pszMessage, arg_ptr);
   va_end( arg_ptr);
   printf( pszBuffer);

   } while (FALSE);

if (pszBuffer) free( pszBuffer);
return;
}

// ---------------------------------------------------------------------------

// scan the next setting from a setup string
// - tokenize first setting in string, replace equal sign, comma and semicolon with zero byte
// parms
//   ppszSetupString
//     in  - pointer to complete settingsstring     like "A=a1,a2,a3;B=b1,b2;C=3;"
//     out - pointer to the next setting in string  like "B=b1,b2;C=3");
//   pulValueCount
//     out - number of subvalues found
//   returns PSZ
//           pointer to name of setting being tokenized

static PSZ __getSettingFromSetupString( PSZ* ppszSetupString, PULONG pulValueCount)
{
         PSZ            pszResult = NULL;

         PSZ            pszEndOfValue;
         PSZ            pszTmp;

         PSZ            pszValue;

do
   {
   // check parms
   if ((!ppszSetupString) || (!pulValueCount))
      break;
   *pulValueCount = 0;

   // search equal sign; exit, if not present
   pszValue = strchr( *ppszSetupString, '=');
   if (!pszValue)
      break;
   pszResult = *ppszSetupString;

   // remove equal sing
   *pszValue = 0;
   pszValue++;

   // search end of setting
   pszEndOfValue = strchr( pszValue, ';');
   if (!pszEndOfValue)
      pszEndOfValue = ENDOFSTRING( pszValue);

   // take care for escape character
   while ((*pszEndOfValue) && (*(pszEndOfValue - 1) == '^'))
      {
      pszTmp = strchr( pszEndOfValue, ';');
      if (pszTmp)
         pszEndOfValue = pszTmp;
      }

   // now remove semicolon, move main pointer to next string
   if (*pszEndOfValue)
      *ppszSetupString = pszEndOfValue + 1;
   else
      *ppszSetupString = NULL;
   *pszEndOfValue = 0;

   // search subvalues, take care for escape character
   *pulValueCount = 1;
   while (*pszValue)
      {
      pszTmp = strchr( pszValue, ',');
      if (!pszTmp)
         pszValue = ENDOFSTRING( pszValue);
      else
         {
         if (*(pszValue - 1) == '^')
            {
            // ignore comma with escape character
            pszValue++;
            continue;
            }
         else
            {
            // subvalue found
            (*pulValueCount)++;
            *pszTmp = 0;
            pszValue = pszTmp + 1;
            }
         }
      }

   } while (FALSE);

return pszResult;
}

// ---------------------------------------------------------------------------

// search value data for a given setting name in value table
static PVALUEDATA __getValueData( PVALUEDATAROOT pvdr, PSZ pszName)
{
         PVALUEDATA  pvd = NULL;
         PVALUEDATA  pvdResult = NULL;

do
   {
   // check parms
   if ((!pvdr) || (!pszName))
      break;

   // no settings available ?
   if (!pvdr->pvdFirstValue)
      break;

   // check all entries
   for (pvd = pvdr->pvdFirstValue; pvd != NULL; pvd = pvd->pvNextValue)
      {
      if (!stricmp( pvd->psd->pszName, pszName))
         {
         pvdResult = pvd;
         break;
         }
      }

   } while (FALSE);

return pvdResult;
}
// ---------------------------------------------------------------------------

// search setting data for a given setting name in setting table
static PSETTINGSDATA __getSettingsData( PSETTINGSDATAROOT psdr, PSZ pszName, ULONG ulSettingId)
{
         PSETTINGSDATA  psd = NULL;
         PSETTINGSDATA  psdResult = NULL;

do
   {
   // check parms
   if ((!psdr) || (!pszName))
      break;

   // no settings available ?
   if (!psdr->psdFirstSetting)
      break;

   // check all entries
   for (psd = psdr->psdFirstSetting; psd != NULL; psd = psd->pvNextSetting)
      {
      // check name if given
      if ((*pszName) && (!stricmp( psd->pszName, pszName)))
         {
         psdResult = psd;
         break;
         }

      // check id, if not zero
      if ((ulSettingId) && (psd->ulSettingId == ulSettingId))
         {
         psdResult = psd;
         break;
         }
      }

   } while (FALSE);

return psdResult;
}

// ---------------------------------------------------------------------------

// macros for converting pointers to handles
#define PTR2HANDLE(p,sig)         ((ULONG)p^(sig << 16))
#define HANDLE2PTR(h,sig)         ((PVOID)(h^(sig << 16)))

// return valid pointer out of mangled pointer
static PVOID __getPointerFromHandle( LHANDLE h, ULONG ulSig)
{
         PVOID          p = NULL;
do
   {
   // check parms
   if (!h)
      break;

   // determine and check the pointer
   p = HANDLE2PTR(h, ulSig);

   if (*(PULONG)p != ulSig)
      p = NULL;

   } while (FALSE);

return p;
}

// ---------------------------------------------------------------------------

// request hab, if necessary. Return TRUE, if it had to be requested,
// so that it can be destroyed on resource cleanup.
static BOOL __requestHandleAnchorBlock( PHAB phab)
{
         BOOL           fHandleAnchorBlockRequested = FALSE;
do
   {
   // check parm
   if (!phab)
      break;

   // need hab to be requested ?
   *phab = WinQueryAnchorBlock( HWND_DESKTOP);

   if (!*phab)
      {
      // yes, request a new one
      *phab = WinInitialize( 0);
      fHandleAnchorBlockRequested = (*phab != NULLHANDLE);
      }

   } while (FALSE);

return fHandleAnchorBlockRequested;
}

// ---------------------------------------------------------------------------

// callback for retrieving an index for a setting and a given (sub)value string
static LONG __getIndexValue( ULONG ulDebugAPIValue, BOOL fAtNewLine, PVALUEDATAROOT pvdr, PVALUEDATA pvd, ULONG ulValueIndex, PSZ pszValue)
{
         CBQUERYINDEX   qi;
         LONG           lResult = -1;

if (!fAtNewLine)
   DEBUGMSGCALLBACK( "\n", 0);

// ask the WPS class for translation of string to control index
if (*pvd->psd->pszName)
   DEBUGMSGCALLBACK( "#   >> callback: translate string \"%s\" to index for setting %s:%u id %u(0x%04x) value is %s\n",
                     pszValue _c_
                     pvd->psd->pszName _c_
                     ulValueIndex _c_
                     pvd->psd->ulSettingId _c_
                     pvd->psd->ulSettingId _c_
                     pszValue);
else
   DEBUGMSGCALLBACK( "#   >> callback: translate string \"%s\" to index for detail %u(0x%04x), value is %s\n",
                     pszValue _c_
                     pvd->psd->ulSettingId _c_
                     pvd->psd->ulSettingId _c_
                     pszValue);

memset( &qi, 0, sizeof( qi));
qi.pvObjectInstance = pvdr->pvObjectInstance;
qi.pszName          = pvd->psd->pszName;
qi.ulSettingId      = pvd->psd->ulSettingId;
qi.pszValue         = pszValue;
qi.ulValueIndex     = ulValueIndex;
if ((pvdr->psdr->pfnCallbackValue)( STM_CALLBACK_QUERYINDEX, &qi, pvdr->pvObjectInstance, pvdr->pvObjectData))
   lResult = qi.ulStringIndex;

if (!fAtNewLine)
   DEBUGMSGCALLBACK( "#   >>  ", 0);

return lResult;
}

// ---------------------------------------------------------------------------

// callback for retrieving a string for a setting and a given (sub)value index number
static PSZ __getIndexString( ULONG ulDebugAPIValue,  BOOL fAtNewLine, PVALUEDATAROOT pvdr, PVALUEDATA pvd, ULONG ulValueIndex, ULONG lValue)
{
         CBQUERYSTRING  qs;
         PSZ            pszResult = "";

if (!fAtNewLine)
   DEBUGMSGCALLBACK( "\n", 0);

// ask the WPS class for translation of index to string
if (*pvd->psd->pszName)
   DEBUGMSGCALLBACK( "#   >> callback: translate index to string for setting %s:%u id %u(0x%04x) value is %u\n",
                     pvd->psd->pszName _c_
                     ulValueIndex _c_
                     pvd->psd->ulSettingId _c_
                     pvd->psd->ulSettingId _c_
                     lValue);
else
   DEBUGMSGCALLBACK( "#   >> callback: translate index to string for detail %u(0x%04x), value is %u\n",
                     pvd->psd->ulSettingId _c_
                     pvd->psd->ulSettingId _c_
                     lValue);

memset( &qs, 0, sizeof( qs));
qs.pvObjectInstance = pvdr->pvObjectInstance;
qs.pszName          = pvd->psd->pszName;
qs.ulSettingId      = pvd->psd->ulSettingId;
qs.ulStringIndex    = lValue;
qs.ulValueIndex     = ulValueIndex;
if ((pvdr->psdr->pfnCallbackValue)( STM_CALLBACK_QUERYSTRING, &qs, pvdr->pvObjectInstance, pvdr->pvObjectData))
   pszResult = qs.szValue;

if (!fAtNewLine)
   DEBUGMSGCALLBACK( "#   >>  ", 0);

return strdup( pszResult);
}

// ---------------------------------------------------------------------------

// update the GUI control according to the value type and the PM class of the control
BOOL static __updateGuiControl( ULONG ulDebugAPIValue, PVALUEDATAROOT pvdr, PVALUEDATA pvd,
                         HWND hwndControl, ULONG ulValueIndex)
{
         BOOL              fResult = FALSE;

         CHAR              szClassname[ 40];
         PSZ               pszClassIndex;
         ULONG             ulValueType;

         HWND              hwndDialog;
         USHORT            usDialogid;
         USHORT            usControlid;
         ULONG             ulWindowStyle;
         HWND              hwndRadioButton;

         CHAR              szValue[ 30];
         PSZ               pszValue;
         LONG              lValue   = 0;
         BOOL              fValue   = FALSE;
         BOOL              fTextValueRequired = FALSE;

         CBQUERYINDEX      qi;

do
   {
   if ((!pvdr) || (!pvd))
      break;

   ulValueType = *(pvd->psd->paulValueType + ulValueIndex);
   hwndDialog  = WinQueryWindow( hwndControl, QW_PARENT);
   usControlid = WinQueryWindowUShort( hwndControl, QWS_ID);
   usDialogid  = WinQueryWindowUShort( hwndDialog, QWS_ID);

   if (!hwndControl)
      {
      DEBUGMSGDETAIL( "#   >> no update GUI for %s:%u, type %s\n",
                      pvd->psd->pszName _c_
                      ulValueIndex _c_
                      __getValueType(ulValueType));
      break;
      }

   DEBUGMSGDETAIL( "#   >> update GUI control %u(0x%04x) of dialog %u(0x%04x), %s:%u, type %s",
                   usControlid _c_
                   usControlid _c_
                   usDialogid _c_
                   usDialogid _c_
                   pvd->psd->pszName _c_
                   ulValueIndex _c_
                   __getValueType(ulValueType));

   // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

   // build a WC_* constant
   pszClassIndex = WtkQueryClassIndex( hwndControl);
   DEBUGMSGDETAIL( " - %s", __getPMClassName( pszClassIndex));

   // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

   // check it a text value is required
   if ((ulValueType == STM_VALUETYPE_INDEX) ||
       (ulValueType == STM_VALUETYPE_INDEXITEM))
      {
      switch ((ULONG) pszClassIndex)
         {
         case (ULONG) WC_MLE:
         case (ULONG) WC_ENTRYFIELD:
         case (ULONG) WC_STATIC:
         case (ULONG) WC_LISTBOX:
         case (ULONG) WC_COMBOBOX:
            fTextValueRequired = TRUE;
            break;

         case (ULONG) WC_SPINBUTTON:
            {
                     ULONG             ulUpperLimit;
                     ULONG             ulLowerLimit;

            // is it an array ?
            if (!WinSendMsg( hwndControl, SPBM_QUERYLIMITS, MPFROMP( &ulUpperLimit),  MPFROMP( &ulLowerLimit)))
               fTextValueRequired = TRUE;
            }
            break;

         } // switch (pszClassIndex)
      }

   if (fTextValueRequired)
      DEBUGMSGDETAIL( " ->text", 0);

   // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

   // determine (text) value
   DEBUGMSGDETAIL( " source 0x%08x", *(pvd->papszValues + ulValueIndex));
   switch (ulValueType)
      {
      case STM_VALUETYPE_INDEX:
         lValue = *(PLONG)*(pvd->papszValues + ulValueIndex);
         if (fTextValueRequired)
            {
            // make it a string here
            // ask the WPS class for translation of index to string
            // this already returns a strdup !
            pszValue = __getIndexString( ulDebugAPIValue, FALSE, pvdr, pvd, ulValueIndex, lValue);
            }
         else
            {
            _ltoa( lValue, szValue, 10);
            pszValue = szValue;
            fValue = lValue;
            DEBUGMSGDETAIL( " - index \"%d\"", lValue);
            }
         fValue = lValue;
         break;

      case STM_VALUETYPE_INDEXITEM:
      case STM_VALUETYPE_LONG:
         lValue = *(PLONG)*(pvd->papszValues + ulValueIndex);
         _ltoa( lValue, szValue, 10);
         pszValue = szValue;
         fValue = lValue;
         break;

      case STM_VALUETYPE_STRING:
         pszValue = (PSZ)*(pvd->papszValues + ulValueIndex);
         break;

      case STM_VALUETYPE_TRUEFALSE:
         fValue = *(PBOOL) *(pvd->papszValues + ulValueIndex);
         strcpy( szValue, (fValue) ? "TRUE" : "FALSE");
         pszValue = szValue;
         break;

      case STM_VALUETYPE_YESNO:
         fValue = *(PBOOL) *(pvd->papszValues + ulValueIndex);
         strcpy( szValue, fValue ? "YES" : "NO");
         pszValue = szValue;
         break;

      case STM_VALUETYPE_ONOFF:
         fValue = *(PBOOL) *(pvd->papszValues + ulValueIndex);
         strcpy( szValue, fValue ? "ON" : "OFF");
         pszValue = szValue;
         break;
      }

   DEBUGMSGDETAIL( " - value: \"%s\"\n", (*pszValue) ? pszValue : "");

   // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

   // now update the GUI control according to its class
   switch ((ULONG) pszClassIndex)
      {

      // -   -   -   -   -   -   -   -   -

      case (ULONG) WC_BUTTON:
         ulWindowStyle = WinQueryWindowULong( hwndControl, QWL_STYLE) & BS_PRIMARYSTYLES;
         switch (ulWindowStyle)
            {
            case BS_CHECKBOX:
            case BS_AUTOCHECKBOX:
            case BS_3STATE:
            case BS_AUTO3STATE:
               WinSendMsg( hwndControl, BM_SETCHECK, MPFROMLONG( fValue), 0);
               WinSendMsg( hwndDialog, WM_CONTROL, MPFROM2SHORT( usControlid, BN_CLICKED), 0);
               fResult = TRUE;
               break;

            case BS_RADIOBUTTON:
            case BS_AUTORADIOBUTTON:
               if (!lValue)
                  hwndRadioButton = hwndControl;
               else
                  {
                  // get handle of radio button with indexed id
                  usControlid += (USHORT) lValue;
                  hwndRadioButton = WinWindowFromID( hwndDialog, usControlid);
                  if (!__isRadioButton( hwndRadioButton))
                     break;
                  }

               // click radio button and tell dialog about it
               WinSendMsg( hwndRadioButton, BM_SETCHECK, MPFROM2SHORT( TRUE, 0), 0);
               WinSendMsg( hwndDialog, WM_CONTROL, MPFROM2SHORT( usControlid, BN_CLICKED), 0);
               fResult = TRUE;
               break;

            } // switch ( ulWindowStyle)
         break;

      case (ULONG) WC_MLE:
      case (ULONG) WC_ENTRYFIELD:
      case (ULONG) WC_STATIC:
         // replace the text with the value
         fResult = WinSetWindowText( hwndControl, pszValue);
         break; // case WC_MLE: case WC_ENTRYFIELD: case WC_STATIC:

      // -   -   -   -   -   -   -   -   -

      case (ULONG) WC_LISTBOX:
      case (ULONG) WC_COMBOBOX:
         {

                  USHORT            usIndex;

         if (*pszValue == 0)
            usIndex = 0;
         else
            {
            if (ulValueType == STM_VALUETYPE_INDEXITEM)
               {
               // just use index as item number
               usIndex = atol( pszValue);
               }
            else
               // search string in listbox and select it, if found
               usIndex = SHORT1FROMMR( WinSendMsg( hwndControl, LM_SEARCHSTRING, MPFROM2SHORT( 0, LIT_FIRST), pszValue));
            }

         if (__isListboxIndexValid( usIndex))
            fResult = LONGFROMMR( WinSendMsg( hwndControl, LM_SELECTITEM, MPFROM2SHORT( usIndex, 0), MPFROM2SHORT( TRUE, 0)));
         }
         break; // case WC_LISTBOX: case WC_COMBOBOX:

      // -   -   -   -   -   -   -   -   -

      case (ULONG) WC_SPINBUTTON:
         {
                  ULONG             ulUpperLimit;
                  ULONG             ulLowerLimit;
                  BOOL              fIsArray;
                  USHORT            usIndex;

         // is it an array ?
         if (WinSendMsg( hwndControl, SPBM_QUERYLIMITS, MPFROMP( &ulUpperLimit),  MPFROMP( &ulLowerLimit)))
            {
            // only numeric values allowed here
            if ((ulValueType == STM_VALUETYPE_LONG) || (ulValueType == STM_VALUETYPE_INDEX))
               {
               // no: just set the numeric value
               if ((lValue >= (LONG) ulLowerLimit) && (lValue >= (LONG) ulUpperLimit))
                  fResult = (BOOL) WinSendMsg( hwndControl, SPBM_SETCURRENTVALUE, MPFROMLONG( lValue), 0);
               }
            }
         else
            {
            // yes: search the value and set it
            usIndex = SHORT1FROMMR( WinSendMsg( hwndControl, LM_SELECTITEM, MPFROM2SHORT( LIT_FIRST, 0), pszValue));
            if (__isListboxIndexValid( usIndex))
               fResult = (BOOL) WinSendMsg( hwndControl, SPBM_SETCURRENTVALUE, MPFROMLONG( (ULONG) usIndex), 0);
            }
         }
         break;

      // -   -   -   -   -   -   -   -   -

      case (ULONG) WC_SLIDER:
      case (ULONG) WC_CIRCULARSLIDER:
         {
         // only supported for numeric values
         if ((ulValueType == STM_VALUETYPE_LONG) || (ulValueType == STM_VALUETYPE_INDEX))
            // update sliderarm position
            fResult = (BOOL) WinSendMsg( hwndControl, SLM_SETSLIDERINFO, MPFROM2SHORT( SMA_SLIDERARMPOSITION,SMA_INCREMENTVALUE), MPFROMLONG(lValue));
         }
         break;

      } // switch (pszClassIndex)

   } while (FALSE);

DEBUGMSGDETAIL( "#   >> update GUI control - fResult=%u\n", fResult);
return fResult;
}

// ---------------------------------------------------------------------------

// get value of GUI control according to the PM class of the control
// - this function returns a strdup, that must be freed bey the caller
PSZ static __getValueFromGuiControl( HWND hwndControl, ULONG ulValueType, PBOOL pfIsNumValue)
{
         BOOL              fResult = FALSE;

         CHAR              szClassname[ 40];
         PSZ               pszClassIndex;

         USHORT            usDialogid;
         USHORT            usControlid;
         HWND              hwndDialog;
         HWND              hwndRadioButton;

         PSZ               pszValue = NULL;
         ULONG             ulValueLen;
         BOOL              fValue;
         LONG              lValue;

         ULONG             ulMaxlen;
         PSZ               pszBuffer;

         CHAR              szLongValue[ 20];

do
   {
   if ((!hwndControl) || (!pfIsNumValue))
      break;
   hwndDialog  = WinQueryWindow( hwndControl, QW_PARENT);
   usControlid = WinQueryWindowUShort( hwndControl, QWS_ID);
   usDialogid  = WinQueryWindowUShort( hwndDialog, QWS_ID);

   // obtain classindex
   pszClassIndex = WtkQueryClassIndex( hwndControl);

   // now get the control value according to its class
   *pfIsNumValue = FALSE;

   switch ((ULONG) pszClassIndex)
      {

      // -   -   -   -   -   -   -   -   -

      case (ULONG) WC_BUTTON:

         if (__isRadioButton( hwndControl))
            {
            // get the index of the radio button group
            lValue = (LONG) WinSendMsg( hwndControl, BM_QUERYCHECKINDEX, 0, 0);
            }
         else
            {
            lValue = (LONG) WinSendMsg( hwndControl, BM_QUERYCHECK, 0, 0);

            }

         pszValue = strdup( "0");
         _ltoa( lValue, pszValue, 10);
         *pfIsNumValue = TRUE;
         break;

      case (ULONG) WC_MLE:
         // not yet implemented
         break;

      case (ULONG) WC_ENTRYFIELD:
         {
         // query the entryfield text
         ulValueLen = WinQueryWindowTextLength( hwndControl) + 1;
         pszValue = malloc( ulValueLen);
         if (!pszValue)
            break;
         WinQueryWindowText( hwndControl, ulValueLen, pszValue);
         }
         break; // case WC_ENTRYFIELD:

      // -   -   -   -   -   -   -   -   -

      case (ULONG) WC_LISTBOX:
      case (ULONG) WC_COMBOBOX:
         {

                  USHORT            usIndex;


         // determine selected entry
         usIndex = SHORT1FROMMR( WinSendMsg( hwndControl, LM_QUERYSELECTION, MPFROM2SHORT( LIT_FIRST, 0), 0));

         if (__isListboxIndexValid( usIndex))
            {
            if (ulValueType == STM_VALUETYPE_INDEXITEM)
               {
               // just save index as item number
               pszValue = malloc( 10);
               if (!pszValue)
                  break;
               sprintf( pszValue, "%u", usIndex);
               *pfIsNumValue = TRUE;
               }
            else
               {
               // get text of entry
               ulValueLen = (ULONG) WinSendMsg( hwndControl, LM_QUERYITEMTEXTLENGTH, MPFROM2SHORT( usIndex, 0), 0);
               pszValue = malloc( ulValueLen + 1);
               if (!pszValue)
                  break;
               WinSendMsg( hwndControl, LM_QUERYITEMTEXT,
                           MPFROM2SHORT( usIndex, (USHORT) ulValueLen),
                           MPFROMP( pszValue));
               *(pszValue + ulValueLen) = 0;
               }
            }
         }
         break; // case WC_LISTBOX: case WC_COMBOBOX:

      // -   -   -   -   -   -   -   -   -

      case (ULONG) WC_SPINBUTTON:
         {
                  ULONG             ulUpperLimit;
                  ULONG             ulLowerLimit;
                  BOOL              fIsArray;
                  USHORT            usIndex;

         // is it an array ?
         if (WinSendMsg( hwndControl, SPBM_QUERYLIMITS, MPFROMP( &ulUpperLimit),  MPFROMP( &ulLowerLimit)))
            {
            // no: query the numeric value
            WinSendMsg( hwndControl, SPBM_QUERYVALUE, MPFROMP( &lValue), 0);
            pszValue = malloc(20);
            _ltoa( lValue, pszValue, 10);
            *pfIsNumValue = TRUE;
            }
         else
            {
            // yes: query the text of the selected item
            usIndex = SHORT1FROMMR( WinSendMsg( hwndControl, LM_SELECTITEM, MPFROM2SHORT( LIT_FIRST, 0), pszValue));
            if (__isListboxIndexValid( usIndex))
               {
               ulValueLen = (ULONG) WinSendMsg( hwndControl, LM_QUERYITEMTEXTLENGTH, MPFROM2SHORT( usIndex, 0), 0);
               pszValue = malloc( ulValueLen);
               if (!pszValue)
                  break;
               WinSendMsg( hwndControl, SPBM_QUERYVALUE, MPFROMP( pszValue), MPFROM2SHORT( (USHORT) ulValueLen, 0));
               }
            }
         }
         break;

      // -   -   -   -   -   -   -   -   -

      case (ULONG) WC_SLIDER:
      case (ULONG) WC_CIRCULARSLIDER:
         {
         // get numeric value
         lValue = (LONG) WinSendMsg( hwndControl, SLM_QUERYSLIDERINFO, MPFROM2SHORT( SMA_SLIDERARMPOSITION,SMA_INCREMENTVALUE), 0);
         _ltoa( lValue, szLongValue, 10);
         pszValue = strdup( szLongValue);
         *pfIsNumValue = TRUE;
         }
         break;

      } // switch (pszClassIndex)

   // value gotten ?
   if (!pszValue)
      break;

   } while (FALSE);

return pszValue;
}

// ---------------------------------------------------------------------------

// WM_CONTROL message check for a change of a GUI control
static PVALUEDATA __checkGUIControlChanged(  ULONG ulDebugAPIValue, PVALUEDATAROOT pvdr,
                                             PULONG pulSubvalue, HWND hwnd, MPARAM mp1, MPARAM mp2)
{

         PVALUEDATA        pvdResult = NULL;
         BOOL              fDetectChange = FALSE;

         PSETTINGSDATAROOT psdr = NULL;
         PVALUEDATA        pvd = NULL;

         USHORT            usControl    = SHORT1FROMMP( mp1);
         USHORT            usNotifyCode = SHORT2FROMMP( mp1);
         USHORT            usDialogId   = WinQueryWindowUShort( hwnd, QWS_ID);
         PSZ               pszClassIndex;

         ULONG             ulValueIndex;

do
   {
   // check parms
   if (!pvdr)
      break;

   // search the contol
   for (pvd = pvdr->pvdFirstValue;
        ((pvd != NULL) && !(pvdResult));
        pvd = pvd->pvNextValue)
      {
      // if this one is a detail, skip
      if (!*pvd->psd->pszName)
         continue;

      // go through all subvalues
      for (ulValueIndex = 0;
           ((ulValueIndex < pvd->psd->ulValueCount) && (!fDetectChange));
           ulValueIndex++)
         {
         // check ID of dialog
         if (*(pvd->psd->pausDialogid + ulValueIndex) != usDialogId)
            continue;

         // check ID of control
         if (*(pvd->psd->pausControlid + ulValueIndex) != usControl)
            continue;

         // get a WC_* constant of classname
         pszClassIndex = WtkQueryClassIndex( WinWindowFromID( hwnd, usControl));

         // determine if check is required
         switch ((ULONG) pszClassIndex)
            {
            case (ULONG) WC_BUTTON:
               fDetectChange = ((usNotifyCode == BN_CLICKED) ||
                                (usNotifyCode == BN_DBLCLICKED));
               break;

            case (ULONG) WC_CIRCULARSLIDER:
               fDetectChange = (usNotifyCode == CSN_CHANGED);
               break;

            case (ULONG) WC_COMBOBOX:
               fDetectChange = (usNotifyCode == CBN_EFCHANGE);
               break;

            case (ULONG) WC_ENTRYFIELD:
               fDetectChange = (usNotifyCode == EN_CHANGE);
               break;

            case (ULONG) WC_LISTBOX:
               fDetectChange = ((usNotifyCode == LN_ENTER) ||
                                (usNotifyCode == LN_SELECT));
               break;

            case (ULONG) WC_MLE:
               fDetectChange = (usNotifyCode == MLN_CHANGE);
               break;

            case (ULONG) WC_SLIDER:
               fDetectChange = (usNotifyCode == SLN_CHANGE);
               break;

            case (ULONG) WC_SPINBUTTON:
               fDetectChange = (usNotifyCode == SPBN_CHANGE);
               break;

            } // switch (pszClassIndex)


         // save pointer and subvalue count
         if (fDetectChange)
            pvdResult = pvd;

         // report subvalue
         if (pulSubvalue)
            *pulSubvalue = ulValueIndex;

         } // for [all subvalues]

      } // for [all settings]

   } while (FALSE);

return pvdResult;
}

// ---------------------------------------------------------------------------

// update a (sub)value target buffer from a the GUI control or to a given value
// according to the value type and the PM class of the control
BOOL static __writeValue( ULONG ulDebugAPIValue, PVALUEDATAROOT pvdr, PVALUEDATA pvd,
                           HWND hwndControl, PSZ pszGivenValue, ULONG ulValueIndex)
{
         BOOL              fChanged = FALSE;
         BOOL              fReportSuccess = TRUE;

         CHAR              szClassname[ 40];
         ULONG             pszClassIndex;

         USHORT            usDialogid;
         USHORT            usControlid;
         HWND              hwndDialog;
         HWND              hwndRadioButton;

         PSZ               pszValue = NULL;
         ULONG             ulValueLen;
         BOOL              fValue;
         LONG              lValue;
         BOOL              fIsNumValue = FALSE;
         CHAR              szSearchValue[ 10];

         ULONG             ulValueType;
         ULONG             ulMaxlen;
         PSZ               pszBuffer;

         CHAR              szLongValue[ 20];
         CBQUERYSTRING     qs;

do
   {
   if (((!hwndControl) && (!pszGivenValue))  ||
       (!pvdr) || (!pvd))
      {
      fReportSuccess = FALSE;
      break;
      }
   ulValueType = *(pvd->psd->paulValueType + ulValueIndex);

   if (pszGivenValue)
      {
      pszValue = strdup( pszGivenValue);
      DEBUGMSGDETAIL( "#   >> write value for %s:%u, type %s\n",
                      pvd->psd->pszName _c_ ulValueIndex _c_ __getValueType(ulValueType));
      }
   else
      {
      hwndDialog  = WinQueryWindow( hwndControl, QW_PARENT);
      usControlid = WinQueryWindowUShort( hwndControl, QWS_ID);
      usDialogid  = WinQueryWindowUShort( hwndDialog, QWS_ID);

      DEBUGMSGDETAIL( "#   >> write from GUI control %u(0x%04x) of dialog %d(0x%04x) to %s:%u, type %s",
                      usControlid _c_
                      usControlid _c_
                      usDialogid _c_
                      usDialogid _c_
                      pvd->psd->pszName _c_
                      ulValueIndex _c_
                      __getValueType(ulValueType));

      // get value
      pszValue = __getValueFromGuiControl( hwndControl, ulValueType, &fIsNumValue);
      }

   if (!pszValue)
      break;

   DEBUGMSGDETAIL( " - value: \"%s\"", (*pszValue) ? pszValue : "");

   // determine value
   ulMaxlen  = (ULONG)*(pvd->paulValueMaxLen + ulValueIndex);
   pszBuffer = (PSZ)*(pvd->papszValues + ulValueIndex);
   switch (ulValueType)
      {
      case STM_VALUETYPE_INDEX:
         if (!fIsNumValue)
            {
            lValue = __getIndexValue( ulDebugAPIValue, FALSE, pvdr, pvd, ulValueIndex, pszValue);
            DEBUGMSGDETAIL( " - index \"%d\"", lValue);
            }
         else
            lValue = atol( pszValue);
         if (*(PLONG)pszBuffer != lValue)
            {
            *(PLONG)pszBuffer = lValue;
            fChanged = TRUE;
            }
         break;

      case STM_VALUETYPE_INDEXITEM:
         // differ between reading a GUI control or using a settings value
         if (hwndControl)
            {
            // pszValue contains the item number
            // store as number
            lValue = atol( pszValue);
            if (*(PLONG)pszBuffer != lValue)
               {
               *(PLONG)pszBuffer = lValue;
               fChanged = TRUE;
               }
            }
         else
            {
            // pszValue contains a settings value
            // use index scheme if getting values from settings
            lValue = __getIndexValue( ulDebugAPIValue, FALSE, pvdr, pvd, ulValueIndex, pszValue);
            DEBUGMSGDETAIL( " - index item \"%d\"", lValue);
            }
         break;

      case STM_VALUETYPE_LONG:
         lValue = atol( pszValue);
         if (*(PLONG)pszBuffer != lValue)
            {
            *(PLONG)pszBuffer = lValue;
            fChanged = TRUE;
            }
         break;

      case STM_VALUETYPE_STRING:
         if (strcmp( pszBuffer, pszValue))
            {
            strcpy( pszBuffer, pszValue);
            fChanged = TRUE;
            }
         break;

      case STM_VALUETYPE_TRUEFALSE:
      case STM_VALUETYPE_YESNO:
      case STM_VALUETYPE_ONOFF:

         strupr( pszValue);
         sprintf( szSearchValue, " %s ", pszValue);
         fValue = (strstr( " TRUE YES ON 1 ", szSearchValue) != NULL);
         if ( *(PBOOL) pszBuffer != fValue)
            {
            *(PBOOL) pszBuffer = fValue;
            fChanged = TRUE;
            }
         break;
      }


   } while (FALSE);

// cleanup
if (pszValue)
   free( pszValue);

if (fReportSuccess)
   DEBUGMSGDETAIL( " - fChanged=%u\n", fChanged);
return fChanged;
}

// ---------------------------------------------------------------------------

// validate a provided (sub)value or a value of a GUI control
// - the validation callback either validates itself or
//   standard validation according to the value type takes place
BOOL static __validateSettingsValue(  ULONG ulDebugAPIValue, PVALUEDATAROOT pvdr, PVALUEDATA pvd,
                                      HWND hwndControl, PSZ pszGivenValue, ULONG ulValueIndex)
{
         BOOL              fResult = FALSE;

         PSZ               pszValue = NULL;
         PSZ               pszIndexValue;
         LONG              lValue   = 0;
         BOOL              fIsNumValue = FALSE;
         ULONG             ulValueType;

         USHORT            usDialogid;
         USHORT            usControlid;
         HWND              hwndDialog;

         CHAR              szSearchValue[ 10];

         CBVALIDATE        v;

do
   {
   if (((!hwndControl) && (!pszGivenValue))  ||
       (!pvdr) || (!pvd))
      break;
   ulValueType = *(pvd->psd->paulValueType + ulValueIndex);

   if (pszGivenValue)
      {
      pszValue = strdup( pszGivenValue);
      DEBUGMSGDETAIL( "#   >> validate value for %s:%u, type %s",
                      pvd->psd->pszName _c_
                      ulValueIndex _c_
                      __getValueType(ulValueType));
      }
   else
      {
      hwndDialog  = WinQueryWindow( hwndControl, QW_PARENT);
      usControlid = WinQueryWindowUShort( hwndControl, QWS_ID);
      usDialogid  = WinQueryWindowUShort( hwndDialog, QWS_ID);

      DEBUGMSGDETAIL( "#   >> validate GUI control %d of dialog %d, %s:%u, type %s",
                      usControlid _c_
                      usDialogid _c_
                      pvd->psd->pszName _c_
                      ulValueIndex _c_
                      __getValueType(ulValueType));

      // get value of control
      pszValue = __getValueFromGuiControl( hwndControl, ulValueType, &fIsNumValue);
      }

   if (!pszValue)
      break;

   DEBUGMSGDETAIL( " - value: \"%s\"\n", (*pszValue) ? pszValue : "");

   // ask the WPS class for validation
   DEBUGMSGCALLBACK( "#   >> callback: validate %s:%u, value: \"%s\"\n",
                     pvd->psd->pszName _c_
                     ulValueIndex _c_
                     pszValue);

   memset( &v, 0, sizeof( v));
   v.pvObjectInstance = pvdr->pvObjectInstance;
   v.pszName          = pvd->psd->pszName;
   v.pszValue         = pszValue;
   v.ulSettingId      = pvd->psd->ulSettingId;
   v.ulValueIndex     = ulValueIndex;
   fResult = (pvdr->psdr->pfnCallbackValue)( STM_CALLBACK_VALIDATE, &v, pvdr->pvObjectInstance, pvdr->pvObjectData);

   // has WPS class done the validation ?
   if (fResult)
      // yes, take the result
      fResult = v.fResult;
   else
      {
      fResult = TRUE;
      // perform standard validation
      switch( *(pvd->psd->paulValueType + ulValueIndex))
         {
         default:
            // ignore all values, if no valid type is being used
            fResult = FALSE;
            break;

         case STM_VALUETYPE_LONG:
            // check all
            fResult = __isnumeric( pszValue);
            break;

         case STM_VALUETYPE_INDEX:
         case STM_VALUETYPE_INDEXITEM:
            pszIndexValue  = __getIndexString( ulDebugAPIValue, TRUE, pvdr, pvd, ulValueIndex, lValue);
            DEBUGMSGCALLBACK( "#   >> ", 0);
            fResult = (pszIndexValue != NULL);
            free( pszIndexValue);
            break;

         case STM_VALUETYPE_STRING:
            // check only string len
            if ((ULONG)(*(pvd->paulValueMaxLen + ulValueIndex)) < strlen( pszValue) + 1)
               fResult = FALSE;
            break;

         case STM_VALUETYPE_TRUEFALSE:
         case STM_VALUETYPE_YESNO:
         case STM_VALUETYPE_ONOFF:
            if (pszGivenValue)
               {
               // check maxlen of value here
               if (strlen( pszValue) > 5)
                  {
                  fResult = FALSE;
                  break;
                  }

               // now check the key
               strupr( pszValue);
               sprintf( szSearchValue, " %s ", pszValue);
               if ((strstr( " TRUE FALSE YES 1 NO ON OFF 0", szSearchValue) == NULL))
                  fResult = FALSE;
               }
            break;

         } // switch

      } // else if (fResult)

   } while (FALSE);

// cleanup
if (pszValue)
   free( pszValue);

DEBUGMSGDETAIL( " - fResult=%u\n", fResult);
return fResult;
}

// ---------------------------------------------------------------------------

// subclass procedure for notebook frame for automatic validation
// of all registered GUI controls within the notebook
static MRESULT EXPENTRY __SettingsNotebookFrameSubclassProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{

         HWND           hwndObject;
         HVALUETABLE    hvt;
         PVALUEDATAROOT pvdr;
         PVALUEDATA     pvd;

hwndObject = WinWindowFromID( hwnd, (ULONG) SIG_SETTINGSMANAGER);
hvt =  (HVALUETABLE) WinQueryWindowULong( hwndObject, QWL_USER);
pvdr = __getPointerFromHandle( hvt, SIG_VALUEMANAGER);
if (!pvdr)
   return FALSE;

switch (msg)
   {

   // -----------------------------------------------------------------

   case WM_SYSCOMMAND:
      {

      switch (LONGFROMMP( mp1))
         {
         case SC_CLOSE:
            if (!WtkValidateObjectValueTable( hvt, hwnd))
               return 0;
            break;
         }
      }
      break;

   } // end switch

return (pvdr->pfnwpNotebookFrameOrg)( hwnd, msg, mp1, mp2);

}

// ###########################################################################

/*
@@WtkCreateClassSettingsTable@SYNTAX
This function creates a settings table for a meta class.

@@WtkCreateClassSettingsTable@PARM@pvObjectClass@in
somSelf of the meta class.

@@WtkCreateClassSettingsTable@PARM@pfnCallbackValue@in
Adress of the :link reftype=hd viewport res=1000.callback procedure:elink..

@@WtkCreateClassSettingsTable@RETURN
Settingstable handle.
:parml compact.
:pt.NULLHANDLE
:pd.Settingstable could not be created.
:pt.other
:pd.handle to the new settingstable
:eparml.

@@WtkCreateClassSettingsTable@REMARKS
This function is to be called during _wpsclsInitData.
:p.
After having created the settingstable, still within _wpsclsInitData
settings and details are to be added to the table by invoking
:ul compact.
:li.:link reftype=hd refid=WtkAddClassSetting.WtkAddClassSetting:elink.
:li.:link reftype=hd refid=WtkAddClassDetail.WtkAddClassDetail:elink.
:eul.
:p.
and the table is to be closed for usage by instances of the WPS class by calling
:ul compact.
:li.:link reftype=hd refid=WtkCloseClassSettingsTable.WtkCloseClassSettingsTable:elink.
:eul.

@@
*/

HSETTINGTABLE APIENTRY WtkCreateClassSettingsTable( PVOID pvObjectClass, PFNCB pfnCallbackValue)
{
         APIRET            rc = NO_ERROR;
         ULONG             ulDebugAPIValue = STM_DEBUG_API_CREATECLASSSETTINGSTABLE;
         HSETTINGTABLE     hst = NULLHANDLE;
         PSETTINGSDATAROOT psdr = NULL;

         CBREPORTINIT      ri;

// set debug mask when DEBUG is turned on
#ifdef DEBUG
__SetDebugInfoMask();
#endif

DEBUGMSG( "# %s\n", __FUNCTION__);

do
   {
   // check parms
   if (!OBJECTVALID( pvObjectClass))
      // no object pointer for _wpSetError, so immediate return,
      return hst;

   if (!pfnCallbackValue)
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // allocate memory for object
   psdr = CLS_ALLOCATEMEMORY( pvObjectClass, sizeof( SETTINGSDATAROOT), &rc);
   if (!psdr)
      {
      rc = ERROR_NOT_ENOUGH_MEMORY;
      break;
      }
   DEBUGMSGCALLBACK( "# >> allocated settings table at 0x%08x\n", psdr);

   // initialize root data
   memset( psdr, 0, sizeof( SETTINGSDATAROOT));
   psdr->ulSig            = SIG_SETTINGSMANAGER;
   psdr->pvObjectClass    = pvObjectClass;
   psdr->pfnCallbackValue = pfnCallbackValue;

   // return mangled pointer
   hst = PTR2HANDLE( psdr, SIG_SETTINGSMANAGER);

   // tell class that we have initiialized
   DEBUGMSGCALLBACK( "#   >> callback: metaclass initialized\n", 0);
   memset( &ri, 0, sizeof( ri));
   (pfnCallbackValue)( STM_CALLBACK_REPORTINIT, &ri, NULL, NULL);

   } while (FALSE);

DEBUGMSG( "#   << hst=0x%08x rc=%u\n", hst _c_ rc);
SETERROR( pvObjectClass, rc);
return hst;
}

// ---------------------------------------------------------------------------

/*
@@WtkDestroyClassSettingsTable@SYNTAX
This function destroys a settings table for a meta class.

@@WtkDestroyClassSettingsTable@PARM@hst@in
Settingstable handle.

@@WtkDestroyClassSettingsTable@RETURN
Success indicator.
:parml compact.
:pt.TRUE
:pd.Settingstable could be destroyed.
:pt.FALSE
:pd.An error occurred.
:eparml.

@@WtkDestroyClassSettingsTable@REMARKS
This function is to be called during _wpclsUnInitData.

@@
*/



BOOL APIENTRY WtkDestroyClassSettingsTable( HSETTINGTABLE hst)
{
         APIRET            rc = NO_ERROR;
         BOOL              fResult = FALSE;
         ULONG             ulDebugAPIValue = STM_DEBUG_API_DESTROYCLASSSETTINGSTABLE;

         PVOID             pvObjectClass;
         PFNCB             pfnCallbackValue;

         PSETTINGSDATAROOT psdr = NULL;
         PSETTINGSDATA     psd = NULL;
         PSETTINGSDATA     psdNext = NULL;

         CBREPORTDESTROYED rd;

DEBUGMSG( "# %s\n", __FUNCTION__);
do
   {
   // check parms
   psdr = __getPointerFromHandle( hst, SIG_SETTINGSMANAGER);
   if (!psdr)
      // no object pointer for _wpSetError, so immediate return,
      return fResult;

   // save some needed pointers
   pvObjectClass    = psdr->pvObjectClass;
   pfnCallbackValue = psdr->pfnCallbackValue;

   // deallocate all data
   psd = psdr->psdFirstSetting;
   while (psd != NULL)
      {
      if (*psd->pszName)
         DEBUGMSGDETAIL( "#   >> free resources for setting %s (id 0x%08x) at 0x%08x\n",
                         psd->pszName _c_
                         psd->ulSettingId _c_
                         psd);
      else
         DEBUGMSGDETAIL( "#   >> free resources for detail 0x%08x at 0x%08x\n",
                         psd->ulSettingId _c_
                         psd);

      // free separate memory
      if (psd->pszAllTitles)
         CLS_FREEMEMORY( psdr->pvObjectClass, psd->pszAllTitles);

      psdNext = psd->pvNextSetting;
      memset( psd, 0, psd->ulThisEntryLen);
      CLS_FREEMEMORY( psdr->pvObjectClass, psd);
      psd = psdNext;
      }

   // free root of list
   if (psdr->pcfi)
      {
      CLS_FREEMEMORY( psdr->pvObjectClass, psdr->pcfi);
      DEBUGMSGDETAIL( "#   >> free resources for details info at 0x%08x\n", psdr->pcfi);
      }


   memset( psdr, 0, sizeof( SETTINGSDATAROOT)); // make sure data is unuseable
   CLS_FREEMEMORY( pvObjectClass, psdr);
   DEBUGMSGDETAIL( "#   >> free resources for settings table at 0x%08x\n", psdr);

   // tell WPS class to clean up callback resources
   DEBUGMSGCALLBACK( "#   >> callback: metaclass destroyed\n", 0);
   memset( &rd, 0, sizeof( rd));
   (pfnCallbackValue)( STM_CALLBACK_REPORTDESTROYED, &rd, NULL, NULL);

   } while (FALSE);

fResult = (rc == NO_ERROR);
DEBUGMSG( "#   << fResult=%u rc=%u\n", fResult _c_ rc);
SETERROR( pvObjectClass, rc);
return fResult;
}

// ---------------------------------------------------------------------------

/*
@@WtkDumpClassSettingsTable@SYNTAX
This function dumps a settings table to the console.

@@WtkDumpClassSettingsTable@PARM@hst@in
Settingstable handle.

@@WtkDumpClassSettingsTable@RETURN
Success indicator.
:parml compact.
:pt.TRUE
:pd.Settingstable could be dumped.
:pt.FALSE
:pd.An error occurred.
:eparml.

@@WtkDumpClassSettingsTable@REMARKS
This function is for debug purposes only. It uses printf
to display the current status of the class settingstable, so that
you can catch the output by linking printf.obj of the :hp2.PM Printf:ehp2.
package to your WPS class while testing.

@@
*/

BOOL APIENTRY WtkDumpClassSettingsTable( HSETTINGTABLE hst)
{
         APIRET            rc = NO_ERROR;
         BOOL              fResult = FALSE;

         PSETTINGSDATAROOT psdr = NULL;
         PSETTINGSDATA     psd = NULL;
         ULONG             i;
         PSZ               pszValue;

do
   {

   // check parms
   psdr = __getPointerFromHandle( hst, SIG_SETTINGSMANAGER);
   if (!psdr)
      // no object pointer for _wpSetError, so immediate return,
      return fResult;


   // is table complete ?
   printf( "table is %scomplete.\n", (psdr->fTableComplete) ? "" : "not ");

   // dump all data
   if (psdr->psdFirstSetting)
      {
      for (psd = psdr->psdFirstSetting; psd != NULL; psd = psd->pvNextSetting)
         {
         printf( "%s, %u, %u\n", psd->pszName, psd->ulValueCount, psd->ulQueryCount);
         }
      }
   else
      printf( "\nno settings defined at this point.\n");

   // done
   fResult = TRUE;

   } while (FALSE);

SETERROR( psdr->pvObjectClass, rc);
fResult = (rc == NO_ERROR);
return fResult;

}

// ---------------------------------------------------------------------------

/*
@@WtkAddClassSetting@SYNTAX
This function adds a setting to the class settings table.

@@WtkAddClassSetting@PARM@hst@in
Settingstable handle.

@@WtkAddClassSetting@PARM@ulSettingId@in
Settings ID.
:p.
This ID must be either zero or unique among all
settings and details added to a class settingstable.

:note.
:ul.
:li.A none-zero value allows to have the corresponding
object instance settings values saved and restored
to or from the WPS repository by
:link reftype=hd refid=WtkSaveObjectState.WtkSaveObjectState:elink.
and
:link reftype=hd refid=WtkRestoreObjectState.WtkRestoreObjectState:elink..
:p.
For the added setting  a range of settings IDs instead of only the specified one
will automatically be reserved for if the setting
:ul.
:li.has subvalues - this is the case if
the string pointed to by :link reftype=hd refid=pszSetting_WtkAddClassSetting.pszSetting:elink.
contains more than one value, separated by commas
:li. and/or can be queried multiple times - this is the case if
:link reftype=hd refid=ulQueryCount_WtkAddClassSetting.ulQueryCount:elink. is greater than 1
:eul.
:p.
All IDs within that range cannot be used by other settings.
The reserved range will be all IDs from :hp2.ulSettingId:ehp2. to :hp2.ulSettingId:ehp2. + (subvaluecount * querycount) - 1.
:color fc=red.
:li.If used (not setting this value to zero), do not change this id
for a given setting in future versions of your WPS class,
otherwise values of existing instances cannot be restored properly with
:link reftype=hd refid=WtkRestoreObjectState.WtkRestoreObjectState:elink..
This may lead to unpredictable results !
:li.Th same applies to the order of subvalues, they may not be changed
in their order, as the calculated ID for a subvalue cannot be changed
for existing instances at a later point of time.
:eul.
:color fc=default.

@@WtkAddClassSetting@PARM@pszSetting@in
Address of the ASCIIZ name and default value string of the setting.
:p.
This string must have the form
:xmp.
NAME=value;
:exmp.
:p.
where :hp1.value:ehp1. can
:ul compact.
:li.may be either empty or specify a default value, which is used, when
no value is specified for this setting when a settings string is sended
to an object of this class
:li.consist of several subvalues, each separated by commas. Each subvalue
then may either be empty or specify a default value. For each subvalue, a comma
must be included within the string.
:eul.
:p.
Here are some examples&colon.
:xmp.
"IPCONFIG=DYNAMIC;"
"LOCALIP=;"
"MTU=1500;"
"DEFAULTROUTE=YES;"
"COMINIT=9600,n,8,1;"
:exmp.
:p.
The following example defines a setting with four subvalues, but with no default values:
:xmp.
"MYSETTING=,,,;"
:exmp.

@@WtkAddClassSetting@PARM@ulQueryCount@in
Query count of a setting.
:p.
This value must not be zero! It may be set to a value
:ul.
:li.of one, indicating that this setting can have only one value.
:li.greater than one, indicating that this setting can have more than one value.
.br
When the WPS class determines the current settings by calling
:link reftype=hd refid=WtkQueryObjectSettings.WtkQueryObjectSettings:elink.
or saves the settings with either
:link reftype=hd refid=WtkSaveObjectState.WtkSaveObjectState:elink.
or
:link reftype=hd refid=WtkSaveObjectSettings.WtkSaveObjectSettings:elink.,
the Settings Manager asks the callback of the WPS class "ulQueryCount" times
for the current value of that setting.
:p.
For to be able to handle a setting with multiple values, it needs to have at
least one subvalue per dimension to build up a twodimensional matrix internally.
:p.
An example for a multidimensional setting is&colon.
:parml tsize=20 break=none.
:pt.name
:pd.CURRENTDIR
:pt.value 1
:pd.drive
:pt.value 2
:pd.current dir of drive
:eparml.
:p.
In order to query all drives, you would set ulQueryCount to 26 and let
:hp2.WtkQueryObjectSettings:ehp2. query 26 times the value for the setting
CURRENTDIR. When being asked for that setting, you would place the current
directory of the drive into the target buffer for that setting
accoring to the query index (0-25). :hp2.WtkQueryObjectSettings:ehp2.
would then place somewhat like this into the resulting settingstring&colon.
:xmp.
"CURRENTDIR=A&colon.,\;"
"CURRENTDIR=C&colon.,\OS2;"
"CURRENTDIR=D&colon.,\TCPIP;"
etc.
:exmp.
:p.
The alternative would be to define a separate setting for each drive
(CURRENTDIRA, CURRENTDIRB) or have a separate WPS class for such
instances.
:eul.

@@WtkAddClassSetting@RETURN
Success indicator.
:parml compact.
:pt.TRUE
:pd.Setting could be added to the class settings table
:pt.FALSE
:pd.An error occurred.
:eparml.

@@WtkAddClassSetting@REMARKS
This function is to be called during _wpclsInitData,
right after having created the class settingstable with
:link reftype=hd refid=WtkCreateClassSettingsTable.WtkCreateClassSettingsTable:elink..
:p.
:hp2.WtkAddClassSetting:ehp2. has to be called once for
each setting to be added to the class settingstable.
:p.
After having added all details and settings to class settingstable,
the WPS Class needs to close the class settingstable with
:link reftype=hd refid=WtkCloseClassSettingsTable.WtkCloseClassSettingsTable:elink.
in order make the table available for the objects (instances) of the WPS class.

@@
*/

#define SETTING_PTRTABLE_COUNT 7

BOOL APIENTRY WtkAddClassSetting( HSETTINGTABLE hst, ULONG ulSettingId, PSZ pszSetting, ULONG ulQueryCount)
{
         APIRET            rc = NO_ERROR;
         BOOL              fResult = FALSE;
         BOOL              fIdUsed = FALSE;
         ULONG             ulDebugAPIValue = STM_DEBUG_API_ADDCLASSSETTING;
         ULONG             i;

         BOOL              fAddingDetailOnly = FALSE;

         PPVOID            ppvValues = NULL;

         ULONG             ulEntryLen;
         ULONG             ulPointerTableLen;
         ULONG             ulValuesFound = 0;

         ULONG             ulTitlesTotalLen = 0;
         ULONG             ulTmpEntryLen;

         PSZ               pszSource;
         PSZ               pszTarget;

         PSZ               pszSettingCopy = NULL;
         PSZ               pszSettingName;
         PBYTE             pbDataStart;

         PSETTINGSDATAROOT psdr = NULL;
         PSETTINGSDATA     psd = NULL;
         PSETTINGSDATA     psdNew = NULL;


         CBQUERYVALUEINFO  qvi;
         CBQUERYDETAILINFO qdi;

// is it for adding a detail only ?
// is table complete ?
if ((EMPTYSTRING( pszSetting)) && (ulQueryCount == 0))
   fAddingDetailOnly = TRUE;

if (!fAddingDetailOnly)
   DEBUGMSG( "# %s: \n", __FUNCTION__);

do
   {
   // check parms
   psdr = __getPointerFromHandle( hst, SIG_SETTINGSMANAGER);
   if (!psdr)
      // no object pointer for _wpSetError, so immediate return,
      return fResult;

   // if no detail, both string and query count must be given
   if ((!fAddingDetailOnly) &&
       ((!ulQueryCount) || (EMPTYSTRING( pszSetting))))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   if (!fAddingDetailOnly)
      {
      // is it a valid settings string ? Also count numbers of values
      pszSettingCopy = strdup( pszSetting);
      pszSettingName = pszSettingCopy;
      if (!pszSettingName)
         {
         rc = ERROR_NOT_ENOUGH_MEMORY;
         break;
         }
      if (! __getSettingFromSetupString( &pszSettingName, &ulValuesFound))
         break;
      }
   else
      {
      // setup for detail
      ulValuesFound = 1;
      pszSetting = "";
      pszSettingCopy = strdup( pszSetting);
      }

   // check if given ID is already used
   if (ulSettingId)
      {
      for (psd = psdr->psdFirstSetting; ((psd != NULL) && (!fIdUsed)); psd = psd->pvNextSetting)
         {
            ULONG          ulFirstId = psd->ulSettingId;
            ULONG          ulLastId  = SETTINGSID( psd->ulSettingId, psd->ulValueCount, psd->ulQueryCount);

         // check first and last id
         fIdUsed = ((ulFirstId >= ulSettingId) && (ulLastId <= ulSettingId));
         if (fIdUsed)
            {
            DEBUGMSGDETAIL( "#   >> ERROR: id %u(0x%04x) for %s ID lies in range %u-%u of setting %s\n",
                            ulSettingId _c_
                            ulSettingId _c_
                            pszSettingCopy _c_
                            ulFirstId _c_
                            ulLastId _c_
                            psd->pszName);
            }

         } // for [all settings]
      if (fIdUsed)
         {
         rc = ERROR_INVALID_PARAMETER;
         break;
         }

      } // if (ulSettingId)

   // reset pointer to start of string
   pszSettingName = pszSettingCopy;

   if (fAddingDetailOnly)
      DEBUGMSGDETAIL( "#   >> adding detail %u(0x%04x)\n",
                      ulSettingId _c_
                      ulSettingId);
   else
      DEBUGMSGDETAIL( "#   >> adding setting \"%s\" id %u(0x%04x)\n",
                      pszSetting _c_
                      ulSettingId _c_
                      ulSettingId);

   // allocate memory for new entry and copy name
   ulPointerTableLen = ulValuesFound * sizeof( PVOID);
   ulEntryLen = sizeof( SETTINGSDATA) + strlen( pszSetting) + 1 + (ulPointerTableLen * SETTING_PTRTABLE_COUNT);
   psdNew = CLS_ALLOCATEMEMORY( psdr->pvObjectClass, ulEntryLen, &rc);
   if (!psdNew)
      {
      rc = ERROR_NOT_ENOUGH_MEMORY;
      break;
      }
   DEBUGMSGCALLBACK( "#   >> allocated setting entry at 0x%08x\n", psdNew);

   memset( psdNew, 0, ulEntryLen);

   psdNew->pszName      = (PBYTE) psdNew +  sizeof( SETTINGSDATA);
   memcpy( psdNew->pszName, pszSettingName, strlen( pszSetting) + 1);
   psdNew->ulValueCount = ulValuesFound;
   psdNew->ulSettingId  = ulSettingId;
   psdNew->ulQueryCount = ulQueryCount;

   // setup pointers
   psdNew->ulThisEntryLen       = ulEntryLen;
   pbDataStart                  = psdNew->pszName + strlen( pszSetting) + 1;
   psdNew->paulValueType        = (PVOID)(pbDataStart);
   psdNew->pausDialogid         = (PVOID)(pbDataStart + (1 * ulPointerTableLen));
   psdNew->pausControlid        = (PVOID)(pbDataStart + (2 * ulPointerTableLen));
   psdNew->pahwndControl        = (PVOID)(pbDataStart + (3 * ulPointerTableLen));
   psdNew->papfnwpSubclass      = (PVOID)(pbDataStart + (4 * ulPointerTableLen));
   psdNew->paszDetailsTitle     = (PVOID)(pbDataStart + (5 * ulPointerTableLen));
   psdNew->paulDispStringMaxLen = (PVOID)(pbDataStart + (6 * ulPointerTableLen));

   if (!fAddingDetailOnly)
      DEBUGMSGDETAIL( "#   >> %u values - %u queries - ID range %u - %u\n",
                      ulValuesFound _c_
                      ulQueryCount _c_
                      ulSettingId _c_
                      SETTINGSID( ulSettingId, ulValuesFound, ulQueryCount));

   // process all settings values
   for (i = 0; i < psdNew->ulValueCount; i++)
      {
      if (!fAddingDetailOnly)
         DEBUGMSGCALLBACK( "#   >> callback: query value info for setting %s:%u id %u(0x%04x)\n",
                           psdNew->pszName _c_
                           i _c_
                           ulSettingId _c_
                           ulSettingId);
      else
         DEBUGMSGCALLBACK( "#   >> callback: query value info for detail %u(0x%04x)\n",
                           ulSettingId  _c_
                           ulSettingId);
      memset( &qvi, 0, sizeof( qvi));
      qvi.pszName       = psdNew->pszName;
      qvi.ulSettingId   = psdNew->ulSettingId;
      qvi.ulValueIndex  = i;
      fResult = (psdr->pfnCallbackValue)( STM_CALLBACK_QUERYVALUEINFO, &qvi, NULL, NULL);
      if (!fResult)
         {
         DEBUGMSGDETAIL( "#   >> could not query value info !\n", 0);
         rc = ERROR_INVALID_DATA;
         break;
         }

      // save values just being queried
      *(psdNew->paulValueType    + i) = qvi.ulValueType;
      *(psdNew->pausDialogid     + i) = qvi.usDialogid;
      *(psdNew->pausControlid    + i) = qvi.usControlid;
      *(psdNew->papfnwpSubclass  + i) = qvi.pfnwpSubclass;

      if (qvi.pszDetailsTitle)
         {
         *(psdNew->paszDetailsTitle + i) = strdup( qvi.pszDetailsTitle);
         ulTitlesTotalLen += strlen( qvi.pszDetailsTitle) + 1;
         }


      // query indexinfo
      if ((qvi.ulValueType == STM_VALUETYPE_INDEX)      ||
          (qvi.ulValueType == STM_VALUETYPE_TRUEFALSE)  ||
          (qvi.ulValueType == STM_VALUETYPE_YESNO)      ||
          (qvi.ulValueType == STM_VALUETYPE_ONOFF)      ||
          (qvi.ulValueType == STM_VALUETYPE_INDEXITEM))
         {
         if (!fAddingDetailOnly)
            DEBUGMSGCALLBACK( "#   >> callback: query detail info for %s:%u\n",
                              psdNew->pszName _c_
                              i);
         else
            DEBUGMSGCALLBACK( "#   >> callback: query detail info for detail %u(0x%04x)\n",
                              ulSettingId _c_
                              ulSettingId);
         memset( &qdi, 0, sizeof( qdi));
         qdi.pszName       = psdNew->pszName;
         qdi.ulSettingId   = psdNew->ulSettingId;
         qdi.ulValueIndex  = i;
         fResult = (psdr->pfnCallbackValue)( STM_CALLBACK_QUERYDETAILINFO, &qdi, NULL, NULL);
         if (!fResult)
            {
            DEBUGMSGDETAIL( "#   >> could not query detail info, set maxlen to 32 !\n", 0);
            qdi.ulDispStringMaxlen = 32;
            fResult = TRUE;
            }
         else
            {
            DEBUGMSGCALLBACK( "#   >> detail stringmaxlen is %u \n", qdi.ulDispStringMaxlen);
            }

         // save value just being queried
         *(psdNew->paulDispStringMaxLen + i) = qdi.ulDispStringMaxlen;
         }

      } // <for all values of a setting>

   if (!fResult)
      break;

   // make sure that we do not insert a duplicate
   psd = __getSettingsData( psdr, psdNew->pszName, ulSettingId);
   if (psd)
      {
      rc = ERROR_ACCESS_DENIED;
      break;
      }

   // copy complete structure into one memory block including the titles
   if (ulTitlesTotalLen)
      {
      // create new block for titles !
      psdNew->pszAllTitles = CLS_ALLOCATEMEMORY( psdr->pvObjectClass, ulTitlesTotalLen, &rc);
      if (!psdNew->pszAllTitles)
         {
         rc = ERROR_NOT_ENOUGH_MEMORY;
         break;
         }

      // now copy all titles
      pszTarget = psdNew->pszAllTitles;
      for (i = 0; i < psdNew->ulValueCount; i++)
         {
         pszSource = *(psdNew->paszDetailsTitle + i);
         if (pszSource)
            {
            strcpy( pszTarget, pszSource);
            free( pszSource);
            *(psdNew->paszDetailsTitle + i) = pszTarget;
            pszTarget = NEXTSTRING( pszTarget);
            }
         }

      } // if (ulTitlesTotalLen)

   // find last pointer and allocate new entry
   if (psdr->psdFirstSetting)
      {
      psd = psdr->psdFirstSetting;
      while (psd->pvNextSetting != NULL)
         {
         psd = psd->pvNextSetting;
         }
      psd->pvNextSetting = psdNew;
      }
   else
      psdr->psdFirstSetting = psdNew;

   } while (FALSE);

// cleanup on error
if (rc != NO_ERROR)
   {
   if (psdr)
      {
      if (psdNew) CLS_FREEMEMORY( psdr->pvObjectClass, psdNew);
      }
   }

// cleanup
if (pszSettingCopy) free( pszSettingCopy);

fResult = (rc == NO_ERROR);
DEBUGMSG( " - fResult=%u rc=%u\n", fResult _c_ rc);
SETERROR( psdr->pvObjectClass, rc);
return fResult;

}

// ---------------------------------------------------------------------------

/*
@@WtkAddClassDetail@SYNTAX
This function adds a detail to the class settings table.

@@WtkAddClassDetail@PARM@hst@in
Settingstable handle.

@@WtkAddClassDetail@PARM@ulSettingId@in
Settings ID.
:p.
This ID must be greater than zero and unique among all
settings and details added to a class settingstable.
While it is used to save and restores object instance
settings values to or from the WPS repository, details
are not stored, but still need an ID to be uniquely
identified, as they do not have a symbolic name.
:p.
See also the remarks on the same parameter
:link reftype=hd refid=ulSettingId_WtkAddClassSetting.ulSettingId:elink.
for the function
:link reftype=hd refid=WtkAddClassSetting.WtkAddClassSetting:elink.,
as both APIs work nearly the same !

@@WtkAddClassDetail@RETURN
Success indicator.
:parml compact.
:pt.TRUE
:pd.Detail could be added to the class settings table
:pt.FALSE
:pd.An error occurred.
:eparml.

@@WtkAddClassDetail@REMARKS
This function is to be called during _wpclsInitData,
right after having created the class settingstable with
:link reftype=hd refid=WtkCreateClassSettingsTable.WtkCreateClassSettingsTable:elink..
:p.
:hp2.WtkAddClassDetail:ehp2. has to be called once for
each detail to be added to the class settingstable.
:p.
After having added all details and settings to class settingstable,
the WPS Class needs to close the class settingstable with
:link reftype=hd refid=WtkCloseClassSettingsTable.WtkCloseClassSettingsTable:elink.
in order make the table available for the objects (instances) of the WPS class.

@@
*/

BOOL APIENTRY WtkAddClassDetail( HSETTINGTABLE hst, ULONG ulSettingId)
{
         BOOL              fResult = FALSE;
         ULONG             ulDebugAPIValue = STM_DEBUG_API_ADDCLASSDETAIL;


DEBUGMSG( "# %s: \n", __FUNCTION__);

// check for required settings id
if (!ulSettingId)
   {
            PSETTINGSDATAROOT psdr = NULL;

   psdr = __getPointerFromHandle( hst, SIG_SETTINGSMANAGER);
   if (psdr)
      SETERROR( psdr->pvObjectClass, ERROR_INVALID_PARAMETER);
   return FALSE;
   }

fResult = WtkAddClassSetting( hst,  ulSettingId, NULL, 0);
return fResult;
}

// ---------------------------------------------------------------------------

/*
@@WtkCloseClassSettingsTable@SYNTAX
This function closes the class settingstable,
so that objects (instances) of the WPS class can
use it.

@@WtkCloseClassSettingsTable@PARM@hst@in
Settingstable handle.

@@WtkCloseClassSettingsTable@RETURN
Success indicator.
:parml compact.
:pt.TRUE
:pd.Class settingstable was closed successfully.
:pt.FALSE
:pd.An error occurred.
:eparml.

@@WtkCloseClassSettingsTable@REMARKS
This function is to be called during _wpclsInitData,
right after having created the class settingstable with
:link reftype=hd refid=WtkCreateClassSettingsTable.WtkCreateClassSettingsTable:elink.
and added all settings and details with
:ul compact.
:li.:link reftype=hd refid=WtkAddClassSetting.WtkAddClassSetting:elink.
:li.:link reftype=hd refid=WtkAddClassDetail.WtkAddClassDetail:elink.
:eul.
:p.
If the WPS class does not close the settingstable using
:hp2.WtkCloseClassSettingsTable:ehp2., it cannot be used by
objects (instances() of the WPS class.

@@
*/

BOOL APIENTRY WtkCloseClassSettingsTable( HSETTINGTABLE hst)
{
         BOOL              fResult = FALSE;
         ULONG             ulDebugAPIValue = STM_DEBUG_API_CLOSECLASSSETTINGSTABLE;

         PSETTINGSDATAROOT psdr = NULL;

DEBUGMSG( "# %s: ", __FUNCTION__);
do
   {
   // check parms
   psdr = __getPointerFromHandle( hst, SIG_SETTINGSMANAGER);
   if (!psdr)
      // no object pointer for _wpSetError, so immediate return,
      return fResult;

   DEBUGMSGDETAIL( " closing settings table ", 0);
   psdr->fTableComplete =TRUE;
   fResult = TRUE;

   } while (FALSE);


DEBUGMSG( " - fResult=%u\n", fResult);
SETERROR( psdr->pvObjectClass, NO_ERROR);
return fResult;

}

// ---------------------------------------------------------------------------

/*
@@WtkQueryObjectClass@SYNTAX
This function queries the somSelf pointer to the object meta class
(WPS metaclass), using the specified settingstable.

@@WtkQueryObjectClass@PARM@hst@in
Settingstable handle.

@@WtkQueryObjectClass@PARM@*ppvObjectClass@out
Pointer to a variable receiving the somSelf pointer to the object meta class.

@@WtkQueryObjectClass@RETURN
Success indicator.
:parml compact.
:pt.TRUE
:pd.somSelf pointer to the meta class could be returned.
:pt.FALSE
:pd.An error occurred.
:eparml.

@@WtkQueryObjectClass@REMARKS
This function can be used within the dialog procedure for property notebook pages
to access methods of the object metaclass.
:p.
By design, only the handle to the valuetable of the object instance is handed over
to the dialog procedure. In order to call WtkQueryObjectClass, you can call
:link reftype=hd refid=WtkQuerySettingsTable.WtkQuerySettingsTable.:elink.
to first obtain the handle to the settingstable being associated
to the object metaclass.

@@
*/

BOOL APIENTRY WtkQueryObjectClass( HSETTINGTABLE hst, PVOID *ppvObjectClass)
{
         BOOL              fResult = FALSE;
         APIRET            rc = NO_ERROR;
         ULONG             ulDebugAPIValue = STM_DEBUG_API_QUERYOBJECTCLASS;
         PSETTINGSDATAROOT psdr = NULL;


DEBUGMSG( "# %s: \n", __FUNCTION__);

do
   {
   if (!ppvObjectClass)
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   psdr = __getPointerFromHandle( hst, SIG_SETTINGSMANAGER);
   if (!psdr)
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   *ppvObjectClass = psdr->pvObjectClass;

   } while (FALSE);

fResult = (rc == NO_ERROR);
SETERROR( psdr->pvObjectClass, rc);
return fResult;
}

// ---------------------------------------------------------------------------

/*
@@WtkCreateObjectValueTable@SYNTAX
This function creates a valuetable for an object
(instance) of a WPS class.

@@WtkCreateObjectValueTable@PARM@hst@in
Settingstable handle.
:p.
:note.
The settingstable being specified must have been
closed before by calling
:link reftype=hd refid=WtkCloseClassSettingsTable.WtkCloseClassSettingsTable:elink..

@@WtkCreateObjectValueTable@PARM@pvObjectInstance@in
somSelf of the object.

@@WtkCreateObjectValueTable@PARM@pvObjectData@in
somThis of the object.

@@WtkCreateObjectValueTable@RETURN
Valuetable handle.
:parml compact.
:pt.NULLHANDLE
:pd.Valuetable could not be created.
:pt.other
:pd.handle to the new settingstable
:eparml.

@@WtkCreateObjectValueTable@REMARKS
This function is to be called during _wpInitData.
This way it creates a table for settingsvalues for each
object (instance) of the WPS class.

@@
*/

#define VALUE_PTRTABLE_COUNT 2

HVALUETABLE APIENTRY WtkCreateObjectValueTable( HSETTINGTABLE hst, PVOID pvObjectInstance, PVOID pvObjectData)
{

         APIRET            rc      = NO_ERROR;
         BOOL              fResult = FALSE;
         ULONG             ulDebugAPIValue = STM_DEBUG_API_CREATEOBJECTVALUETABLE;

         ULONG             i;

         HVALUETABLE       hvt = NULLHANDLE;
         PVALUEDATAROOT    pvdr = NULL;
         PVALUEDATA        pvd = NULL;
         PVALUEDATA        pvdNew = NULL;

         ULONG             ulPointerTableLen;
         ULONG             ulValueDataLen;
         PBYTE             pbDataStart;

         PSETTINGSDATAROOT psdr = NULL;
         PSETTINGSDATA     psd = NULL;

         CBQUERYTARGETBUF  qtb;

DEBUGMSG( "# %s\n", __FUNCTION__);

do
   {
   // check parms
   if (!OBJECTVALID( pvObjectInstance))
      // no object pointer for _wpSetError, so immediate return,
      return hvt;


   // check class settings !
   psdr = __getPointerFromHandle( hst, SIG_SETTINGSMANAGER);
   if (!psdr)
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // is any setting registered ?
   if (!psdr->psdFirstSetting)
      {
      rc = ERROR_INVALID_DATA;
      break;
      }

   // is table not complete ?
   if (!psdr->fTableComplete)
      {
      rc = ERROR_INVALID_DATA;
      break;
      }

   // allocate memory for object
   pvdr = OBJ_ALLOCATEMEMORY( pvObjectInstance, sizeof( VALUEDATAROOT), &rc);
   if (!pvdr)
      {
      rc = ERROR_NOT_ENOUGH_MEMORY;
      break;
      }

   // return mangled pointer later
   hvt = PTR2HANDLE( pvdr, SIG_VALUEMANAGER);

   // initialize root data
   memset( pvdr, 0, sizeof( VALUEDATAROOT));
   pvdr->ulSig            = SIG_VALUEMANAGER;
   pvdr->pvObjectInstance = pvObjectInstance;
   pvdr->pvObjectData     = pvObjectData;
   pvdr->psdr             = psdr;

   // now get all needed pointers to target buffers
   // for all registered class settings
   for (psd = psdr->psdFirstSetting; psd != NULL; psd = psd->pvNextSetting)
      {


      // allocate memory for value data
      ulPointerTableLen = psd->ulValueCount * sizeof( PVOID);
      ulValueDataLen    = sizeof( VALUEDATA) + (ulPointerTableLen * VALUE_PTRTABLE_COUNT);
      pvdNew = OBJ_ALLOCATEMEMORY( pvdr->pvObjectInstance, ulValueDataLen, &rc);
      if (!pvdNew)
         {
         rc = ERROR_NOT_ENOUGH_MEMORY;
         break;
         }
      memset( pvdNew, 0, ulValueDataLen);
      pvdNew->psd = psd;

      // setup pointers
      pbDataStart                 = (PBYTE) pvdNew + sizeof( VALUEDATA);
      pvdNew->papszValues         = (PVOID)(pbDataStart);
      pvdNew->paulValueMaxLen     = (PVOID)(pbDataStart + (1 * ulPointerTableLen));

      // save pointer to data
      if (!pvdr->pvdFirstValue)
         pvdr->pvdFirstValue = pvdNew;
      else
         pvd->pvNextValue = pvdNew;
      pvd = pvdNew;

      // process all settings values
      for (i = 0; i < psd->ulValueCount; i++)
         {
         if (*psd->pszName)
            DEBUGMSGDETAIL( "#   >> adding value data for setting %s:%u\n",
                            psd->pszName _c_
                            i);
         else
            DEBUGMSGDETAIL( "#   >> adding value data for detail %u(0x%04x)\n",
                            psd->ulSettingId _c_
                            psd->ulSettingId);

         // ask back for target buffer for this value
         DEBUGMSGCALLBACK( "#   >> callback: query target buffer for %s:%u\n", pvd->psd->pszName _c_ i);
         memset( &qtb, 0, sizeof( qtb));
         qtb.pvObjectInstance = pvObjectInstance;
         qtb.pszName          = psd->pszName;
         qtb.ulSettingId      = psd->ulSettingId;
         qtb.ulValueIndex     = i;
         fResult = (psdr->pfnCallbackValue)( STM_CALLBACK_QUERYTARGETBUF, &qtb, pvdr->pvObjectInstance, pvdr->pvObjectData);
         if (!fResult)
            {
            DEBUGMSGDETAIL( "#   >> could not query target buffer !\n", 0);
            continue;
            }
         else
            {
            DEBUGMSGCALLBACK( "#   >>  ", 0);
            DEBUGMSGDETAIL( "    reported at 0x%08x, len %u(0x%04x)\n", qtb.pvTarget _c_ qtb.ulBufMax _c_ qtb.ulBufMax);
            }

         // save values just being queried
         *(pvdNew->papszValues     + i) = qtb.pvTarget;
         *(pvdNew->paulValueMaxLen + i) = qtb.ulBufMax;

         } // <for all values of a setting>

      } // <for all available settings definitions >

   } while (FALSE);

// clean up on error
if (rc != NO_ERROR)
   {
   if (hvt)
      {
      // destroy table (handle is not available yet)
      WtkDestroyObjectValueTable( hvt);
      hvt = NULLHANDLE;
      }
   }

DEBUGMSG( "#   << hvt=0x%08x rc=%u\n", hvt _c_ rc);
SETERROR( pvObjectInstance, rc);
return hvt;
}


// ---------------------------------------------------------------------------

/*
@@WtkDestroyObjectValueTable@SYNTAX
This function detroys a valuetable for an object
(instance) of a WPS class.

@@WtkDestroyObjectValueTable@PARM@hvt@in
Valuetable handle.

@@WtkDestroyObjectValueTable@RETURN
Success indicator.
:parml compact.
:pt.TRUE
:pd.Valuetable could be destroyed.
:pt.FALSE
:pd.An error occurred.
:eparml.

@@WtkDestroyObjectValueTable@REMARKS
This function is to be called during _wpUnInitData.

@@
*/

BOOL APIENTRY WtkDestroyObjectValueTable( HVALUETABLE hvt)
{
         APIRET            rc      = NO_ERROR;
         BOOL              fResult = FALSE;
         ULONG             ulDebugAPIValue = STM_DEBUG_API_DESTROYOBJECTVALUETABLE;

         PVOID             pvObjectInstance;

         PVALUEDATAROOT    pvdr = NULL;
         PVALUEDATA        pvd = NULL;
         PVALUEDATA        pvdNext = NULL;

DEBUGMSG( "# %s\n", __FUNCTION__);
do
   {
   // check parms
   pvdr = __getPointerFromHandle( hvt, SIG_VALUEMANAGER);
   if (!pvdr)
      // no object pointer for _wpSetError, so immediate return,
      return fResult;


   // deallocate all data
   pvd = pvdr->pvdFirstValue;
   while( pvd != NULL)
      {

      if (*pvd->psd->pszName)
         DEBUGMSGDETAIL( "#   >> cleanup valuedata for setting %s id %u(0x%04x)\n",
                         pvd->psd->pszName _c_
                         pvd->psd->ulSettingId _c_
                         pvd->psd->ulSettingId);
      else
         DEBUGMSGDETAIL( "#   >> cleanup valuedata for detail %u(0x%04x)\n",
                         pvd->psd->ulSettingId _c_
                         pvd->psd->ulSettingId);

      pvdNext = pvd->pvNextValue;
      OBJ_FREEMEMORY( pvdr->pvObjectInstance, pvd);
      pvd = pvdNext;
      }

   DEBUGMSGDETAIL( "#   >> free allocated object details data at 0x%08x\n", pvdr->pszObjectDetailsItems);
   OBJ_FREEMEMORY( pvdr->pvObjectInstance, pvdr->pszObjectDetailsItems);

   pvObjectInstance = pvdr->pvObjectInstance;
   memset( pvdr, 0, sizeof( VALUEDATAROOT)); // make sure data is unuseable
   OBJ_FREEMEMORY( pvObjectInstance, pvdr);

   } while (FALSE);

fResult = (rc == NO_ERROR);
DEBUGMSG( "#   << fResult=%u rc=%u\n", fResult _c_ rc);
SETERROR( pvObjectInstance, rc);
return fResult;

}

// ---------------------------------------------------------------------------

/*
@@WtkEvaluateObjectSettings@SYNTAX
This function evaluates a settingsstring for a WPS class.
This includes validation of the values and the update
of both the object valuetable and object details,
and, if desired, the update of controls of open settings
notebook dialog being associated to certain settings
(sub)values.

@@WtkEvaluateObjectSettings@PARM@hvt@in
Valuetable handle.

@@WtkEvaluateObjectSettings@PARM@pszSetup@in
Setup string from _wpSetup.

@@WtkEvaluateObjectSettings@RETURN
Success indicator.
:parml compact.
:pt.TRUE
:pd.String could be evaluated.
:pt.FALSE
:pd.An error occurred.
:eparml.

@@WtkEvaluateObjectSettings@REMARKS
This function is to be called during _wpSetup in order
to handle the given setup string.
:note.
You still must pass pszSetup to the parent class, because
:hp2.WtkEvaluateObjectSettings:ehp2. will only handle the settings
being implemented by your WPS class !
:p.
If a settings notebook of the object is open, and you asked
the settings manager for automatic update of GUI controls,
the appropriate GUI controls are automatically updated.

@@
*/

BOOL APIENTRY WtkEvaluateObjectSettings( HVALUETABLE hvt, PSZ pszSetup)
{

         BOOL              fResult = FALSE;
         APIRET            rc = NO_ERROR;
         ULONG             ulDebugAPIValue = STM_DEBUG_API_EVALUATEOBJECTSETTINGS;

         ULONG             i;
         PVALUEDATAROOT    pvdr = NULL;
         PVALUEDATA        pvd = NULL;
         PSETTINGSDATA     psd = NULL;


         PSZ               pszCopy = NULL;
         PSZ               pszCurrentSetting;
         ULONG             ulValuesFound;

         PSZ               pszSetting;
         PSZ               pszCurrentValue;
         PSZ               pszValue;
         PSZ               pszDefault;

         BOOL              fValue;
         LONG              lValue;
         BOOL              fIndexTranslated = FALSE;

         CHAR              szSearchValue[ 10];

         BOOL              fValid;
         PSZ               pszDigit;

         BOOL              fChangedAll = FALSE;
         BOOL              fChangedSetting = FALSE;


         CBVALIDATE        v;

         CBREPORTCHANGED   rch;
         CBREPORTSAVED     rs;

         HWND              hwndDialog;

DEBUGMSG( "# %s: %s\n", __FUNCTION__ _c_ (pszSetup) ? pszSetup : "<NULL>");

do
   {
   // check parms
   pvdr = __getPointerFromHandle( hvt, SIG_VALUEMANAGER);
   if (!pvdr)
      {
      DEBUGMSG( "# error: no value data root pointer, abort!\n", 0);
      // no object pointer for _wpSetError, so immediate return,
      return fResult;
      }

   if (!pvdr->pvdFirstValue)
      {
      rc = ERROR_INVALID_DATA;
      break;
      }

   if ((!pszSetup) || (!*pszSetup))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   DEBUGMSGDETAIL( "#   >> evaluating setup string: %s\n", pszSetup);

   // crate copy of setup string
   pszCopy = strdup( pszSetup);
   if (!pszCopy)
      {
      rc = ERROR_NOT_ENOUGH_MEMORY;
      break;
      }

   pszCurrentSetting = pszCopy;
   while (pszCurrentSetting != NULL)
      {
      // get first next tokenized settings string
      // - return value pszSetting will point to the current setting
      // - pszCurrentSetting will be moved to following setting on return
      pszSetting = __getSettingFromSetupString( &pszCurrentSetting, &ulValuesFound);
      if (!pszSetting)
         break;

      // do we have that setting ?
      pvd = __getValueData( pvdr, pszSetting);
      if (!pvd)
         continue;

      // validate this setting
      // check this settings
      if (stricmp(  pszSetting, pvd->psd->pszName))
         continue;

      DEBUGMSGDETAIL( "#   >> setting %s found\n", pvd->psd->pszName);

      // validate  all settings values
      fValid            = TRUE;
      fChangedSetting   = FALSE;
      pszCurrentValue   = pszSetting;
      pszDefault        = pvd->psd->pszName; // setting defaults after this string
                                             // see WtkAddClassSetting calling
                                             // __getSettingFromSetupString

      // validate all new values
      // __getSettingFromSetupString has tokenized the string
      for (i = 0; ((i < pvd->psd->ulValueCount) && (fValid)); i++)
         {
         // make sure that we do not scan for more values than available
         if (i < ulValuesFound)
            pszCurrentValue = NEXTSTRING( pszCurrentValue);
         else
            pszCurrentValue = "";

         // is the value empty and a default available ?
         pszDefault = NEXTSTRING( pszDefault);
         if (*pszCurrentValue != 0)
            pszValue = pszCurrentValue;
         else
            pszValue = pszDefault;

         // is it valid ?
         fValid = __validateSettingsValue( ulDebugAPIValue, pvdr, pvd, NULLHANDLE, pszValue, i);

         } // for (i = 0; i < pvd->psd->ulValueCount; i++)

      // ignore complete setting, if one value is invalid
      DEBUGMSGDETAIL( "#   >> setting is %svalid.\n", (fValid) ? "" : "not ");
      if (!fValid)
         continue;

      // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

      // process all settings values
      // __getSettingFromSetupString has tokenized the string
      pszCurrentValue   = pszSetting;
      pszDefault        = pvd->psd->pszName; // setting defaults after this string
      for (i = 0; i < pvd->psd->ulValueCount; i++)
         {
         // make sure that we do not scan for more values than available
         if (i < ulValuesFound)
            pszCurrentValue = NEXTSTRING( pszCurrentValue);
         else
            pszCurrentValue = "";

         // is the value empty and a default available ?
         pszDefault = NEXTSTRING( pszDefault);
         if (*pszCurrentValue != 0)
            pszValue = pszCurrentValue;
         else
            pszValue = pszDefault;

         // update the value
         if (__writeValue( ulDebugAPIValue, pvdr, pvd, NULLHANDLE, pszValue, i))
            {
            fChangedSetting = TRUE;
            fChangedAll = TRUE;
            }

         } // <for all values of a setting>

      // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

      // update GUI controls, if needed
      // process all settings values
      pszValue = pszSetting;
      for (i = 0; i < pvd->psd->ulValueCount; i++)
         {
         // make sure that we do not scan for more values than available
         if (i < ulValuesFound)
            pszValue = NEXTSTRING( pszValue);
         else
            pszValue = "";

         // update GUI Control
         __updateGuiControl( ulDebugAPIValue, pvdr, pvd, *(pvd->psd->pahwndControl + i), i);

         } // <for all values of a setting>

      // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

      if (fChangedSetting)
         {
         // tell WPS class that we have stored new value(s)
         // for this setting
         DEBUGMSGCALLBACK( "#   >> callback: change notification for setting %s\n", pvd->psd->pszName);
         hwndDialog = WinQueryWindow( *(pvd->psd->pahwndControl), QW_PARENT);
         memset( &rch, 0, sizeof( rch));
         rch.pvObjectInstance = pvdr->pvObjectInstance;
         rch.pszName          = pvd->psd->pszName;
         rch.ulSettingId      = pvd->psd->ulSettingId;
         (pvdr->psdr->pfnCallbackValue)( STM_CALLBACK_REPORTCHANGED, &rch, pvdr->pvObjectInstance, pvdr->pvObjectData);
         } // if (fChangedSetting)

      } // while <search thru all settings in pszSetup>

   if (fChangedAll)
      {
//    // tell WPS class that we have saved data
//    DEBUGMSGCALLBACK( "#   >> callback: save settings notification\n", 0);
//    memset( &rs, 0, sizeof( rs));
//    rs.fSaved = TRUE;
//    fResult = (pvdr->psdr->pfnCallbackValue)( STM_CALLBACK_REPORTSAVED, &rs, pvdr->pvObjectInstance, pvdr->pvObjectData);

      // tell WPS class that changes of new values have finished
      DEBUGMSGCALLBACK( "#   >> callback: change complete notification\n", 0);
      memset( &rch, 0, sizeof( rch));
      rch.pvObjectInstance = pvdr->pvObjectInstance;
      (pvdr->psdr->pfnCallbackValue)( STM_CALLBACK_REPORTCHANGED, &rch, pvdr->pvObjectInstance, pvdr->pvObjectData);

      // refresh open detail views
      REFRESHDETAILS( pvdr->pvObjectInstance);
      }

   } while (FALSE);

// cleanup
if (pszCopy) free( pszCopy);


fResult = (rc == NO_ERROR);
DEBUGMSG( "#   << fResult=%u rc=%u\n", fResult _c_ rc);
SETERROR( pvdr->pvObjectInstance, rc);
return fResult;

}

// ---------------------------------------------------------------------------

/*
@@WtkQueryObjectSettings@SYNTAX
This function queries the current settings of an object
being implented by the WPS class.

@@WtkQueryObjectSettings@PARM@hvt@in
Valuetable handle.

@@WtkQueryObjectSettings@PARM@pszBuffer@out
The address of a buffer in into which the resulting
settings string is returned.
:p.
In order to only query the length of this string,
leave this parameter NULL.

@@WtkQueryObjectSettings@PARM@pulMaxlen@inout
The address of a variable for the length, in bytes,
of the buffer described by :hp1.pszBuffer:ehp1..

@@WtkQueryObjectSettings@RETURN
Success indicator.
:parml compact.
:pt.TRUE
:pd.String could be queried
:pt.FALSE
:pd.An error occurred.
:eparml.

@@WtkQueryObjectSettings@REMARKS
This function is to be called during _wpSaveState
in order to save the current settings implemented by the WPS class.
:note.
Doing this, you still must call _wpSaveState of the parent class, as
:hp2.WtkQueryObjectSettings:ehp2. will only return and thus
you can only save the settings being implemented by your WPS class !

@@
*/

#define BUFFER_ALLOC_STEP 1024

BOOL APIENTRY WtkQueryObjectSettings( HVALUETABLE hvt, PSZ pszBuffer, PULONG pulMaxlen)
{

         BOOL              fResult = FALSE;
         APIRET            rc = NO_ERROR;
         ULONG             ulDebugAPIValue = STM_DEBUG_API_QUERYOBJECTSETTINGS;

         ULONG             i,q;

         PVALUEDATAROOT    pvdr = NULL;
         PVALUEDATA        pvd = NULL;
         PSETTINGSDATA     psd = NULL;

         CBQUERYVALUE      qv;

         BOOL              fSettingHasValue = FALSE;

         PVOID             pvBuffer   = NULL;
         ULONG             ulBufferSize = 0;

         PSZ               pszSetting;
         ULONG             ulSettingLen;

         PSZ               pszSettingValue;
         BOOL              fSettingValue;
         LONG              lSettingValue;
         PSZ               p;


if (pszBuffer)
   {
   DEBUGMSG( "# %s\n", __FUNCTION__);
   }

do
   {
   // check parms
   pvdr = __getPointerFromHandle( hvt, SIG_VALUEMANAGER);
   if (!pvdr)
      // no object pointer for _wpSetError, so immediate return,
      return fResult;

   if ((!pvdr->pvdFirstValue) || (!pulMaxlen))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // allocate memory and collect all settings
   ulBufferSize += BUFFER_ALLOC_STEP;
   pvBuffer = malloc( BUFFER_ALLOC_STEP);
   if (!pvBuffer)
      {
      rc = ERROR_NOT_ENOUGH_MEMORY;
      break;
      }

   // scan once for all settings being supported
   ulSettingLen = 0;
   p = pvBuffer;
   *p = 0;

   for (pvd = pvdr->pvdFirstValue; pvd != NULL; pvd = pvd->pvNextValue)
      {
      // if this one is a detail, skip
      if (!*pvd->psd->pszName)
         continue;

      for (q = 0; q < pvd->psd->ulQueryCount; q++)
         {
         // tell WPS class that we need multiple queries
         if (pszBuffer)
            {
            DEBUGMSGCALLBACK( "#   >> callback: request value #%u of %s\n", q _c_ pvd->psd->pszName);
            }
         memset( &qv, 0, sizeof( qv));
         qv.pszName       = pvd->psd->pszName;
         qv.ulSettingId   = pvd->psd->ulSettingId;
         qv.ulQueryIndex  = q;
         fResult = (pvdr->psdr->pfnCallbackValue)( STM_CALLBACK_QUERYVALUE, &qv, pvdr->pvObjectInstance, pvdr->pvObjectData);

         // bail out if consecutive query fails
         if ((q > 0) && (!fResult))
            break;

         // enlarge buffer if necessary
         if ((strlen( pvBuffer) + 1 + ulSettingLen) > ulBufferSize)
            {
            ulBufferSize += BUFFER_ALLOC_STEP;
            pvBuffer = realloc( pvBuffer, ulBufferSize);
            }

         // save start position of this setting and prewrite setting
         pszSetting = p;
         fSettingHasValue = TRUE;
         sprintf( ENDOFSTRING( p), "%s=", pvd->psd->pszName);

         // add all values
         for (i = 0; i < pvd->psd->ulValueCount; i++)
            {

            // determine value
            pszSettingValue  = *(pvd->papszValues + i);
            switch( *(pvd->psd->paulValueType + i))
               {
               case STM_VALUETYPE_STRING:
                  if (strlen( pszSettingValue))
                     strcat( ENDOFSTRING( p), pszSettingValue);
                  else
                     fSettingHasValue = FALSE;
                  break;

               case STM_VALUETYPE_INDEX:
               case STM_VALUETYPE_INDEXITEM:
                  {
                            PSZ        pszValue;

                  lSettingValue = *(PLONG) pszSettingValue;
                  // ask the WPS class for translation of index to string
                  // this returns a strdup !
                  pszValue = __getIndexString( ulDebugAPIValue, TRUE, pvdr, pvd, i, lSettingValue);
                  strcat( ENDOFSTRING( p), pszValue);
                  free( pszValue);
                  }
                  break;

               case STM_VALUETYPE_LONG:
                  lSettingValue = *(PLONG) pszSettingValue;
                  sprintf( ENDOFSTRING( p), "%d", lSettingValue);
                  break;

               case STM_VALUETYPE_TRUEFALSE:
                  fSettingValue = *(PBOOL) pszSettingValue;
                  sprintf( ENDOFSTRING( p), "%s", (fSettingValue) ? "TRUE" : "FALSE");
                  break;

               case STM_VALUETYPE_YESNO:
                  fSettingValue = *(PBOOL) pszSettingValue;
                  sprintf( ENDOFSTRING( p), "%s", (fSettingValue) ? "YES" : "NO");
                  break;

               case STM_VALUETYPE_ONOFF:
                  fSettingValue = *(PBOOL) pszSettingValue;
                  sprintf( ENDOFSTRING( p), "%s", (fSettingValue) ? "ON" : "OFF");
                  break;

               default:
                  // ignore value on unknown type
                  fSettingHasValue = FALSE;
                  break;
               }

            // add a comma for each setting except the only/last one
            if (i < pvd->psd->ulValueCount - 1)
               strcat( p, ",");

            } // for [value index]

         // concatenate semicolon
         strcat( p, ";");

         // has this setting values ? otherwise it is ignored !
         if (fSettingHasValue)
            {
            if (pszBuffer)
               {
               DEBUGMSGDETAIL( "#   >> %s\n", p);
               }
            p = ENDOFSTRING( p);
            }
         else
            {
            pszSetting = p;
            *p = 0;
            }

         } // for [all queries for a setting]

      } // for [all settings]


   // copy current settings
   if (pszBuffer)
      {
      // check buffer overflow
      if (*pulMaxlen < strlen( pvBuffer) + 1)
         {
         rc = ERROR_BUFFER_OVERFLOW;
         break;
         }

      strcpy( pszBuffer, pvBuffer);
      }

   // report actual len
   *pulMaxlen = strlen( pvBuffer) + 1;

   } while (FALSE);

// clean up
if (pvBuffer) free( pvBuffer);

if (pszBuffer)
   {
   DEBUGMSG( "#   << fResult=%u rc=%u settings: %s\n", fResult _c_ rc _c_ pszBuffer);
   }

fResult = (rc == NO_ERROR);
SETERROR( pvdr->pvObjectInstance, rc);
return fResult;

}

// ---------------------------------------------------------------------------

/*
@@WtkQueryObjectInstance@SYNTAX
This function queries the somSelf pointer to the object class
(WPS object instance) and the somThis pointer to its instance data.

@@WtkQueryObjectInstance@PARM@hvt@in
Valuetable handle.

@@WtkQueryObjectInstance@PARM@*ppvObjectInstance@out
Pointer to a variable receiving the somSelf pointer to the object class.
:p.
Either this or the ppvObjectData parameter may be NULL, but not both.

@@WtkQueryObjectInstance@PARM@*ppvObjectData@out
Pointer to a variable receiving the somThis pointer to the data of the object class.
:p.
Either this or the ppvObjectInstance parameter may be NULL, but not both.

@@WtkQueryObjectInstance@RETURN
Success indicator.
:parml compact.
:pt.TRUE
:pd.somSelf and somThis pointers to the object class could be returned.
:pt.FALSE
:pd.An error occurred.
:eparml.

@@WtkQueryObjectInstance@REMARKS
This function can be used within the dialog procedure for property notebook pages
to access the methods and the data of the object class.

@@
*/

BOOL APIENTRY WtkQueryObjectInstance( HVALUETABLE hvt, PVOID *ppvObjectInstance, PVOID *ppvObjectData)
{
         BOOL              fResult = FALSE;
         APIRET            rc = NO_ERROR;
         ULONG             ulDebugAPIValue = STM_DEBUG_API_QUERYOBJECTINSTANCE;
         PVALUEDATAROOT    pvdr = NULL;


DEBUGMSG( "# %s: \n", __FUNCTION__);

do
   {
   if ((!ppvObjectInstance) && (!ppvObjectData))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   pvdr = __getPointerFromHandle( hvt, SIG_VALUEMANAGER);
   if (!pvdr)
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   if (ppvObjectInstance)
      *ppvObjectInstance = pvdr->pvObjectInstance;

   if (ppvObjectData)
      *ppvObjectData = pvdr->pvObjectData;

   } while (FALSE);

fResult = (rc == NO_ERROR);
SETERROR( pvdr->pvObjectInstance, rc);
return fResult;
}

// ---------------------------------------------------------------------------

/*
@@WtkQuerySettingstable@SYNTAX
This function queries the handle to the settingstable related
to the specified valuetable.

@@WtkQuerySettingstable@PARM@hvt@in
Valuetable handle.

@@WtkQuerySettingstable@PARM@phst@out
Pointer to a variable receiving the handle to the settingstable.

@@WtkQuerySettingstable@RETURN
Success indicator.
:parml compact.
:pt.TRUE
:pd.The handle to the settingstable could be queried.
:pt.FALSE
:pd.An error occurred.
:eparml.

@@WtkQuerySettingstable@REMARKS
This function can be used within the dialog procedure for property notebook pages
to obtain a pointer to the object metaclass by a call to
:link reftype=hd refid=WtkQueryObjectClass.WtkQueryObjectClass:elink..

@@
*/

BOOL APIENTRY WtkQuerySettingstable( HVALUETABLE hvt, PHSETTINGTABLE phst)
{
         BOOL              fResult = FALSE;
         APIRET            rc = NO_ERROR;
         ULONG             ulDebugAPIValue = STM_DEBUG_API_QUERYSETTINGTABLE;
         PVALUEDATAROOT    pvdr = NULL;


DEBUGMSG( "# %s: \n", __FUNCTION__);

do
   {
   if (!phst)
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   pvdr = __getPointerFromHandle( hvt, SIG_VALUEMANAGER);
   if (!pvdr)
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // return mangled pointer
   *phst = PTR2HANDLE( pvdr->psdr, SIG_SETTINGSMANAGER);

   } while (FALSE);

fResult = (rc == NO_ERROR);
SETERROR( pvdr->pvObjectInstance, rc);
return fResult;
}

// ---------------------------------------------------------------------------

/*
@@WtkRegisterSettingsNotebook@SYNTAX
This function reqisters the notebook for automatic
validation of all registered settings notebookpages,
when the notebook is closed.

@@WtkRegisterSettingsNotebook@PARM@hvt@in
Valuetable handle.

@@WtkRegisterSettingsNotebook@PARM@hwndNotebook@in
Notebook handle.

@@WtkRegisterSettingsNotebook@RETURN
Success indicator.
:parml compact.
:pt.TRUE
:pd.Notebook could be registered.
:pt.FALSE
:pd.An error occurred.
:eparml.

@@WtkRegisterSettingsNotebook@REMARKS
This function is to be called in _wpAddSettingsPages, after
the WPS class added all settings notebook pages.
:p.
When the notebook is closed, all values of the objects valuetable
are checked. If the settings notebookpage containing the GUI control
for a value has been registerd (thus become active)
:ul compact.
:li.the value of this control is validated
:li.if requested from the WPS class, on validation error
:ul compact.
:li.the notebook page of the errant GUI control is brought to top
:li.the errant GUI receives focus
:li.the WPS class is being notified of the error, so that it can take
the appropriate action
:li.the close request is ignored
:eul.
:eul.

@@
*/

BOOL APIENTRY WtkRegisterSettingsNotebook( HVALUETABLE hvt, HWND hwndNotebook)
{

         BOOL              fResult = FALSE;
         APIRET            rc = NO_ERROR;
         ULONG             ulDebugAPIValue = STM_DEBUG_API_REGISTERSETTINGSNOTEBOOK;

         HWND              hwndFrame;
static   PSZ               pszObjectWindowClass = "SettingsManager";
         HWND              hwndObject = NULLHANDLE;

         ULONG             i,q;

         PVALUEDATAROOT    pvdr = NULL;
         PVALUEDATA        pvd = NULL;

         HAB               hab = NULLHANDLE;
         BOOL              fHandleAnchorBlockRequested = FALSE;


DEBUGMSG( "# %s\n", __FUNCTION__);
do
   {
   // check parms
   pvdr = __getPointerFromHandle( hvt, SIG_VALUEMANAGER);
   if (!pvdr)
      // no object pointer for _wpSetError, so immediate return,
      return fResult;

   // do we have a hab ? if not, request one
   fHandleAnchorBlockRequested = __requestHandleAnchorBlock( &hab);
   if (!hab)
      {
      rc = ERROR_INVALID_FUNCTION;
      break;
      }

   // check parms
   if ((!pvdr->pvdFirstValue) || (!hwndNotebook) || (!WinIsWindow( hab, hwndNotebook)))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // whois the frame ?
   hwndFrame = WinQueryWindow( hwndNotebook, QW_PARENT);

   // create own object window class
   if (!WinRegisterClass( hab, pszObjectWindowClass, NULL, 0, sizeof( PVOID)))
      break;


   // create object window as child of frame
   hwndObject = WinCreateWindow( hwndFrame, pszObjectWindowClass, "",
                                 0, 0, 0, 0, 0, hwndFrame, HWND_BOTTOM,
                                 (ULONG) SIG_SETTINGSMANAGER, NULL, NULL);
   if (!hwndObject)
      break;

   // now attach the handle to the date
   if (! WinSetWindowULong( hwndObject, QWL_USER, hvt))
      break;

   // now subclass the
   DEBUGMSGDETAIL( "#   >> subclassing notebook frame %p\n", hwndFrame);
   pvdr->pfnwpNotebookFrameOrg = (PFNWP) WinSubclassWindow( hwndFrame, __SettingsNotebookFrameSubclassProc);

   } while (FALSE);

// clean up
if (fHandleAnchorBlockRequested) WinTerminate( hab);

fResult = (rc == NO_ERROR);
DEBUGMSG( "#   << fResult=%u rc=%u\n", fResult _c_ rc);
SETERROR( pvdr->pvObjectInstance, rc);
return fResult;

}

// ---------------------------------------------------------------------------

/*
@@WtkRegisterSettingsDialog@SYNTAX
This function reqisters a dialog of a settings notebookpage
for automatic update and restore of the GUI dialog to and from
the valuetable of the WPS object.

@@WtkRegisterSettingsDialog@PARM@hvt@in
Valuetable handle.

@@WtkRegisterSettingsDialog@PARM@hwndDialog@in
Dialog handle of the settings notebook page.

@@WtkRegisterSettingsDialog@RETURN
Success indicator.
:parml compact.
:pt.TRUE
:pd.Dialog could be registered.
:pt.FALSE
:pd.An error occurred.
:eparml.

@@WtkRegisterSettingsDialog@REMARKS
This function is to be called in the WM_INITDLG
message handling of the window procedure being
used for the settings notebookpages.

@@
*/

BOOL APIENTRY WtkRegisterSettingsDialog( HVALUETABLE hvt, HWND hwndDialog)
{

         BOOL              fResult = FALSE;
         APIRET            rc = NO_ERROR;
         ULONG             ulDebugAPIValue = STM_DEBUG_API_REGISTERSETTINGSDIALOG;

         ULONG             i,q;

         PVALUEDATAROOT    pvdr = NULL;
         PVALUEDATA        pvd = NULL;

         HAB               hab = NULLHANDLE;
         BOOL              fHandleAnchorBlockRequested = FALSE;

         PSZ               pszClassIndex;
         USHORT            usDialogid;
         USHORT            usControlid;
         HWND              hwndControl;
         PFNWP             pfnwpSubclass;

         ULONG             ulMaxlen;
         BOOL              fIndexedValue;

         CBINITCONTROL     ic;


DEBUGMSG( "# %s\n", __FUNCTION__);
do
   {
   // check parms
   pvdr = __getPointerFromHandle( hvt, SIG_VALUEMANAGER);
   if (!pvdr)
      // no object pointer for _wpSetError, so immediate return,
      return fResult;

   // do we have a hab ? if not, request one
   fHandleAnchorBlockRequested = __requestHandleAnchorBlock( &hab);
   if (!hab)
      {
      rc = ERROR_INVALID_FUNCTION;
      break;
      }

   // check parms
   if ((!pvdr->pvdFirstValue) || (!hwndDialog) || (!WinIsWindow( hab, hwndDialog)))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // get the id of the dialog
   usDialogid = WinQueryWindowUShort( hwndDialog, QWS_ID);

   DEBUGMSGDETAIL( "#   >> registering for dialog %u(0x%04x) \n", usDialogid _c_ usDialogid);

   // now check all settings for the dialog id
   for (pvd = pvdr->pvdFirstValue; pvd != NULL; pvd = pvd->pvNextValue)
      {

      // add all values
      for (i = 0; i < pvd->psd->ulValueCount; i++)
         {
         // check for dialog id
         usControlid = *(pvd->psd->pausControlid + i);
         if (*(pvd->psd->pausDialogid + i) == usDialogid)
            {

            // save the window handle
            // if subcontrol is not available, hwnd is still NULL
            hwndControl = WinWindowFromID( hwndDialog, usControlid);
            if (hwndControl)
               {
               *(pvd->psd->pahwndControl + i) = hwndControl;
               DEBUGMSGDETAIL( "#   >> register control %u(0x%04x) for %s:%u id %u(0x%04x), hwnd=0x%08x\n",
               usControlid        _c_
               usControlid        _c_
               pvd->psd->pszName  _c_
               i                  _c_
               pvd->psd->ulSettingId  _c_
               pvd->psd->ulSettingId  _c_
               hwndControl);

               // do our own initialisation first
               pszClassIndex = WtkQueryClassIndex( hwndControl);
               fIndexedValue = FALSE;
               switch ((ULONG) pszClassIndex)
                  {
                  case (ULONG) WC_ENTRYFIELD:
                     switch (*(pvd->psd->paulValueType + i))
                        {
                        case STM_VALUETYPE_STRING:
                           // make sure that input cannot be longer
                           // than target buffer !
                           ulMaxlen  = (ULONG)*(pvd->paulValueMaxLen + i) - 1;
                           break;

                        case STM_VALUETYPE_INDEX:
                        case STM_VALUETYPE_INDEXITEM:
                        case STM_VALUETYPE_TRUEFALSE:
                        case STM_VALUETYPE_YESNO:
                        case STM_VALUETYPE_ONOFF:
                           fIndexedValue = TRUE;
                           // fallthru !!!

                        case STM_VALUETYPE_LONG:
                           if (fIndexedValue)
                              // for indexed values make sure that entry field can not
                              // be modified and accepts values of max display len
                              ulMaxlen  = (ULONG)*(pvd->psd->paulDispStringMaxLen + i);
                           else
                              ulMaxlen = 16;

                           // set entry field to readonly
                           WinSetWindowULong( hwndControl, QWL_STYLE,
                                              WinQueryWindowULong( hwndControl, QWL_STYLE) | ES_READONLY);
                           break;
                        }

                     WinSendMsg( hwndControl, EM_SETTEXTLIMIT, MPFROMLONG( ulMaxlen), 0);
                     break;
                  }

               // ask the WPS class to initialize the control
               DEBUGMSGCALLBACK( "#   >> callback: request initialization of control %u(0x%04x) dialog %u(0x%04x)\n",
                                 usControlid _c_
                                 usControlid _c_
                                 usDialogid  _c_
                                 usDialogid);
               memset( &ic, 0, sizeof( ic));
               ic.usDialogid  = usDialogid;
               ic.usControlid = usControlid;
               ic.hwndDialog  = hwndDialog;
               ic.hwndControl = hwndControl;
               (pvdr->psdr->pfnCallbackValue)( STM_CALLBACK_INITCONTROL, &ic, pvdr->pvObjectInstance, pvdr->pvObjectData);

               // subclass control, if desired
               pfnwpSubclass = *(pvd->psd->papfnwpSubclass + i);
               if (pfnwpSubclass)
                  {
                  fResult = (BOOL) WinSubclassWindow( hwndControl, pfnwpSubclass);
                  DEBUGMSGDETAIL( "#   >> subclass control %u(0x%04x) for %s:%u, %ssuccessful\n",
                  usControlid        _c_
                  usControlid        _c_
                  pvd->psd->pszName  _c_
                  i                  _c_
                  (fResult) ? "" : "not ");
                  }
               }
            else
               DEBUGMSGDETAIL( "#   >> skipping control %u(0x%04x) for %s:%u, not found\n",
               usControlid        _c_
               usControlid        _c_
               pvd->psd->pszName  _c_
               i);

            } // if (*(pvd->pausDialogid + i) == usDialogid)
         else
            {
            DEBUGMSGDETAIL( "#   >> skipping control %u(0x%04x) for %s:%u: other dialog %u(0x%04x)\n",
            usControlid        _c_
            usControlid        _c_
            pvd->psd->pszName  _c_
            i                  _c_
            *(pvd->psd->pausDialogid + i) _c_
            *(pvd->psd->pausDialogid + i));
            }

         } // for [all subvalues]

      } // for [all settings]

   // ask the WPS class to initialize all other controls (control id == -1)
   DEBUGMSGCALLBACK( "#   >> callback: request initialization of other controls of dialog %u(0x%04x) \n",
                     usDialogid _c_
                     usDialogid);
   memset( &ic, 0, sizeof( ic));
   ic.usDialogid  = usDialogid;
   ic.usControlid = -1;
   ic.hwndDialog  = hwndDialog;
   ic.hwndControl = hwndControl;
   (pvdr->psdr->pfnCallbackValue)( STM_CALLBACK_INITCONTROL, &ic, pvdr->pvObjectInstance, pvdr->pvObjectData);

   } while (FALSE);

// clean up
if (fHandleAnchorBlockRequested) WinTerminate( hab);

fResult = (rc == NO_ERROR);
DEBUGMSG( "#   << fResult=%u rc=%u\n", fResult _c_ rc);
SETERROR( pvdr->pvObjectInstance, rc);
return fResult;

}

// ---------------------------------------------------------------------------

/*
@@WtkDeregisterSettingsDialog@SYNTAX
This function dereqisters a dialog of a settings notebookpage.

@@WtkDeregisterSettingsDialog@PARM@hvt@in
Valuetable handle.

@@WtkDeregisterSettingsDialog@PARM@hwndDialog@in
Dialog handle of the settings notebook page.

@@WtkDeregisterSettingsDialog@RETURN
Success indicator.
:parml compact.
:pt.TRUE
:pd.Dialog could be deregistered.
:pt.FALSE
:pd.An error occurred.
:eparml.

@@WtkDeregisterSettingsDialog@REMARKS
This function is to be called in the WM_DESTROY
message handling of the window procedure being
used for the settings notebookpages.

@@
*/

BOOL APIENTRY WtkDeregisterSettingsDialog( HVALUETABLE hvt, HWND hwndDialog)
{

         BOOL              fResult = FALSE;
         APIRET            rc = NO_ERROR;
         ULONG             ulDebugAPIValue = STM_DEBUG_API_DEREGISTERSETTINGSDIALOG;

         ULONG             i,q;

         PVALUEDATAROOT    pvdr = NULL;
         PVALUEDATA        pvd = NULL;

         HAB               hab = NULLHANDLE;
         BOOL              fHandleAnchorBlockRequested = FALSE;

         USHORT            usDialogid;
         BOOL              fDeregisterAll = (!hwndDialog);

DEBUGMSG( "# %s\n", __FUNCTION__);

do
   {
   // check parms
   pvdr = __getPointerFromHandle( hvt, SIG_VALUEMANAGER);
   if (!pvdr)
      // no object pointer for _wpSetError, so immediate return,
      return fResult;

   // do we have a hab ? if not, request one
   fHandleAnchorBlockRequested = __requestHandleAnchorBlock( &hab);
   if (!hab)
      {
      rc = ERROR_INVALID_FUNCTION;
      break;
      }

   // check parms
   if ((!pvdr->pvdFirstValue) ||
       ((!fDeregisterAll) && (!WinIsWindow( hab, hwndDialog))))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }


   // get the id of the dialog
   usDialogid = WinQueryWindowUShort( hwndDialog, QWS_ID);

   // now check all settings for the dialog id
   for (pvd = pvdr->pvdFirstValue; pvd != NULL; pvd = pvd->pvNextValue)
      {

      // add all values
      for (i = 0; i < pvd->psd->ulValueCount; i++)
         {
         // check for dialog id
         if ((fDeregisterAll) || (*(pvd->psd->pausDialogid + i) == usDialogid))
            {
            if (*(pvd->psd->pahwndControl + i))
               DEBUGMSGDETAIL( "#   >> deregistering control for %s:%u\n",
                               pvd->psd->pszName _c_ i);
            *(pvd->psd->pahwndControl + i) = NULLHANDLE;
            }

         } // for [all subvalues]

      } // for [all settings]

   } while (FALSE);

// clean up
if (fHandleAnchorBlockRequested) WinTerminate( hab);

fResult = (rc == NO_ERROR);
DEBUGMSG( "#   << fResult=%u rc=%u \n", fResult _c_ rc);
SETERROR( pvdr->pvObjectInstance, rc);
return fResult;

}

// ---------------------------------------------------------------------------

/*
@@WtkReadObjectValueTable@SYNTAX
This function reads the object valuetable and
transfers them to the GUI controls of a given
settings notebookpage.

@@WtkReadObjectValueTable@PARM@hvt@in
Valuetable handle.

@@WtkReadObjectValueTable@PARM@hwndDialog@in
Dialog handle of the settings notebook page.

@@WtkReadObjectValueTable@RETURN
Success indicator.
:parml compact.
:pt.TRUE
:pd.Values could be transferred to the notebook page dialog.
:pt.FALSE
:pd.An error occurred.
:eparml.

@@WtkReadObjectValueTable@REMARKS
This function is to be called in the
message handling of the window procedure being
used for the settings notebookpages.
:p.
It should be called when handling the WM_INITDLG
message and the WM_COMMAND message for the undo
button in order to transfer the current object
settings values to the notebook settings page.

@@
*/

BOOL APIENTRY WtkReadObjectValueTable( HVALUETABLE hvt, HWND hwndDialog)
{

         BOOL              fResult = FALSE;
         APIRET            rc = NO_ERROR;
         ULONG             ulDebugAPIValue = STM_DEBUG_API_READVALUETABLE;

         ULONG             i,q;

         PVALUEDATAROOT    pvdr = NULL;
         PVALUEDATA        pvd = NULL;

         HAB               hab = NULLHANDLE;
         BOOL              fHandleAnchorBlockRequested = FALSE;

         USHORT            usDialogid;

DEBUGMSG( "# %s\n", __FUNCTION__);
do
   {
   // check parms
   pvdr = __getPointerFromHandle( hvt, SIG_VALUEMANAGER);
   if (!pvdr)
      // no object pointer for _wpSetError, so immediate return,
      return fResult;

   // do we have a hab ? if not, request one
   fHandleAnchorBlockRequested = __requestHandleAnchorBlock( &hab);
   if (!hab)
      {
      rc = ERROR_INVALID_FUNCTION;
      break;
      }

   // check parms
   if ((!pvdr->pvdFirstValue) || (!hwndDialog) || (!WinIsWindow( hab, hwndDialog)))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // get the id of the dialog
   usDialogid = WinQueryWindowUShort( hwndDialog, QWS_ID);

   // now check all settings for the dialog id
   for (pvd = pvdr->pvdFirstValue; pvd != NULL; pvd = pvd->pvNextValue)
      {

      // add all values
      for (i = 0; i < pvd->psd->ulValueCount; i++)
         {
         // check for dialog id
         if (*(pvd->psd->pausDialogid + i) == usDialogid)
            {
            // update the GUI control with current value
            __updateGuiControl( ulDebugAPIValue, pvdr, pvd, *(pvd->psd->pahwndControl + i), i);
            }

         } // for [all subvalues]

      } // for [all settings]

   } while (FALSE);

// clean up
if (fHandleAnchorBlockRequested) WinTerminate( hab);

fResult = (rc == NO_ERROR);
DEBUGMSG( "#   << fResult=%u rc=%u\n", fResult _c_ rc);
SETERROR( pvdr->pvObjectInstance, rc);
return fResult;

}

// ---------------------------------------------------------------------------

/*
@@WtkWriteObjectValueTable@SYNTAX
This function reads the GUI controls of the given
settings notebookpage and updates the corresponding
values in the object valuetable.

@@WtkWriteObjectValueTable@PARM@hvt@in
Valuetable handle.

@@WtkWriteObjectValueTable@PARM@hwndDialog@in
Dialog handle of the settings notebook page.

@@WtkWriteObjectValueTable@RETURN
Success indicator.
:parml compact.
:pt.TRUE
:pd.Values could be transferred to the object valuetable.
:pt.FALSE
:pd.An error occurred.
:eparml.

@@WtkWriteObjectValueTable@REMARKS
This function is to be called in the WM_DESTROY
message handling of the window procedure being
used for the settings notebookpages, so that the
values of the GUI controls are written to the object
valuetable, when the notebook is closed.

@@
*/

BOOL APIENTRY WtkWriteObjectValueTable( HVALUETABLE hvt, HWND hwndDialog)
{
         BOOL              fResult = FALSE;
         APIRET            rc = NO_ERROR;
         ULONG             ulDebugAPIValue = STM_DEBUG_API_WRITEVALUETABLE;

         ULONG             i,q;

         PVALUEDATAROOT    pvdr = NULL;
         PVALUEDATA        pvd = NULL;

         HAB               hab = NULLHANDLE;
         BOOL              fHandleAnchorBlockRequested = FALSE;

         USHORT            usDialogid;
         BOOL              fInvalidValue = FALSE;
         PSZ               pszNewValue;

         BOOL              fChanged = FALSE;

         CBREPORTSAVED     rs;
         CBREPORTCHANGED   rch;



DEBUGMSG( "# %s\n", __FUNCTION__);
do
   {
   // check parms
   pvdr = __getPointerFromHandle( hvt, SIG_VALUEMANAGER);
   if (!pvdr)
      // no object pointer for _wpSetError, so immediate return,
      return fResult;

   // do we have a hab ? if not, request one
   fHandleAnchorBlockRequested = __requestHandleAnchorBlock( &hab);
   if (!hab)
      {
      rc = ERROR_INVALID_FUNCTION;
      break;
      }

   // check parms
   if ((!pvdr->pvdFirstValue) || (!hwndDialog) || (!WinIsWindow( hab, hwndDialog)))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // get the id of the dialog
   usDialogid = WinQueryWindowUShort( hwndDialog, QWS_ID);

   // get all values again and write them to the value table
   for (pvd = pvdr->pvdFirstValue; ((pvd != NULL)); pvd = pvd->pvNextValue)
      {
      for (i = 0; i < pvd->psd->ulValueCount; i++)
         {
         // check for dialog id
         if (*(pvd->psd->pausDialogid + i) == usDialogid)
            {
            // update the value from GUI control
            if (__writeValue( ulDebugAPIValue, pvdr, pvd, *(pvd->psd->pahwndControl + i), NULL, i))
               {
               // notify
               DEBUGMSGCALLBACK( "#   >> callback: change notification for setting %s\n", pvd->psd->pszName);
               hwndDialog = WinQueryWindow( *(pvd->psd->pahwndControl), QW_PARENT);
               memset( &rch, 0, sizeof( rch));
               rch.pvObjectInstance = pvdr->pvObjectInstance;
               rch.pszName          = pvd->psd->pszName;
               rch.ulSettingId      = pvd->psd->ulSettingId;
               (pvdr->psdr->pfnCallbackValue)( STM_CALLBACK_REPORTCHANGED, &rch, pvdr->pvObjectInstance, pvdr->pvObjectData);

               fChanged = TRUE;
               }
            }

         } // for [all subvalues]

      } // for [all settings]

   if (fChanged)
      {
      // tell WPS class that we have saved data
//    DEBUGMSGCALLBACK( "#   >> callback: save settings notification\n", 0);
//    memset( &rs, 0, sizeof( rs));
//    rs.fSaved = TRUE;
//    fResult = (pvdr->psdr->pfnCallbackValue)( STM_CALLBACK_REPORTSAVED, &rs, pvdr->pvObjectInstance, pvdr->pvObjectData);

      // tell WPS class that changes of new values have finished
      DEBUGMSGCALLBACK( "#   >> callback: change complete notification\n", 0);
      memset( &rch, 0, sizeof( rch));
      rch.pvObjectInstance = pvdr->pvObjectInstance;
      (pvdr->psdr->pfnCallbackValue)( STM_CALLBACK_REPORTCHANGED, &rch, pvdr->pvObjectInstance, pvdr->pvObjectData);

      // refresh open detail views
      REFRESHDETAILS( pvdr->pvObjectInstance);
      }


   } while (FALSE);

// clean up
if (fHandleAnchorBlockRequested) WinTerminate( hab);

fResult = (rc == NO_ERROR);
DEBUGMSG( "#   << fResult=%u rc=%u\n", fResult _c_ rc);
SETERROR( pvdr->pvObjectInstance, rc);
return fResult;

}

// ---------------------------------------------------------------------------

/*
@@WtkQueryGUIControlsChanged@SYNTAX
This function checks a GUI controls of a dialog for changes
by examining the parameters of WM_CONTROL messages.

@@WtkQueryGUIControlsChanged@PARM@hvt@in
Valuetable handle.

@@WtkQueryGUIControlsChanged@PARM@hwnd@in
Dialog handle of the settings notebook page.

@@WtkQueryGUIControlsChanged@PARM@mp1@in
The message parameter 1 of the WM_CONTROL message.

@@WtkQueryGUIControlsChanged@PARM@mp2@in
The message parameter 2 of the WM_CONTROL message.

@@WtkQueryGUIControlsChanged@PARM@pfOrgValue@out
Adress of a flag variable indicating if all controls
of the dialog have the undo value.

@@WtkQueryGUIControlsChanged@RETURN
Change indicator.
:parml compact.
:pt.TRUE
:pd.Value has changed
:pt.FALSE
:pd.Value has not changed
:eparml.

@@WtkQueryGUIControlsChanged@REMARKS
This function is to be called during WM_CONTROL handling
in order to determine wether a GUI control registered with
the Settings Manager has changed.

@@
*/

BOOL APIENTRY WtkQueryGUIControlsChanged( HVALUETABLE hvt, HWND hwnd, MPARAM mp1, MPARAM mp2, PBOOL pfOrgValue)
{

         BOOL              fResult = FALSE;
         APIRET            rc = NO_ERROR;
         ULONG             i;

         ULONG             ulDebugAPIValue = STM_DEBUG_API_QUERYGUICONTROLSCHANGED;

         PVALUEDATAROOT    pvdr = NULL;
         PVALUEDATA        pvd;

         ULONG             ulValueType;
         HWND              hwndDialog;
         USHORT            usDialogid;
         BOOL              fValueMismatch = FALSE;
         BOOL              fOrgValue = FALSE;

DEBUGMSG( "# %s\n", __FUNCTION__);

do
   {
   // check parms
   pvdr = __getPointerFromHandle( hvt, SIG_VALUEMANAGER);
   if (!pvdr)
      {
      // no object pointer for _wpSetError, so immediate return,
      return fResult;
      }

   // check is required only for certain contol messages ...
   fResult = (__checkGUIControlChanged( ulDebugAPIValue, pvdr, NULL, hwnd, mp1, mp2) != NULL);
   if (!fResult)
      break;

   // get the id of the dialog
   usDialogid = WinQueryWindowUShort( hwnd, QWS_ID);

   // now check all values for this dialog
   fOrgValue = TRUE;
   for (pvd = pvdr->pvdFirstValue; ((pvd != NULL)); pvd = pvd->pvNextValue)
      {
      for (i = 0; i < pvd->psd->ulValueCount; i++)
         {
         // check for dialog id
         if (*(pvd->psd->pausDialogid + i) == usDialogid)
            {
                     BOOL           fIsNumValue;
                     USHORT         usControl = *(pvd->psd->pausControlid + i);
                     HWND           hwndControl = WinWindowFromID( hwnd, usControl);
                     PSZ            pszGuiValue = NULL;

            // compare the value with the on in the table
            ulValueType   = *(pvd->psd->paulValueType + i);
            pszGuiValue   = __getValueFromGuiControl( hwndControl, ulValueType, &fIsNumValue);

            if (pszGuiValue)
               {
               if (fIsNumValue)
                  fValueMismatch = ( atol( pszGuiValue) != **(pvd->papszValues + i));
               else
                  fValueMismatch = (strcmp( pszGuiValue, *(pvd->papszValues + i)));

               if (fValueMismatch)
                  {
                           PSZ            pszClassIndex;

                  // report change , because the table contains a different value
                  pszClassIndex = WtkQueryClassIndex( WinWindowFromID( hwnd, usControl));
                  DEBUGMSGDETAIL( "#   >> control %s id %u in dialog %u changed \n",
                                   __getPMClassName( pszClassIndex) _c_
                                   *(pvd->psd->pausControlid + i) _c_
                                   *(pvd->psd->pausDialogid + i));

                  // save change flag
                  fOrgValue = FALSE;
                  }
               }

            // free memory again
            if (pszGuiValue)
               free( pszGuiValue);
            }
         } // for [all subvalues]
      } // for [all settings]

   } while (FALSE);


// fill target flag field
if (pfOrgValue)
   *pfOrgValue = fOrgValue;

// clean up
DEBUGMSG( "#   << fResult=%u \n", fResult);
SETERROR( pvdr->pvObjectInstance, rc);
return fResult;

}

// ---------------------------------------------------------------------------

/*
@@WtkValidateObjectValueTable@SYNTAX
This function validates values of all active GUI
controls registered for values in the objects valuetable
and triggers the callback routine for validation and
error report. On error, the notebook is switched to the page
with the control having invalid content and the focus is set
to that control.
:note.
It is recommended not to call this function by the
code of your WPS class directly, when validation should
take place only on closing the settings notebook (see remarks).

@@WtkValidateObjectValueTable@PARM@hvt@in
Valuetable handle.

@@WtkValidateObjectValueTable@PARM@hwndNotebook@in
Notebook handle.

@@WtkValidateObjectValueTable@RETURN
Success indicator.
:parml compact.
:pt.TRUE
:pd.The values of all registered GUI controls are valid.
:pt.FALSE
:pd.A validation error occurred.
:eparml.

@@WtkValidateObjectValueTable@REMARKS
If validation should take place on close of the settings notebook,
this function should not be called directly by the code
of your WPS class directly.
:p.
Instead, the notebook frame should be registered with
:link reftype=hd refid=WtkRegisterSettingsNotebook.WtkRegisterSettingsNotebook:elink.
during the processing of _wpAddSettingsPages(). This will
let Settings Manager subclass the notebook frame and
call WtkValidateObjectValueTable automatically when the
notebook is closed.
:p.
If it is required to validate GUI controls also on other events,
WtkValidateObjectValueTable can still be used for that.
Note however that it is not possible to validate only for one
dialog registered with Settings Manager. Instead, always the
registered controls of all registered dialogs are validated -
this may result in switching to another notebook page, if there
a GUI control contains invalid contents!
@@
*/

BOOL APIENTRY WtkValidateObjectValueTable( HVALUETABLE hvt, HWND hwndNotebook)
{

         BOOL              fResult = FALSE;
         APIRET            rc = NO_ERROR;
         ULONG             ulDebugAPIValue = STM_DEBUG_API_VALIDATEVALUETABLE;

         ULONG             i,q;

         PVALUEDATAROOT    pvdr = NULL;
         PVALUEDATA        pvd = NULL;

         HWND              hwndControl;
         HWND              hwndDialog;
         USHORT            usControlid;
         USHORT            usDialogid;

         ULONG             ulPageId;
         HWND              hwndPage;

         HAB               hab = NULLHANDLE;
         BOOL              fHandleAnchorBlockRequested = FALSE;

         CBREPORTERROR     re;

DEBUGMSG( "# %s\n", __FUNCTION__);
do
   {
   // check parms
   pvdr = __getPointerFromHandle( hvt, SIG_VALUEMANAGER);
   if (!pvdr)
      // no object pointer for _wpSetError, so immediate return,
      return fResult;

   // do we have a hab ? if not, request one
   fHandleAnchorBlockRequested = __requestHandleAnchorBlock( &hab);
   if (!hab)
      {
      rc = ERROR_INVALID_FUNCTION;
      break;
      }

   // check parms
   if (!pvdr->pvdFirstValue)
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // now check all settings for the dialog id
   fResult = TRUE;
   for (pvd = pvdr->pvdFirstValue; ((pvd != NULL) && (fResult)); pvd = pvd->pvNextValue)
      {

      // add all values
      for (i = 0; ((i < pvd->psd->ulValueCount) && (fResult)); i++)
         {
         hwndControl = *(pvd->psd->pahwndControl + i);
         if ((WinIsWindow( hab, hwndControl)) && (WinIsWindowEnabled( hwndControl)))
            {
            fResult = __validateSettingsValue( ulDebugAPIValue, pvdr, pvd, *(pvd->psd->pahwndControl + i), NULL, i);
            if (!fResult)
               {
               // report the error to WPS class
               usControlid = WinQueryWindowUShort( hwndControl, QWS_ID);
               hwndDialog  = WinQueryWindow( hwndControl, QW_PARENT);
               usDialogid  = WinQueryWindowUShort( hwndDialog, QWS_ID);

               // determine the page of the control
               // NOTE: the notebook page id is not equal to the dialog id !
               ulPageId = (ULONG) WinSendMsg( hwndNotebook, BKM_QUERYPAGEID, 0, MPFROM2SHORT( BKA_FIRST, 0));
               while (ulPageId)
                  {
                  // is it the correct page
                  hwndPage = (HWND) WinSendMsg( hwndNotebook, BKM_QUERYPAGEWINDOWHWND, MPFROMLONG( ulPageId), 0);
                  if (hwndPage == hwndDialog)
                     break;

                  // check next page
                  ulPageId = (ULONG) WinSendMsg( hwndNotebook, BKM_QUERYPAGEID, MPFROMLONG( ulPageId), MPFROM2SHORT( BKA_NEXT, 0));
                  }

               // page found ? then turn to it
               if (ulPageId)
                  {
                  // select page
                  WinSendMsg( hwndNotebook, BKM_TURNTOPAGE, MPFROMLONG( ulPageId), 0);


                  // set focus to control
                  WinSetFocus( HWND_DESKTOP, hwndControl);
                  }

               DEBUGMSGCALLBACK( "#   >> callback: report error of control %u dialog %u\n", usControlid _c_ usDialogid);
               memset( &re, 0, sizeof( re));
               re.pvObjectInstance = pvdr->pvObjectInstance;
               re.ulSettingId      = pvd->psd->ulSettingId;
               re.pszName          = pvd->psd->pszName;
               re.usDialogid       = usDialogid;
               re.usControlid      = usControlid;
               re.hwndDialog       = hwndDialog;
               re.hwndControl      = hwndControl;
               fResult = (pvdr->psdr->pfnCallbackValue)( STM_CALLBACK_REPORTERROR, &re, pvdr->pvObjectInstance, pvdr->pvObjectData);
               DEBUGMSGCALLBACK( "#   >>  callback returning %u\n", fResult);
               if (!fResult)
                  {
                  rc = ERROR_INVALID_DATA;
                  break;
                  }
               }
            }

         } // for [all subvalues]

      } // for [all settings]

   } while (FALSE);

// clean up
if (fHandleAnchorBlockRequested) WinTerminate( hab);

fResult = (rc == NO_ERROR);
DEBUGMSG( "#   << fResult=%u rc=%u\n", fResult _c_ rc);
SETERROR( pvdr->pvObjectInstance, rc);
return fResult;

}


// ---------------------------------------------------------------------------

/*
@@WtkRelocateNotebookpageControls@SYNTAX
This function makes certain pushbuttons a notebook pushbutton
and relocates all other controls of a settings notebook page,
when the WPS class is being run under OS/2 WARP 4. This allows
to use the same dialog template under OS/2 WARP 3 and WARP 4.

@@WtkRelocateNotebookpageControls@PARM@hwndDialog@in
Dialog handle of the settings notebookpage.

@@WtkRelocateNotebookpageControls@RETURN
Success indicator.
:parml compact.
:pt.TRUE
:pd.Relocation successful or not OS/2 Warp 4 running.
:pt.FALSE
:pd.A validation error occurred.
:eparml.

@@WtkRelocateNotebookpageControls@REMARKS
This function first of all determines, if OS/2 WARP 4
is running. If not, it aborts with no error. Otherwise
:ul compact.
:li.the layout of all groupboxes is examined and
the lowest groupbox (nearest to y-coordinate 0) is determined.
:li.all pushbuttons below the lowest groupbox are made notebook buttons,
thus being moved out of the dialog.
:li.all other controls are moved downwards, so that the gap of
the now missing pusbuttons is closed.
:eul.

@@
*/

BOOL APIENTRY WtkRelocateNotebookpageControls( HWND hwndDialog)
{
         BOOL           fResult = FALSE;
         APIRET         rc = NO_ERROR;
         HENUM          henum = NULLHANDLE;
         HWND           hwndControl = NULLHANDLE;
         ULONG          ulWindowStyle;

         SWP            swp;
         ULONG          ulMinY = -1;
         ULONG          ulDelta;
         ULONG          ulX, ulY;

         PSZ            pszClassIndex;
         BOOL           fRelocationRequired = FALSE;
         BOOL           fRelocateControl;

do
   {
   // don't execute for WARP 3
   if (!WtkIsWarp4())
      {
      fResult = TRUE;
      break;
      }

   // check parms
   henum = WinBeginEnumWindows( hwndDialog);
   hwndControl = WinGetNextWindow( henum);
   if (hwndControl == NULLHANDLE)
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // check all pushbuttons
   while (hwndControl)
      {
      do
         {
         // check for lowest groupbox control
         pszClassIndex = WtkQueryClassIndex( hwndControl);
         ulWindowStyle = WinQueryWindowULong( hwndControl, QWL_STYLE) & BS_PRIMARYSTYLES;
         if ((pszClassIndex == WC_STATIC) && (ulWindowStyle == SS_GROUPBOX))
            {
            // determine lowest y position of groupbox control
            WinQueryWindowPos( hwndControl, &swp);
            ulDelta = swp.y;
            if (ulDelta < ulMinY)
               ulMinY = ulDelta;
            break;
            }

         // is it a pushbutton and not a notebook button ?
         ulWindowStyle = WinQueryWindowULong( hwndControl, QWL_STYLE) & BS_PRIMARYSTYLES;
         if (ulWindowStyle == BS_PUSHBUTTON)
            fRelocationRequired = TRUE;

         } while (FALSE);

      // next control please
      hwndControl = WinGetNextWindow( henum);

      }
   WinEndEnumWindows( henum);

   // do nothing ?
   if ((!fRelocationRequired) || (ulMinY == -1))
      break;

   // now make lower pushbuttons notebookbuttons
   // and relocate all other controls
   henum = WinBeginEnumWindows( hwndDialog);
   hwndControl = WinGetNextWindow( henum);

   while (hwndControl)
      {
      do
         {

         // check for pushbuttons here
         pszClassIndex = WtkQueryClassIndex( hwndControl);
         fRelocateControl = FALSE;
         WinQueryWindowPos( hwndControl, &swp);
         if (pszClassIndex == WC_BUTTON)
            {
            // is it a pushbutton at the bottom of the page ? then make it a notebook button
            ulWindowStyle = WinQueryWindowULong( hwndControl, QWL_STYLE) & BS_PRIMARYSTYLES;
            ulDelta = swp.y + swp.cy;

            if ((ulWindowStyle == BS_PUSHBUTTON) && (ulDelta < ulMinY))
               {
               WinSetWindowULong( hwndControl, QWL_STYLE,
                                  WinQueryWindowULong( hwndControl, QWL_STYLE) | BS_NOTEBOOKBUTTON);
               }
            else
               fRelocateControl = TRUE;
            }
         else
           fRelocateControl = TRUE;

         // relocate the control
         if (fRelocateControl)
            {
            // set the new position (move only !)
            WinQueryWindowPos( hwndControl, &swp);
            ulX = swp.x;
            ulY = swp.y - ulMinY + 20;
            WinSetWindowPos( hwndControl,  HWND_TOP, ulX, ulY, 0, 0, SWP_MOVE);

            // for some controls, e,g, for entryfields, PM "adjusts" the layout :-(
            // As this looks disorted, we check the new position
            WinQueryWindowPos( hwndControl, &swp);
            if (swp.y != ulY)
               {
               // PM has recalculated, so lets fool him by setting new values.
               // PM will again "correct" the new coordinates, resulting in those
               // values, that we wanted from the beginning
               swp.x += 2* (ulX - swp.x);
               swp.y += 2* (ulY - swp.y);
               WinSetWindowPos( hwndControl,  HWND_TOP, swp.x, swp.y, 0, 0, SWP_MOVE);
               }

            } // if (fRelocateControl)

         } while (FALSE);

      // next control please
      hwndControl = WinGetNextWindow( henum);

      }

   WinEndEnumWindows( henum);

   } while (FALSE);


// cleanup
if (henum) WinEndEnumWindows( henum);
fResult = (rc == NO_ERROR);
return fResult;

}

// ---------------------------------------------------------------------------

/*
@@WtkSaveObjectState@SYNTAX
This saves the current settings values of an object instance within the WPS repository.

@@WtkSaveObjectState@PARM@hvt@in
Valuetable handle.

@@WtkSaveObjectState@PARM@pszClass@in
Address of a unique ASCIIZ string.
:p.
The class name is recommended, but not enforced.

@@WtkSaveObjectState@RETURN
Success indicator.
:parml compact.
:pt.TRUE
:pd.Settings could be saved.
:pt.FALSE
:pd.A validation error occurred.
:eparml.

@@WtkSaveObjectState@REMARKS
This function is to be called during _wpSaveState to save all
current settings values handled by Settings and Details Manager.
:p.
You need to specify unique settings ids in order to have settings values
stored with this API.
:p.
The settings can be restored by using
:link reftype=hd refid=WtkRestoreObjectState.WtkRestoreObjectState.:elink..

@@
*/

BOOL APIENTRY WtkSaveObjectState( HVALUETABLE hvt, PSZ pszClass)
{
         BOOL              fResult = FALSE;
         APIRET            rc = NO_ERROR;
         ULONG             i,q;
         ULONG             ulDebugAPIValue = STM_DEBUG_API_SAVESTATE;
         PVALUEDATAROOT    pvdr = NULL;

         PVALUEDATA        pvd;

         ULONG             ulSettingId;
         CBQUERYVALUE      qv;
         CBREPORTSAVED     rs;

DEBUGMSG( "# %s\n", __FUNCTION__);
do
   {
   // check parms
   pvdr = __getPointerFromHandle( hvt, SIG_VALUEMANAGER);
   if (!pvdr)
      // no object pointer for _wpSetError, so immediate return,
      return fResult;
   if ((!pszClass) || (!*pszClass))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   DEBUGMSG( "#   >> class: %s\n", pszClass);

   // now save all values
   for (pvd = pvdr->pvdFirstValue; ((pvd != NULL)); pvd = pvd->pvNextValue)
      {
      // we can only save settings with an ID
      if (!pvd->psd->ulSettingId)
         {
         DEBUGMSG( "#   >> warning: cannot save setting %s, no ID\n", pvd->psd->pszName);
         continue;
         }

      // go through all queries
      for (q = 0; q < pvd->psd->ulQueryCount; q++)
         {
         // call back for indexed values if querycount greater 1
         if (pvd->psd->ulQueryCount > 0)
            {
            memset( &qv, 0, sizeof( qv));
            qv.pszName       = pvd->psd->pszName;
            qv.ulSettingId   = pvd->psd->ulSettingId;
            qv.ulQueryIndex  = q;
            fResult = (pvdr->psdr->pfnCallbackValue)( STM_CALLBACK_QUERYVALUE, &qv, pvdr->pvObjectInstance, pvdr->pvObjectData);
            }

         for (i = 0; i < pvd->psd->ulValueCount; i++)
            {
            // calculate current setting id
            ulSettingId = SETTINGSID( pvd->psd->ulSettingId, i + 1, q + 1);

            switch( *(pvd->psd->paulValueType))
               {
               case STM_VALUETYPE_STRING:
                  DEBUGMSG( "#   >> save %u(0x%04x): %s=%s\n",
                            ulSettingId _c_
                            ulSettingId _c_
                            pvd->psd->pszName _c_
                            *(pvd->papszValues));
                  _wpSaveString( pvdr->pvObjectInstance, pszClass,
                                 ulSettingId, *(pvd->papszValues));
                  break;

               case STM_VALUETYPE_INDEX:
               case STM_VALUETYPE_LONG:
               case STM_VALUETYPE_TRUEFALSE:
               case STM_VALUETYPE_YESNO:
               case STM_VALUETYPE_ONOFF:
               case STM_VALUETYPE_INDEXITEM:
                  DEBUGMSG( "#   >> save %u(0x%04x): %s=%u\n",
                            ulSettingId _c_
                            ulSettingId _c_
                            pvd->psd->pszName _c_
                            **(pvd->papszValues));

                  _wpSaveLong( pvdr->pvObjectInstance, pszClass,
                               ulSettingId, **(pvd->papszValues));
                  break;

               default:
                  DEBUGMSG( "#   >> warning: cannot save setting %s %u(0x%04x), invalid type %u\n",
                            pvd->psd->pszName _c_
                            ulSettingId _c_
                            ulSettingId _c_
                            *(pvd->psd->paulValueType));
                  break;
               }

            } // for [all subvalues]

         } // for [all queries

      } // for [all settings]

   // tell WPS class that we have saved data
   DEBUGMSGCALLBACK( "#   >> callback: save settings notification\n", 0);
   memset( &rs, 0, sizeof( rs));
   rs.fSaved = TRUE;
   fResult = (pvdr->psdr->pfnCallbackValue)( STM_CALLBACK_REPORTSAVED, &rs, pvdr->pvObjectInstance, pvdr->pvObjectData);

   } while (FALSE);

// clean up
fResult = (rc == NO_ERROR);
DEBUGMSG( "#   << fResult=%u rc=%u\n", fResult _c_ rc);
SETERROR( pvdr->pvObjectInstance, rc);
return fResult;

}

// ---------------------------------------------------------------------------

/*
@@WtkRestoreObjectState@SYNTAX
This function restores the settings of an object instance being
previously saved with
:link reftype=hd refid=WtkSaveObjectState.WtkSaveObjectState:elink..
Both the settings valuetable and currently active GUI controls
corresponding to the restored values are updated.

@@WtkRestoreObjectState@PARM@hvt@in
Valuetable handle.

@@WtkRestoreObjectState@PARM@pszClass@in
Address of a unique ASCIIZ string.
:p.
The class name is recommended, but not enforced.

@@WtkRestoreObjectState@RETURN
Success indicator.
:parml compact.
:pt.TRUE
:pd.Settings could be restored and updated.
:pt.FALSE
:pd.A validation error occurred.
:eparml.

@@WtkRestoreObjectState@REMARKS
This function is to be called during _wpRestoreState to restore all
current settings values handled by Settings and Details Manager.
:p.
You need to specify unique settings ids in order to have settings values
stored with this API.
:p.
This function restores the settings being previously saved with
:link reftype=hd refid=WtkSaveObjectState.WtkSaveObjectState:elink..

@@
*/

BOOL APIENTRY WtkRestoreObjectState( HVALUETABLE hvt, PSZ pszClass)
{
         BOOL              fResult = FALSE;
         APIRET            rc = NO_ERROR;
         ULONG             i,q;
         ULONG             ulDebugAPIValue = STM_DEBUG_API_RESTORESTATE;
         PVALUEDATAROOT    pvdr = NULL;

         PVALUEDATA        pvd;
         BOOL              fRestored;
         ULONG             ulValueLen;
         CHAR              szValue[ _MAX_PATH * 2];

         ULONG             ulSettingId;
         CBQUERYVALUE      qv;

DEBUGMSG( "# %s\n", __FUNCTION__);
do
   {
   // check parms
   pvdr = __getPointerFromHandle( hvt, SIG_VALUEMANAGER);
   if (!pvdr)
      // no object pointer for _wpSetError, so immediate return,
      return fResult;
   if ((!pszClass) || (!*pszClass))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   DEBUGMSG( "#   >> class: %s\n", pszClass);

   // restore settings here
   for (pvd = pvdr->pvdFirstValue; ((pvd != NULL)); pvd = pvd->pvNextValue)
      {
      // we can only save settings with an ID
      if (!pvd->psd->ulSettingId)
         {
         DEBUGMSG( "#   >> warning: cannot restore setting %s, no ID\n", pvd->psd->pszName);
         continue;
         }

      // go through all queries
      for (q = 0; q < pvd->psd->ulQueryCount; q++)
         {
         // call back for indexed values if querycount greater 1
         if (pvd->psd->ulQueryCount > 0)
            {
            memset( &qv, 0, sizeof( qv));
            qv.pszName       = pvd->psd->pszName;
            qv.ulSettingId   = pvd->psd->ulSettingId;
            qv.ulQueryIndex  = q;
            fResult = (pvdr->psdr->pfnCallbackValue)( STM_CALLBACK_QUERYVALUE, &qv, pvdr->pvObjectInstance, pvdr->pvObjectData);
            }

         for (i = 0; i < pvd->psd->ulValueCount; i++)
            {
            // calculate current setting id
            ulSettingId = SETTINGSID( pvd->psd->ulSettingId, i + 1, q + 1);

            switch( *(pvd->psd->paulValueType))
               {
               case STM_VALUETYPE_STRING:

                  ulValueLen = *(pvd->paulValueMaxLen);
                  fRestored = _wpRestoreString( pvdr->pvObjectInstance, pszClass,
                                                ulSettingId, *(pvd->papszValues), &ulValueLen);
                  DEBUGMSG( "#   >> %srestored %u(%u): %s=%s\n",
                            (fRestored) ? "" : "not " _c_
                            ulSettingId _c_ pvd->psd->ulSettingId _c_
                            pvd->psd->pszName _c_
                            *(pvd->papszValues));
                  break;

               case STM_VALUETYPE_INDEX:
               case STM_VALUETYPE_LONG:
               case STM_VALUETYPE_TRUEFALSE:
               case STM_VALUETYPE_YESNO:
               case STM_VALUETYPE_ONOFF:
               case STM_VALUETYPE_INDEXITEM:

                  DEBUGMSG( "#   >> current value %s==%u\n",
                            pvd->psd->pszName _c_
                            **(pvd->papszValues));

                  fRestored = _wpRestoreLong( pvdr->pvObjectInstance, pszClass,
                                              ulSettingId, (PULONG)*(pvd->papszValues));

                  DEBUGMSG( "#   >> %srestored %u(%u): %s==%u\n",
                            (fRestored) ? "" : "not " _c_
                            ulSettingId _c_ pvd->psd->ulSettingId _c_
                            pvd->psd->pszName _c_
                            **(pvd->papszValues));
                  break;

               default:
                  DEBUGMSG( "#   >> warning: cannot restore setting %s, invalid type %u\n", pvd->psd->pszName _c_ *(pvd->psd->paulValueType));
                  break;
               }

            } // for [all subvalues]

         } // for [all queries]


      } // for [all settings]

   } while (FALSE);

fResult = (rc == NO_ERROR);
DEBUGMSG( "#   << fResult=%u rc=%u\n", fResult _c_ rc);
SETERROR( pvdr->pvObjectInstance, rc);
return fResult;

}

// ---------------------------------------------------------------------------

/*
@@WtkSaveObjectSettings@SYNTAX
This saves the current settings of an object instance values externally in
an INI file of choice. Use
:link reftype=hd refid=WtkSaveObjectState.WtkSaveObjectState:elink. instead
to save the settings within the WPS repository as usual.

@@WtkSaveObjectSettings@PARM@hvt@in
Valuetable handle.

@@WtkSaveObjectSettings@PARM@pszFilename@in
Address of the ASCIIZ filename of the INI file to write the settings to.
:p.
The following names are reserved to write to the user or system ini&colon.
:parml tsize=10 break=none.
:pt.:hp2.Name:ehp2.
:pd.:hp2.INI file:ehp2.
:pt.USER
:pd.write to the user ini file using HINI_USER(PROFILE)
:pt.SYSTEM
:pd.write to the systemr ini file using HINI_SYSTEM(PROFILE)
:eparml.

@@WtkSaveObjectSettings@PARM@pszApp@in
Name of the INI application name.
:p.
:note.
This application name should identify the WPS class in order
not to mix up with settings of other WPS classes using this function,
if writing to the same INI file !

@@WtkSaveObjectSettings@PARM@pszKey@in
Name of the INI application key.
:p.
This parameter can be left NULL, then the persistent WPS object handle
is being used to generate a unique key id.
:note.
If the WPS class specifies a key, it is its responsability to make sure,
that the key is unique in order to not mix up with settings of another
object of the same class.

@@WtkSaveObjectSettings@RETURN
Success indicator.
:parml compact.
:pt.TRUE
:pd.Settings could be saved.
:pt.FALSE
:pd.A validation error occurred.
:eparml.

@@WtkSaveObjectSettings@REMARKS
This function is to be called during _wpSaveState, if the settings
are to be saved externally as a settings string.
:p.
While the settings of general WPS classes can only be stored in the OS/2
INI file, this function saves the object settings being handled by this
settings manager API in an INI file of choice.
:p.
The settings can be restored by using
:link reftype=hd refid=WtkRestoreObjectSettings.WtkRestoreObjectSettings.:elink..

@@
*/

BOOL APIENTRY WtkSaveObjectSettings( HVALUETABLE hvt, PSZ pszFilename, PSZ pszApp, PSZ pszKey)
{
         BOOL              fResult = FALSE;
         APIRET            rc = NO_ERROR;
         ULONG             ulDebugAPIValue = STM_DEBUG_API_SAVESETTINGS;

         PVALUEDATAROOT    pvdr = NULL;

         HAB               hab = NULLHANDLE;
         HINI              hini = NULLHANDLE;
         BOOL              fHandleAnchorBlockRequested = FALSE;

         PSZ               pszSettings = NULL;
         ULONG             ulSettingsLen;

         CHAR              szKey[ 20];

         CBREPORTSAVED     rs;

DEBUGMSG( "# %s\n", __FUNCTION__);
do
   {
   // check parms
   pvdr = __getPointerFromHandle( hvt, SIG_VALUEMANAGER);
   if (!pvdr)
      // no object pointer for _wpSetError, so immediate return,
      return fResult;

  if (!pszApp)
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // do we have a hab ? if not, request one
   fHandleAnchorBlockRequested = __requestHandleAnchorBlock( &hab);
   if (!hab)
      {
      rc = ERROR_INVALID_FUNCTION;
      break;
      }

   // check key
   if (!pszKey) pszKey = __ultohex( (LONG) QUERYHANDLE( pvdr->pvObjectInstance), szKey);

   // open the profile
   hini = __openProfile( hab, pszFilename);
   if (!hini)
      {
      rc = ERRORIDERROR( WinGetLastError( hab));
      break;
      }

   // query the settingslen and get memory
   if (!WtkQueryObjectSettings( hvt, NULL, &ulSettingsLen ))
      break;
   pszSettings = malloc( ulSettingsLen);
   if (!pszSettings)
      {
      rc = ERROR_NOT_ENOUGH_MEMORY;
      break;
      }

   // query the settings
   if (!WtkQueryObjectSettings( hvt, pszSettings, &ulSettingsLen ))
      break;

   // save the string
   if (!PrfWriteProfileString( hini, pszApp, pszKey, pszSettings))
      {
      rc = ERRORIDERROR( WinGetLastError( hab));
      break;
      }

   // tell WPS class that we have saved data
   DEBUGMSGCALLBACK( "#   >> callback: save settings notification\n", 0);
   memset( &rs, 0, sizeof( rs));
   rs.fSaved = TRUE;
   fResult = (pvdr->psdr->pfnCallbackValue)( STM_CALLBACK_REPORTSAVED, &rs, pvdr->pvObjectInstance, pvdr->pvObjectData);

   } while (FALSE);

// clean up
if (pszSettings) free( pszSettings);
if ((hini) && (hini != HINI_USER) && (hini != HINI_SYSTEM)) PrfCloseProfile( hini);
if (fHandleAnchorBlockRequested) WinTerminate( hab);

fResult = (rc == NO_ERROR);
DEBUGMSG( "#   << fResult=%u rc=%u\n", fResult _c_ rc);
SETERROR( pvdr->pvObjectInstance, rc);
return fResult;

}

// ---------------------------------------------------------------------------

/*
@@WtkRestoreObjectSettings@SYNTAX
This function restores the settings being previously saved with
:link reftype=hd refid=WtkSaveObjectSettings.WtkSaveObjectSettings.:elink..
Both the settings valuetable and currently active GUI controls
corresponding to the restored values are updated.

@@WtkRestoreObjectSettings@PARM@hvt@in
Valuetable handle.

@@WtkRestoreObjectSettings@PARM@pszFilename@in
Address of the ASCIIZ filename of the INI file to read the settings from.
:p.
The following names are reserved to read from the user or system ini&colon.
:parml tsize=10.
:pt.:hp2.Name:ehp2.
:pd.:hp2.INI file:ehp2.
:pt.USER
:pd.write to the user ini file using HINI_USER(PROFILE)
:pt.SYSTEM
:pd.write to the systemr ini file using HINI_SYSTEM(PROFILE)
:eparml.

@@WtkRestoreObjectSettings@PARM@pszApp@in
Name of the INI application name.
:p.
:note.
This application name should identify the WPS class in order
not to mix up with settings of other WPS classes using this function,
if writing to the same INI file !

@@WtkRestoreObjectSettings@PARM@pszKey@in
Name of the INI application key.
:p.
This parameter can be left NULL, then the persistent WPS object handle
is being used to generate a unique key id.
:note.
If the WPS class specifies a key, it is its responsability to make sure,
that the key is unique in order to not mix up with settings of another
object of the same class.

@@WtkRestoreObjectSettings@RETURN
Success indicator.
:parml compact.
:pt.TRUE
:pd.Settings could be restored and updated.
:pt.FALSE
:pd.A validation error occurred.
:eparml.

@@WtkRestoreObjectSettings@REMARKS
This function is to be called during _wpRestoreState, if the
settings have been saved externally before as a settings string.
:p.
This function restores the settings being previously saved with
:link reftype=hd refid=WtkSaveObjectSettings.WtkSaveObjectSettings.:elink..

@@
*/

BOOL APIENTRY WtkRestoreObjectSettings( HVALUETABLE hvt, PSZ pszFilename, PSZ pszApp, PSZ pszKey)
{
         BOOL              fResult = FALSE;
         APIRET            rc = NO_ERROR;
         ULONG             ulDebugAPIValue = STM_DEBUG_API_RESTORESETTINGS;
         PVALUEDATAROOT    pvdr = NULL;

         HAB               hab = NULLHANDLE;
         HINI              hini = NULLHANDLE;
         BOOL              fHandleAnchorBlockRequested = FALSE;

         PSZ               pszSettings = NULL;
         ULONG             ulSettingsLen;

         CHAR              szKey[ 20];

DEBUGMSG( "# %s\n", __FUNCTION__);
do
   {
   // check parms
   pvdr = __getPointerFromHandle( hvt, SIG_VALUEMANAGER);
   if (!pvdr)
      // no object pointer for _wpSetError, so immediate return,
      return fResult;

  if (!pszApp)
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // do we have a hab ? if not, request one
   fHandleAnchorBlockRequested = __requestHandleAnchorBlock( &hab);
   if (!hab)
      {
      rc = ERROR_INVALID_FUNCTION;
      break;
      }

   // check key
   if (!pszKey) pszKey = __ultohex( (LONG) QUERYHANDLE( pvdr->pvObjectInstance), szKey);

   // open the profile
   hini = __openProfile( hab, pszFilename);
   if (!hini)
      {
      rc = ERRORIDERROR( WinGetLastError( hab));
      break;
      }

   // query the settingslen and get memory
   if (!PrfQueryProfileSize( hini, pszApp, pszKey, &ulSettingsLen ))
      {
      rc = ERRORIDERROR( WinGetLastError( hab));
      break;
      }

   pszSettings = malloc( ulSettingsLen + 1);
   if (!pszSettings)
      {
      rc = ERROR_NOT_ENOUGH_MEMORY;
      break;
      }

   // query the settings
   if (!PrfQueryProfileString( hini, pszApp, pszKey, "", pszSettings, ulSettingsLen))
      {
      rc = ERRORIDERROR( WinGetLastError( hab));
      break;
      }

   // evaluate the string
   if (!WtkEvaluateObjectSettings( hvt, pszSettings))
      break;

   } while (FALSE);

// clean up
if (pszSettings) free( pszSettings);
if ((hini) && (hini != HINI_USER) && (hini != HINI_SYSTEM)) PrfCloseProfile( hini);
if (fHandleAnchorBlockRequested) WinTerminate( hab);

fResult = (rc == NO_ERROR);
DEBUGMSG( "#   << fResult=%u rc=%u\n", fResult _c_ rc);
SETERROR( pvdr->pvObjectInstance, rc);
return fResult;

}

// ---------------------------------------------------------------------------

/*
@@WtkQueryClassDetailsInfo@SYNTAX
This function is to be called by the override of the method _wpclsQueryDetailsInfo.

@@WtkQueryClassDetailsInfo@PARM@hst@in
Settingstable handle.

@@WtkQueryClassDetailsInfo@PARM@ppClassFieldInfo@in
Adress of the pointer variable to the details info.

@@WtkQueryClassDetailsInfo@PARM@pSize@in
Adress of the size variable of details data.

@@WtkQueryClassDetailsInfo@PARM@ulParentColumns@in
Number of parent columns being already inserted by the parent class.

@@WtkQueryClassDetailsInfo@RETURN
Number of total columns including the own ones.

@@WtkQueryClassDetailsInfo@REMARKS
This function can be called as follows during _wpclsQueryDetailsInfo
to automatically setup the details info&colon.
:xmp.
return WtkQueryClassDetailsInfo(
   _hst,
   (PVOID) ppClassFieldInfo,
   pSize,
   M_<wps_class>_parent_M_<wps_parent_class>_wpclsQueryDetailsInfo(
      somSelf,
      ppClassFieldInfo,
      pSize));
:exmp.

@@
*/

ULONG APIENTRY WtkQueryClassDetailsInfo( HSETTINGTABLE hst, PVOID *ppClassFieldInfo, PULONG pSize, ULONG ulParentColumns)
{
         ULONG             ulColumnsCount = ulParentColumns;
         APIRET            rc = NO_ERROR;
         ULONG             ulDebugAPIValue = STM_DEBUG_API_QUERYDETAILSINFO;

         ULONG             ulDetailsInfoLen;
         ULONG             i,x;

         ULONG             ulDetailsCount = 0;
         ULONG             ulDetailsAdded = 0;

         ULONG             ulFieldOffset = 0;
         ULONG             ulFieldType;
         PSZ               pszFieldType;
         ULONG             ulFieldFlags;
         ULONG             ulFieldLen;
         BOOL              fIndexedValue;
         PCLASSFIELDINFO   pcfi;


         PSETTINGSDATAROOT psdr = NULL;
         PSETTINGSDATA     psd = NULL;
         PSETTINGSDATA     psdNext = NULL;

DEBUGMSG( "# %s\n", __FUNCTION__);
do
   {
   // check parms
   psdr = __getPointerFromHandle( hst, SIG_SETTINGSMANAGER);
   if (!psdr)
      break;

   // determine, how muchdetails are to be displayed
   if (psdr->psdFirstSetting)
      {
      for (psd = psdr->psdFirstSetting; psd != NULL; psd = psd->pvNextSetting)
         {
         for (i = 0; i < psd->ulValueCount; i++)
            {
            if (*(psd->paszDetailsTitle + i))
               ulDetailsCount++;
            }
         }
      }
   if (!ulDetailsCount)
      break;


   DEBUGMSGDETAIL( "#   >> %u parent columns", ulParentColumns);
   if (ppClassFieldInfo)
      DEBUGMSGDETAIL( ", buffer at 0x%08x", *ppClassFieldInfo);
   if (pSize)
      DEBUGMSGDETAIL( ", size %u", *pSize);
   DEBUGMSGDETAIL( "\n",0);


   // get memory for detailsinfo
   ulDetailsInfoLen = ulDetailsCount * sizeof( CLASSFIELDINFO);

   // is addition of data required ?
   if (ppClassFieldInfo)
      {

      if (psdr->pcfi)
         {
         CLS_FREEMEMORY( psdr->pvObjectClass, psdr->pcfi);
         psdr->pcfi = NULL;
         }
      psdr->pcfi = CLS_ALLOCATEMEMORY( psdr->pvObjectClass, ulDetailsInfoLen, &rc);
      if (!psdr->pcfi)
         break;
      memset( psdr->pcfi, 0, ulDetailsInfoLen);

      // go thru all settings values
      for (psd = psdr->psdFirstSetting, pcfi = psdr->pcfi; psd != NULL; psd = psd->pvNextSetting)
         {
         for (i = 0; i < psd->ulValueCount; i++)
            {

            if (*(psd->paszDetailsTitle + i))
               {
               __getDetailsType( *(psd->paulValueType + i), &ulFieldType, &ulFieldFlags, &ulFieldLen, &pszFieldType, &fIndexedValue);

               pcfi->cb        = sizeof(CLASSFIELDINFO);
               pcfi->flData    = CFA_FIREADONLY;
               pcfi->flTitle   = CFA_CENTER | CFA_STRING | CFA_FITITLEREADONLY;
               pcfi->pNextFieldInfo = pcfi + 1;
               pcfi->pTitleData = (PVOID) *(psd->paszDetailsTitle + i);
               pcfi->flCompare  = SORTBY_SUPPORTED;

               pcfi->flData           |= ulFieldType | ulFieldFlags;
               pcfi->offFieldData      = ulFieldOffset;
               pcfi->ulLenFieldData    = ulFieldLen;
               pcfi->DefaultComparison = CMP_EQUAL;

               // add separator after this column
               ulDetailsAdded++;
               if (ulDetailsAdded != ulDetailsCount)
                  pcfi->flData |= CFA_SEPARATOR;
               else
                  pcfi->pNextFieldInfo = NULL;

               // count up offsets in details data
               ulFieldOffset += ulFieldLen;

               if (*psd->pszName)
                  DEBUGMSGDETAIL( "#   >> adding details info for %s:%u, type %s, flags %p, len %u, title \"%s\"\n",
                  psd->pszName                                _c_
                  i                                           _c_
                  pszFieldType                                _c_
                  ulFieldType                                 _c_
                  ulFieldLen                                  _c_
                  pcfi->pTitleData);
               else
                  DEBUGMSGDETAIL( "#   >> adding details info for %u, type %s, flags %p, len %u, title \"%s\"\n",
                  psd->ulSettingId                            _c_
                  pszFieldType                                _c_
                  ulFieldType                                 _c_
                  ulFieldLen                                  _c_
                  pcfi->pTitleData);

               // use next record
               pcfi++;
               }
            }
         }
      } // if (ppClassFieldInfo)
   else
      {
      // go thru all settings values
      for (psd = psdr->psdFirstSetting; psd != NULL; psd = psd->pvNextSetting)
         {
         for (i = 0; i < psd->ulValueCount; i++)
            {
            if (*(psd->paszDetailsTitle + i))
               {
               __getDetailsType( *(psd->paulValueType + i), &ulFieldType, &ulFieldFlags, &ulFieldLen, &pszFieldType, &fIndexedValue);

               // count up offsets in details data
               ulFieldOffset += ulFieldLen;

               if (*psd->pszName)
                  DEBUGMSGDETAIL( "#   >> reporting details info for %s:%u id %u(0x%04x), type %s, len %u\n",
                  psd->pszName       _c_
                  i                  _c_
                  psd->ulSettingId   _c_
                  psd->ulSettingId   _c_
                  pszFieldType       _c_
                  ulFieldLen);
               else
                  DEBUGMSGDETAIL( "#   >> reporting details info for detail %u(0x%04x), type %s, len %u\n",
                  psd->ulSettingId   _c_
                  psd->ulSettingId   _c_
                  pszFieldType       _c_
                  ulFieldLen);
               }
            }
         }
      }

   // is query only for size and columns count ?
   psdr->ulDetailsCount = ulDetailsCount;
   psdr->ulDetailsSize  = ulFieldOffset;
   ulColumnsCount += ulDetailsCount;
   if (pSize)
      *pSize += ulFieldOffset;

   // store ptr to own fieldinfo into last fieldinfo struc of parent
   if (ppClassFieldInfo)
      {
      if (*ppClassFieldInfo)
         {

         DEBUGMSGDETAIL( "#   << parent colums %u\n", ulParentColumns);
         pcfi = *ppClassFieldInfo;
         for (i = 0; i < ulParentColumns - 1; i++)
            {
            DEBUGMSGDETAIL( "#   << col %u at 0x%08x, next 0x%08x\n",
                            i _c_
                            pcfi _c_
                            pcfi->pNextFieldInfo);
            pcfi = pcfi->pNextFieldInfo;
            }

         DEBUGMSGDETAIL( "#   << col %u at 0x%08x, next 0x%08x \n",
                         i _c_
                         pcfi _c_
                         pcfi->pNextFieldInfo);
         DEBUGMSGDETAIL( "#   << adding own %u column info at 0x%08x to chain\n",
                         ulDetailsCount _c_
                         psdr->pcfi);

         pcfi->pNextFieldInfo = psdr->pcfi;
         }
      else
         *ppClassFieldInfo = psdr->pcfi;
      }

   } while (FALSE);

   DEBUGMSGDETAIL( "#   << %u total columns", ulColumnsCount);
   if (pSize)
      DEBUGMSGDETAIL( ", new size %u", *pSize);
   DEBUGMSGDETAIL( "\n",0);

return ulColumnsCount;
}

// ---------------------------------------------------------------------------

/*
@@WtkQueryObjectDetailsData@SYNTAX
This function is to be called by the override of the method _wpQueryDetailsData
to automatically setup the details data.

@@WtkQueryObjectDetailsData@PARM@hvt@in
Valuetable handle.

@@WtkQueryObjectDetailsData@PARM@ppDetailsData@in
Adress of the pointer variable to the details data.

@@WtkQueryObjectDetailsData@PARM@pcp@in
Adress of the size variable of details data.

@@WtkQueryObjectDetailsData@PARM@ulParentColumns@in
Number of parent columns being already inserted by the parent class.

@@WtkQueryObjectDetailsData@RETURN
Success indicator.
:parml compact.
:pt.TRUE
:pd.Successful completion.
:pt.FALSE
:pd.An error occurred.
:eparml.

@@WtkQueryObjectDetailsData@REMARKS
This function can be called as follows during _wpQueryDetailsData
to automatically setup the details data&colon.
:xmp.
<wps_class>_parent_<wps_parent_class>_wpQueryDetailsData(somSelf,
                                                         ppDetailsData,
                                                         pcp);

return WtkQueryObjectDetailsData( _hvt, ppDetailsData, pcp);
:exmp.

@@
*/
ULONG APIENTRY WtkQueryObjectDetailsData( HVALUETABLE hvt, PVOID* ppDetailsData, PULONG pcp)
{

         BOOL              fResult = FALSE;
         APIRET            rc = NO_ERROR;
         ULONG             ulDebugAPIValue = STM_DEBUG_API_QUERYDETAILSDATA;

         PVALUEDATAROOT    pvdr = NULL;
         PVALUEDATA        pvd = NULL;
         PVALUEDATA        pvdNext = NULL;

         ULONG             i;
         ULONG             ulFieldType;
         ULONG             ulFieldFlags;
         ULONG             ulFieldLen;
         PSZ               pszFieldType;
         BOOL              fIndexedValue;
         PSZ              *ppszDetailsData;

         ULONG             ulObjectDataSize = 0;
         ULONG             ulObjectDataItemSize;
         PSZ               pszDebugItemType;

         PSZ               pszObjectDetailsItem;
         PSZ               pszIndexData;

DEBUGMSG( "# %s\n", __FUNCTION__);
do
   {
   // check parms
   pvdr = __getPointerFromHandle( hvt, SIG_VALUEMANAGER);

   if (!pvdr)
      break;

   // no values available ?
   if ((!pvdr->pvdFirstValue) || (!pvdr->psdr->ulDetailsSize))
      break;

   DEBUGMSGDETAIL( "#   >>", 0);
   if (ppDetailsData)
      DEBUGMSGDETAIL( " buffer at %p,", *ppDetailsData);
   if (pcp)
      DEBUGMSGDETAIL( " size %u", *pcp);
   DEBUGMSGDETAIL( "\n",0);

   if (ppDetailsData)
      {
      DEBUGMSGDETAIL( "#   >> adding %u details of total size %u\n",
                      pvdr->psdr->ulDetailsCount _c_
                      pvdr->psdr->ulDetailsSize);

      // determine size of all object details data we need to provide in
      // separate memory
      ppszDetailsData = (PSZ*) *ppDetailsData;
      ulObjectDataSize = 0;
      for (pvd = pvdr->pvdFirstValue; pvd != NULL; pvd = pvd->pvNextValue)
         {
         for (i = 0; i < pvd->psd->ulValueCount; i++)
            {
            if (*(pvd->psd->paszDetailsTitle + i))
               {

               __getDetailsType( *(pvd->psd->paulValueType + i), &ulFieldType, &ulFieldFlags, &ulFieldLen, &pszFieldType, &fIndexedValue);

               // add maxlen of data to coutn var
               switch( ulFieldType)
                  {
                  case CFA_STRING:
                  case CFA_ULONG:

                     if (!fIndexedValue)
                        {
                        if (ulFieldType == CFA_STRING)
                           {
                           // add maxlen of string
                           ulObjectDataItemSize = *(pvd->paulValueMaxLen + i);
                           pszDebugItemType = "string";

                           }
                        else if (ulFieldType == CFA_ULONG)
                           {
                           ulObjectDataItemSize = sizeof( ULONG);
                           pszDebugItemType = "long value";
                           }
                        }
                     else
                        {
                        ulObjectDataItemSize = *(pvd->psd->paulDispStringMaxLen + i);
                        pszDebugItemType = "indexed details string";
                        }
                     break;

                  case CFA_DATE:
                  case CFA_TIME:
                     ulObjectDataItemSize = sizeof( ULONG);
                     pszDebugItemType = "time/date value";
                     break;


                  default:
                     DEBUGMSGDETAIL( " - invalid field type !!!\n",0);
                     break;
                  }

               if (*pvd->psd->pszName)
                  DEBUGMSGCALLBACK( "#   >> size of %s for setting %s:%u id %u(0x%04x) is %u\n",
                                    pszDebugItemType _c_
                                    pvd->psd->pszName _c_
                                    i                 _c_
                                    pvd->psd->ulSettingId _c_
                                    pvd->psd->ulSettingId _c_
                                    ulObjectDataItemSize);
               else
                  DEBUGMSGCALLBACK( "#   >> %s for detail id %u(0x%04x) is %u\n",
                                    pszDebugItemType _c_
                                    i                _c_
                                    pvd->psd->ulSettingId _c_
                                    pvd->psd->ulSettingId _c_
                                    ulObjectDataItemSize);

               // add item size
               DEBUGMSGCALLBACK( "#   >> increase total data size %u by %u\n", ulObjectDataSize _c_ ulObjectDataItemSize);

               ulObjectDataSize += ulObjectDataItemSize;

               } // if (*(pvd->psd->paszDetailsTitle + i))

            } // for (i = 0; i < pvd->psd->ulValueCount; i++)

         } // for (pvd = pvdr->pvdFirstValue; pvd != NULL; pvd = pvd->pvNextValue)

      // ------------------------------------------------------------------------------------------

      // get memory for details data (don't mix with details data pointed to by *ppDetailsData)
      DEBUGMSGDETAIL( "#   >> free previously allocated object details data at 0x%08x\n", pvdr->pszObjectDetailsItems);
      OBJ_FREEMEMORY( pvdr->pvObjectInstance, pvdr->pszObjectDetailsItems);

      pvdr->pszObjectDetailsItems = OBJ_ALLOCATEMEMORY( pvdr->pvObjectInstance, ulObjectDataSize, &rc);
      DEBUGMSGDETAIL( "#   >> allocate object details data at 0x%08x, size %u(0x%04x)\n",
                      pvdr->pszObjectDetailsItems _c_
                      ulObjectDataSize _c_
                      ulObjectDataSize);
      if (!pvdr->pszObjectDetailsItems)
         break;
      pszObjectDetailsItem = pvdr->pszObjectDetailsItems;

      // ------------------------------------------------------------------------------------------

      // go thru all settings values
      ppszDetailsData = (PSZ*) *ppDetailsData;
      for (pvd = pvdr->pvdFirstValue; pvd != NULL; pvd = pvd->pvNextValue)
         {
         for (i = 0; i < pvd->psd->ulValueCount; i++)
            {
            if (*(pvd->psd->paszDetailsTitle + i))
               {

               __getDetailsType( *(pvd->psd->paulValueType + i), &ulFieldType, &ulFieldFlags, &ulFieldLen, &pszFieldType, &fIndexedValue);

               if (*pvd->psd->pszName)
                  DEBUGMSGDETAIL( "#   >> adding details data at 0x%08x for setting %s:%u id %u(0x%04x), type %s, len %u",
                  ppszDetailsData              _c_
                  pvd->psd->pszName     _c_
                  i                     _c_
                  pvd->psd->ulSettingId _c_
                  pvd->psd->ulSettingId _c_
                  pszFieldType          _c_
                  ulFieldLen);
               else
                  DEBUGMSGDETAIL( "#   >> adding details data at 0x%08x for detail %u(0x%04x), type %s, len %u",
                  ppszDetailsData              _c_
                  pvd->psd->ulSettingId _c_
                  pvd->psd->ulSettingId _c_
                  pszFieldType          _c_
                  ulFieldLen);

               // add data to buffer

               switch( ulFieldType)
                  {
                  case CFA_STRING:
                  case CFA_ULONG:

                     if (!fIndexedValue)
                        {
                        if (ulFieldType == CFA_STRING)
                           {
                           // copy string to details data
                           strcpy( pszObjectDetailsItem, (PSZ) *(pvd->papszValues + i));
                           *ppszDetailsData = pszObjectDetailsItem;
                           DEBUGMSGDETAIL( ", points to 0x%08x, value is \"%s\"\n",
                                           *ppszDetailsData _c_
                                           *ppszDetailsData);

                           // adjust item pointer
                           pszObjectDetailsItem = NEXTSTR( pszObjectDetailsItem);
                           }
                        else if (ulFieldType == CFA_ULONG)
                           {
                           // provide ULONG value
                           memcpy( pszObjectDetailsItem, (PVOID) *(pvd->papszValues + i), ulFieldLen) ;
                           *ppszDetailsData = pszObjectDetailsItem;
                           DEBUGMSGDETAIL( ", points to 0x%08x, value is %u\n",
                                           *ppszDetailsData _c_
                                           *ppszDetailsData);

                           // adjust item pointer
                           pszObjectDetailsItem += ulFieldLen;
                           }
                        }
                     else
                        {
                                 CBQUERYDETAILSTRING qds;
                                 PULONG         pulIndex;
                                 ULONG          ulBuflen;
                                 ULONG          ulItemStrLen;
                                 BOOL           fResult;

                        // prepare buffer in details data for string
                        // and provide the pointer to it
                        pulIndex      = (PULONG)*(pvd->papszValues + i);

                        // ask the WPS class for translation of index to display string
                        DEBUGMSGCALLBACK( "\n", 0);
                        if (*pvd->psd->pszName)
                           DEBUGMSGCALLBACK( "#     >> callback: translate index to display string for setting %s:%u id %u(0x%04x), value is %u",
                                             pvd->psd->pszName _c_
                                             i                 _c_
                                             pvd->psd->ulSettingId _c_
                                             pvd->psd->ulSettingId _c_
                                             *pulIndex);
                        else
                           DEBUGMSGCALLBACK( "#     >> callback: translate index to display string for detail %u(0x%04x), value is %u",
                                             pvd->psd->ulSettingId _c_
                                             pvd->psd->ulSettingId _c_
                                             *pulIndex);

                        memset( &qds, 0, sizeof( qds));
                        qds.pvObjectInstance = pvdr->pvObjectInstance;
                        qds.pszName          = pvd->psd->pszName;
                        qds.ulSettingId      = pvd->psd->ulSettingId;
                        qds.ulStringIndex    = *pulIndex;
                        qds.ulValueIndex     = i;

                        ulBuflen = *(pvd->psd->paulDispStringMaxLen + i);
                        fResult = (pvdr->psdr->pfnCallbackValue)( STM_CALLBACK_QUERYDETAILSTRING, &qds, pvdr->pvObjectInstance, pvdr->pvObjectData);
                        if ((fResult) && (qds.pszValue) && (*qds.pszValue))
                           {
                           // store translated string
                           ulItemStrLen = MIN( strlen( qds.pszValue), ulBuflen - 1);
                           strncpy( pszObjectDetailsItem, qds.pszValue, ulItemStrLen);
                           *(pszObjectDetailsItem + ulItemStrLen) = 0;
                           }
                        else
                           {
                           // display error characters
                           memset( pszObjectDetailsItem, '?', ulBuflen - 1);
                           }

                        // copy string to details data
                        *ppszDetailsData = pszObjectDetailsItem;
                        DEBUGMSGDETAIL( ", indexed detail points to 0x%08x, value is \"%s\"\n",
                                        *ppszDetailsData _c_
                                        *ppszDetailsData);

                        // adjust item pointer
                        pszObjectDetailsItem = NEXTSTR( pszObjectDetailsItem);
                        }
                     break;

                  case CFA_DATE:
                  case CFA_TIME:
                     // provide ULONG value
                     memcpy( pszObjectDetailsItem, (PVOID) *(pvd->papszValues + i), ulFieldLen) ;
                     *ppszDetailsData = pszObjectDetailsItem;
                     DEBUGMSGDETAIL( ", timestamp value points to 0x%08x, value is %u\n",
                                     *ppszDetailsData _c_
                                     *ppszDetailsData);

                     // adjust item pointer
                     pszObjectDetailsItem += ulFieldLen;
                     break;


                  default:
                     DEBUGMSGDETAIL( " - invalid field type !!!\n",0);
                     break;
                  }

               // count up offsets in details data
               ppszDetailsData = (PSZ *) ((PBYTE)ppszDetailsData + ulFieldLen);
               }

            } // for (i = 0; i < pvd->psd->ulValueCount; i++)

         } // for (pvd = pvdr->pvdFirstValue; pvd != NULL; pvd = pvd->pvNextValue)


      } // if (ppDetailsData)
   else
      DEBUGMSGDETAIL( "#   >> reporting %u details of total size %u\n",
                      pvdr->psdr->ulDetailsCount _c_
                      pvdr->psdr->ulDetailsSize);

   // ------------------------------------------------------------------------------------------


//    pszObjectDetailsItem = pvdr->pszObjectDetailsItems;

   // ------------------------------------------------------------------------------------------

   // hand over results
   // - adjust size

   // adjust pointer
   if (ppDetailsData)
      {
      *ppDetailsData  = ((PBYTE) (*ppDetailsData)) + pvdr->psdr->ulDetailsSize;
      // make a last check of pointers
      if (!((PSZ) ppszDetailsData == (PSZ) *ppDetailsData))
         break;
      }
   // or size field
   else if (pcp)
      {
      DEBUGMSGDETAIL( "#   >> adding %u to size\n", pvdr->psdr->ulDetailsSize);
      *pcp += pvdr->psdr->ulDetailsSize;
      }

   } while (FALSE);

DEBUGMSGDETAIL( "#   << fResult=%u", fResult);
if (ppDetailsData)
   DEBUGMSGDETAIL( ", buffer now at %p", *ppDetailsData);
if (pcp)
   DEBUGMSGDETAIL( ", size %u", *pcp);
DEBUGMSGDETAIL( "\n",0);
return fResult;

}

// ---------------------------------------------------------------------------

/*
@@WtkQueryIconData@SYNTAX
This function is to be called by the override of the method _wpclsQueryIconData
or _wpQueryIconData to automatically setup the information for a
replacement icon being read from a resource DLL.

@@WtkQueryIconData@PARM@pIconInfo@in
Address to the icon information data.
:p.
This paramater may be NULL to only query the
size of the icon information.

@@WtkQueryIconData@PARM@hmodResource@in
Handle to the DLL holding the icon resource.

@@WtkQueryIconData@PARM@ulResId@in
Resource identifier for the icon.

@@WtkQueryIconData@RETURN
Size of the icon information returned.

@@WtkQueryIconData@REMARKS
This function can be called as follows during _wpclsQueryIconData
or _wpQueryIconData as follows to automatically setup the icon
data&colon.
:xmp.
return WtkQueryIconData( pIconInfo,
                         WtkGetModuleHandle( (PFN)__FUNCTION__),
                         IDRES_ICON);
:exmp.

@@
*/
ULONG APIENTRY WtkQueryIconData( PICONINFO pIconInfo, HMODULE hmodResource, ULONG ulResId)
{
         ULONG             ulDataLength = sizeof( ICONINFO);
         ULONG             ulDebugAPIValue = STM_DEBUG_API_QUERYICONDATA;
         APIRET            rc = NO_ERROR;

DEBUGMSG( "# %s\n", __FUNCTION__);
if (pIconInfo)
   {
   memset( pIconInfo, 0, ulDataLength);
   pIconInfo->cb      = ulDataLength;
   pIconInfo->fFormat = ICON_RESOURCE;
   pIconInfo->hmod    = hmodResource;
   pIconInfo->resid   = ulResId;
   DEBUGMSG( "#   >> reporting module 0x%08x resource 0x%04x\n", hmodResource _c_ ulResId);
   }

DEBUGMSG( "#   << data len=%u\n", ulDataLength _c_ rc);
return ulDataLength;
}

// ---------------------------------------------------------------------------

#ifdef DEBUG

static VOID __SetDebugInfoMask( VOID)
{

/** set mask here, when debugging this module **/
ulGlobalDebugMask =
//     /* global  */
//     STM_DEBUG_ALL                             |
//     STM_DEBUG_NONE                            |
//
//     /* mask for debug info details */
       STM_DEBUG_MASK_FUNC_DETAILS               |
//     STM_DEBUG_FUNC_ENTRYEXIT                  |
//     STM_DEBUG_FUNC_DETAILS                    |
//     STM_DEBUG_FUNC_MAIN                       | /* combination */
//     STM_DEBUG_FUNC_CALLBACKS                  |
//
//     /* mask for APIs concerning all provided APIs */
       STM_DEBUG_MASK_API                        |
//
//     /* masks for APIs concerning the metaclass part */
//     STM_DEBUG_MASK_API_METACLASS              |
//     STM_DEBUG_API_CREATECLASSSETTINGSTABLE    |
//     STM_DEBUG_API_DESTROYCLASSSETTINGSTABLE   |
//     STM_DEBUG_API_ADDCLASSSETTING             |
//     STM_DEBUG_API_ADDCLASSDETAIL              |
//     STM_DEBUG_API_CLOSECLASSSETTINGSTABLE     |
//     STM_DEBUG_API_QUERYDETAILSINFO            |
//     STM_DEBUG_API_QUERYICONDATA               |
//     STM_DEBUG_API_QUERYOBJECTCLASS            |
//
//     /* masks for APIs concerning the object instance part */
//     STM_DEBUG_MASK_API_OBJECTCLASS            |
//     STM_DEBUG_API_CREATEOBJECTVALUETABLE      |
//     STM_DEBUG_API_DESTROYOBJECTVALUETABLE     |
//     STM_DEBUG_API_EVALUATEOBJECTSETTINGS      |
//     STM_DEBUG_API_QUERYOBJECTSETTINGS         |
//     STM_DEBUG_API_QUERYGUICONTROLSCHANGED     |
//     STM_DEBUG_API_REGISTERSETTINGSDIALOG      |
//     STM_DEBUG_API_DEREGISTERSETTINGSDIALOG    |
//     STM_DEBUG_API_REGISTERSETTINGSNOTEBOOK    |
//     STM_DEBUG_API_VALIDATEVALUETABLE          |
//     STM_DEBUG_API_READVALUETABLE              |
//     STM_DEBUG_API_WRITEVALUETABLE             |
//     STM_DEBUG_API_SAVESETTINGS                |
//     STM_DEBUG_API_RESTORESETTINGS             |
//     STM_DEBUG_API_QUERYDETAILSDATA            |
//     STM_DEBUG_API_SAVESTATE                   |
//     STM_DEBUG_API_RESTORESTATE                |
//     STM_DEBUG_API_QUERYOBJECTINSTANCE         |
       0;
}

#endif
