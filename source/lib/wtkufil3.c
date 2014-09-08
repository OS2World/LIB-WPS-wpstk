/****************************** Module Header ******************************\
*
* Module Name: wtkufil3.c
*
* Source for file and directory helper functions
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: wtkufil3.c,v 1.2 2003-04-24 14:33:30 cla Exp $
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
@@WtkFileMaskExists@SYNTAX
This function checks, if a set of files exist, and returns the
first file being found.

@@WtkFileMaskExists@PARM@pszFileMask@in
Address of the ASCIIZ path name of the fileset to search.
:p.
The path name may contain wildcard characters.
:p.
The path name may contain :hp2.?&colon.:ehp2. for the drive in order to
search files on the boot drive.

@@WtkFileMaskExists@PARM@pszFirstFile@out
The address of a buffer in into which the name of the first
file being found is returned.

@@WtkFileMaskExists@PARM@ulBuflen@in
The length, in bytes, of the buffer described by :hp1.pszFirstFile:ehp1..

@@WtkFileMaskExists@RETURN
Success indicator.
:parml compact.
:pt.TRUE
:pd.File(s) exist(s).
:pt.FALSE
:pd.File(s) not found.
:eparml.

@@WtkFileMaskExists@REMARKS
This function supports boot drive recognition.
Specify :hp2.?&colon.:ehp2. for the drive in order to
search the files on the boot drive.

@@
*/

BOOL APIENTRY WtkFileMaskExists( PSZ pszFileMask, PSZ pszFirstFile, ULONG ulBuflen)
{
         APIRET         rc = NO_ERROR;
         HDIR           hdir = HDIR_CREATE;

do
   {
   // check parameters
   if ((pszFileMask          == NULL)     ||
       (*pszFileMask         == 0)        ||
       (strlen( pszFileMask) > _MAX_PATH))
      break;

   // get first file
   rc = WtkGetNextFile( pszFileMask,
                        &hdir,
                        pszFirstFile,
                        ulBuflen);
   if (rc == NO_ERROR)
      DosFindClose( hdir);
   else
      break;

   } while (FALSE);

return (rc == NO_ERROR);
}

// ---------------------------------------------------------------------------

/*
@@WtkGetNextFile@SYNTAX
This function provides a simple way to search files with
one API call instead of using DosFindFirst / DosFindNext / DosFindClose.

@@WtkGetNextFile@PARM@pszFileMask@in
Address of the ASCIIZ file mask of the files to be searched.

@@WtkGetNextFile@PARM@phdir@inout
Address of the directory handle associated with this request.
:p.
Before calling WtkGetNextFile for the first time to
search a given file mask, the directory handle must be set to HDIR_CREATE.
On subsequent calls, the directory handle must not be modified.

:color fc=red.
:note text='IMPORTANT:'.
:ul compact.
:li.The directory handle will be automatically closed, when
WtkGetNextFile is being called, until it returns ERROR_NO_MORE_FILES.
Otherwise the caller must explicitely use DosFindClose to close the directory handle.
:eul.
:color fc=default.

@@WtkGetNextFile@PARM@pszNextFile@out
The address of a buffer, into which the name of the recently
found file is returned.

@@WtkGetNextFile@PARM@ulBuflen@in
The length, in bytes, of the buffer described by :hp1.pszNextFile:ehp1..

@@WtkGetNextFile@RETURN
Return Code.
:p.
WtkGetNextFile returns one of the following return codes&colon.

:parml compact break=none.
:pt.0
:pd.NO_ERROR
:pt.87
:pd.ERROR_INVALID_PARAMETER
:eparml.
:p.
or return codes of the following functions
:ul compact.
:li.DosFindFirst
:li.DosFindNext
:eul.

@@WtkGetNextFile@REMARKS
Before calling WtkGetNextFile for the first time to
search a given file mask, the directory handle must be set to HDIR_CREATE.
On subsequent calls, the directory handle must not be modified.

:color fc=red.
:note text='IMPORTANT:'.
:ul compact.
:li.The directory handle will be automatically closed, when
WtkGetNextFile is being called, until it returns ERROR_NO_MORE_FILES.
Otherwise the caller must explicitely use DosFindClose to close the directory handle.
:eul.
:color fc=default.

@@
*/

APIRET APIENTRY WtkGetNextFile( PSZ pszFileMask, PHDIR phdir, PSZ pszNextFile, ULONG ulBuflen)
{
         APIRET         rc = NO_ERROR;
         PSZ            pszFilenamePart;
         FILEFINDBUF3   ffb3;
         ULONG          ulFilecount = 1;
         CHAR           szNextFile[ _MAX_PATH];
         CHAR           szName[ _MAX_PATH];

do
   {
   // check parms
   if ((pszFileMask   == NULL) ||
       (phdir         == NULL) ||
       (pszNextFile   == NULL))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // copy name and replace ?: with bootdrive
   strcpy( szName, pszFileMask);
   __PatchBootDrive( szName);

   // get first/next file
   if (*phdir == NULLHANDLE)
      *phdir = HDIR_CREATE;
   if (*phdir == HDIR_CREATE)
      {
      rc = DosFindFirst( szName,
                         phdir,
                         FILE_ARCHIVED | FILE_SYSTEM | FILE_HIDDEN | FILE_READONLY,
                         &ffb3,
                         sizeof( ffb3),
                         &ulFilecount,
                         FIL_STANDARD);
      }
   else
      rc = DosFindNext( *phdir,
                        &ffb3,
                        sizeof( ffb3),
                        &ulFilecount);

   if (rc != NO_ERROR)
      break;

   // Namensteil isolieren
   strcpy( szNextFile, szName);
   pszFilenamePart = WtkFilespec( szNextFile, WTK_FILESPEC_NAME);
   strcpy( pszFilenamePart, ffb3.achName);

   // does buffer fit ?
   if ((strlen( szNextFile) + 1) > ulBuflen)
      {
      rc = ERROR_BUFFER_OVERFLOW;
      break;
      }

   // hand over result
   strcpy( pszNextFile, szNextFile);

   } while ( FALSE);


// cleanup
if (phdir)
   if (rc == ERROR_NO_MORE_FILES)
      DosFindClose( *phdir);

return rc;
}

