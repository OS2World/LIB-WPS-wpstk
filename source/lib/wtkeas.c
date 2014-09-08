/****************************** Module Header ******************************\
*
* Module Name: wtkueas.c
*
* Source for extended attributes manager functions.
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: wtkeas.c,v 1.8 2003-04-24 14:33:26 cla Exp $
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
* ===========================================================================
*
\***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INCL_DOS
#define INCL_ERRORS
#include <os2.h>

#include "wtkeas.h"
#include "wpstk.ih"

#define NEXTSTRING(s)    (s+strlen(s) + 1)
#define MAX(a,b)        (a > b ? a : b)
#define MIN(a,b)        (a < b ? a : b)

#define ODDPTR(p)      ((PVOID)((PBYTE) p + ((ULONG)p % 2)))

// define internal magig EA handle for creating an empty handle
#define HEA_EMPTY      ((USHORT) -2)

// define pointer to EA type
typedef USHORT  EATYPE;
typedef PUSHORT PEATYPE;

// define calue for end-of-list, used in
#define NO_MORE_EAVALUES (ULONG) -1

#pragma pack(1)

// define struct for value data
typedef struct _EAVALUE
   {
         USHORT         usEntryLen;
         CHAR           chEntry[1];
   } EAVALUE, *PEAVALUE;


// define struct for SVST header inlusive length field and address of data
//   EAT_ASCII   000A Hello John
//   EAT_BINARY  0003 0x12 0x21 0x34
typedef struct _EASVST
   {
         USHORT         usEaType;
         EAVALUE        eavalue;
   } EASVST, *PEASVST;


// define struct for MVMT header
//   EAT_MVMT  0000  0002  EAT_ASCII   000A Hello John
//                         EAT_BINARY  0003 0x12 0x21 0x34
typedef struct _EAMVMT
   {
         USHORT         usHeaderType;
         USHORT         usCodepage;
         USHORT         usEntries;
         EASVST         easvstList[ 1];
   } EAMVMT, *PEAMVMT;

//   EAT_MVST 0000 0003 EAT_ASCII 0004 Mark
//                                0005 Ellen
//                                0003 Liz
typedef struct _EAMVST
   {
         USHORT         usHeaderType;
         USHORT         usCodepage;
         USHORT         usEntries;
         USHORT         usEaType;
         EAVALUE        eavalueList[1];
   } EAMVST, *PEAMVST;


// ------------------------------------

// structure for storing EAs of a file node
// a mangled pointer to this memory is returned
// as handle to EA
typedef struct _EAINDEX
   {
         ULONG          ulSig;
         CHAR           szNode[ _MAX_PATH];
         CHAR           szEaName[ _MAX_PATH];
         BOOL           fSearchStarted;
         ULONG          ulEaType;
         ULONG          ulIndex;
         PFEA2LIST      pfea2l;
         PEATYPE        peat;
   } EAINDEX, *PEAINDEX;



// memory sig for EA data
#define SIG_EAMANAGER 0x4145   /* 'AE' -  stored like 'EA' on intel ;-) */

// ===========================================================================

// macros for converting pointers to handles
#define PTR2HANDLE(p,sig)         ((ULONG)p^(sig << 16))
#define HANDLE2PTR(h,sig)         ((PVOID)(h^(sig << 16)))

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
static BOOL __isEaTypeValid( ULONG ulEaType, BOOL fAllowWildcard)
{
switch (ulEaType)
   {
   case EAT_ASN1:
   case EAT_EA:
   case EAT_BINARY:
   case EAT_ASCII:
   case EAT_BITMAP:
   case EAT_METAFILE:
   case EAT_ICON:
      return TRUE;

   case WTK_EAT_ANY:
      return fAllowWildcard;

   default:
      return FALSE;
   }
}

// ---------------------------------------------------------------------------
static BOOL __isEaTypeMultiType( ULONG ulEaType)
{
switch (ulEaType)
   {
   case EAT_MVMT:
   case EAT_MVST:
      return TRUE;

   default:
      return FALSE;
   }
}

// ---------------------------------------------------------------------------

static ULONG __getEaDataLen( PEATYPE peatype)
{
         ULONG          ulEaDataLen = 0;
         ULONG          i;
         PEAMVMT        peamvmt;
         PEAMVST        peamvst;
         PEASVST        peasvst;
         PEAVALUE       peavalue;

do
   {
   switch (*peatype)
      {
      // unsupported type
      case EAT_ASN1:
      default:
         break;

      // -------------------------

      // handle single value single type type EAs
      // should normally not be used !!!
      case EAT_EA:
      case EAT_BINARY:
      case EAT_ASCII:
      case EAT_BITMAP:
      case EAT_METAFILE:
      case EAT_ICON:
            peasvst = (PEASVST) peatype;
            peavalue = &peasvst->eavalue;
            ulEaDataLen = sizeof( EASVST) - 1 + peasvst->eavalue.usEntryLen;
         break;

      // -------------------------

      case EAT_MVMT:
         //   EAT_MVMT  0000  0002  EAT_ASCII   000A Hello John
         //                         EAT_BINARY  0003 0x12 0x21 0x34

         // address current value
         peamvmt = (PEAMVMT) peatype;
         peavalue = NULL;
         for (i = 0, peasvst = (PEASVST) &peamvmt->easvstList[ 0]; i < peamvmt->usEntries; i++)
            {
            // address current value
            peavalue = &peasvst->eavalue;

            // address next SVST ea
            peasvst = (PEASVST) ((PBYTE) peasvst + sizeof( EASVST) - 1 + peavalue->usEntryLen);
            }

         ulEaDataLen = (PBYTE) peasvst - (PBYTE) peamvmt;
         break;

      // -------------------------

      case EAT_MVST:
         //   EAT_MVST 0000 0003 EAT_ASCII 0004 Mark
         //                                0005 Ellen
         //                                0003 Liz

         // address current value
         peamvst = (PEAMVST) peatype;
         for (i = 0, peavalue = (PEAVALUE) &peamvst->eavalueList[0]; i < peamvst->usEntries; i++)
            {
            // address next SVST EA
             peavalue = (PEAVALUE) ((PBYTE) peavalue + sizeof( EAVALUE) - 1 + peavalue->usEntryLen);
            }
         ulEaDataLen = (PBYTE) peavalue - (PBYTE) peamvst;
         break;

      } // end switch

   } while (FALSE);

return ulEaDataLen;

}

// ---------------------------------------------------------------------------

