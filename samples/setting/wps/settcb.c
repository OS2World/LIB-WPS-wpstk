/****************************** Module Header ******************************\
*
* Module Name: settcb.c
*
* callback source of test class of
* settings and details manager WPS sample
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: settcb.c,v 1.7 2005-10-17 22:29:23 cla Exp $
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

/* C Runtime */
#define EXTERN
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>


/* OS/2 Toolkit */
#define INCL_ERRORS
#define INCL_DOS
#define INCL_WIN
#define INCL_PM
#include <os2.h>

/* WPS Toolkit */
#define INCL_WTK
#include <wtk.h>

#include "const.h"
#include "settcb.h"
#include "settcls.ih"
#include "settdlg.rch"

// handle non standard extensions
#ifndef _fileno
#define _fileno fileno
#endif

// helper macros
#define EMPTYSTRING(p) (!p || (!*(PBYTE)p))
#define ENDOFSTRING(s) (s+strlen(s))
#define NEXTSTRING(s)  (s+strlen(s)+1)

static   PSZ            pszIpAddressErrorCaption = "Address validation error";
static   PSZ            pszIpAddressErrorMessage = "The specified IP address is not valid.";

static   PSZ            pszGlobalModemList = NULL;


// ------------------------------------------------------

static BOOL __isIpLabel( PSZ pszString)
{
         BOOL           fValid = TRUE;

         PSZ            p;
         ULONG          i;

         CHAR           szIpLabel[ 16];

do
   {
   // check Parms
   if ((pszString == NULL) ||
       (*pszString == 0)   ||
       (strlen( pszString) > sizeof( szIpLabel) - 1))
      {
      fValid = FALSE;
      break;
      }

   // divide into four bytes
   strcpy( szIpLabel, pszString);
   p = szIpLabel;
   i = 0;
   while ((*p != 0) && (fValid))
      {
      if (*p == '.')
         {
         // count and replace dot
         i++;
         *p = 0;
         }
      else
         fValid = (isdigit( *p) != 0);
      p++;
      }
   if (!fValid)
      break;

   // we need exactly four byte-values with three dots
   if (i != 3)
      {
      fValid = FALSE;
      break;
      }

   // check the byte values
   p = szIpLabel;
   for (i = 0; i < 4; i++, p = p + strlen( p) + 1)
      {
      if ((*p == 0) || (atol( p) > 255))
         {
         fValid = FALSE;
         break;
         }
      }

   } while (FALSE);

return fValid;
}

// -----------------------------------------------------------------------------

static MRESULT EXPENTRY __IpAddressSubclassProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)

{

         CHAR           szClassName[ 20];
         CLASSINFO      classinfo;

// query classinfo for procedure
WinQueryClassName( hwnd, sizeof( szClassName), szClassName);
WinQueryClassInfo( WinQueryAnchorBlock( hwnd), szClassName, &classinfo);


switch( msg )
   {

   case WM_CHAR:
      {
         USHORT fs = CHARMSG(&msg)->fs;
         CHAR   ch = CHARMSG(&msg)->chr;
         USHORT vbits = fs & (KC_ALT | KC_CTRL | KC_VIRTUALKEY);
         BOOL   fKeyIsUp = (fs & KC_KEYUP > 0);

      if (((ch == 32) && (vbits == KC_VIRTUALKEY)) ||  // detect blanks
          ((ch >  32) && (vbits == 0)))                // check all non control chars
         {
         // only digits and dots please
         if (((!isdigit( ch)) && ( ch != '.')) || (ch == 32))
            {
            // if not, make one sound ...
            if(!fKeyIsUp)
               WinAlarm( HWND_DESKTOP, WA_ERROR);
            // ... and skip key
            return (MRESULT) TRUE;
            }
         }
      }

   } // end switch

return classinfo.pfnWindowProc( hwnd, msg, mp1, mp2);

}

// -----------------------------------------------------------------------------

