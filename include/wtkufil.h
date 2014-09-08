/****************************** Module Header ******************************\
*
* Module Name: wtkufil.h
*
* include file for file helper functions.
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: wtkufil.h,v 1.6 2005-01-02 23:30:12 cla Exp $
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

#ifndef WTKUTLFILE_INCLUDED
#define WTKUTLFILE_INCLUDED File helper functions

#ifdef __cplusplus
      extern "C" {
#endif


/*** check existance of files & directories ********************************/
BOOL APIENTRY WtkFileExists( PSZ pszName);
BOOL APIENTRY WtkDirExists( PSZ pszName);
BOOL APIENTRY WtkFileMaskExists( PSZ pszFileMask, PSZ pszFirstFile, ULONG ulBuflen);
BOOL APIENTRY WtkIsFile( PSZ pszName);      /* equivalent to WtkFileExists */
BOOL APIENTRY WtkIsDirectory( PSZ pszName); /* equivalent to WtkDirExists  */

/*** get fullname of directory or file *************************************/
APIRET APIENTRY WtkQueryFullname( PSZ pszName, PSZ pszBuffer, ULONG ulBuflen);

/*** extended file handling ************************************************/
APIRET APIENTRY WtkDeleteFile( PSZ pszName);
APIRET APIENTRY WtkMoveFile( PSZ pszOld, PSZ pszNew);

/*** query disk and directory at one time **********************************/
APIRET APIENTRY WtkQueryCurrentDir(  ULONG ulDiskNum, PSZ pszBuffer, ULONG ulBuflen);
APIRET APIENTRY WtkSetCurrentDir( PSZ pszDirectory);

/*** create/delete path ****************************************************/
APIRET APIENTRY WtkCreatePath( PSZ pszPath);
APIRET APIENTRY WtkDeletePath( PSZ pszPath);

/*** easy version of DosFindFirst/DosFindNext ******************************/
APIRET APIENTRY WtkGetNextFile( PSZ pszFileMask, PHDIR phdir,
                                PSZ pszNextFile, ULONG ulBuflen);
APIRET APIENTRY WtkGetNextDirectory( PSZ pszFileMask, PHDIR phdir,
                            PSZ pszNextDirectory, ULONG ulBuflen);

/*** search part of filename ***********************************************/
PSZ APIENTRY WtkFilespec( PSZ pszName, ULONG ulPart);
#define WTK_FILESPEC_PATHNAME  1
#define WTK_FILESPEC_NAME      2
#define WTK_FILESPEC_EXTENSION 3

/*** get specific information about file ***********************************/
BOOL APIENTRY WtkFileModified( PSZ pszName, PFILESTATUS3 pfs3);
ULONG APIENTRY WtkQueryFileSize( PSZ pszName);

/*** read file into memory  ************************************************/
APIRET APIENTRY WtkReadFile( PSZ pszName, PSZ* ppszBuffer, PULONG pulBuflen);
APIRET APIENTRY WtkReadFilePart( PSZ pszName, ULONG ulOffset, PVOID pvBuffer, ULONG ulBuflen);
APIRET APIENTRY WtkWriteFile( PSZ pszName, PSZ pszBuffer, ULONG ulBuflen, BOOL fAppend);

/*** create tmp file *******************************************************/
APIRET APIENTRY WtkCreateTmpFile( PSZ pszFileMask, PSZ pszBuffer, ULONG ulBuflen);

/*** check file contents ***************************************************/
BOOL APIENTRY WtkFileIsEmpty( PSZ pszName);

#ifdef __cplusplus
        }
#endif

#endif /* WTKUTLFILE_INCLUDED */

