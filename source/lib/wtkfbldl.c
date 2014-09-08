/****************************** Module Header ******************************\
*
* Module Name: wtkfbldl.c
*
* Source for access functions for buidlevel information
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2005
*
* $Id: wtkfbldl.c,v 1.5 2006-12-02 21:06:56 cla Exp $
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
#include <ctype.h>

#define INCL_DOS
#define INCL_GPI
#define INCL_ERRORS
#include <os2.h>

#include "wtkfbldl.h"
#include "wtkufil.h"
#include "wpstk.ih"

#include "wtklx.h"

#define BLDLEVEL_MASK "@#%s:%s#@%s"
#define BLDLEVEL_INFOLEN(pbl) (strlen( pbl->pszVendor) + strlen( pbl->pszRevision) + strlen( pbl->pszDescription) + 5)

#define BLDLEVEL_ORDINAL  0

// ----------------------------------------------------------------------

// some macros for file access

#define MOVEFILEPTR(o)                         \
rc = DosSetFilePtr( hfile,                     \
                    o,                         \
                    FILE_BEGIN,                \
                    &ulFilePtr);               \
if (rc != NO_ERROR)                            \
   break;

#define READFILE(h,p,s)                        \
rc = DosRead( h,                               \
              p,                               \
              s,                               \
              &ulBytesRead);                   \
if (rc != NO_ERROR)                            \
   break;                                      \
if (ulBytesRead != s)                          \
   {                                           \
   rc = ERROR_READ_FAULT;                      \
   break;                                      \
   }

#define WRITEFILE(h,p,s)                       \
{                                              \
rc = DosWrite( h,                              \
               p,                              \
               s,                              \
               &ulBytesWritten);               \
if (rc != NO_ERROR)                            \
   break;                                      \
if (ulBytesWritten != s)                       \
   {                                           \
   rc = ERROR_READ_FAULT;                      \
   break;                                      \
   }                                           \
}

#define QUERYFILEPTR()                         \
{                                              \
rc = DosSetFilePtr( hfile,                     \
                    0,                         \
                    FILE_CURRENT,              \
                    &ulFilePtr);               \
}

// =============================================================================================
// this function compares revision numbers
//  -  up to 7 revision levels
//  -  separated by any nodigit characters
// where the non-digit characters are assumed to have the same
// position in both revision strings !
// valid strings are e.g.: 1.2.5    4.24.7   12_5_6_6   1.10b3
// returns: FALSE, if equal
//          TRUE, if unequal

static ULONG _makeRevArray( PSZ pszRev, PULONG paulEntries, ULONG ulMaxEntries)
{

         ULONG          ulEntriesConverted = 0;
         ULONG          i;
         PULONG         pulEntry;
         PSZ            p, e;
do
   {
   // check parms
   if ((!pszRev)      ||
       (!paulEntries) ||
       (!ulMaxEntries))
      break;

   // init
   memset( paulEntries, 0, ulMaxEntries * sizeof( ULONG));

   // fill array
   for (i = 0, p = pszRev, pulEntry = paulEntries;
        ((*p) && (i < ulMaxEntries));
        i++, pulEntry++)
      {
      // skip all chars not being decimal
      while ((*p) && (!isdigit(*p)))
         p++;
      if (!*p)
         break;

      // read this number
      *pulEntry = strtol( p, &e, 10);

      // read on from end of this part
      p = e;
      }

   } while ( FALSE);

return ulEntriesConverted;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

#define MAX_NUMBERS 7
static BOOL _revComp( PSZ pszRev1, PSZ pszRev2, ULONG ulCheckType)
{

         BOOL           fMatch = TRUE;
         ULONG          ulResult = WTK_BLREVCHECK_EQUAL;
         ULONG          i;
  
         ULONG          aulRev1[ MAX_NUMBERS];
         ULONG          aulRev2[ MAX_NUMBERS];

do
   {
   // tokenize numbers
   _makeRevArray( pszRev1, aulRev1, MAX_NUMBERS);
   _makeRevArray( pszRev2, aulRev2, MAX_NUMBERS);

   // now compare
   for (i = 0; ((i < MAX_NUMBERS) && (fMatch)) ; i++)
      {
      // check for greater
      if (aulRev1[ i] > aulRev2[ i])
         {
         ulResult = WTK_BLREVCHECK_GREATER;
         break;
         }

      // check for lesser
      else if (aulRev1[ i] < aulRev2[ i])
         {
         ulResult = WTK_BLREVCHECK_LESSER;
         break;
         }

      } // for

   } while ( FALSE);

fMatch = ((ulResult & ulCheckType) > 0);
return fMatch;
}

// =============================================================================

/*
@@WtkGetBlSignature@SYNTAX
This function loads the buildlevel information from an executable file.

@@WtkGetBlSignature@PARM@pszExeFile@in
Address of the ASCII pathname of the executable file
p.
The path name may contain :hp2.?&colon.:ehp2. for the drive in order to
search the file on the boot drive.

@@WtkGetBlSignature@PARM@pbl@in
Address of a PBLDLEVEL structure to receive the buildlevel information.

@@WtkGetBlSignature@PARM@ulBuflen@in
The length, in bytes, of the buffer described by :hp1.pbBuildLevel:ehp1..

@@WtkGetBlSignature@RETURN
Return Code.
:p.
WtkGetBlSignature returns one of the following return codes:
:parml compact break=none.
:pt.0
:pd.NO_ERROR
:pt.87
:pd.ERROR_INVALID_PARAMETER
:eparml.
:p.
or return codes of the following functions
:ul compact.
:li.DosOpen
:eul.

@@WtkGetBlSignature@REMARKS
This function supports boot drive recognition.
Specify :hp2.?&colon.:ehp2. for the drive in order to
search the file on the boot drive.
@@
*/