static APIRET __getEaFromDisk( PSZ pszName, PSZ pszEaName, PFEA2LIST  *ppfea2l,  PULONG pulEaDataLen)
{
         APIRET         rc     = NO_ERROR;
         FILESTATUS4    fs4;

         EAOP2          eaop2;
         PGEA2LIST      pgea2l = NULL;
         PFEA2LIST      pfea2l = NULL;

         PGEA2          pgea2;
         PFEA2          pfea2;
         ULONG          ulGea2Len = 0;
         ULONG          ulFea2Len = 0;
         PEATYPE        peatype;

do
   {
   // check parameters
   if ((!pszName)      ||
       (!pszEaName)    ||
       (!ppfea2l))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }


   // get EA size
   rc = DosQueryPathInfo( pszName,
                          FIL_QUERYEASIZE,
                          &fs4,
                          sizeof( fs4));
   if (rc != NO_ERROR)
      break;

   // no eas here ?
   if (fs4.cbList == 0)
      {
      rc = ERROR_NO_MORE_FILES;
      break;
      }

   // determine required space
   // - for ulFea2Len use at least 2 * Gea2Len,
   //   see docs for DosQueryPathInfo parameter pInfoBuf
   ulGea2Len = sizeof( GEA2LIST) + strlen( pszEaName);
   ulFea2Len = 2 * MAX(fs4.cbList, ulGea2Len);

   // get memory for GEA2LIST telling what ea we want
   if ((pgea2l = malloc(  ulGea2Len)) == 0)
      {
      rc = ERROR_NOT_ENOUGH_MEMORY;
      break;
      }
   memset( pgea2l, 0, ulGea2Len);

   // get memory for FEA2LIST reporting the value(s)
   if ((pfea2l = malloc( ulFea2Len)) == 0)
      {
      rc = ERROR_NOT_ENOUGH_MEMORY;
      break;
      }
   memset( pfea2l, 0, ulFea2Len);

   // init ptrs and do the query
   memset( &eaop2, 0, sizeof( EAOP2));
   eaop2.fpGEA2List = pgea2l;
   eaop2.fpFEA2List = pfea2l;
   pfea2l->cbList = ulFea2Len;
   pgea2l->cbList = ulGea2Len;

   pgea2 = &pgea2l->list[ 0];
   pfea2 = &pfea2l->list[ 0];

   // setting up get ea list struct with name we are looking for
   pgea2->oNextEntryOffset  = 0;
   pgea2->cbName            = strlen( pszEaName);
   strcpy( pgea2->szName, pszEaName);

   rc = DosQueryPathInfo( pszName,
                          FIL_QUERYEASFROMLIST,
                          &eaop2,
                          sizeof( eaop2));
   if (rc != NO_ERROR)
      break;

   // hand over result
   *ppfea2l      = pfea2l;
   peatype       = (PEATYPE)((PBYTE) pfea2->szName + pfea2->cbName + 1);
   if (pulEaDataLen)
      *pulEaDataLen = __getEaDataLen( peatype);

   } while (FALSE);

// cleanup
if (pgea2l) free( pgea2l);
if (rc != NO_ERROR)
   if (pfea2l) free( pfea2l);

return rc;

}

// ---------------------------------------------------------------------------

static APIRET __getEaValueFromList( PEAINDEX peai, PBYTE *ppbData, PULONG pulDatalen, PULONG pulEaType)
{
         APIRET         rc     = NO_ERROR;
         ULONG          i;

         PEATYPE        peatype;

         PEAMVMT        peamvmt;
         PEAMVST        peamvst;
         PEASVST        peasvst;
         PEAVALUE       peavalue;

         USHORT         usEntriesMatched;
         ULONG          ulDatalen;
         PBYTE          pbData;

         BOOL           fAnyType;

do
   {
   // check parms
   if ((!peai)       ||
       (!ppbData)    ||
       (!pulDatalen))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // init flag
   fAnyType = (peai->ulEaType == WTK_EAT_ANY);

   // check for pointer to type field
   peatype = peai->peat;
   if (!peatype)
      {
      rc = ERROR_INVALID_DATA;
      break;
      }

   // get value
   switch (*peatype)
      {

      // EA is empty or unsupported type
      case EAT_ASN1:
      default:
         peavalue      = NULL;
         peai->ulIndex = NO_MORE_EAVALUES;
         break;

      // -------------------------

      // hande single value single type type EAs
      // should normally not be used !!!
      case EAT_BINARY:
      case EAT_ASCII:
      case EAT_BITMAP:
      case EAT_METAFILE:
      case EAT_ICON:
      case EAT_EA:
         if (!peai->ulIndex)
            {
            if ((fAnyType) || (*peai->peat == peai->ulEaType))
               {
               peasvst    = (PEASVST) peai->peat;
               peavalue   = &peasvst->eavalue;
               ulDatalen  = peavalue->usEntryLen;
               pbData     = &peavalue->chEntry[ 0];
               *pulEaType = peasvst->usEaType;
               }

            peai->ulIndex++;
            }
         else
            peai->ulIndex = NO_MORE_EAVALUES;
         break;

      // -------------------------

      case EAT_MVMT:
         // define struct for MVMT header
         //   EAT_MVMT  0000  0002  EAT_ASCII   000A Hello John
         //                         EAT_BINARY  0003 0x12 0x21 0x34

         // have we already read all values ?
         peamvmt = (PEAMVMT) peatype;
         if (peai->ulIndex > (ULONG) peamvmt->usEntries)
            {
            peai->ulIndex = NO_MORE_EAVALUES;
            break;
            }

         // address current value
         peavalue         = NULL;
         usEntriesMatched = 0;
         for (i = 0, peasvst = (PEASVST) &peamvmt->easvstList[ 0]; i < peamvmt->usEntries; i++)
            {
            // address current value
            peavalue = &peasvst->eavalue;

            // does entry match the requested type ?
            if ((fAnyType) || (peai->ulEaType == peasvst->usEaType))
               usEntriesMatched++;

            // did we reach the right entry ?
            if (usEntriesMatched > peai->ulIndex)
               break;

            // address next SVST ea
            peasvst = (PEASVST) ((PBYTE) peasvst + sizeof( EASVST) - 1 + peavalue->usEntryLen);
            }

         // could we find enough matching values ?
         if (usEntriesMatched > peai->ulIndex)
            {
            ulDatalen  = peavalue->usEntryLen;
            pbData     = &peavalue->chEntry[ 0];
            *pulEaType = peasvst->usEaType;

            // increment index, so that next call reads next value
            peai->ulIndex++;

            }
         else
            peai->ulIndex = NO_MORE_EAVALUES;

         break;

      // -------------------------

      case EAT_MVST:
         //   EAT_MVST 0000 0003 EAT_ASCII 0004 Mark
         //                                0005 Ellen
         //                                0003 Liz


         // have we already read all values ?
         peamvst = (PEAMVST) peatype;
         if (peai->ulIndex >= (ULONG) peamvst->usEntries)
            {
            peai->ulIndex = NO_MORE_EAVALUES;
            break;
            }

         // does entry match the requested type ?
         if ((!fAnyType) && (peai->ulEaType != peamvst->usEaType))
            {
            peai->ulIndex = NO_MORE_EAVALUES;
            break;
            }

         // address current value
         for (i = 0, peavalue = (PEAVALUE) &peamvst->eavalueList[0]; i < peai->ulIndex; i++)
            {
            // address next SVST EA
             peavalue = (PEAVALUE) ((PBYTE) peavalue + sizeof( EAVALUE) - 1 + peavalue->usEntryLen);
            }

         ulDatalen  = peavalue->usEntryLen;
         pbData     = &peavalue->chEntry[ 0];
         *pulEaType = peamvst->usEaType;

         // increment index, so that next call reads next value
         peai->ulIndex++;

         break;

      } // end switch

   // hand over result
   *ppbData    = pbData;
   *pulDatalen = ulDatalen;

   } while (FALSE);

return rc;

}

