/****************************** Module Header ******************************\
*
* Module Name: _init.c
*
* text initialization file access functions sample
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: _init.c,v 1.3 2004-04-15 21:27:50 cla Exp $
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

#define INCL_WTKFILEINIT
#define INCL_WTKUTLFILE
#include <wtk.h>

// -----------------------------------------------------------------------------

#define MAX_TEMP_FILES 5
#define PRINTSEPARATOR printf("\n------------------------------------------\n")

#define NEXTSTRING(s)  (s+strlen(s)+1)

// -----------------------------------------------------------------------------

static VOID _displayList( PSZ pszList)
{
if (pszList)
   {
   while (*pszList)
      {
      printf( "%s ", pszList);
      pszList = NEXTSTRING( pszList);
      }
   printf( "\n");
   }
}

// -----------------------------------------------------------------------------

int main ( int argc, char *argv[])
{

         APIRET         rc = NO_ERROR;
         BOOL           fResult;
         PSZ            p;

         CHAR           szTestFile[ _MAX_PATH];
         CHAR           szBackupFile[ _MAX_PATH];
         HINIT          hinit = NULLHANDLE;
         INITPARMS      ip;

         PSZ            pszSection;
         PSZ            pszKey;
         PSZ            pszKeyValue;

         CHAR           szKeyValue[ _MAX_PATH];
         ULONG          ulValueLen;

do
   {

   // check parameter
   if (argc < 2)
      {
      printf( "error: no initialization file specified\n");
      rc = ERROR_INVALID_PARAMETER;
      break;
      }
   printf( "\nusing commandline parameter %s as source filename\n", argv[ 1]);
   rc = WtkQueryFullname( argv[ 1], szTestFile, sizeof( szTestFile));
   if (rc != NO_ERROR)
      {
      printf( "\nerror: WtkQueryFullname: cannot query fullname of parameter. rc=%u\n", rc);
      break;
      }
   else
      printf( "\nWtkQueryFullname: query fullname of %s\n  %s\n", argv[ 1], szTestFile);

   // ======================================================================

   PRINTSEPARATOR;

   printf( "\nOpening initialization file: %s\n", szTestFile);
   memset( &ip, 0, sizeof( ip));
   rc = WtkOpenInitProfile( szTestFile, &hinit, WTK_INIT_OPEN_READWRITE, 0, NULL);
   if (rc == NO_ERROR)
      printf( "- WtkOpenInitProfile: file opened, handle: %u\n", hinit);
   else
      {
      printf( "- error: WtkOpenInitProfile: file %s could NOT be read. rc=%u\n", szTestFile, rc);
      break;
      }

   // ======================================================================

   PRINTSEPARATOR;

   pszSection = "SECTION1";
   pszKey     = "Key2";
   printf( "\nquerying value for: %s - %s\n", pszSection, pszKey);
   ulValueLen = WtkQueryInitProfileString( hinit, pszSection, pszKey, NULL, szKeyValue, sizeof( szKeyValue));
   if (ulValueLen > 0)
      printf( "- WtkQueryInitProfileString: value is: %s\n", szKeyValue);
   else
      {
      printf( "- error: WtkQueryInitProfileString: key could not be read.\n");
      rc = ERROR_INVALID_DATA;
      break;
      }

   // ======================================================================

   PRINTSEPARATOR;

   printf( "\ncheck modification status\n");
   fResult = WtkInitProfileModified( hinit);
   printf( "- WtkInitProfileModified: text initialization file HAS %sbeen modified\n", (fResult) ? "" : "NOT ");


   // ======================================================================

   PRINTSEPARATOR;

   pszSection  = "SECTION1";
   pszKey      = "KeyX";
   pszKeyValue = "valuex";

   printf( "\nsetting value of new key: %s - %s to : %s\n", pszSection, pszKey, pszKeyValue);
   rc = WtkWriteInitProfileString( hinit, pszSection, pszKey, pszKeyValue);
   if (rc == NO_ERROR)
      printf( "- WtkWriteInitProfileString: key written successfully.\n");
   else
      {
      printf( "- error: WtkWriteInitProfileString: key could not be written. rc=%u\n", rc);
      rc = ERROR_INVALID_DATA;
      break;
      }

   // ======================================================================

   PRINTSEPARATOR;

   pszSection  = "SECTIONNEW";
   pszKey      = "KeyNew";
   pszKeyValue = "valueNew";

   printf( "\nsetting value of new key in new section: %s - %s to : %s\n", pszSection, pszKey, pszKeyValue);
   rc = WtkWriteInitProfileString( hinit, pszSection, pszKey, pszKeyValue);
   if (rc == NO_ERROR)
      printf( "- WtkWriteInitProfileString: key written successfully.\n");
   else
      {
      printf( "- error: WtkWriteInitProfileString: key could not be written. rc=%u\n", rc);
      rc = ERROR_INVALID_DATA;
      break;
      }

   // ======================================================================

   PRINTSEPARATOR;

   pszSection = "SECTION1";

   printf( "\nquerying all sections\n");
   ulValueLen = WtkQueryInitProfileString( hinit, NULL, NULL, NULL, szKeyValue, sizeof( szKeyValue));
   if (ulValueLen > 0)
      {
      printf( "- WtkQueryInitProfileString: section list is: ");
      _displayList( szKeyValue);
      }
   else
      {
      printf( "- error: WtkQueryInitProfileString: section list could not be read.\n");
      rc = ERROR_INVALID_DATA;
      break;
      }

   printf( "\nquerying list of all keys in section %s\n", pszSection);
   ulValueLen = WtkQueryInitProfileString( hinit, pszSection, NULL, NULL, szKeyValue, sizeof( szKeyValue));
   if (ulValueLen > 0)
      {
      printf( "- WtkQueryInitProfileString: key list for section %s is: ", pszSection);
      _displayList( szKeyValue);
      }
   else
      {
      printf( "- error: WtkQueryInitProfileString: key list could not be read.\n");
      rc = ERROR_INVALID_DATA;
      break;
      }

   printf( "\nquery profile size of all keys in section: %s\n", pszSection);
   fResult = WtkQueryInitProfileSize( hinit, pszSection, NULL, &ulValueLen);
   if (fResult)
      printf( "- WtkQueryInitProfileSize: size of all keynames in section %s is %u\n", pszSection, ulValueLen);
   else
      {
      printf( "- error: WtkQueryInitProfileSize:: could not query profile size\n");
      rc = ERROR_INVALID_DATA;
      break;
      }

   // ======================================================================

   PRINTSEPARATOR;

   printf( "\ncheck modification status\n");
   fResult = WtkInitProfileModified( hinit);
   printf( "- WtkInitProfileModified: text initialization file HAS %sbeen modified\n", (fResult) ? "" : "NOT ");

   // ======================================================================

   PRINTSEPARATOR;
   strcpy( szBackupFile, szTestFile);
   p = WtkFilespec( szBackupFile, WTK_FILESPEC_EXTENSION);
   strcpy( p, "bak");

   printf( "\nClosing initialization file, writing backup file %s\n", szBackupFile);
   rc = WtkCloseInitProfileBackup( hinit, TRUE, szBackupFile);
   if (rc == NO_ERROR)
      {
      printf( "- WtkCloseInitProfileBackup: file closed\n");
      hinit = NULLHANDLE;
      break;
      }
   else
      {
      printf( "- error: WtkCloseInitProfileBackup: error closing file, rc=%u\n", rc);
      break;
      }



   } while (FALSE);

// close file
if (hinit)
   WtkCloseInitProfile( hinit, WTK_INIT_CLOSE_DISCARD);
return rc;
}

