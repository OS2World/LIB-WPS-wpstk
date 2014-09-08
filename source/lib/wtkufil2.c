/****************************** Module Header ******************************\
*
* Module Name: wtkufil2.c
*
* Source for file and directory helper functions
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: wtkufil2.c,v 1.2 2003-04-24 14:33:30 cla Exp $
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
@@WtkCreatePath@SYNTAX
This function creates a complete path.
An error is returned only, if the last subdirectory in the pathname
could not be created.

@@WtkCreatePath@PARM@pszPath@in
Address of the ASCIIZ path name of the path to be created.
:p.
The path name may contain :hp2.?&colon.:ehp2. for the drive in order to
create the path on the boot drive.

@@WtkCreatePath@RETURN
Return Code.
:p.
WtkCreatePath returns one of the following return codes&colon.

:parml compact break=none.
:pt.0
:pd.NO_ERROR
:pt.87
:pd.ERROR_INVALID_PARAMETER
:eparml.
:p.
or return codes of the following functions
:ul compact.
:li.DosCreateDir
:eul.

@@WtkCreatePath@REMARKS
WtkCreatePath tries to create all directories within the path.
An error is returned only, if the last subdirectory in the pathname
could not be created.
:p.
This function supports boot drive recognition.
Specify :hp2.?&colon.:ehp2. for the drive in order to
create the path on the boot drive.

@@
*/

APIRET APIENTRY WtkCreatePath( PSZ pszPath)

{

         BOOL           fIsModified = TRUE;
         APIRET         rc = NO_ERROR;

         CHAR           szTmp[ _MAX_PATH];
         PSZ            pszEndOfPath;

do
   {
   // check parms
   if ((!pszPath) || (!pszPath))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // use a copy
   strcpy( szTmp, pszPath);

   // copy name and replace ?: with bootdrive
   __PatchBootDrive( szTmp);

   // skip over a drive letter
   // (or whatever will be before a colon in the future)
   pszEndOfPath = strchr( szTmp, ':');
   if (pszEndOfPath)
      pszEndOfPath++;
   else
      pszEndOfPath = szTmp;

   // if UNC name, skip server name and alias
   if (!strncmp( pszEndOfPath, "\\\\", 2))
      {
      // no server name given ?
      // error !
      pszEndOfPath = strchr( pszEndOfPath + 2, '\\');
      if (!pszEndOfPath)
         {
         rc = ERROR_INVALID_PARAMETER;
         break;
         }

      // no path specified after aliasname ?
      // abort with no error
      pszEndOfPath = strchr( pszEndOfPath + 1, '\\');
      if (!pszEndOfPath)
         break;
      pszEndOfPath++;
      if (!*pszEndOfPath)
         break;
      }

   // go through all path components
   pszEndOfPath = strchr( pszEndOfPath, '\\');
   while (pszEndOfPath != NULL)
      {
      // go through all path components
      // ignore errors here !
      *pszEndOfPath = 0;
      rc = DosCreateDir( szTmp, NULL);
      *pszEndOfPath = '\\';
      pszEndOfPath = strchr( pszEndOfPath + 1, '\\');
     }

   // create final directory and
   // return error here
   rc = DosCreateDir( szTmp, NULL);

   } while (FALSE);

return rc;

}

// ---------------------------------------------------------------------------

/*
@@WtkDeletePath@SYNTAX
This function deletes a complete path. An error is returned
only, if the last subdirectory in the pathname
could not be deleted.

@@WtkDeletePath@PARM@pszPath@in
Address of the ASCIIZ path name of the path to be deleted.
:p.
The path name may contain :hp2.?&colon.:ehp2. for the drive in order to
delete the path on the boot drive.

@@WtkDeletePath@RETURN
Return Code.
:p.
WtkDeletePath returns one of the following return codes&colon.

:parml compact break=none.
:pt.0
:pd.NO_ERROR
:pt.87
:pd.ERROR_INVALID_PARAMETER
:eparml.
:p.
or return codes of the following functions
:ul compact.
:li.DosDeleteDir
:eul.

@@WtkDeletePath@REMARKS
WtkDeletePath tries to delete all directories within the path.
An error is returned only, if the last subdirectory in the pathname
could not be deleted.
:p.
This function supports boot drive recognition.
Specify :hp2.?&colon.:ehp2. for the drive in order to
delete the file on the boot drive.

@@
*/

APIRET APIENTRY WtkDeletePath( PSZ pszPath)

{

         APIRET         rc = NO_ERROR;

         CHAR           szTmp[ _MAX_PATH];
         PSZ            pszEndOfPath;

do
   {
   // check parms
   if ((!pszPath) || (!pszPath))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // use a copy
   strcpy( szTmp, pszPath);

   // replace ?: with bootdrive
   __PatchBootDrive( szTmp);

   // now remove top level directory
   // return error, if that fails
   rc = DosDeleteDir( szTmp);
   if (rc != NO_ERROR)
      break;

   // go through all path components
   pszEndOfPath = strrchr( szTmp, '\\');
   while (pszEndOfPath != NULL)
      {
      // go through all path components
      // ignore errors here, as the calls will not
      // do any harm
      *pszEndOfPath = 0;
      rc = DosDeleteDir( szTmp);
      if (rc != NO_ERROR)
         break;
      pszEndOfPath = strrchr( szTmp, '\\');
     }

   // ignore last errors
   rc = NO_ERROR;

   } while (FALSE);

return rc;

}