// ===========================================================================

/*
@@WtkQueryEaSize@SYNTAX
This function determines the size of a given
extended attribute of a file or directory.

@@WtkQueryEaSize@PARM@pszName@in
Address of the ASCIIZ path name of the file or directory.
:p.
The path name may contain :hp2.?&colon.:ehp2. for the drive in order to
open the file or directory on the boot drive.
:p.
The name may not include wildcards.

@@WtkQueryEaSize@PARM@pszEaName@in
Address of the ASCIIZ name of the extended attribute.

@@WtkQueryEaSize@PARM@pulSize@out
The address of a variable containing the length of the extended attribute.

@@WtkQueryEaSize@RETURN
Return Code.
:p.
WtkQueryEaSize returns one of the following return codes&colon.

:parml compact break=none.
:pt.0
:pd.NO_ERROR
:pt.87
:pd.ERROR_INVALID_PARAMETER
:eparml.
:p.
or return codes of the following functions
:ul compact.
:li.DosQueryPathInfo
:eul.

@@WtkQueryEaSize@REMARKS
This function supports boot drive recognition.
Specify :hp2.?&colon.:ehp2. for the drive in order to
open the file or directory on the boot drive.

@@
*/

APIRET APIENTRY WtkQueryEaSize( PSZ pszName, PSZ pszEaName, PULONG pulSize)
{

         APIRET         rc     = NO_ERROR;

         CHAR           szName[ _MAX_PATH];
         PFEA2LIST      pfea2l = NULL;
         PFEA2          pfea2;
         PEAINDEX       peai   = NULL;
         ULONG          ulDatalen = 0;


do
   {
   // check parameters
   if ((!pszName)    ||
       (!pszEaName)  ||
       (!pulSize))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // copy name and replace ?: with bootdrive
   strcpy( szName, pszName);
   __PatchBootDrive( szName);

   // get current EAs
   rc = __getEaFromDisk( szName, pszEaName, &pfea2l, &ulDatalen);
   if (rc != NO_ERROR)
      break;

   // hand over result
   *pulSize = ulDatalen;

   } while (FALSE);

// cleanup
if (pfea2l) free( pfea2l);

return rc;

}

// ---------------------------------------------------------------------------

/*
@@WtkReadEa@SYNTAX
This function reads an extended attribute of a given file or directory.

@@WtkReadEa@PARM@pszName@in
Address of the ASCIIZ path name of the file or directory.
:p.
The path name may contain :hp2.?&colon.:ehp2. for the drive in order to
open the file or directory on the boot drive.
:p.
The name may not include wildcards.

@@WtkReadEa@PARM@pszEaName@in
Address of the ASCIIZ name of the extended attribute.

@@WtkReadEa@PARM@phea@out
The address of a variable containing the handle to the extended attribute

@@WtkReadEa@RETURN
Return Code.
:p.
WtkReadEa returns one of the following return codes&colon.

:parml compact break=none.
:pt.0
:pd.NO_ERROR
:pt.8
:pd.ERROR_NOT_ENOUGH_MEMORY
:pt.87
:pd.ERROR_INVALID_PARAMETER
:eparml.
:p.
or return codes of the following functions
:ul compact.
:li.DosQueryPathInfo
:eul.

@@WtkReadEa@REMARKS
This function allocates resources for handling the value of the
given extended attribute, which must be freeed by calling WtkCloseEa,
when not longer needed.
:p.
This function supports boot drive recognition.
Specify :hp2.?&colon.:ehp2. for the drive in order to
open the file or directory on the boot drive.

@@
*/

APIRET APIENTRY WtkReadEa( PSZ pszName, PSZ pszEaName, PHEA phea)
{

         APIRET         rc     = NO_ERROR;

         CHAR           szName[ _MAX_PATH];
         PFEA2LIST      pfea2l = NULL;
         PFEA2          pfea2;
         PEAINDEX       peai = NULL;
         ULONG          ulBuflen;

do
   {
   // check parameters
   if ((!pszName)    ||
       (!pszEaName)  ||
       (!phea))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // copy name and replace ?: with bootdrive
   strcpy( szName, pszName);
   __PatchBootDrive( szName);

   // --------------------------------------------------------

   if (*phea == HEA_EMPTY)
      {
      // initialize empty list
      ulBuflen = sizeof( FEA2LIST) + strlen( pszEaName) + 3;
      pfea2l = malloc( ulBuflen);
      if (!pfea2l)
         {
         rc = ERROR_NOT_ENOUGH_MEMORY;
         break;
         }
      memset( pfea2l, 0, ulBuflen);
      pfea2l->cbList = ulBuflen;
      pfea2 = &pfea2l->list[ 0];
      pfea2->cbName = strlen( pszEaName);
      strcpy( pfea2->szName, pszEaName);
      }
   else
      {
      // get current EAs
      rc = __getEaFromDisk( pszName, pszEaName, &pfea2l, NULL);
      if (rc != NO_ERROR)
         break;
      }

   // initialize internal memory
   ulBuflen =  sizeof( EAINDEX);
   peai = malloc( ulBuflen);
   if (!peai)
      {
      rc = ERROR_NOT_ENOUGH_MEMORY;
      break;
      }

   memset( peai, 0, ulBuflen);
   peai->ulSig     = SIG_EAMANAGER;
   DosQueryPathInfo( szName, FIL_QUERYFULLNAME, peai->szNode, sizeof( peai->szNode));
   strcpy( peai->szEaName, pszEaName);
   peai->ulIndex   = 0;
   peai->pfea2l    = pfea2l;

   pfea2           = &pfea2l->list[0];
   peai->peat      = (PEATYPE)((PBYTE) pfea2->szName + pfea2->cbName + 1);

   // return mangled pointer
   *phea = PTR2HANDLE( peai, SIG_EAMANAGER);

   } while (FALSE);

// cleanup
if (rc != NO_ERROR)
   if (pfea2l) free( pfea2l);

return rc;

}

