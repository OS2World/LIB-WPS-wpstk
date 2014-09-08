/****************************** Module Header ******************************\
*
* Module Name: wtkufil.c
*
* Source for file and directory helper functions
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: wtkufil7.c,v 1.4 2005-03-05 20:28:35 cla Exp $
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
@@WtkDeleteFile@SYNTAX
This function deletes a file, despite of any attributes set.

@@WtkDeleteFile@PARM@pszName
Address of the name of the file.
:p.
The file name may contain :hp2.?&colon.:ehp2.
for the drive in order to delete the file from the boot drive.

@@WtkDeleteFile@RETURN
Return Code.
:p.
WtkDeleteFile returns one of the following return codes&colon.

:parml compact break=none.
:pt.0
:pd.NO_ERROR
:pt.87
:pd.ERROR_INVALID_PARAMETER
:eparml.
:p.
or return codes of the following functions
:ul compact.
:li.DosDelete
:eul.

@@WtkDeleteFile@REMARKS
In opposite to DosDelete, this function resets all attributes
before deleting it.
:p.
This function supports boot drive recognition.
Specify :hp2.?&colon.:ehp2. for the drive in order to
delete the file from the boot drive.

@@
*/

APIRET APIENTRY WtkDeleteFile( PSZ pszName)
{

         APIRET         rc = NO_ERROR;
         CHAR           szName[ _MAX_PATH];

         FILESTATUS3    fs3;

do
   {
   // check parms
   if ((!pszName) ||
       (!*pszName))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // copy name and replace ?: with bootdrive
   strcpy( szName, pszName);
   __PatchBootDrive( szName);

   // determine attributes
   rc = DosQueryPathInfo( szName, FIL_STANDARD, &fs3, sizeof( fs3));
   if (rc != NO_ERROR)
      break;

   // remove attributes
   fs3.attrFile = 0;
   rc = DosSetPathInfo( szName, FIL_STANDARD, &fs3,  sizeof( fs3), 0);
   if (rc != NO_ERROR)
      break;

   // finally delete it
   rc = DosDelete( szName);
   if (rc != NO_ERROR)
      break;

   } while (FALSE);

return rc;
}

// ---------------------------------------------------------------------------

/*
@@WtkMoveFile@SYNTAX
This function moves a file. If a file has to be moved
across partitions, it is copied instead.

@@WtkMoveFile@PARM@pszOld
Address of the old name of the file.
:p.
The file name may contain :hp2.?&colon.:ehp2.
for the drive in order to move the file from the boot drive.

@@WtkMoveFile@PARM@pszNew
Address of the new name of the file.
:p.
The file name may contain :hp2.?&colon.:ehp2.
for the drive in order to move the file to the boot drive.

@@WtkMoveFile@RETURN
Return Code.
:p.
WtkMoveFile returns one of the following return codes&colon.

:parml compact break=none.
:pt.0
:pd.NO_ERROR
:pt.87
:pd.ERROR_INVALID_PARAMETER
:eparml.
:p.
or return codes of the following functions
:ul compact.
:li.DosMove
:li.DosCopy
:li.DosDelete
:eul.

@@WtkMoveFile@REMARKS
In opposite to DosMove, this function can also move files
across partition boundaries by in that case copying the old
to the new file and then deleting the old file.
:p.
This function supports boot drive recognition.
Specify :hp2.?&colon.:ehp2. for the drive in order to
to move the file from ot to the boot drive.

@@
*/

APIRET APIENTRY WtkMoveFile( PSZ pszOld, PSZ pszNew)
{

         APIRET         rc = NO_ERROR;
         CHAR           szOld[ _MAX_PATH];
         CHAR           szNew[ _MAX_PATH];
         ULONG          ulDriveLen;

do
   {
   // check parms
   if ((!pszOld) ||
       (!*pszOld) ||
       (!pszNew) ||
       (!*pszNew))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // copy name and replace ?: with bootdrive
   strcpy( szOld, pszOld);
   __PatchBootDrive( szOld);
   strcpy( szNew, pszNew);
   __PatchBootDrive( szNew);

  // determine full pathnames
  rc = DosQueryPathInfo( szOld, FIL_QUERYFULLNAME, szOld ,sizeof( szOld));
  if (rc != NO_ERROR)
     break;
  rc = DosQueryPathInfo( szNew, FIL_QUERYFULLNAME, szNew ,sizeof( szNew));
  if (rc != NO_ERROR)
     break;

   // is drive equal
   ulDriveLen = strchr( szOld, ':') - &szOld[ 0] + 1;
   if (!strnicmp( szOld, szNew, ulDriveLen))
     {
     // then move on same partition directly
     rc = DosMove( szOld, szNew);
     break;
     }

   // files are on different partitions, move and delete
   rc = DosCopy( szOld, szNew, DCPY_EXISTING); 
   if (rc != NO_ERROR)
      break;

   rc = DosDelete( szOld);
   if (rc != NO_ERROR)
      break;


   } while (FALSE);

return rc;
}