APIRET APIENTRY WtkGetBlSignature( PSZ pszExeFile, PBLDLEVEL pbl, ULONG ulBuflen)
{
         APIRET         rc = NO_ERROR;
         CHAR           szName[ _MAX_PATH];

         ULONG          ulAction;
         ULONG          ulFilePtr;
         ULONG          ulBytesRead;
         ULONG          ulBufSize;

         HFILE          hfile = -1;
         DOSEXEHEADER   dosexeheader;
         LXHEADER       lxheader;

         PBYTE          pbNRNTable = NULL;

         PBYTE          p;
         PBYTE          pSig = NULL;
         BYTE           bOrdinal;

         PSZ            pszSig;
         PSZ            pszVendor;
         PSZ            pszRevision;
         PSZ            pszDescription;
do
   {
   // check parms
   if ((!pszExeFile)  ||
       (!*pszExeFile) ||
       (!ulBuflen))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // copy name and replace ?: with bootdrive
   strcpy( szName, pszExeFile);
   __PatchBootDrive( szName);

   // open file
   rc = DosOpen( szName,
                 &hfile,
                 &ulAction,
                 0,
                 0,
                 OPEN_ACTION_FAIL_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS,
                 OPEN_SHARE_DENYNONE | OPEN_ACCESS_READONLY,
                 NULL);
   if (rc != NO_ERROR)
      break;

   // read DOS EXE header
   READFILE( hfile, &dosexeheader, sizeof( dosexeheader));
   if (dosexeheader.usSig != SIG_DOSEXEHDR)
      {
      rc = ERROR_INVALID_DATA;
      break;
      }

   // read LX header
   MOVEFILEPTR( dosexeheader.ulOfsExtHeader);
   READFILE( hfile, &lxheader, sizeof( lxheader));
   if (lxheader.usSig != SIG_LXHDR)
      {
      rc = ERROR_INVALID_DATA;
      break;
      }

   // no table given ?
   if (!lxheader.ulCbNResTab)
      {
      rc = ERROR_NO_MORE_FILES;
      break;
      }

   // get memory for table
   pbNRNTable = malloc( lxheader.ulCbNResTab);
   if (!pbNRNTable)
      {
      rc = ERROR_NOT_ENOUGH_MEMORY;
      break;
      }

   // read complete non-resident name table
   MOVEFILEPTR( lxheader.ulNResTabOfs);
   READFILE( hfile, pbNRNTable, lxheader.ulCbNResTab);

   // now search ordinal zero
   p = pbNRNTable;
   while (*p != 0)
      {
      // is it ordinal zero ?
      bOrdinal = *(p + *p + 1);
      if (bOrdinal == 0)
         {
         pSig = p;
         break;
         }
      // go to next entry
      p += *p + 2;
      }

   // ordinal zero not found ?
   if (!pSig)
      {
      rc = ERROR_NO_MORE_FILES;
      break;
      }

   // scan sig: is it valid ?
   rc = ERROR_NO_MORE_FILES;
   pszSig = pSig + 1;
   *(pszSig + *pSig) = 0;
   if (strncmp( pszSig, "@#", 2))
      break;
   pszVendor = pszSig + 2;
   pszSig = strchr( pszVendor, ':');
   if (!pszSig)
      break;
   *pszSig = 0;
   pszRevision = pszSig + 1;
   pszSig = strstr( pszRevision, "#@");
   if (!pszSig)
      break;
   *pszSig = 0;
   pszDescription = pszSig + 2;
   rc = NO_ERROR;

   // if it is an extended description, skip
   // extended information
   if (!strncmp( pszDescription, "##1##", 5))
      {
      pszSig = strstr( pszDescription, "@@");
      if (pszSig)
         pszDescription = pszSig + 2;
      }

   // skip leading blanks of description
   while ((*pszDescription) && (*pszDescription <= 32))
      pszDescription++;

   // calculate required size
   ulBufSize = sizeof( BLDLEVEL)        +
               strlen( pszVendor)       + 1 +
               strlen( pszRevision)     + 1 +
               strlen( pszDescription);        // last byte included in BLDLEVEL
   if (ulBufSize > ulBuflen)
      {
      rc = ERROR_BUFFER_OVERFLOW;
      break;
      }

   // fill buffer
   memset( pbl, 0, ulBuflen);

   pszSig = pbl->bData;
   strcpy( pszSig, pszVendor);
   pbl->pszVendor = pszSig;

   pszSig = NEXTSTR( pszSig);
   strcpy( pszSig, pszRevision);
   pbl->pszRevision = pszSig;

   pszSig = NEXTSTR( pszSig);
   strcpy( pszSig, pszDescription);
   pbl->pszDescription = pszSig;

   } while (FALSE);

// cleanup
if (pbNRNTable) free( pbNRNTable);
DosClose( hfile);
return rc;
}

