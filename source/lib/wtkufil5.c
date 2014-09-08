/****************************** Module Header ******************************\
*
* Module Name: wtkufil5.c
*
* Source for file and directory helper functions
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: wtkufil5.c,v 1.2 2003-04-24 14:33:30 cla Exp $
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
#define INCL_ERRORS
#include <os2.h>

#include "wtkufil.h"
#include "wpstk.ih"

// ---------------------------------------------------------------------------

/*
@@WtkFilespec@SYNTAX
This function searches for certain pathname components
in a given pathname.

@@WtkFilespec@PARM@pszName@in
Address of the ASCIIZ filename, of which a part ist to be returned.

@@WtkFilespec@PARM@ulPart@in
The type of information to be returned.

:parml.
:pt.WTK_FILESPEC_PATHNAME
:pd.returns a pointer to the begin of the path, not icluding a possibly specified drive
:pt.WTK_FILESPEC_NAME
:pd.returns a pointer to the filename
:pt.WTK_FILESPEC_EXTENSION
:pd.returns a pointer to the last file extension
:eparml.

@@WtkFilespec@RETURN
Pointer to the requested pathname component.

If the requested component could not be found,
NULL is returned.

@@WtkFilespec@REMARKS
WtkFilespec returns a pointer to a part fo the string, that
is being passed with pszName.

@@
*/

PSZ APIENTRY WtkFilespec( PSZ pszName, ULONG ulPart)
{
         PSZ            pszNamePart = NULL;
         ULONG          ulAdjust = 0;
do
   {
   // check parm
   if (pszName == NULL)
      break;

   switch (ulPart)
      {
      case WTK_FILESPEC_EXTENSION:
         pszNamePart = strrchr( pszName, '.');
         ulAdjust = 1;
         break;

      case WTK_FILESPEC_NAME:
         pszNamePart = strrchr( pszName, '\\');
         ulAdjust = 1;
         // fall thru !!!

      case WTK_FILESPEC_PATHNAME:
         if (!pszNamePart)
            {
            pszNamePart = strrchr( pszName, ':');
            ulAdjust = 1;
            if (pszNamePart == NULL)
               {
               pszNamePart = pszName;
               ulAdjust = 0;
               }
            }

         // make sure there is a path component
         // if requested
         if ((ulPart == WTK_FILESPEC_PATHNAME)  &&
             (!strchr( pszName, '\\')))
            pszNamePart = NULL;

         break;

      default:
         break;
      }

   if (pszNamePart != NULL)
      {
      // adjust if necessary
      pszNamePart += ulAdjust;

      // return error, if string at end !
      if (!*pszNamePart)
         pszNamePart = NULL;
      }

   } while (FALSE);

return pszNamePart;

}

// ---------------------------------------------------------------------------

/*
@@WtkCreateTmpFile@SYNTAX
This function creates a temporary file of zero byte size
with a generic name. Specify a filemask without drive and path
in order to create the file in the %TMP% directory.

@@WtkCreateTmpFile@PARM@pszFileMask@in
Address of the ASCIIZ filemask, which is to be used to create
a temporary file.
:p.
The filename must contain one or more :hp2.?:ehp2., which are being
replaced by random digits.
:p.
If the file mask specified with pszFileMask does neither contain
a drive or a path specification, the temporary file is being created
in the %TMP% directory. In order to create a temporary file in the
current directory, you may specify a ".\" as the path component.
:p.
The path name may contain :hp2.?&colon.:ehp2. for the drive in order to
create the temporary file on the boot drive.

@@WtkCreateTmpFile@PARM@pszBuffer@out
The address of a buffer, into which the full name
of the newly created temporary file is returned.

@@WtkCreateTmpFile@PARM@ulBuflen@in
The length, in bytes, of the buffer described by :hp1.pszBuffer:ehp1..

@@WtkCreateTmpFile@RETURN
Return Code.
:p.
WtkCreateTmpFile returns one of the following return codes&colon.
:parml compact break=none.
:pt.0
:pd.NO_ERROR
:pt.87
:pd.ERROR_INVALID_PARAMETER
:pt.111
:pd.ERROR_BUFFER_OVERFLOW
:pt.123
:pd.ERROR_INVALID_NAME
:eparml.
:p.
or return codes of the following functions
:ul compact.
:li.DosOpen
:eul.


@@WtkCreateTmpFile@REMARKS
If the file mask specified with pszFileMask does neither contain
a drive or a path specification, the temporary file is being created
in the %TMP% directory. In order to create a temporary file in the
current directory, you may specify a ".\" as the path component.
:p.
:p.
This function supports boot drive recognition.
Specify :hp2.?&colon.:ehp2. for the drive in order to
create the temporary file on the boot drive.


@@
*/

