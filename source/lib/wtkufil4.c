/****************************** Module Header ******************************\
*
* Module Name: wtkufil4.c
*
* Source for file and directory helper functions
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: wtkufil4.c,v 1.3 2005-01-02 23:30:14 cla Exp $
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
@@WtkQueryFileSize@SYNTAX
This function returns the size of a given file.

@@WtkQueryFileSize@PARM@pszName@in
Address of the ASCIIZ path name of the file or directory,
which size is to be returned.
:p.
The path name may contain :hp2.?&colon.:ehp2. for the drive in order to
search the file on the boot drive.

@@WtkQueryFileSize@RETURN
Length of file.
:parml compact.
:pt.Zero
:pd.File is of zero length or does not exist.
:pt.Other
:pd.Size of the file.
:eparml.

@@WtkQueryFileSize@REMARKS
This function supports boot drive recognition.
Specify :hp2.?&colon.:ehp2. for the drive in order to
search the file on the boot drive.

@@
*/

ULONG APIENTRY WtkQueryFileSize( PSZ pszName)

{
         ULONG          ulFileSize = 0;
         APIRET         rc = NO_ERROR;
         CHAR           szName[ _MAX_PATH];
         FILESTATUS3    fs3;

do
   {
   // check parms
   if (pszName == NULL)
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

   // report size
   ulFileSize = fs3.cbFile;

   } while (FALSE);

return ulFileSize;

}

// ---------------------------------------------------------------------------

/*
@@WtkReadFilePart@SYNTAX
This function reads a part of a file into memory.

@@WtkReadFilePart@PARM@pszName@in
Address of the ASCIIZ pathname, from which the file part is to be read into memory.
:p.
The path name may contain :hp2.?&colon.:ehp2. for the drive in order to
read the file from the boot drive.

@@WtkReadFilePart@PARM@ulOffset@in
The offset of the file part.

@@WtkReadFilePart@PARM@pvBuffer@out
The address of a buffer, into which the file part
is being read into.

@@WtkReadFilePart@PARM@ulBuflen@in
The length, in bytes, of the part of the file, and thus the buffer.

@@WtkReadFilePart@RETURN
Return Code.
:p.
WtkReadFilePart returns one of the following return codes&colon.
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
:li.DosOpen
:li.DosRead
:eul.

@@WtkReadFilePart@REMARKS
- none -

@@
*/

APIRET APIENTRY WtkReadFilePart( PSZ pszName, ULONG ulOffset, PVOID pvBuffer, ULONG ulBuflen)

{
         APIRET         rc = NO_ERROR;

         CHAR           szName[ _MAX_PATH];

         HFILE          hfile = -1;
         ULONG          ulAction;
         ULONG          ulBytesRead;
         ULONG          ulFilePtr;

do
   {
   // check parms
   if ((!pszName)    ||
       (!pvBuffer))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // copy name and replace ?: with bootdrive
   strcpy( szName, pszName);
   __PatchBootDrive( szName);


   // open the file and read it
   rc = DosOpen( szName,
                 &hfile,
                 &ulAction,
                 0,
                 FILE_NORMAL,
                 OPEN_ACTION_FAIL_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS,
                 OPEN_ACCESS_READONLY | OPEN_SHARE_DENYWRITE,
                 (PEAOP2)NULL);
   if (rc != NO_ERROR)
      break;

   // seek to offset
   if (ulOffset)
      {
      rc = DosSetFilePtr( hfile, ulOffset, FILE_BEGIN, &ulFilePtr);
      if (rc != NO_ERROR)
         break;
      }

   // read file
   rc = DosRead( hfile, pvBuffer, ulBuflen, &ulBytesRead);
   if ((rc != NO_ERROR) || (ulBuflen != ulBytesRead))
      break;

   } while (FALSE);

// cleanup
DosClose( hfile);
return rc;

}

// ---------------------------------------------------------------------------

/*
@@WtkReadFile@SYNTAX
This function reads a file completely into memory.

@@WtkReadFile@PARM@pszName@in
Address of the ASCIIZ pathname, which is to be read into memory.
:p.
The path name may contain :hp2.?&colon.:ehp2. for the drive in order to
read the file from the boot drive.

@@WtkReadFile@PARM@ppszBuffer@out
The address of a pointer variable to a buffer, into which the full file
is being read into. The memory allocated by WtkReadFile must be freed
by the caller using free().

@@WtkReadFile@PARM@pulBuflen@out
The address of a variable containing the length of the file, and thus the buffer.

@@WtkReadFile@RETURN
Return Code.
:p.
WtkReadFile returns one of the following return codes&colon.
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
:li.DosOpen
:li.DosRead
:eul.


@@WtkReadFile@REMARKS
The memory allocated by WtkReadFile must be freed
by the caller using free().
:p.
The buffer returned containing the file contents is appended
by an extra zero byte in order to ease handling of text files.
:p.
This function supports boot drive recognition.
Specify :hp2.?&colon.:ehp2. for the drive in order to
read the file from the boot drive.


@@
*/

APIRET APIENTRY WtkReadFile( PSZ pszName, PSZ *ppszBuffer, PULONG pulBuflen)