// ----------------------------------------------------------------------

/*
@@WtkCheckBlSignature@SYNTAX
This function checks the buildlevel information from an executable file
and compares it with provided values.

@@WtkCheckBlSignature@PARM@pszExeFile@in
Address of the ASCII pathname of the executable file
p.
The path name may contain :hp2.?&colon.:ehp2. for the drive in order to
search the file on the boot drive.

@@WtkCheckBlSignature@PARM@pszVendor@in
Address of the vendor name or NULL.
:p.
If this parameter is NULL, the vendor ist ot checked.
The comparison is case sensitive.

@@WtkCheckBlSignature@PARM@pszRevision@in
Address of the revision to compare with or NULL.
:p.
If this parameter is NULL, the revision is not checked.
I it is specified, the parameter :hp2.ulRevCheck:ehp2. must be 
specified as well, otherwise an error is returned.

@@WtkCheckBlSignature@PARM@ulRevCheck@in
A variable specifying how to compare the revision specified
in the parameter :hp2.pszRevision:ehp2.
:p.
Specify one of the following options, matching the specified condition:
:parml compact.
:pt.WTK_BLREVCHECK_EQUAL
:pd.revision number of the file is equal
:pt.WTK_BLREVCHECK_GREATER
:pd.revision number of the file is greater
:pt.WTK_BLREVCHECK_LESSER
:pd.revision number of the file is lesser
:pt.WTK_BLREVCHECK_GREATER_OR_EQUAL
:pd.revision number of the file is greater or equal
:pt.WTK_BLREVCHECK_LESSER_OR_EQUAL
:pd.revision number of the file is lesser or equal
:pt.WTK_BLREVCHECK_NOT_EQUAL
:pd.revision number of the file is not equal
:eparml.

@@WtkCheckBlSignature@PARM@pszDescription@in
Address of the description to compare with or NULL.
:p.
If this parameter is NULL, the description is not checked.
The comparison is case sensitive.

@@WtkCheckBlSignature@RETURN
Return Code.
:p.
WtkCheckBlSignature returns one of the following return codes:
:parml compact break=none.
:pt.0
:pd.NO_ERROR
:pt.87
:pd.ERROR_INVALID_PARAMETER
:eparml.
:p.
or return codes of the following functions
:ul compact.
:li.WtkGetBlSignature
:eul.

@@WtkCheckBlSignature@REMARKS
This function supports boot drive recognition.
Specify :hp2.?&colon.:ehp2. for the drive in order to
search the file on the boot drive.

@@
*/