static VOID __readModemList( PSZ *ppszModemList)
{

         PSZ            pszEtc;
         CHAR           szModemFile[ _MAX_PATH];
         CHAR           szLine[ 1024];
         FILE          *hfModemList = NULL;
         PSZ            p;
static   PSZ            pszNoValue = "<none>";

         struct stat    statFile;
         PSZ            pszModem;
         ULONG          ulModemsAdded = 0;


do
   {
   // check parms
   if (!ppszModemList)
      break;

   // assemble filename
   pszEtc = getenv("ETC");
   if (!pszEtc)
      break;
   sprintf( szModemFile, "%s\\modem.lst", pszEtc);

   // open the file and determine filesize
   hfModemList = fopen( szModemFile, "r");
   if (!hfModemList)
      break;

   // determine filesize and allocate memory
   _fstat( _fileno( hfModemList), &statFile);
   *ppszModemList = malloc( statFile.st_size);
   if (!*ppszModemList)
      break;
   memset( *ppszModemList, 0, statFile.st_size);
   pszModem = *ppszModemList;
   strcpy( pszModem, pszNoValue);
   pszModem = NEXTSTRING( pszModem);

   // read all lines
   while (!feof( hfModemList))
      {
      // read line
      if (!fgets( szLine, sizeof( szLine), hfModemList))
         break;

      // skip comments
      if (szLine[0 ] == '*')
         continue;

      // cut at col 45 and cut of blanks
      szLine[ 44] = 0;
      p = &szLine[ 43];
      while (*p == 32)
         {
         *p = 0;
         p--;
         }

      // add this modem to the list
      strcpy( pszModem, szLine);
      pszModem = NEXTSTRING( pszModem);
      ulModemsAdded++;

      } // while (!feof( hfModemList))

// printf( ">>> %s: %u modems found\n", __FUNCTION__, ulModemsAdded);


   } while (FALSE);

// cleanup
if (hfModemList) fclose( hfModemList);
return;
}

// -----------------------------------------------------------------------------

static VOID __fillReadmeMle( HWND hwndDialog, USHORT usControlId) 
{
         BOOL           fResult = FALSE;
         APIRET         rc = NO_ERROR;
         HMODULE        hmod;

do
   {
   // determine handle of this DLL
   hmod = WtkGetModuleHandle( (PFN)__fillReadmeMle);
// printf( ">>> %s: hmod=%u\n", __FUNCTION__, hmod);
   if (!hmod)
      break;

   // load text into MLE
   fResult = WtkAddTextResourceToMLE( hwndDialog, usControlId, hmod, RT_USER_DATAFILE, IDRES_TEXT_WELCOME);
   if (!fResult)
      rc = ERRORIDERROR( WinGetLastError( WinQueryAnchorBlock( HWND_DESKTOP)));
// printf( ">>> %s: hwnd=%p id=%p result=%u rc=%u/%p\n", __FUNCTION__, hwndDialog, usControlId, fResult, rc, rc);

   } while (FALSE);

return;
}

// -----------------------------------------------------------------------------

static VOID __insertListIntoListbox( HWND hwndListbox, PSZ pszList)
{

         PSZ            pszModem;
         ULONG          ulModemsAdded = 0;
do
   {
   if ((!pszList) || (!*pszList))
      break;

   // scan thru list
   while (*pszList)
      {
      WinSendMsg( hwndListbox, LM_INSERTITEM, MPFROMSHORT(LIT_SORTASCENDING), MPFROMP( pszList));
      pszList = NEXTSTRING( pszList);
      ulModemsAdded++;
      }

// printf( ">>> %s: %u modems added to listbox\n", __FUNCTION__, ulModemsAdded);

   } while (FALSE);

// select first entry anyway
WinSendMsg( hwndListbox, LM_SELECTITEM, MPFROMSHORT( 0), MPFROMSHORT( TRUE));

return;
}

// -----------------------------------------------------------------------------