// ---------------------------------------------------------------------------

/*
@@WtkFindFirstEaValue@SYNTAX
This function scans a extended attribute previously read by
WtkReadEa for the first value of a given type.

@@WtkFindFirstEaValue@PARM@hea@in
Handle to the extended attribute.

@@WtkFindFirstEaValue@PARM@pulEaType@inout
The address of a variable containing the type of the searched/found
extended attribute.
:p.
All subsequent searches with WtkFindNextEaValue
will use this type value. Specify WTK_EAT_ANY on input
to search for values of any type.

@@WtkFindFirstEaValue@PARM@pszValue@out
The address of a buffer in into which the found value
is returned.

@@WtkFindFirstEaValue@PARM@pulBuflen@inout
The address of a variable containing the length, in bytes, of the buffer
described by :hp1.pszValue:ehp1..
:p.
On return, this variable contains the length
of the returned value.

@@WtkFindFirstEaValue@RETURN
Return Code.
:p.
WtkFindFirstEaValue returns one of the following return codes&colon.

:parml compact break=none.
:pt.0
:pd.NO_ERROR
:pt.6
:pd.ERROR_INVALID_HANDLE
:pt.18
:pd.ERROR_NO_MORE_FILES
:pt.111
:pd.ERROR_BUFFER_OVERFLOW
:eparml.

@@WtkFindFirstEaValue@REMARKS
Unlike DosFindFirst, this API does not allocate resources, that must be freed.
The resources for handling the value of the extended attribute are rather allocated
by WtkReadEa, which must be freeed by calling WtkCloseEa, when not longer needed.
:p.
You can start searches with WtkFindFirstEaValue on a given extended attribute as often,
as you like. The search will then always just start at the first value again.

@@
*/

APIRET APIENTRY WtkFindFirstEaValue( HEA hea, PULONG pulEaType, PSZ pszValue, PULONG pulBuflen)
{

         APIRET         rc     = NO_ERROR;
         ULONG          i;

         PEATYPE        peatype;
         PEAINDEX       peai;
         ULONG          ulEaType;

         ULONG          ulDatalen;
         PBYTE          pbData;
         ULONG          ulEaTypeFound;

do
   {

   // check parm
   peai = __getPointerFromHandle( hea, SIG_EAMANAGER);
   if (!peai)
      {
      rc = ERROR_INVALID_HANDLE;
      break;
      }

   // is list at end ? (should not happen)
   if (peai->ulIndex == NO_MORE_EAVALUES)
      {
      rc = ERROR_NO_MORE_FILES;
      break;
      }

   // reset handle to begin of search
   peai->ulEaType       = *pulEaType;
   peai->ulIndex        = 0;
   peai->fSearchStarted = TRUE;
   rc = WtkFindNextEaValue( hea, pulEaType, pszValue, pulBuflen);

   } while (FALSE);

return rc;

}

// ---------------------------------------------------------------------------

/*
@@WtkFindNextEaValue@SYNTAX
This function scans a extended attribute previously scanned by
WtkFindFirstEaValue for the next value(s) of a given type.

@@WtkFindNextEaValue@PARM@hea@in
Handle to the extended attribute.

@@WtkFindNextEaValue@PARM@pulEaType@out
The address of a variable containing the type of the found
extended attribute.
:p.
All searches with WtkFindNextEaValue
use the type being specified with :hp1.pulEaType:ehp1. on
WtkFindFirstEaValue,

@@WtkFindNextEaValue@PARM@pszValue@out
The address of a buffer in into which the found value
is returned.

@@WtkFindNextEaValue@PARM@pulBuflen@inout
The address of a variable containing the length, in bytes, of the buffer
described by :hp1.pszValue:ehp1..
:p.
On return, this variable contains the length
of the returned value.

@@WtkFindNextEaValue@RETURN
Return Code.
:p.
WtkFindNextEaValue returns one of the following return codes&colon.

:parml compact break=none.
:pt.0
:pd.NO_ERROR
:pt.1
:pd.ERROR_INVALID_FUNCTION
:pt.6
:pd.ERROR_INVALID_HANDLE
:pt.18
:pd.ERROR_NO_MORE_FILES
:pt.87
:pd.ERROR_INVALID_PARAMETER
:pt.111
:pd.ERROR_BUFFER_OVERFLOW
:eparml.

@@WtkFindNextEaValue@REMARKS
If the extended attribute was not scanned with WtkFindFirstEaValue before using
WtkFindNextEaValue, ERROR_INVALID_FUNCTION is returned.
:p.
If no more value according to the type specified on WtkFindFirstEaValue can be found,
ERROR_NO_MORE_FILES is returned.

@@
*/

APIRET APIENTRY WtkFindNextEaValue( HEA hea, PULONG pulEaType, PSZ pszValue, PULONG pulBuflen)
{

         APIRET         rc     = NO_ERROR;
         ULONG          i;

         PEATYPE        peatype;
         PEAINDEX       peai;
         ULONG          ulEaType;

         ULONG          ulDatalen;
         PBYTE          pbData;
         ULONG          ulEaTypeFound;

do
   {
   // check parameters - pulEaType may be NULL
   if (!pszValue)
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // --------------------------------------------------------

   // check parm
   peai = __getPointerFromHandle( hea, SIG_EAMANAGER);
   if (!peai)
      {
      rc = ERROR_INVALID_HANDLE;
      break;
      }

   // is list at end ? (should not happen)
   if (peai->ulIndex == NO_MORE_EAVALUES)
      {
      rc = ERROR_NO_MORE_FILES;
      break;
      }

   // was this EA already scanned
   if (!peai->fSearchStarted)
      {
      rc = ERROR_INVALID_FUNCTION;
      break;
      }

   // search the list for the first EA matching the requested type
   rc = __getEaValueFromList( peai, &pbData, &ulDatalen, &ulEaTypeFound);

   // close handle, if current value is empty
   if ((!pbData) || (peai->ulIndex == NO_MORE_EAVALUES))
      {
      rc = ERROR_NO_MORE_FILES;
      break;
      }

   // does buffer fit ?
   if ((!*pulBuflen) && (ulDatalen > *pulBuflen))
      {
      rc = ERROR_BUFFER_OVERFLOW;
      peai->ulIndex++; // give second chance !
      }

   // hand over data
   memset( pszValue, 0, *pulBuflen);
   memcpy( pszValue, pbData, ulDatalen);
   *pulBuflen = ulDatalen;
   if (pulEaType)
      *pulEaType = ulEaTypeFound;

   } while (FALSE);

return rc;

}


// ===========================================================================