APIRET APIENTRY WtkCreateTmpFile( PSZ pszFileMask, PSZ pszBuffer, ULONG ulBuflen)

{
         APIRET         rc = NO_ERROR;

         CHAR           szFile[ _MAX_PATH];
         PSZ            pszFilename;
         PSZ            pszFilenameOrg = NULL;
         BOOL           fContainsPathDrive = FALSE;

         PSZ            pszTmpDir;
         PSZ            pszWildCard;

         ULONG          ulMilliseconds;
         CHAR           szMilliseconds[ 20];
         PSZ            pszChar;

         HFILE          hfTmpFile = -1;
         ULONG          ulAction;

static   ULONG          ulCounter = 0;


do
   {
   // check parms
   if ((!pszFileMask) || (!pszBuffer))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // check if name has wildcards
   pszFilename = WtkFilespec( pszFileMask, WTK_FILESPEC_NAME);
   if (!strchr( pszFilename, '?'))
      {
      rc = ERROR_INVALID_NAME;  // ok not really, but we need wildcard chars !
      break;
      }

   pszFilenameOrg = strdup( pszFilename);

   // is it containing a path or drive letter ?
   if (strchr( pszFileMask, '\\')) fContainsPathDrive = TRUE;
   if (strchr( pszFileMask, ':'))  fContainsPathDrive = TRUE;

   // if it does not contain a drive, try to get the temp directory
   if (!fContainsPathDrive)
      {
      pszTmpDir = getenv( "TMP");
      if (pszTmpDir)
         sprintf( szFile, "%s\\%s", pszTmpDir, pszFileMask);
      else
         strcpy( szFile, pszFileMask);
      }
   else
      strcpy( szFile, pszFileMask);

   // replace ?: with bootdrive
   __PatchBootDrive( szFile);

   // does buffer fit ?
   if (strlen( szFile) + 1 > ulBuflen)
      {
      rc = ERROR_BUFFER_OVERFLOW;
      break;
      }

   // now try to get a unique filename
   DosQuerySysInfo( QSV_MS_COUNT, QSV_MS_COUNT, &ulMilliseconds, sizeof( ulMilliseconds));
   ulCounter += 321;             // cheap way to assure that we get different numbers
   ulMilliseconds += ulCounter;  // even within consecutive calls within milliseconds
   _ltoa( (ULONG) ulMilliseconds, szMilliseconds, 10);
   pszChar = &szMilliseconds[ strlen( szMilliseconds) - 1];

   rc = ERROR_ACCESS_DENIED;
   pszFilename = WtkFilespec( szFile, WTK_FILESPEC_NAME);
   while (rc != NO_ERROR)
      {
      // replace all wildcards in string
      pszWildCard = strchr( pszFilenameOrg, '?');
      while (pszWildCard)
         {
         // replace character in copy of string with rightmost char of hundred seconds
         *(pszFilename + (pszWildCard - pszFilenameOrg)) = *pszChar;

         // address next available char from millisecond counter
         pszChar--;
         if (pszChar < szMilliseconds)
            pszChar = &szMilliseconds[ strlen( szMilliseconds) - 1]; // start again

         //  address next wildcardchar
         pszWildCard = strchr( pszWildCard + 1, '?');
         }

      // try to create a zero byte file
      rc = DosOpen( szFile,
                    &hfTmpFile,
                    &ulAction,
                    0,
                    FILE_NORMAL,
                    OPEN_ACTION_CREATE_IF_NEW | OPEN_ACTION_FAIL_IF_EXISTS,
                    OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYREADWRITE,
                    (PEAOP2)NULL);
      }
   if (rc != NO_ERROR)
      break;

   // hand over filename
   strcpy( pszBuffer, szFile);


   } while (FALSE);

// cleanup
if (pszFilenameOrg) free( pszFilenameOrg);
DosClose( hfTmpFile);

return rc;

}