// ---------------------------------------------------------------------------

/*
@@WtkGetNextDirectory@SYNTAX
This function provides a simple way to search directories with
one API call instead of using DosFindFirst / DosFindNext / DosFindClose.

@@WtkGetNextDirectory@PARM@pszFileMask@in
Address of the ASCIIZ file mask of the files to be searched.

@@WtkGetNextDirectory@PARM@phdir@inout
Address of the directory handle associated with this request.
:p.
Before calling WtkGetNextDirectory for the first time to
search a given file mask, the directory handle must be set to HDIR_CREATE.
On subsequent calls, the directory handle must not be modified.

:color fc=red.
:note text='IMPORTANT:'.
:ul compact.
:li.The directory handle will be automatically closed, when
WtkGetNextDirectory is being called, until it returns ERROR_NO_MORE_FILES.
Otherwise the caller must explicitely use DosFindClose to close the directory handle.
:eul.
:color fc=default.

@@WtkGetNextDirectory@PARM@pszNextDirectory@out
The address of a buffer, into which the name of the recently
found file is returned.

@@WtkGetNextDirectory@PARM@ulBuflen@in
The length, in bytes, of the buffer described by :hp1.pszNextDirectory:ehp1..

@@WtkGetNextDirectory@RETURN
Return Code.
:p.
WtkGetNextDirectory returns one of the following return codes&colon.

:parml compact break=none.
:pt.0
:pd.NO_ERROR
:pt.87
:pd.ERROR_INVALID_PARAMETER
:eparml.
:p.
or return codes of the following functions
:ul compact.
:li.DosFindFirst
:li.DosFindNext
:eul.

@@WtkGetNextDirectory@REMARKS
Before calling WtkGetNextDirectory for the first time to
search a given file mask, the directory handle must be set to HDIR_CREATE.
On subsequent calls, the directory handle must not be modified.

:color fc=red.
:note text='IMPORTANT:'.
:ul compact.
:li.The directory handle will be automatically closed, when
WtkGetNextDirectory is being called, until it returns ERROR_NO_MORE_FILES.
Otherwise the caller must explicitely use DosFindClose to close the directory handle.
:eul.
:color fc=default.

@@
*/

APIRET APIENTRY WtkGetNextDirectory( PSZ pszFileMask, PHDIR phdir, PSZ pszNextDirectory, ULONG ulBuflen)
{
         APIRET         rc = NO_ERROR;
         PSZ            pszFilenamePart;
         FILEFINDBUF3   ffb3;
         ULONG          ulFilecount = 1;
         CHAR           szNextDirectory[ _MAX_PATH];
         CHAR           szName[ _MAX_PATH];

do
   {
   // check parms
   if ((pszFileMask   == NULL) ||
       (phdir         == NULL) ||
       (pszNextDirectory   == NULL))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // copy name and replace ?: with bootdrive
   strcpy( szName, pszFileMask);
   __PatchBootDrive( szName);

   // get first/next file
   if (*phdir == NULLHANDLE)
      *phdir = HDIR_CREATE;
   if (*phdir == HDIR_CREATE)
      {
      rc = DosFindFirst( szName,
                         phdir,
                         MUST_HAVE_DIRECTORY | FILE_ARCHIVED | FILE_SYSTEM | FILE_HIDDEN | FILE_READONLY,
                         &ffb3,
                         sizeof( ffb3),
                         &ulFilecount,
                         FIL_STANDARD);
      }
   else
      rc = DosFindNext( *phdir,
                        &ffb3,
                        sizeof( ffb3),
                        &ulFilecount);

   if (rc != NO_ERROR)
      break;

   // skip dot entries
   while ((rc == NO_ERROR) &&
          ((!strcmp( ffb3.achName, ".")) || (!strcmp( ffb3.achName, ".."))))
      {
      rc = DosFindNext( *phdir,
                        &ffb3,
                        sizeof( ffb3),
                        &ulFilecount);
      }
   if (rc != NO_ERROR)
      break;

   // isolate name part
   strcpy( szNextDirectory, szName);
   pszFilenamePart = WtkFilespec( szNextDirectory, WTK_FILESPEC_NAME);
   strcpy( pszFilenamePart, ffb3.achName);

   // does buffer fit ?
   if ((strlen( szNextDirectory) + 1) > ulBuflen)
      {
      rc = ERROR_BUFFER_OVERFLOW;
      break;
      }

   // hand over result
   strcpy( pszNextDirectory, szNextDirectory);

   } while ( FALSE);


// cleanup
if (phdir)
   if (rc == ERROR_NO_MORE_FILES)
      DosFindClose( *phdir);

return rc;
}

