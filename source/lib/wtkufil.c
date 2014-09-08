/****************************** Module Header ******************************\
*
* Module Name: wtkufil.c
*
* Source for file and directory helper functions
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: wtkufil.c,v 1.11 2003-04-24 14:33:29 cla Exp $
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

static BOOL __NodeExists( PSZ pszName, BOOL fSearchDirectory)
{
         APIRET         rc = NO_ERROR;
         BOOL           fResult = FALSE;
         CHAR           szName[ _MAX_PATH];
         FILESTATUS3    fs3;
         BOOL           fIsDirectory = FALSE;


do
   {
   // check parameters
   if ((pszName  == NULL) ||
       (*pszName == 0))
      break;

   // copy name and replace ?: with bootdrive
   strcpy( szName, pszName);
   __PatchBootDrive( szName);

   // search entry
   rc = DosQueryPathInfo( szName,
                          FIL_STANDARD,
                          &fs3,
                          sizeof( fs3));
   if (rc != NO_ERROR)
      break;

   // check for directory or file
   fIsDirectory = ((fs3.attrFile & FILE_DIRECTORY) > 0);
   fResult = (fIsDirectory == fSearchDirectory);


   } while (FALSE);

return fResult;
}

// ---------------------------------------------------------------------------

/*
@@WtkFileExists@SYNTAX
This function checks, if a given file exists.

@@WtkFileExists@PARM@pszName@in
Address of the ASCIIZ path name of the file to be searched.
:p.
The path name may contain :hp2.?&colon.:ehp2. for the drive in order to
open the file on the boot drive.
:p.
The name may not include wildcards. In order to search with wildcards,
use :link reftype=hd refid=WtkFileMaskExists.WtkFileMaskExists:elink..

@@WtkFileExists@RETURN
Success indicator.
:parml compact.
:pt.TRUE
:pd.File exists.
:pt.FALSE
:pd.File not found.
:eparml.

@@WtkFileExists@REMARKS
This function is identical to :link reftype=hd refid=WtkIsFile.WtkIsFile:elink..
:p.
This function supports boot drive recognition.
Specify :hp2.?&colon.:ehp2. for the drive in order to
open the file on the boot drive.

@@
*/

BOOL APIENTRY WtkFileExists( PSZ pszName)
{
return __NodeExists( pszName, FALSE);
}

// ---------------------------------------------------------------------------

/*
@@WtkDirExists@SYNTAX
This function checks, if a given directory exists.

@@WtkDirExists@PARM@pszName@in
Address of the ASCIIZ path name of the directory to be searched.
:p.
The path name may contain :hp2.?&colon.:ehp2. for the drive in order to
open the directory on the boot drive.
:p.
The name may not include wildcards.

@@WtkDirExists@RETURN
Success indicator.
:parml compact.
:pt.TRUE
:pd.Directory exists.
:pt.FALSE
:pd.Directory not found.
:eparml.

@@WtkDirExists@REMARKS
This function is identical to :link reftype=hd refid=WtkIsDirectory.WtkIsDirectory:elink..
:p.
This function supports boot drive recognition.
Specify :hp2.?&colon.:ehp2. for the drive in order to
open the directory on the boot drive.

@@
*/

BOOL APIENTRY WtkDirExists( PSZ pszName)
{
return __NodeExists( pszName, TRUE);
}

// ---------------------------------------------------------------------------

/*
@@WtkIsFile@SYNTAX
This function checks, if a given name is the name of a file.

@@WtkIsFile@PARM@pszName@in
Address of the ASCIIZ path name of the file to be searched.
:p.
The path name may contain :hp2.?&colon.:ehp2. for the drive in order to
search the file on the boot drive.
:p.
The name may not include wildcards. In order to search with wildcards,
use :link reftype=hd refid=WtkFileMaskExists.WtkFileMaskExists:elink..

@@WtkIsFile@RETURN
Success indicator.
:parml compact.
:pt.TRUE
:pd.Given name is an existing file.
:pt.FALSE
:pd.File not found or given name is not a file.
:eparml.

@@WtkIsFile@REMARKS
This function is identical to :link reftype=hd refid=WtkFileExists.WtkFileExists:elink..
:p.
This function supports boot drive recognition.
Specify :hp2.?&colon.:ehp2. for the drive in order to
search the file on the boot drive.

@@
*/

BOOL APIENTRY WtkIsFile( PSZ pszName)
{
return __NodeExists( pszName, FALSE);
}

// ---------------------------------------------------------------------------

/*
@@WtkIsDirectory@SYNTAX
This function checks, if a given name is the name of a directory.

@@WtkIsDirectory@PARM@pszName@in
Address of the ASCIIZ path name of the directory to be searched.
:p.
The path name may contain :hp2.?&colon.:ehp2. for the drive in order to
search the directory on the boot drive.
:p.
The name may not include wildcards.

@@WtkIsDirectory@RETURN
Success indicator.
:parml compact.
:pt.TRUE
:pd.Given name is an existing directory.
:pt.FALSE
:pd.Directory not found or given name is not a directory.
:eparml.

@@WtkIsDirectory@REMARKS
This function is identical to :link reftype=hd refid=WtkDirExists.WtkDirExists:elink..
:p.
This function supports boot drive recognition.
Specify :hp2.?&colon.:ehp2. for the drive in order to
search the directory on the boot drive.

@@
*/