/*
@@WtkCreateEa@SYNTAX
This function creates an extended attribute with no values in memory.

@@WtkCreateEa@PARM@pszName@in
Address of the ASCIIZ path name of the file or directory, that the
extended attribute shall belong to.
:p.
The path name may contain :hp2.?&colon.:ehp2. for the drive in order to
open the file or directory on the boot drive.
:p.
The name may not include wildcards.

@@WtkCreateEa@PARM@pszEaName@in
Address of the ASCIIZ name of the extended attribute.

@@WtkCreateEa@PARM@phea@out
The address of a variable containing the handle to the extended attribute

@@WtkCreateEa@RETURN
Return Code.
:p.
WtkCreateEa returns one of the following return codes&colon.

:parml compact break=none.
:pt.0
:pd.NO_ERROR
:pt.8
:pd.ERROR_NOT_ENOUGH_MEMORY
:pt.87
:pd.ERROR_INVALID_PARAMETER
:eparml.

@@WtkCreateEa@REMARKS
This function allocates resources for handling the value of the
given extended attribute, which must be freeed by calling WtkCloseEa,
when not longer needed.
:p.
This function supports boot drive recognition.
Specify :hp2.?&colon.:ehp2. for the drive in order to
open the file or directory on the boot drive.

@@
*/

APIRET APIENTRY WtkCreateEa( PSZ pszName, PSZ pszEaName, PHEA phea)
{

         APIRET         rc     = NO_ERROR;


do
   {
   // check parameters
   if (!phea)
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // use the read function with magic handle
   *phea = HEA_EMPTY;
   rc = WtkReadEa( pszName, pszEaName, phea);

   } while (FALSE);

return rc;

}

// ---------------------------------------------------------------------------

/*
@@WtkAppendEaValue@SYNTAX
This function appends a value to an extended attribute in memory.

@@WtkAppendEaValue@PARM@hea@in
Handle to the extended attribute.

@@WtkAppendEaValue@PARM@ulMultiType@in
Multitype of the value to be added to the extended attribute.
:p.
Specify
:parml compact.
:pt.EAT_MVST
:pd.to add a value to a multi value, single type extended attribute.
.br
This only works, if this is the first value being appended or the existing extended attribute
is already multi value, single type, and :hp1.ulEaType:ehp1. is of the same type as the
existing value(s).
:pt.EAT_MVMT
:pd.to add a value to a multi value, multi type extended attribute.
.br
This only works, if this is the first value being appended or the existing extended attribute
is already multi value, multi type.
:pt.Other EAT_*
:pd.to add a value to a single value, single type extended attribute
.br
This only works, if this is the first value being appended, otherwise it would not be
single value, single type ...
:eparml.

@@WtkAppendEaValue@PARM@ulEaType@in
Type of the value to be added to the extended attribute.
:p.
Specify
:parml compact.
:pt.EAT_ASCII
:pd.to add a string value
:pt.EAT_BINARY
:pd.to add a binary value
:pt.EAT_ICON
:pd.to add an icon
:pt.EAT_BITMAP
:pd.to add a bitmap
:pt.EAT_METAFILE
:pd.to add a meta file
:pt.EAT_EA
:pd.to add the name of an embedded extended attribute as an ASCII string
:eparml.

@@WtkAppendEaValue@PARM@pbValue@in
The address of a buffer of the value to be written.

@@WtkAppendEaValue@PARM@ulValuelen@in
The length, in bytes, of the buffer described by :hp1.pbValue:ehp1..

@@WtkAppendEaValue@RETURN
Return Code.
:p.
WtkAppendEaValue returns one of the following return codes&colon.

:parml compact break=none.
:pt.0
:pd.NO_ERROR
:pt.6
:pd.ERROR_INVALID_HANDLE
:pt.8
:pd.ERROR_NOT_ENOUGH_MEMORY
:pt.13
:pd.ERROR_INVALID_DATA
:pt.87
:pd.ERROR_INVALID_PARAMETER
:eparml.

@@WtkAppendEaValue@REMARKS
This function allocates resources for handling the value of the
given extended attribute, which must be freeed by calling WtkCloseEa,
when not longer needed.

@@
*/

APIRET APIENTRY WtkAppendEaValue( HEA hea, ULONG ulMultiType, ULONG ulEaType, PBYTE pbValue,  ULONG ulValuelen)
{

         APIRET         rc = NO_ERROR;

         PEAINDEX       peai;
         BOOL           fMultiType          = FALSE;
         PFEA2LIST      pfea2l              = NULL;

         PFEA2          pfea2;
         ULONG          ulExistingEaListLen = 0;
         ULONG          ulExistingEaDataLen = 0;
         BOOL           fEaExists           = FALSE;

         PEATYPE        peatype;
         PEAMVMT        peamvmt;
         PEAMVST        peamvst;
         PEASVST        peasvst;
         PEAVALUE       peavalue;
         ULONG          ulEAListLen;
         ULONG          ulValueBufferLen;

         PBYTE          pbData;
         ULONG          ulDatalen;


do
   {
   // check parameters
   if (!pbValue)
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // check value type
   if (!__isEaTypeValid( ulEaType, FALSE))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // check parm
   peai = __getPointerFromHandle( hea, SIG_EAMANAGER);
   if (!peai)
      {
      rc = ERROR_INVALID_HANDLE;
      break;
      }

   pfea2l = peai->pfea2l;
   ulExistingEaDataLen = __getEaDataLen( peai->peat);

   // check multi type
   fMultiType = __isEaTypeMultiType( ulMultiType);

   // check type of existing EA - must match
   if (ulExistingEaDataLen > 0)
      {
      peatype = (PEATYPE) NEXTSTRING( pfea2l->list[ 0].szName);
      switch (*peatype)
         {
         case EAT_MVST:
         case EAT_MVMT:
            if (ulMultiType != *peatype)
               rc = ERROR_INVALID_DATA;
            break;

         default:
           break;
         }
      } // if (ulExistingEaDataLen > 0)
   if (rc != NO_ERROR)
      break;


   // determine buffer required
   if (ulExistingEaDataLen == 0)
      // completely new structure required
      switch (ulMultiType)
         {
         case EAT_MVMT: ulValueBufferLen =  sizeof( EAMVMT) - 1; break;
         case EAT_MVST: ulValueBufferLen =  sizeof( EAMVST) - 1; break;
         default:       ulValueBufferLen =  sizeof( EASVST) - 1; break;
         }
   else
      // only new value is appended
      switch (ulMultiType)
         {
         case EAT_MVMT: ulValueBufferLen =  sizeof( EASVST)  - 1; break;
         case EAT_MVST: ulValueBufferLen =  sizeof( EAVALUE) - 1; break;
         default:       ulValueBufferLen =  sizeof( EAVALUE) - 1; break;
         }

   // take existing data and/or len of newly appended value into account
   ulValueBufferLen += ulExistingEaDataLen + ulValuelen;

   // now determine, what space the complete EA list requires
   ulEAListLen = strlen( peai->szEaName) + 1 +
                 sizeof( FEA2LIST)   +
                 ulValueBufferLen;

   // get memory for new FEA2LIST and store new pointer to index data
   if ((pfea2l = malloc( ulEAListLen)) == 0)
      {
      rc = ERROR_NOT_ENOUGH_MEMORY;
      break;
      }

   memset( pfea2l, 0, ulEAListLen);
   memcpy( pfea2l, peai->pfea2l, peai->pfea2l->cbList);
   free( peai->pfea2l);
   peai->pfea2l = pfea2l;
   pfea2           = &pfea2l->list[0];
   peai->peat      = (PEATYPE)((PBYTE) pfea2->szName + pfea2->cbName + 1);

   // now append the new value
   switch (ulMultiType)
      {
      case EAT_MVMT:
         // increment counter (and setup fields for case that EA is new)
         peamvmt = (PEAMVMT) NEXTSTRING( pfea2l->list[ 0].szName);
         peamvmt->usHeaderType = ulMultiType;
         peamvmt->usCodepage   = 0;
         peamvmt->usEntries++;
         // append value
         peasvst = (ulExistingEaDataLen) ?
                      ((PEASVST) ((PBYTE) peamvmt + ulExistingEaDataLen)) :
                      ((PEASVST) &peamvmt->easvstList[ 0]);
         peasvst->usEaType           = ulEaType;
         peasvst->eavalue.usEntryLen = ulValuelen;
         memcpy( &peasvst->eavalue.chEntry[0], pbValue, ulValuelen);
         break;

      case EAT_MVST:
         // increment counter (and setup fields for case that EA is new)
         peamvst = (PEAMVST) NEXTSTRING( pfea2l->list[ 0].szName);
         peamvst->usHeaderType = ulMultiType;
         peamvst->usCodepage   = 0;
         peamvst->usEaType     = ulEaType;
         peamvst->usEntries++;
         // append value
         peavalue = (ulExistingEaDataLen) ?
                       ((PEAVALUE) ((PBYTE) peamvst + ulExistingEaDataLen)) :
                       ((PEAVALUE) &peamvst->eavalueList[0]);
         peavalue->usEntryLen  = ulValuelen;
         memcpy( &peavalue->chEntry[0], pbValue, ulValuelen);
         break;

      default:
         // single value single type
         peasvst = (PEASVST) NEXTSTRING( pfea2l->list[ 0].szName);
         peasvst->usEaType            = ulEaType;
         peasvst->eavalue.usEntryLen  = ulValuelen;
         memcpy( &peasvst->eavalue.chEntry[0], pbValue, ulValuelen);
         break;

      } // switch (ulMultiType)

   // setup len fields
   pfea2l->list[ 0].cbValue = ulValueBufferLen;
   pfea2l->cbList           = ulEAListLen;

   } while (FALSE);

return rc;

}