APIRET APIENTRY WtkCheckBlSignature( PSZ pszExeFile, PSZ pszVendor, PSZ pszRevision, ULONG ulRevCheck, PSZ pszDescription)
{
         APIRET         rc = NO_ERROR;
         CHAR           szName[ _MAX_PATH];

         BYTE           abData[ _MAX_PATH];
         PBLDLEVEL      pbl = (PVOID) abData;

do
   {
   // check parms
   if (!pszExeFile)
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }
   if ((!pszVendor)   &&
       (!pszRevision) &&
       (!pszDescription))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }
    if ((pszRevision) && (!ulRevCheck))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // copy name and replace ?: with bootdrive
   strcpy( szName, pszExeFile);
   __PatchBootDrive( szName);

   // get buildlevel
   rc = WtkGetBlSignature( szName, pbl, sizeof( abData));
   if (rc != NO_ERROR)
      break;

   // now check values
   rc = ERROR_INVALID_DATA;
   if (pszVendor)
      if (strcmp( pszVendor, pbl->pszVendor))
         break;

   if (pszRevision)
      if (!_revComp( pbl->pszRevision, pszRevision, ulRevCheck))
         break;

   if (pszDescription)
      if (strcmp( pszDescription, pbl->pszDescription))
         break;

   // no error found
   rc = NO_ERROR;

   } while ( FALSE);

return rc;
}

// ----------------------------------------------------------------------

/*
@@WtkSetBlSignature@SYNTAX
This function sets the buildlevel information of an executable file.

@@WtkSetBlSignature@PARM@pszExeFile@in
Address of the ASCII pathname of the executable file
p.
The path name may contain :hp2.?&colon.:ehp2. for the drive in order to
search the file on the boot drive.

@@WtkSetBlSignature@PARM@pbl@in
Address of a PBLDLEVEL structure containing the buildlevel information to be set.
:p.
The parameter :hp2.ulOptions:ehp2. must be set to specify which fields
of this information are to be set

@@WtkSetBlSignature@PARM@ulOptions@in
A variable specifying which fields of the information are to be set.
:p.
Specify one of the following options, matching the specified condition:
:parml compact.
:pt.WTK_BLSET_VENDOR
:pd.set the vendor field
:pt.WTK_BLSET_REVISION
:pd.set the revision number field
:pt.WTK_BLSET_DESCRIPTION
:pd.set the description field
:pt.WTK_BLSET_ALL
:pd.set all fields
:eparml.

@@WtkSetBlSignature@RETURN
Return Code.
:p.
WtkSetBlSignature returns one of the following return codes:
:parml compact break=none.
:pt.0
:pd.NO_ERROR
:pt.87
:pd.ERROR_INVALID_PARAMETER
:eparml.
:p.
or return codes of the following functions
:ul compact.
:li.WtkGetBlSignature
:li.DosOpen
:li.DosRead
:li.DosSetFilePtr
:eul.

@@WtkSetBlSignature@REMARKS
This function supports boot drive recognition.
Specify :hp2.?&colon.:ehp2. for the drive in order to
search the file on the boot drive.

@@
*/

