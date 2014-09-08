/****************************** Module Header ******************************\
*
* Module Name: _dynamic.c
*
* dynamic linkage sample
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: _dynamic.c,v 1.2 2003-12-05 00:24:17 cla Exp $
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INCL_ERRORS
#define INCL_DOSFILEMGR
#include <os2.h>

#define INCL_WTKUTLFILE
#include <wtk.h>

// -----------------------------------------------------------------------------

#define PRINTSEPARATOR printf("\n------------------------------------------\n")

int main ( int argc, char *argv[])
{

         APIRET         rc = NO_ERROR;
         CHAR           szVersion[ 16];
         CHAR           szDir[ _MAX_PATH];
         HDIR           hdir;
         PSZ            p;

do
   {
   // ======================================================================

   PRINTSEPARATOR;

   printf( "\nWtkQueryVersion: check version of runtime DLL\n");
   rc = WtkQueryVersion( szVersion, sizeof( szVersion));
   if (rc != NO_ERROR)
      {
      printf( "\nWtkQueryVersion: error: cannot determine version of runtime DLL. rc=%u\n", rc);
      break;
      }
   printf( "WPSTK runtime DLL version: %s\n"
           "         expected version: %s\n",
           szVersion, WPSTK_VERSION);

   if (!strcmp( szVersion, WPSTK_VERSION))
      printf( "version of runtime DLL matches !\n");
   else
      {
      printf( "\nerror: unexpected version of runtime DLL. rc=%u\n", rc);
      break;
      }

   // show all directories below ?:\OS2
   p = "?:\\OS2\\ARCHIVES";
   printf( "\nWtkGetNextDirectory: show all directories below %s\n", p);
   sprintf( szDir, "%s\\*", p);
   hdir = NULLHANDLE; // let WtkGetNextFile start a new search

   do
      {
      // get first/next file
      rc = WtkGetNextDirectory( szDir,  &hdir, szDir, sizeof( szDir));

      // break, if no more files
      if (rc == ERROR_NO_MORE_FILES)
         {
         // everything ok
         rc = NO_ERROR;
         break;
         }

      // break also on real error
      if (rc != NO_ERROR)
         break;

      // show the directory
      printf( "  - %s\n", szDir);
      } while (TRUE);

   if (rc != NO_ERROR)
      break;

   // ======================================================================

   PRINTSEPARATOR;


   } while (FALSE);

return rc;
}

// -----------------------------------------------------------------------------

static APIRET __CleanupPath( PSZ pszTemporaryPath)
{
         APIRET         rc = NO_ERROR;

do
   {
   // check parm
   if (!pszTemporaryPath)
      break;

   // delete path
   rc = WtkDeletePath( pszTemporaryPath);
   if (rc == NO_ERROR)
      printf( "\nWtkDeletePath: deleted temporary path\n  %s\n", pszTemporaryPath);
   else
      printf( "\nWtkDeletePath: error: cannot delete temporary path. rc=%u\n  %s\n", rc, pszTemporaryPath);
   } while (FALSE);

return rc;

}

// -----------------------------------------------------------------------------

static APIRET __TestFilespec( PSZ pszFile)
{
         APIRET         rc = NO_ERROR;
         PSZ            pszFileSpec;

do
   {
   printf( "\nWtkFilespec examining \"%s\":\n", pszFile);
   pszFileSpec = WtkFilespec( pszFile, WTK_FILESPEC_PATHNAME);
   if (pszFileSpec)
      printf( " - path starts at: %s\n", pszFileSpec);
   else
      {
      printf( " - error : path could not be determined\n");
      rc= ERROR_INVALID_FUNCTION;
      }

   pszFileSpec = WtkFilespec( pszFile, WTK_FILESPEC_NAME);
   if (pszFileSpec)
      printf( " - name starts at: %s\n", pszFileSpec);
   else
      {
      printf( " - WtkFilespec: error: name could not be determined\n");
      rc= ERROR_INVALID_FUNCTION;
      }
   pszFileSpec = WtkFilespec( pszFile, WTK_FILESPEC_EXTENSION);
   if (pszFileSpec)
      printf( " - extension starts at: %s\n", pszFileSpec);
   else
      {
      printf( " - WtkFilespec: error: extension could not be determined\n");
      rc= ERROR_INVALID_FUNCTION;
      }

   } while (FALSE);

return rc;

}

