/****************************** Module Header ******************************\
*
* Module Name: wtkufil6.c
*
* Source for file and directory helper functions
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: wtkufil6.c,v 1.3 2009-11-17 22:00:34 cla Exp $
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
@@WtkFileModified@SYNTAX
This function detects modifications of a file or directory.

@@WtkFileModified@PARM@pszName@in
Address of the ASCIIZ path name of the file or directory,
which is to be checked for modifications.
:p.
The path name may contain :hp2.?&colon.:ehp2. for the drive in order to
search the file or directory on the boot drive.

@@WtkFileModified@PARM@pfs3@in
Address of a FILESTATUS3 structure previously being filled by
DosQueryPathInfo or WtkFileModified for the same file. Every call
to WtkFileModified will update that structure, if the file has
modified.

@@WtkFileModified@RETURN
Modification indicator.
:parml compact.
:pt.TRUE
:pd.File has been modified
:pt.FALSE
:pd.
:ul compact.
:li.File has not been modified or could not be found
:li.An invalid parameter has been specified.
:eul.
:eparml.

@@WtkFileModified@REMARKS
Before calling WtkFileModified, the caller must get the
:hp2.FIL_STANDARD:ehp2. information from DosQueryPathInfo
for the file or directory in order to be able to determine,
wether the file has been modified or not. Also the file status
data may have been updated by a previous call to WtkFileModified.
:p.
Note that modifications of the date and time of last access
in the FILESTATUS3 structure are ignored.
:p.
This function supports boot drive recognition.
Specify :hp2.?&colon.:ehp2. for the drive in order to
search the file or directory on the boot drive.

@@
*/

BOOL APIENTRY WtkFileModified( PSZ pszName, PFILESTATUS3 pfs3)
{
         BOOL           fIsModified = TRUE;
         APIRET         rc = NO_ERROR;
         ULONG          ulBytesWritten;
         FILESTATUS3    fs3Current;
         CHAR           szName[ _MAX_PATH];

do
   {
   // check parms
   if ((pszName == NULL) ||
       (pfs3    == NULL))
      break;

   // copy name and replace ?: with bootdrive
   strcpy( szName, pszName);
   __PatchBootDrive( szName);

   // aktuellen Status holen
   rc = DosQueryPathInfo( szName,
                          FIL_STANDARD,
                          &fs3Current,
                          sizeof( fs3Current));
   if (rc != NO_ERROR)
      break;

   // copy over last access
   memcpy( &(pfs3->fdateLastAccess),
           &(fs3Current.fdateLastAccess),
           sizeof( FDATE) + sizeof( FTIME));

   // check date & time fields
   if ( memcmp( pfs3, &fs3Current, sizeof( FILESTATUS3)) == 0)
      {
      // file has not changed
      fIsModified = FALSE;
      }
   else
      {
      // copy file status
      memcpy( pfs3, &fs3Current, sizeof( FILESTATUS3));
      }

   } while (FALSE);

return fIsModified;

}

// ---------------------------------------------------------------------------

/*
@@WtkQueryFullname@SYNTAX
This function queries the full name of a file or directory.

@@WtkQueryFullname@PARM@pszName@in
Address of the ASCIIZ path name of the file or direcory.
:p.
The path name may contain :hp2.?&colon.:ehp2. for the drive in order to
search the file on the boot drive.
:p.
The name may include wildcards.

@@WtkQueryFullname@PARM@pszBuffer@out
The address of a buffer in into which the full name
of the given file or directory is returned.

@@WtkQueryFullname@PARM@ulBuflen@in
The length, in bytes, of the buffer described by :hp1.pszBuffer:ehp1..

@@WtkQueryFullname@RETURN
Return Code.
:p.
WtkQueryFullname returns one of the following return codes&colon.

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
:li.DosQueryPathInfo
:eul.

@@WtkQueryFullname@REMARKS
This function supports boot drive recognition.
Specify :hp2.?&colon.:ehp2. for the drive in order to
search the file on the boot drive.

@@
*/

APIRET APIENTRY WtkQueryFullname( PSZ pszName, PSZ pszBuffer, ULONG ulBuflen)
{
         APIRET         rc = NO_ERROR;
         CHAR           szName[ _MAX_PATH];
         FILESTATUS3    fs3;

do
   {
   // check parameters
   if ((pszName  == NULL) ||
       (*pszName == 0))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // copy name and replace ?: with bootdrive
   strcpy( szName, pszName);
   __PatchBootDrive( szName);

   // search entry
   rc = DosQueryPathInfo( szName,
                          FIL_QUERYFULLNAME,
                          &szName,
                          sizeof( szName));
   if (rc != NO_ERROR)
      break;

    // does buffer fit ?
    if (strlen( szName) + 1 > ulBuflen)
       {
       rc = ERROR_BUFFER_OVERFLOW;
       break;
       }

    // hand over result
    strcpy( pszBuffer, szName);

   } while (FALSE);

return rc;

}