static BOOL  __isInList( PSZ pszString, PSZ pszList)
{
         BOOL           fResult = FALSE;
         PSZ            pszEntry;
         ULONG          ulEntryCount = 0;

do
   {
   // check parms
   if ((!pszString) || (!*pszString) || (!pszList) || (!*pszList))
      break;

   // scan thru list
   while (*pszList)
      {
      ulEntryCount++;
      if (!strcmp( pszString, pszList))
         {
         fResult = TRUE;
         break;
         }
      pszList = NEXTSTRING( pszList);
      }

   } while (FALSE);

return fResult;
}


// #############################################################################

BOOL SettingsCallbackProc( ULONG ulAction, PVOID pvData, PVOID pvObjectInstance, PVOID pvObjectData)
{

         BOOL           fResult = FALSE;
         SettingsClass     *somSelf = (SettingsClass*) pvObjectInstance;
         SettingsClassData *somThis = (SettingsClassData*) pvObjectData;

switch (ulAction)
   {

   // =================================

   case STM_CALLBACK_REPORTINIT:
      {
      // initialize callback resources
      __readModemList( &pszGlobalModemList);
      }
      break; // case STM_CALLBACK_REPORTINIT:

   // =================================

#define SETVALINFO(i,t,d,c,dt)          \
         case i:                        \
            pqvi->ulValueType = t;      \
            pqvi->usDialogid  = d;      \
            pqvi->usControlid = c;      \
            pqvi->pszDetailsTitle = dt; \
            fResult = TRUE;             \
            break;

   case STM_CALLBACK_QUERYVALUEINFO:

      {
                PCBQUERYVALUEINFO pqvi = pvData;

      // handle the details first, they have
      // setting ids

      switch (pqvi->ulSettingId)
         {

         // details
         SETVALINFO( IDDETAIL_LASTSAVED_DATE,  STM_VALUETYPE_CDATE,  IDDLG_UNUSED,  IDDLG_UNUSED, "Last saved date");
         SETVALINFO( IDDETAIL_LASTSAVED_TIME,  STM_VALUETYPE_CTIME,  IDDLG_UNUSED,  IDDLG_UNUSED, "Last saved time");

         // login properties
         SETVALINFO( IDSET_USERNAME,    STM_VALUETYPE_STRING, IDDLG_DLG_CONNECTION_LOGIN, IDDLG_EF_USERNAME,    "Username");
         SETVALINFO( IDSET_PASSWORD,    STM_VALUETYPE_STRING, IDDLG_DLG_CONNECTION_LOGIN, IDDLG_EF_PASSWORD,    NULL);
         SETVALINFO( IDSET_PHONENUMBER, STM_VALUETYPE_STRING, IDDLG_DLG_CONNECTION_LOGIN, IDDLG_EF_PHONENUMBER, "Phonenumber");

         // connection properties
         SETVALINFO( IDSET_IPCONFIG,     STM_VALUETYPE_INDEX,  IDDLG_DLG_CONNECTION_CONNECT, IDDLG_RB_CFGDYNAMIC,   NULL);
         SETVALINFO( IDSET_LOCALIP,      STM_VALUETYPE_STRING, IDDLG_DLG_CONNECTION_CONNECT, IDDLG_EF_LOCALIP,      NULL);
         SETVALINFO( IDSET_GATEWAY,      STM_VALUETYPE_STRING, IDDLG_DLG_CONNECTION_CONNECT, IDDLG_EF_GATEWAY,      NULL);
         SETVALINFO( IDSET_NETMASK,      STM_VALUETYPE_STRING, IDDLG_DLG_CONNECTION_CONNECT, IDDLG_EF_NETMASK,      NULL);
         SETVALINFO( IDSET_MTU,          STM_VALUETYPE_LONG,   IDDLG_DLG_CONNECTION_CONNECT, IDDLG_EF_MTU,          NULL);
         SETVALINFO( IDSET_NAMESERVER,   STM_VALUETYPE_STRING, IDDLG_DLG_CONNECTION_CONNECT, IDDLG_EF_NAMESERVER,   NULL);
         SETVALINFO( IDSET_DEFAULTROUTE, STM_VALUETYPE_YESNO,  IDDLG_DLG_CONNECTION_CONNECT, IDDLG_CB_DEFAULTROUTE, NULL);

         // modem property
         SETVALINFO( IDSET_MODEMTYPE,    STM_VALUETYPE_STRING, IDDLG_DLG_CONNECTION_MODEM,   IDDLG_CO_MODEMTYPE,    "Modem");
         }

      // extra switch to set subclass 
      // (didn't want to add that to the above macro...)
      switch (pqvi->ulSettingId)
         {
         case IDSET_LOCALIP:
         case IDSET_GATEWAY:
         case IDSET_NETMASK:
         case IDSET_NAMESERVER:
            pqvi->pfnwpSubclass = __IpAddressSubclassProc;
            break;
         }

      }
      break; // case STM_CALLBACK_QUERYVALUEINFO:

   // =================================

#define SETBUFINFO(i,v)                  \
         case i:                         \
            pqtb->ulBufMax = sizeof( v); \
            pqtb->pvTarget = &v;         \
            fResult = TRUE;              \
            break

   case STM_CALLBACK_QUERYTARGETBUF:

      {
                PCBQUERYTARGETBUF pqtb = pvData;


      switch (pqtb->ulSettingId)
         {

         // details
         SETBUFINFO( IDDETAIL_LASTSAVED_DATE, _cdateLastWritten);
         SETBUFINFO( IDDETAIL_LASTSAVED_TIME, _ctimeLastWritten);

         // login properties
         SETBUFINFO( IDSET_USERNAME,    _szUser);
         SETBUFINFO( IDSET_PASSWORD,    _szPassword);
         SETBUFINFO( IDSET_PHONENUMBER, _szPhoneNumber);


         // connection properties
         SETBUFINFO( IDSET_IPCONFIG,     _ulConfigType);
         SETBUFINFO( IDSET_LOCALIP,      _szLocalIp);
         SETBUFINFO( IDSET_GATEWAY,      _szGatewayIp);
         SETBUFINFO( IDSET_NETMASK,      _szNetmaskIp);
         SETBUFINFO( IDSET_MTU,          _ulMTU);
         SETBUFINFO( IDSET_NAMESERVER,   _szNameserver);
         SETBUFINFO( IDSET_DEFAULTROUTE, _fDefaultRoute);

         // modem property
         SETBUFINFO( IDSET_MODEMTYPE,    _szModemName);

         }

      }
      break; // case STM_CALLBACK_QUERYTARGETBUF:

   // =================================

   case STM_CALLBACK_VALIDATE:

      {
                PCBVALIDATE pv = pvData;

      // verify IP Labels
      switch (pv->ulSettingId)
         {
         case IDSET_LOCALIP:
         case IDSET_GATEWAY:
         case IDSET_NETMASK:
         case IDSET_NAMESERVER:
            pv->fResult = __isIpLabel( pv->pszValue);
            fResult = TRUE;
            break;

         case IDSET_MODEMTYPE:
            pv->fResult = __isInList( pv->pszValue, pszGlobalModemList);
            fResult = TRUE;
            break;
         }
      }

      break; // case STM_CALLBACK_VALIDATE:

   // =================================

   case STM_CALLBACK_QUERYINDEX:

      {
                PCBQUERYINDEX pqi = pvData;

      // return index values
      switch (pqi->ulSettingId)
         {
         case IDSET_IPCONFIG:
            if (!stricmp( pqi->pszValue, "DYNAMIC"))
               {
               pqi->ulStringIndex = 0;
               fResult = TRUE;
               }
            else if (!stricmp( pqi->pszValue, "STATIC"))
               {
               pqi->ulStringIndex = 1;
               fResult = TRUE;
               }
            break;
         }

      }

      break; // case STM_CALLBACK_QUERYINDEX:

   // =================================

   case STM_CALLBACK_QUERYSTRING:

      {
                PCBQUERYSTRING pqs = pvData;

      // return strings
      switch (pqs->ulSettingId)
         {
         case IDSET_IPCONFIG:
            fResult = TRUE;
            switch (pqs->ulStringIndex)
               {
               case 0:  strcpy( pqs->szValue, "DYNAMIC"); break;
               case 1:  strcpy( pqs->szValue, "STATIC");  break;
               default: fResult = FALSE;                  break;
               }
            break;
         }
      }

      break; // case STM_CALLBACK_QUERYSTRING:

   // =================================

   case STM_CALLBACK_REPORTCHANGED:
      {
                PCBREPORTCHANGED prch = pvData;
      }
      break; // case STM_CALLBACK_REPORTCHANGED:

   // =================================

   case STM_CALLBACK_QUERYVALUE:
      {
                PCBQUERYVALUE pqv = pvData;
      // no multiple queries
      }
      break; // case STM_CALLBACK_QUERYVALUE:

   // =================================

   case STM_CALLBACK_INITCONTROL:
      {
                PCBINITCONTROL pic = pvData;
                USHORT              usButtonId;

      // init controls
      // NOTE: for being able to use a simple switch here,
      //       the control ids MUST BE UNIQUE over all dialogs !!!
      switch (pic->usControlid)
         {
         case IDDLG_CO_MODEMTYPE:
            __insertListIntoListbox( pic->hwndControl, pszGlobalModemList);
            break;

         case (USHORT) -1:
            // this case will be called once for each dialog
            // it is used to initialize all other controls, which
            // are not serving for details or settings
            switch (pic->usDialogid)
               {
               case IDDLG_DLG_CONNECTION_WELCOME:
                   // initialize all other controls of the welcome dialog
                   __fillReadmeMle( pic->hwndDialog, IDDLG_MLE_README);
                  break;
               }

            break;
         }


      }
      break; // case STM_CALLBACK_INITCONTROL:

   // =================================

   case STM_CALLBACK_REPORTERROR:
      {
                PCBREPORTERROR pre = pvData;

      // NOTE: for being able to use a simple switch here,
      //       the control ids MUST BE UNIQUE over all dialogs !!!

      switch (pre->usControlid)
         {
         case (USHORT) IDDLG_EF_LOCALIP:
         case (USHORT) IDDLG_EF_GATEWAY:
         case (USHORT) IDDLG_EF_NAMESERVER:
            // tell the user what is wrong
            WinMessageBox( HWND_DESKTOP, pre->hwndDialog,
                           "The specified value is not a valid IP adress label.",
                           "Input Error!", 0, MB_CANCEL | MB_MOVEABLE | MB_ERROR);
            break;

         case (USHORT) IDDLG_EF_NETMASK:
            WinMessageBox( HWND_DESKTOP, pre->hwndDialog,
                           "The specified value is not a valid netmask.",
                           "Input Error!", 0, MB_CANCEL | MB_MOVEABLE | MB_ERROR);
            break;

         case (USHORT) IDDLG_CO_MODEMTYPE:
            // ignore this error
            fResult = TRUE;
            break;

         default:
            break;
         }
      }
      break; // case STM_CALLBACK_REPORTERROR:

   // =================================

   case STM_CALLBACK_REPORTSAVED:
      {
      // save last modification time
      WtkTimeToCDateTime( NULL, &_cdateLastWritten, &_ctimeLastWritten);
      }
      break; // case STM_CALLBACK_REPORTSAVED:

   // =================================

   case STM_CALLBACK_REPORTDESTROYED:
      {
      // cleanup callback resources
      if (pszGlobalModemList) free( pszGlobalModemList);
      }
      break; // case STM_CALLBACK_REPORTDESTROYED:

   }

return fResult;

}