{
         APIRET         rc = NO_ERROR;
         CHAR           szName[ _MAX_PATH];

         PVOID          pvData = NULL;
         ULONG          ulFileSize;

do
   {
   // check parms
   if ((!pszName)    ||
       (!ppszBuffer) ||
       (!pulBuflen))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // copy name and replace ?: with bootdrive
   strcpy( szName, pszName);
   __PatchBootDrive( szName);

   // determine size of file as default
   ulFileSize = WtkQueryFileSize ( szName);

   // get memory
   *ppszBuffer = malloc( ulFileSize + 1);
   if (!*ppszBuffer)
      {
      rc = ERROR_NOT_ENOUGH_MEMORY;
      break;
      }
   *(*ppszBuffer + ulFileSize) = 0;

   // read complete file
   rc = WtkReadFilePart( pszName, 0, *ppszBuffer, ulFileSize);
   if (rc != NO_ERROR)
      break;

   // report filesize
   *pulBuflen = ulFileSize;

   } while (FALSE);


// cleanup
if ((rc != NO_ERROR) && (*ppszBuffer))
   {
   free( *ppszBuffer);
   *ppszBuffer = 0;
   }
return rc;
}

// ---------------------------------------------------------------------------

/*
@@WtkWriteFile@SYNTAX
This function writes a data buffer to a file. The file can
either be overwritten or the data can be appended to the file.

@@WtkWriteFile@PARM@pszName@in
Address of the ASCIIZ pathname, which is to be overwritten
by the given data.
:p.
The path name may contain :hp2.?&colon.:ehp2. for the drive in order to
write the file on the boot drive.

@@WtkWriteFile@PARM@pszBuffer@in
The address of a buffer, which is to be written to a file.

@@WtkWriteFile@PARM@ulBuflen@in
The length, in bytes, of the buffer described by :hp1.pszBuffer:ehp1..

@@WtkWriteFile@PARM@fAppend@in
Flag for appending the data to a file rather than overwriting the
contents of an existing file.

@@WtkWriteFile@RETURN
Return Code.
:p.
WtkWriteFile returns one of the following return codes&colon.
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
:li.DosWrite
:eul.


@@WtkWriteFile@REMARKS
This function supports boot drive recognition.
Specify :hp2.?&colon.:ehp2. for the drive in order to
write the file on the boot drive.


@@
*/

APIRET APIENTRY WtkWriteFile( PSZ pszName, PSZ pszBuffer, ULONG ulBuflen, BOOL fAppend)

{
         APIRET         rc = NO_ERROR;

         CHAR           szName[ _MAX_PATH];
         ULONG          ulFileSize;

         HFILE          hfile = -1;
         ULONG          ulAction;
         ULONG          ulBytesWritten;
         ULONG          ulFilePos;

do
   {
   // check parms
   if ((!pszName)   ||
       (!pszBuffer))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // copy name and replace ?: with bootdrive
   strcpy( szName, pszName);
   __PatchBootDrive( szName);

   // open the file and read it
   rc = DosOpen( szName,
                 &hfile,
                 &ulAction,
                 0,
                 FILE_NORMAL,
                 (fAppend) ?
                    OPEN_ACTION_CREATE_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS     :
                    OPEN_ACTION_CREATE_IF_NEW | OPEN_ACTION_REPLACE_IF_EXISTS,
                 OPEN_ACCESS_WRITEONLY | OPEN_SHARE_DENYWRITE,
                 (PEAOP2)NULL);
   if (rc != NO_ERROR)
      break;

   // seek to end of file to append
   if (fAppend)
      {
      rc = DosSetFilePtr( hfile, 0, FILE_END, &ulFilePos);
      if (rc != NO_ERROR)
         break;
      }

   // write file
   rc = DosWrite( hfile, pszBuffer, ulBuflen, &ulBytesWritten);
   if ((rc != NO_ERROR) || (ulBuflen != ulBytesWritten))
      break;

   } while (FALSE);

// cleanup
DosClose( hfile);
return rc;

}

// ---------------------------------------------------------------------------

/*
@@WtkFileIsEmpty@SYNTAX
This function checks, if a text file is empty. This is the case, if the
file does not exist or is smaller than 1Kb of size and contains
only whitespace.

@@WtkFileIsEmpty@PARM@pszName@in
Address of the ASCIIZ path name of the file to be checked
:p.
The path name may contain :hp2.?&colon.:ehp2. for the drive in order to
search the file on the boot drive.
:p.
The name may not include wildcards.

@@WtkFileIsEmpty@RETURN
Success indicator.
:parml compact.
:pt.TRUE
:pd.The specified file is considered being empty
:pt.FALSE
:pd.The specified file is considered not being empty
:eparml.

@@WtkFileIsEmpty@REMARKS
This function is intended to detect, if a given text file 
contains only empty lines, and otherwise is empty.

@@
*/

BOOL APIENTRY WtkFileIsEmpty( PSZ pszName)
{
         APIRET         rc = NO_ERROR;
         PSZ            p;

         BOOL           fResult = TRUE;
         CHAR           szName[ _MAX_PATH];

         ULONG          ulFilesize;
         PSZ            pszFileContents = NULL;


do
   {
   // check parameters
   if ((pszName  == NULL) ||
       (*pszName == 0))
      break;

   // copy name and replace ?: with bootdrive
   strcpy( szName, pszName);
   __PatchBootDrive( szName);

   // query the size
   ulFilesize = WtkQueryFileSize( szName);
   if (!ulFilesize)
      break;

   // for sizes larger than one KB, consider file not empty
   if (ulFilesize > 1024)
      break;

   // read contents
   rc = WtkReadFile( szName, &pszFileContents, &ulFilesize);
   if (rc != NO_ERROR)
      {
      fResult = FALSE;
      break;
      }

   // check for non-whitespace
   fResult = TRUE;
   p = pszFileContents;
   while (*p)
      {
      if (*p > 32)
         {
         fResult = FALSE;
         break;
         }
       p++;
       }


   } while (FALSE);

// cleanup
if (pszFileContents) free( pszFileContents);
return fResult;
}