APIRET APIENTRY WtkSetBlSignature( PSZ pszExeFile, PBLDLEVEL pbl, ULONG ulOptions)
{
         APIRET         rc = NO_ERROR;
         CHAR           szName[ _MAX_PATH];

         HFILE          hfile = -1;

         ULONG          ulAction;
         ULONG          ulBytesRead;
         ULONG          ulBytesWritten;
         ULONG          ulFilePtr;
         ULONG          ulExeFileLen;

         DOSEXEHEADER   dosexeheader;
         LXHEADER       lxheader;


         PBLDLEVEL      pblUpdate = NULL;
         ULONG          ulBuflen;

         PSZ            pszInfo = NULL;
         ULONG          ulInfoLen;

static   PSZ            pszEmptyStr = "";

         PBYTE          pbNRNTable = NULL;
         PBYTE          pbNRNTableNew = NULL;
         ULONG          ulNewSize;

         ULONG          ulEndOfNRNTOfs;
         BOOL           fRelocateData = FALSE;
         PBYTE          pbRelocateData = NULL;
         ULONG          ulRelocateBufSize;
         ULONG          ulRelocateOfs;

         ULONG          ulBufSize;

         USHORT         usOrdinal;
         PBYTE          p;
         PBYTE          pNew;
         BYTE           bLen;
         BOOL           fAdded = FALSE;
         LONG           lSizeDiff;

do
   {
   // check parms
   if((!pszExeFile) ||
      (!pbl)        ||
      (!ulOptions))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // copy name and replace ?: with bootdrive
   strcpy( szName, pszExeFile);
   __PatchBootDrive( szName);

   // ---------------------------------------------

   // get memory for current information
   ulBuflen = 1024;
   pblUpdate = malloc( ulBuflen);
   if (!pblUpdate)
      {
      rc = ERROR_NOT_ENOUGH_MEMORY;
      break;
      }
   memset( pblUpdate, 0, ulBuflen);

   // and retrieve it
   rc = WtkGetBlSignature( szName, pblUpdate, ulBuflen);
   if (rc != NO_ERROR)
      {
      // no data found, init to empty strings
      pblUpdate->pszVendor      = pszEmptyStr;
      pblUpdate->pszRevision    = pszEmptyStr;
      pblUpdate->pszDescription = pszEmptyStr;
      rc = NO_ERROR;
      }

   // now check for updated values
   if (ulOptions & WTK_BLSET_VENDOR)
      pblUpdate->pszVendor = (pbl->pszVendor) ? pbl->pszVendor : pszEmptyStr;

   if (ulOptions & WTK_BLSET_REVISION)
      pblUpdate->pszRevision = (pbl->pszRevision) ? pbl->pszRevision : pszEmptyStr;

   if (ulOptions & WTK_BLSET_DESCRIPTION)
      pblUpdate->pszDescription = (pbl->pszDescription) ? pbl->pszDescription : pszEmptyStr;


   // determine signature to write to executable
   ulInfoLen = BLDLEVEL_INFOLEN( pblUpdate);
   pszInfo = malloc( ulInfoLen + 1); // add one terminating byte, not written to exe 
   sprintf( pszInfo, BLDLEVEL_MASK,
            pblUpdate->pszVendor,
            pblUpdate->pszRevision,
            pblUpdate->pszDescription);

   // ---------------------------------------------
   // open file
   rc = DosOpen( szName,
                 &hfile,
                 &ulAction,
                 0,
                 0,
                 OPEN_ACTION_FAIL_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS,
                 OPEN_SHARE_DENYREADWRITE | OPEN_ACCESS_READWRITE,
                 NULL);
   if (rc != NO_ERROR)
      break;

   // read DOS EXE header
   MOVEFILEPTR( 0);
   READFILE( hfile, &dosexeheader, sizeof( dosexeheader));
   if (dosexeheader.usSig != SIG_DOSEXEHDR)
      {
      rc = ERROR_INVALID_DATA;
      break;
      }

   // read LX header
   MOVEFILEPTR( dosexeheader.ulOfsExtHeader);
   READFILE( hfile, &lxheader, sizeof( lxheader));
   if (lxheader.usSig != SIG_LXHDR)
      {
      rc = ERROR_INVALID_DATA;
      break;
      }

   // determine exe file len, in case a new NRN table has to be appended
   DosSetFilePtr( hfile, 0, FILE_END, &ulExeFileLen);

   // ------------ check if any data is placed behind non resident name tabe

   if (lxheader.ulCbNResTab)
      {
      ulRelocateOfs = lxheader.ulNResTabOfs + lxheader.ulCbNResTab;
      fRelocateData = (ulExeFileLen != ulRelocateOfs);

      // ------------ read data after NRNT
   
      if (fRelocateData)
         {
         ulRelocateBufSize = ulExeFileLen - ulRelocateOfs;
         pbRelocateData = malloc( ulRelocateBufSize);
         if (!pbRelocateData)
            {
            rc = ERROR_NOT_ENOUGH_MEMORY;
            break;
            }
         MOVEFILEPTR( ulRelocateOfs);
         READFILE( hfile, pbRelocateData, ulRelocateBufSize);
         }
      }

   // ------------ prepare non resident name table

   // get memory for old and new table
   ulNewSize = lxheader.ulCbNResTab + strlen( pszInfo) + 3 + 1;
   pbNRNTable = malloc( (lxheader.ulCbNResTab) ? lxheader.ulCbNResTab : 1);
   pbNRNTableNew = malloc( ulNewSize + 32);
   if ((!pbNRNTable) || (!pbNRNTableNew))
      {
      rc = ERROR_NOT_ENOUGH_MEMORY;
      break;
      }

   // read complete non-resident name table
   if (lxheader.ulCbNResTab)
      {
      MOVEFILEPTR( lxheader.ulNResTabOfs);
      READFILE( hfile, pbNRNTable, lxheader.ulCbNResTab);
      }
   else
      *pbNRNTable = 0;

   // now search ordinal zero
   p = pbNRNTable;
   pNew = pbNRNTableNew;
   while (*p != 0)
      {
      // is it our ordinal ?
      bLen      = *p;
      usOrdinal = *(PUSHORT)(p + bLen + 1);

      if (usOrdinal != BLDLEVEL_ORDINAL)
         {
         memcpy( pNew, p, bLen + 3);
         }
      else
         {
         bLen = strlen( pszInfo);
         *pNew = bLen;
         memcpy( pNew + 1, pszInfo, bLen);
         *(PUSHORT)(pNew + bLen + 1) = BLDLEVEL_ORDINAL;
         fAdded = TRUE;
         }

      // go to next entry
      p  += *p + 3;
      pNew += *pNew + 3;
      }

   // append entry if necessary
   if (!fAdded)
      {
      bLen = strlen( pszInfo);
      *pNew = bLen;
      memcpy( pNew + 1, pszInfo, bLen);
      *(PUSHORT)(pNew + bLen + 1) = BLDLEVEL_ORDINAL;
      pNew += *pNew + 3;
      }

   // append empty entry and modify exe header
   *pNew = 0;

   // recalculate effective size of new table
   ulNewSize = pNew + 1 - pbNRNTableNew;

   // ------------ modify exe header to allow proper calculation of CRC

   // if new table has to be appended, set offset
   if (!lxheader.ulCbNResTab)
      lxheader.ulNResTabOfs = ulExeFileLen;

   // determine how size of NRNT changes
   lSizeDiff = (LONG) ulNewSize - (LONG) lxheader.ulCbNResTab;
   lxheader.ulCbNResTab = ulNewSize;

   // modify offset of all offsets behind non resident name table
   if (fRelocateData)
      {

      // first of all, modify the offset for relocated data
      ulRelocateOfs += lSizeDiff;

      //  adjust all offset fields

#define ADJUSTOFS(v) if ((v) && (v > lxheader.ulNResTabOfs)) v += lSizeDiff

      ADJUSTOFS( lxheader.ulObjTabOfs);
      ADJUSTOFS( lxheader.ulObjMapOfs);
      ADJUSTOFS( lxheader.ulIterMapOfs);
      ADJUSTOFS( lxheader.ulRsrcTabOfs);
      ADJUSTOFS( lxheader.ulResTabOfs);
      ADJUSTOFS( lxheader.ulEntTabOfs);
      ADJUSTOFS( lxheader.ulDirTabOfs);
      ADJUSTOFS( lxheader.ulFPageTabOfs);
      ADJUSTOFS( lxheader.ulFRecTabOfs);
      ADJUSTOFS( lxheader.ulImpModOfs);
      ADJUSTOFS( lxheader.ulImpProcOfs);
      ADJUSTOFS( lxheader.ulPageSumOfs);
      ADJUSTOFS( lxheader.ulDataPageOfs);
      ADJUSTOFS( lxheader.ulDebugInfoOfs);
      }

   // write updated exe header
   // read LX header
   MOVEFILEPTR( dosexeheader.ulOfsExtHeader);
   WRITEFILE( hfile, &lxheader, sizeof( lxheader));

   // ------------ write new non-resident name table

   // write new non-resident name table
   MOVEFILEPTR( lxheader.ulNResTabOfs);
   WRITEFILE( hfile, pbNRNTableNew, lxheader.ulCbNResTab);

   // ------------ write debug info

   // write relocated data
   if (fRelocateData)
      WRITEFILE( hfile, pbRelocateData, ulRelocateBufSize);

   // ------------ set new file size (just in case file gets shorter)

   // set filesize
   QUERYFILEPTR();
   DosSetFileSize( hfile, ulFilePtr);

   } while ( FALSE);

// cleanup
if (pbNRNTable) free( pbNRNTable);
if (pbNRNTableNew) free( pbNRNTableNew);
if (pbRelocateData) free( pbRelocateData);

if (pblUpdate) free( pblUpdate);
if (pszInfo) free( pszInfo);
DosClose( hfile);
return rc;
}