// ---------------------------------------------------------------------------

/*
@@WtkSaveEa@SYNTAX
This function writes an extended attribute from memory to disk.

@@WtkSaveEa@PARM@hea@in
Handle to the extended attribute.

@@WtkSaveEa@PARM@fWriteThru@in
A flag to bypass the cache and write the extended attribute directly to disk.

@@WtkSaveEa@PARM@fEA@in
The flags to be set for this extended attribute flags.
:p.
Specify FEA_NEEDEDEA in order to mark the extended attribute as critical.
Otherwise set this parameter to zero.

@@WtkSaveEa@RETURN
Return Code.
:p.
WtkSaveEa returns one of the following return codes&colon.

:parml compact break=none.
:pt.0
:pd.NO_ERROR
:pt.6
:pd.ERROR_INVALID_HANDLE
:eparml.
:p.
or return codes of the following functions
:ul compact.
:li.DosSetPathInfo
:eul.

@@WtkSaveEa@REMARKS
This function does not deallocate resources for handling the value of the
given extended attribute. They must be freeed by calling WtkCloseEa,
when not longer needed.

@@
*/

APIRET APIENTRY WtkSaveEa( HEA hea, BOOL fWriteThru, BYTE fEA)
{

         APIRET         rc = NO_ERROR;
         EAOP2          eaop2;
         PEAINDEX       peai;

do
   {
   // check parm
   peai = __getPointerFromHandle( hea, SIG_EAMANAGER);
   if (!peai)
      {
      rc = ERROR_INVALID_HANDLE;
      break;
      }

   // write the list
   eaop2.fpGEA2List = NULL;
   eaop2.fpFEA2List = peai->pfea2l;
   rc = DosSetPathInfo( peai->szNode,
                        FIL_QUERYEASIZE,
                        &eaop2,
                        sizeof( eaop2),
                        (fWriteThru) ? DSPI_WRTTHRU : 0);


   } while (FALSE);

return rc;

}

// ---------------------------------------------------------------------------

/*
@@WtkCloseEa@SYNTAX
This function releases the reosurces for handling an extended attribute.

@@WtkCloseEa@PARM@hea@in
Handle to the extended attribute.

@@WtkCloseEa@RETURN
Return Code.
:p.
WtkCloseEa returns one of the following return codes&colon.

:parml compact break=none.
:pt.0
:pd.NO_ERROR
:pt.6
:pd.ERROR_INVALID_HANDLE
:eparml.

@@WtkCloseEa@REMARKS
This function does not write the extended attribute to disk.
Use WtkSaveEa before to write changes to disk, if necessary.

@@
*/

APIRET APIENTRY WtkCloseEa( HEA hea)
{

         APIRET         rc     = NO_ERROR;
         PEAINDEX       peai;
do
   {
   // check parm
   peai = __getPointerFromHandle( hea, SIG_EAMANAGER);
   if (!peai)
      {
      rc = ERROR_INVALID_HANDLE;
      break;
      }

   // close handle
   if (peai->pfea2l) free( peai->pfea2l);
   memset( peai, 0, sizeof( EAINDEX));
   free( peai);

   } while (FALSE);

return rc;

}

// ---------------------------------------------------------------------------