BOOL APIENTRY WtkIsDirectory( PSZ pszName)
{
return __NodeExists( pszName, TRUE);
}

// ---------------------------------------------------------------------------

/*
@@WtkQueryCurrentDir@SYNTAX
This function queries the current drive and directory for
a given drive.

@@WtkQueryCurrentDir@PARM@ulDiskNum@in
The drive number.
:p.
The value 0 means the current drive, 1 means drive A, 2 means drive B, 3
means drive C, and so on. The maximum possible value is 26, which
corresponds to drive Z.

@@WtkQueryCurrentDir@PARM@pszBuffer@out
Address of a buffer, where the given drive and the current directory
of that drive is being returned.

@@WtkQueryCurrentDir@PARM@ulBuflen@in
The length, in bytes, of the buffer described by :hp1.pszBuffer:ehp1..

@@WtkQueryCurrentDir@RETURN
Return Code.
:p.
WtkQueryCurrentDir returns one of the following return codes&colon.

:parml compact break=none.
:pt.0
:pd.NO_ERROR
:pt.87
:pd.ERROR_INVALID_PARAMETER
:pt.111
:pd.ERROR_BUFFER_OVERFLOW
:eparml.
:p.
or return codes of the following functions
:ul compact.
:li.DosQueryCurrentDisk
:li.DosQueryCurrentDir
:eul.

@@WtkQueryCurrentDir@REMARKS
In opposite to DosQueryCurrentDir, this function returns both the
drive and directory, thus providing a full pathname.

@@
*/

APIRET APIENTRY WtkQueryCurrentDir( ULONG ulDiskNum, PSZ pszBuffer, ULONG ulBuflen)
{

         APIRET         rc = NO_ERROR;

         CHAR           szDir[ _MAX_PATH];
         ULONG          ulPathLen = sizeof( szDir) - 2;

         ULONG          ulDisk;
         ULONG          ulDiskMap;

do
   {
   // check parms
   if (!pszBuffer)
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   if (!ulDiskNum)
      {
      // get current drive
      rc = DosQueryCurrentDisk( &ulDisk, &ulDiskMap);
      if (rc != NO_ERROR)
         break;
      }
   else
      ulDisk = ulDiskNum;
   sprintf( szDir, "%c:\\", ulDisk + 'A' - 1);

   // get current dir
   ulPathLen = sizeof( szDir) - strlen( szDir);
   rc = DosQueryCurrentDir( 0, &szDir[ strlen( szDir)], &ulPathLen);
   if (rc != NO_ERROR)
      break;

   // does buffer fit ?
   if (strlen( szDir) + 1 > ulBuflen)
      {
      rc = ERROR_BUFFER_OVERFLOW;
      break;
      }

   // hand over result
   strcpy( pszBuffer, szDir);

   } while (FALSE);

return rc;
}

// ---------------------------------------------------------------------------

/*
@@WtkSetCurrentDir@SYNTAX
This function sets the current drive and the directory for
a given drive.
If a drive is not given, the current drive
is being assumed.

@@WtkSetCurrentDir@PARM@pszDirectory
The drive and directory to be set current.
:p.
If a drive is not given, the current drive
is being assumed.
:p.
The directory name may contain :hp2.?&colon.:ehp2.
for the drive in order to set the boot drive as the default drive.

@@WtkSetCurrentDir@RETURN
Return Code.
:p.
WtkSetCurrentDir returns one of the following return codes&colon.

:parml compact break=none.
:pt.0
:pd.NO_ERROR
:pt.87
:pd.ERROR_INVALID_PARAMETER
:eparml.
:p.
or return codes of the following functions
:ul compact.
:li.DosSetDefaultDisk
:li.DosSetCurrentDir
:eul.

@@WtkSetCurrentDir@REMARKS
In opposite to DosSetCurrentDir, this function sets also a given
drive as default disk.
:p.
This function supports boot drive recognition.
Specify :hp2.?&colon.:ehp2. for the drive in order to
set the boot drive as the default drive.

@@
*/

APIRET APIENTRY WtkSetCurrentDir( PSZ pszDirectory)
{

         APIRET         rc = NO_ERROR;
         CHAR           szDirectory[ _MAX_PATH];

         PSZ            pszColon;
         ULONG          ulDrive = 0;

do
   {
   // check parms
   if ((!pszDirectory) || (!*pszDirectory))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // copy name and replace ?: with bootdrive
   strcpy( szDirectory, pszDirectory);
   __PatchBootDrive( szDirectory);


   // separate drive and path
   pszColon = strchr( szDirectory, ':');
   if (pszColon)
      {
      // set the default disk
      ulDrive = toupper( szDirectory[ 0]) - 'A' + 1;
      rc = DosSetDefaultDisk( ulDrive);
      if (rc != NO_ERROR)
         break;
      }

   // set directory on that drive
   rc = DosSetCurrentDir( szDirectory);

   } while (FALSE);

return rc;
}

