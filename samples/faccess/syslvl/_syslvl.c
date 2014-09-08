/****************************** Module Header ******************************\
*
* Module Name: _syslvl.c
*
* SYSLEVEL file access functions sample
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: _syslvl.c,v 1.9 2008-02-08 20:25:09 cla Exp $
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

#define INCL_WTKFILESYSLEVEL
#define INCL_WTKUTLFILE
#include <wtk.h>

// -----------------------------------------------------------------------------

#define MAX_TEMP_FILES 5
#define PRINTSEPARATOR printf("\n------------------------------------------\n")

// -----------------------------------------------------------------------------

static VOID _dumpdata( PSZ pszTestFile, PSYSLEVELINFO psli)
{
printf( "- WtkQuerySyslevelInfo: data retrieved\n"
        "\n"
        "%s\n"
        "--------------------------------------\n"
        "   system id: %X\n"
        "component id: %s\n"
        "        name: %s\n"
        "     version: %x.%x.%x\n"
        "  CSD prefix: %s\n"
        "CSD language: %c\n"
        " CSD current: %s\n"
        "CSD previous: %s\n"
        "    CSD date: %u\n"
        "   app. data: %s\n",
        pszTestFile,
        psli->usSysId,
        psli->szComponentId,
        psli->szName,
        psli->bVersionMajor,
        psli->bVersionMinor,
        psli->bVersionRefresh,
        psli->szCsdPrefix,
        psli->chCsdLanguage,
        psli->szCurrentCsd,
        psli->szPreviousCsd,
        psli->usDate,
        psli->szAppType);
return;
}

// -----------------------------------------------------------------------------

int main ( int argc, char *argv[])
{

         APIRET         rc = NO_ERROR;
         CHAR           szTestFile[ _MAX_PATH];
         HSYSLEVEL      hsl = NULLHANDLE;
         SYSLEVELINFO   sli;



do
   {

   // check parameter
   if (argc < 2)
      {
      printf( "error: no syslevel file specified\n");
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

   printf( "\nOpening syslevel file: %s\n", szTestFile);
   rc = WtkOpenSyslevel( szTestFile, &hsl);
   if (rc == NO_ERROR)
      printf( "- WtkOpenSyslevel: file opened, handle: %u\n", hsl);
   else
      {
      printf( "- error: WtkOpenSyslevel: file %s could NOT be read. rc=%u\n", szTestFile, rc);
      break;
      }

   // ======================================================================

   PRINTSEPARATOR;
   printf( "\nretrieving data\n");
   rc = WtkQuerySyslevelInfo( hsl, &sli);
   if (rc == NO_ERROR)
      _dumpdata( szTestFile, &sli);
   else
      {
      printf( "- error: WtkQuerySyslevelInfo: error retrieving syslevel data. rc=%u\n", rc);
      break;
      }

   // ======================================================================

   PRINTSEPARATOR;

   sli.usSysId = 0x4948;
   sli.bVersionMajor     = 0x05;
   sli.bVersionMinor     = 0x43;
   sli.bVersionRefresh   = 0x02;
   sli.chCsdLanguage     = 'G';
   strncpy( sli.szComponentId, "NEWCOMP",  WTK_SYSLEVEL_MAXLEN_COMPID);
   strncpy( sli.szName,        "test app", WTK_SYSLEVEL_MAXLEN_NAME);
   strncpy( sli.szCurrentCsd,  "2223",     WTK_SYSLEVEL_MAXLEN_CSDNUM);
   printf( "\nmodifying data\n");
   rc = WtkSetSyslevelInfo( hsl, WTK_SYSLEVEL_UPDATE_ALL, &sli);
   if (rc == NO_ERROR)
      printf( "- WtkSetSyslevelInfo: syslevel data successfully updated\n");
   else
      {
      printf( "- error: WtkSetSyslevelInfo: data could not be updated. rc=%u\n", rc);
      break;
      }

   // ======================================================================

   PRINTSEPARATOR;
   printf( "\nrereading data\n");
   rc = WtkQuerySyslevelInfo( hsl, &sli);
   if (rc == NO_ERROR)
      _dumpdata( szTestFile, &sli);
   else
      {
      printf( "- error: WtkQuerySyslevelInfo: error retrieving syslevel data. rc=%u\n", rc);
      break;
      }

   // ======================================================================

   PRINTSEPARATOR;

   printf( "\nClosing syslevel file\n");
   rc = WtkCloseSyslevel( hsl, WTK_SYSLEVEL_CLOSE_UPDATE);
   if (rc == NO_ERROR)
      {
      printf( "- WtkCloseSyslevel: file closed\n");
      hsl = NULLHANDLE;
      break;
      }
   else
      {
      printf( "- error: WtkCloseSyslevel: error closing file, rc=%u\n", rc);
      break;
      }



   } while (FALSE);

// close file
if (hsl)
   WtkCloseSyslevel( hsl, WTK_SYSLEVEL_CLOSE_DISCARD);
return rc;
}

