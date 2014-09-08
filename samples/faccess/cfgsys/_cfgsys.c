/****************************** Module Header ******************************\
*
* Module Name: _cfgsys.c
*
* CONFIG.SYS file access functions sample
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: _cfgsys.c,v 1.6 2004-05-01 13:10:25 cla Exp $
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

#define INCL_WTKFILECFGSYS
#define INCL_WTKUTLFILE
#include <wtk.h>

// -----------------------------------------------------------------------------

#define MAX_TEMP_FILES 5
#define PRINTSEPARATOR  printf("\n------------------------------------------\n")
#define PRINTSEPARATOR2 printf("----------------------------\n")


// -----------------------------------------------------------------------------

int main ( int argc, char *argv[])
{

         APIRET         rc = NO_ERROR;

static   CHAR           szConfigFile[ _MAX_PATH];
static   CHAR           szBackupFile[ _MAX_PATH];

         PSZ            pszUpdateFile = argv[ 1];

         PSZ            pszUpdate = NULL;
         ULONG          ulUpdateLen;

do
   {

   // check parameter
   if (argc < 2)
      {
      printf( "error: no file with update data specified\n");
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // ======================================================================

   PRINTSEPARATOR;

   printf( "\ncreating backup of config.sys\n");

   rc = WtkQueryFullname( "?:\\CONFIG.SYS", szConfigFile, sizeof( szConfigFile));
   if (rc != NO_ERROR)
      {
      printf( "- error: WtkQueryFullName: cannot determine CONFIG.SYS.\n");
      break;
      }

   rc = WtkCreateTmpFile( "config.???", szBackupFile, sizeof( szBackupFile));
   if (rc != NO_ERROR)
      {
      printf( "- error: WtkCreateTmpFile: cannot create backup file\n");
      break;
      }

   rc = DosCopy( szConfigFile, szBackupFile, DCPY_EXISTING);
   if (rc != NO_ERROR)
      {
      printf( "- error: DosCopy: cannot create backup\n");
      break;
      }

   // ======================================================================

   PRINTSEPARATOR;

   printf( "\nreading file: %s\n", pszUpdateFile);
   rc = WtkReadFile( pszUpdateFile, &pszUpdate, &ulUpdateLen);
   if (rc != NO_ERROR)
      {
      printf( "- error: WtkReadFile: update data could not be read.\n");
      break;
      }
   PRINTSEPARATOR2;
   printf( "%s", pszUpdate);
   PRINTSEPARATOR2;


   printf( "\nproccessing CONFIG.SYS update\n", pszUpdateFile);
   rc = WtkUpdateConfigsys( pszUpdate, WTK_CFGSYS_UPDATE_ADD, NULL);
   if (rc != NO_ERROR)
      {
      printf( "- error: WtkUpdateConfigsys: could not update config.sys, rc=%u\n", rc);
      break;
      }

   printf( "\nproccessing CONFIG.SYS update removal\n", pszUpdateFile);
   rc = WtkUpdateConfigsys( pszUpdate, WTK_CFGSYS_UPDATE_DELETE, NULL);
   if (rc != NO_ERROR)
      {
      printf( "- error: WtkUpdateConfigsys: could not update config.sys, rc=%u\n", rc);
      break;
      }

   // ======================================================================

   PRINTSEPARATOR;

   printf( "\nrestoring backup of config.sys\n");
   rc = DosCopy( szBackupFile, szConfigFile, DCPY_EXISTING);
   if (rc != NO_ERROR)
      {
      printf( "- error: DosCopy: cannot restore backup\n");
      break;
      }

   WtkDeleteFile( szBackupFile);

   // ======================================================================

   PRINTSEPARATOR;

   } while (FALSE);

// cleanup
if (pszUpdate) free( pszUpdate);
return rc;
}