/*
@@WtkGetNextEaValue@SYNTAX
This function reads the value of an extended attribute of a given file or directory.
It provides the use of WtkReadEa / WtkFind*EaValue / WtkCloseEa in one API by calling
it several times, until ERROR_NO_MORE_FILES is returned.

@@WtkGetNextEaValue@PARM@pszName@in
Address of the ASCIIZ path name of the file or directory.
:p.
The path name may contain :hp2.?&colon.:ehp2. for the drive in order to
open the file or directory on the boot drive.
:p.
The name may not include wildcards.

@@WtkGetNextEaValue@PARM@pszEaName@in
Address of the ASCIIZ name of the extended attribute.

@@WtkGetNextEaValue@PARM@pulEaType@inout
The address of a variable containing the type of the searched/found
extended attribute.
:p.
Specify WTK_EAT_ANY on input to search for values of any type.

@@WtkGetNextEaValue@PARM@phea@inout
The address of a variable containing the handle to the extended attribute.
:p.
On the first call to WtkGetNextEaValue, set this variable to HEA_CREATE.
On return, *phea will contain the correct handle value for subsequent calls.

@@WtkGetNextEaValue@PARM@pszValue@out
The address of a buffer in into which the found value
is returned.

@@WtkGetNextEaValue@PARM@pulBuflen@inout
The address of a variable containing the length, in bytes, of the buffer
described by :hp1.pszValue:ehp1..
:p.
On return, this variable contains the length
of the returned value.

@@WtkGetNextEaValue@RETURN
Return Code.
:p.
WtkGetNextEaValue returns one of the following return codes&colon.

:parml compact break=none.
:pt.0
:pd.NO_ERROR
:pt.8
:pd.ERROR_NOT_ENOUGH_MEMORY
:pt.18
:pd.ERROR_NO_MORE_FILES
:pt.87
:pd.ERROR_INVALID_PARAMETER
:eparml.
:p.
or return codes of the following functions
:ul compact.
:li.DosQueryPathInfo
:eul.

@@WtkGetNextEaValue@REMARKS
This function supports boot drive recognition.
Specify :hp2.?&colon.:ehp2. for the drive in order to
open the file or directory on the boot drive.
:p.
This function is meant to provide you with easy handling of extended attributes.
Instead of calling WtkReadEa / WtkFind*EaValue / WtkCloseEa, you can retrieve
all values of an extended attribute by just calling WtkGetNextEaValue, until it
returns ERROR_NO_MORE_FILES.
:p.
This function allocates resources for handling the value of the
given extended attribute by callig WtkReadEa. They must either be freed
by
:ul compact.
:li.calling WtkGetNextEaValue, until it is returning ERROR_NO_MORE_FILES. This will call
WtkCloseEa implicitely.
:li.calling WtkClose
:eul.
.br

@@
*/

APIRET APIENTRY WtkGetNextEaValue( PSZ pszName, PSZ pszEaName, PULONG pulEaType, PHEA phea, PSZ pszValue, PULONG pulBuflen)
{

         APIRET         rc = NO_ERROR;

do
   {
   // check parameter - others are checked by WtkFindFirstEa/NextEa
   if (!phea)
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   if (*phea == HEA_CREATE)
      {
      // find first EA
      rc = WtkReadEa( pszName, pszEaName, phea);
      if (rc != NO_ERROR)
         break;
      // find first EA
      rc = WtkFindFirstEaValue( *phea, pulEaType, pszValue, pulBuflen);
      }
   else
      // find all other EAs
      rc = WtkFindNextEaValue( *phea, pulEaType, pszValue, pulBuflen);

   // implicit close on end of list
   if (rc == ERROR_NO_MORE_FILES)
      WtkCloseEa( *phea);

   } while (FALSE);

return rc;

}

// ---------------------------------------------------------------------------

/*
@@WtkPutEaValue@SYNTAX
This function appends a value to an extended attribute directly on disk.

@@WtkPutEaValue@PARM@pszName@in
Address of the ASCIIZ path name of the file or directory.
:p.
The path name may contain :hp2.?&colon.:ehp2. for the drive in order to
search the file or directory on the boot drive.
:p.
The name may not include wildcards.

@@WtkPutEaValue@PARM@pszEaName@in
Address of the ASCIIZ name of the extended attribute.

@@WtkPutEaValue@PARM@ulMultiType@in
Multitype of the value to be added to the extended attribute.
:p.
Specify
:parml compact.
:pt.EAT_MVST
:pd.to add a value to a multi value, single type extended attribute.
.br
This only works, if this is the first value being appended or the existing extended attribute
is already multi value, single type, and :hp1.ulEaType:ehp1. is of the same type as the
existing value(s).
:pt.EAT_MVMT
:pd.to add a value to a multi value, multi type extended attribute.
.br
This only works, if this is the first value being appended or the existing extended attribute
is already multi value, multi type.
:pt.Other EAT_*
:pd.to add a value to a single value, single type extended attribute
.br
This only works, if this is the first value being appended, otherwise it would not be
single value, single type ...
:eparml.

@@WtkPutEaValue@PARM@ulEaType@in
Type of the value to be added to the extended attribute.
:p.
Specify
:parml compact.
:pt.EAT_ASCII
:pd.to add a string value
:pt.EAT_BINARY
:pd.to add a binary value
:pt.EAT_ICON
:pd.to add an icon
:pt.EAT_BITMAP
:pd.to add a bitmap
:pt.EAT_METAFILE
:pd.to add a meta file
:pt.EAT_EA
:pd.to add the name of an embedded extended attribute as an ASCII string
:eparml.

@@WtkPutEaValue@PARM@pbValue@in
The address of a buffer of the value to be written.

@@WtkPutEaValue@PARM@ulValuelen@in
The length, in bytes, of the buffer described by :hp1.pbValue:ehp1..

@@WtkPutEaValue@PARM@fWriteThru@in
A flag to bypass the cache and write the extended attribute directly to disk.

@@WtkPutEaValue@PARM@fEA@in
The flags to be set for this extended attribute flags.
:p.
Specify FEA_NEEDEDEA in order to mark the extended attribute as critical.
Otherwise set this parameter to zero.

@@WtkPutEaValue@RETURN
Return Code.
:p.
WtkPutEaValue returns one of the following return codes&colon.

:parml compact break=none.
:pt.0
:pd.NO_ERROR
:pt.8
:pd.ERROR_NOT_ENOUGH_MEMORY
:pt.13
:pd.ERROR_INVALID_DATA
:pt.87
:pd.ERROR_INVALID_PARAMETER
:eparml.
:p.
or return codes of the following functions
:ul compact.
:li.DosSetPathInfo
:eul.

@@WtkPutEaValue@REMARKS
This function supports boot drive recognition.
Specify :hp2.?&colon.:ehp2. for the drive in order to
open the file or directory on the boot drive.
:p.
This function is meant to provide you with easy handling of extended attributes.
Instead of calling WtkCreateEa / WtkReadEa / WtkAppendEaValue / WtkSaveEa /
WtkCloseEa, you can directly append a value to an extended attribute by
just calling WtkPutEaValue.
:p.
The drawback is, that if you are required to append several values, each call
needs to read the existing values for the given extended attribute again from
disk in order to he list of values.
:p.
This is not that important, if you do not query a lot of extended attributes
of a lot of files at one time, as extended attributes can of course be cached,
provided that you did not set fWriteThru to TRUE. In that case, from the second
call of WtkPutEaValue this is most likely a in-memory operation.
:p.
Nevertheless, if you handle very large lists of values for a given extended
attribute, or you want to bypass the cache, it is faster to use the
WtkCreateEa / WtkReadEa / WtkAppendEaValue / WtkSaveEa / WtkCloseEa
functions and thus write to the disk only once.

@@
*/

