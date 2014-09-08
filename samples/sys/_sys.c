/****************************** Module Header ******************************\
*
* Module Name: _sys.c
*
* system related functions sample
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: _sys.c,v 1.7 2007-02-16 19:28:07 cla Exp $
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
#include <time.h>

#define INCL_ERRORS
#include <os2.h>

#define INCL_WTKUTLSYSTEM
#define INLC_WTKUTLLOCALE
#include <wtk.h>

// -----------------------------------------------------------------------------

VOID _displayRunningUnder( PSZ pszFunctionName, PSZ pszOsName, BOOL fRunningUnder)
{
printf( "\n%s: this program is %srunning under %s\n",
        pszFunctionName, (fRunningUnder) ? "" : "_not_ ", 
        pszOsName, fRunningUnder);
}

// -----------------------------------------------------------------------------

#define PRINTSEPARATOR printf("\n------------------------------------------\n")


int main ( int argc, char *argv[])
{

         APIRET         rc = NO_ERROR;

         ULONG          ulOperatingSystem;
         PSZ            pszOperatingSystem;

         PSZ            pszLangTableFormat;

do
   {

   // ----------------------------------------------------------------------

   PRINTSEPARATOR;

   // use diverse routines to determine the operating system used
   _displayRunningUnder( "WtkIsWarp4",       "OS/2 Warp 4 or later", WtkIsWarp4());
   _displayRunningUnder( "WtkIsOS2",         "OS/2 Warp",            WtkIsOS2());
   _displayRunningUnder( "WtkIseComStation", "eComStation",          WtkIseComStation());

   ulOperatingSystem = WtkQueryOperatingSystem();
   switch( ulOperatingSystem)
      {
      case WTK_OSTYPE_ECS: pszOperatingSystem = "eComStation";                  break;
      case WTK_OSTYPE_OS2: pszOperatingSystem = "OS/2";                         break;
      default:             pszOperatingSystem = "an unknown operatiing system"; break; 
      }
   printf( " \nWtkQueryOperatingSystem: this program is running under %s\n", pszOperatingSystem);

   // ----------------------------------------------------------------------

   PRINTSEPARATOR;

   // query the boot drive
   printf( "\nThe boot drive currently is: %c:\n", WtkQueryBootDrive());

   // ----------------------------------------------------------------------

   PRINTSEPARATOR;

   pszLangTableFormat = "%-22s  %-5s  %-5s  %s\n";

   // print header
   printf( "\n");
   printf(  pszLangTableFormat, "",            " ISO",  " ISO",  "");
   printf(  pszLangTableFormat, "Name of API", "639-1", "639-2", "Remark");
   printf(  pszLangTableFormat, "----------------------", 
            "-----", "-----", "---------------------------");

   // query system language code
   printf(  pszLangTableFormat, "WtkQuerySysLanguage",
            "", WtkQuerySysLanguage(), 
            "outdated - do not longer use!");

   // query system language codes
   printf(  pszLangTableFormat, "WtkQuerySystemLanguage",
            WtkQuerySystemLanguage( WTK_LANGUAGEID_639_1),
            WtkQuerySystemLanguage( WTK_LANGUAGEID_639_2),
            "derived from SYSLEVEL.OS2");

   // query locale language identifier
   printf(  pszLangTableFormat, "WtkQueryLocaleLanguage",
            WtkQueryLocaleLanguage( WTK_LANGUAGEID_639_1),
            WtkQueryLocaleLanguage( WTK_LANGUAGEID_639_2),
            "derived from def. locale object");

   // ----------------------------------------------------------------------

   PRINTSEPARATOR;

   } while (FALSE);

return rc;
}