APIRET APIENTRY WtkPutEaValue( PSZ pszName, PSZ pszEaName, ULONG ulMultiType, ULONG ulEaType, PBYTE pbValue, ULONG ulValuelen, BOOL fWriteThru, BYTE fEA)
{

         APIRET         rc = NO_ERROR;
         HEA            hea = NULLHANDLE;



do
   {
   // check parameters
   if ((!pszName)    ||
       (!pszEaName)  ||
       (!*pszEaName))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // read existing EAs
   rc = WtkReadEa( pszName, pszEaName, &hea);
   if (rc != NO_ERROR)
      break;

   // apend new value
   rc = WtkAppendEaValue( hea, ulMultiType, ulEaType, pbValue,  ulValuelen);
   if (rc != NO_ERROR)
      break;

   // write new EA to disk
   rc = WtkSaveEa( hea, fWriteThru, fEA);

   } while (FALSE);

// cleanup
if (hea) WtkCloseEa( hea);

return rc;

}

// ---------------------------------------------------------------------------

/*
@@WtkWriteStringEa@SYNTAX
This function attaches a string extended attribute to a file or directory.

@@WtkWriteStringEa@PARM@pszName@in
Address of the ASCIIZ path name of the file or directory.
:p.
The path name may contain :hp2.?&colon.:ehp2. for the drive in order to
search the file or directory on the boot drive.
:p.
The name may not include wildcards.

@@WtkWriteStringEa@PARM@pszEaName@in
Address of the ASCIIZ name of the extended attribute.

@@WtkWriteStringEa@PARM@pszEaValue@in
The address of a string to be written.

@@WtkWriteStringEa@RETURN
Return Code.
:p.
WtkWriteStringEa returns one of the following return codes&colon.

:parml compact break=none.
:pt.0
:pd.NO_ERROR
:pt.6
:pd.ERROR_INVALID_HANDLE
:pt.8
:pd.ERROR_NOT_ENOUGH_MEMORY
:pt.13
:pd.ERROR_INVALID_DATA
:pt.87
:pd.ERROR_INVALID_PARAMETER
:eparml.
:p.
or return codes of the following functions
:ul compact.
:li.DosSetPathInfo
:eul.

@@WtkWriteStringEa@REMARKS
This function supports boot drive recognition.
Specify :hp2.?&colon.:ehp2. for the drive in order to
open the file or directory on the boot drive.
:p.
This function is meant to provide you with easy handling of
string extended attributes. The drawback is that for simplicity
you cannot
:ul compact.
:li.attach more than one string to the specified extended attribute
(but that is what is needed for most cases anyway)
:li.write through (ommit the cache)
:li.mark the extended attribute as critical
:eul.
:p.
If these limitations does not meet your requirements, use the
move flexible call :link reftype=hd viewport refid=WtkPutEaValue.WtkPutEaValue:elink.
instead.

@@
*/

APIRET APIENTRY WtkWriteStringEa( PSZ pszName, PSZ pszEaName, PSZ pszEaValue)
{

         APIRET         rc = NO_ERROR;
         HEA            hea = NULLHANDLE;

do
   {
   // check parameters
   if ((!pszName)    ||
       (!pszEaName)  ||
       (!*pszEaName))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // create new EA, this will overwrite an existing EA of the same name.
   rc = WtkCreateEa( pszName, pszEaName, &hea);
   if (rc != NO_ERROR)
      break;

   // apend new value
   rc = WtkAppendEaValue( hea, EAT_ASCII, EAT_ASCII, pszEaValue, strlen( pszEaValue));
   if (rc != NO_ERROR)
      break;

   // write new EA to disk
   rc = WtkSaveEa( hea, FALSE, FALSE);

   } while (FALSE);

// cleanup
if (hea) WtkCloseEa( hea);

return rc;

}

// ---------------------------------------------------------------------------

/*
@@WtkReadStringEa@SYNTAX
This function queries a string extended attribute of a file or directory.

@@WtkReadStringEa@PARM@pszName@in
Address of the ASCIIZ path name of the file or directory.
:p.
The path name may contain :hp2.?&colon.:ehp2. for the drive in order to
search the file or directory on the boot drive.
:p.
The name may not include wildcards.

@@WtkReadStringEa@PARM@pszEaName@in
Address of the ASCIIZ name of the extended attribute.
:p.
If multiple values and/or types for the specified extended attribute
are attached to the file or directory, the first string attribute
is returned.

@@WtkReadStringEa@PARM@pszEaValue@out
The address of a buffer in into which the value
of the given extended attribute is returned.

@@WtkReadStringEa@PARM@pulBuflen@inout
The address of a variable containing the length, in bytes, of the buffer
described by :hp1.pszEaValue:ehp1..

@@WtkReadStringEa@RETURN
Return Code.
:p.
WtkReadStringEa returns one of the following return codes&colon.

:parml compact break=none.
:pt.0
:pd.NO_ERROR
:pt.6
:pd.ERROR_INVALID_HANDLE
:pt.8
:pd.ERROR_NOT_ENOUGH_MEMORY
:pt.13
:pd.ERROR_INVALID_DATA
:pt.87
:pd.ERROR_INVALID_PARAMETER
:eparml.
:p.
or return codes of the following functions
:ul compact.
:li.DosSetPathInfo
:eul.

@@WtkReadStringEa@REMARKS
This function supports boot drive recognition.
Specify :hp2.?&colon.:ehp2. for the drive in order to
open the file or directory on the boot drive.
:p.
This function is meant to provide you with easy handling of
string extended attributes. The drawback is that for simplicity
you cannot
:ul compact.
:li.read more than one value from the string extended attribute
(but that is what is needed for most cases anyway)
:eul.

:p.
If these limitations does not meet your requirements, use the
move flexible call :link reftype=hd viewport refid=WtkPutEaValue.WtkPutEaValue:elink.
instead.

@@
*/

APIRET APIENTRY WtkReadStringEa( PSZ pszName, PSZ pszEaName, PSZ pszEaValue, PULONG pulBuflen)
{

         APIRET         rc = NO_ERROR;
         HEA            hea = HEA_CREATE;
         ULONG          ulEaType = EAT_ASCII;

do
   {
   // check parameters
   if ((!pszName)    ||
       (!pszEaName)  ||
       (!*pszEaName) ||
       (!pszEaValue) ||
       (!pulBuflen))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // read attribute
   rc = WtkGetNextEaValue(  pszName, pszEaName, &ulEaType, &hea, pszEaValue, pulBuflen);

   } while (FALSE);

// cleanup
if (hea) WtkCloseEa( hea);

return rc;

}

