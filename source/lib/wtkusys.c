/****************************** Module Header ******************************\
*
* Module Name: wtkusys.c
*
* Source for system utility functions.
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: wtkusys.c,v 1.16 2008-09-10 00:12:40 cla Exp $
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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INCL_DOS
#define INCL_WIN
#define INCL_ERRORS
#include <os2.h>

#include "wtkusys.h"
#include "wtkulmd.h"
#include "wpstk.ih"

#define VERSION_MAJOR 0
#define VERSION_MINOR 1

// set to 1 to search for SECURIT2.DLL on eCS
#define USE_ECS_SECURITY_DLL 1
// set to 1 to check for a specific entry point (does currently not work)
#define CHECK_FOR_FUNCTIONS  0

#pragma pack(1)

typedef struct _SYSLANGINFO
   {
         PSZ            pszLanguageChars;  /* valid language chars                 */
         PSZ            pszLanguageId1;    /* two letter ISO 639-1 language code   */
         PSZ            pszLanguageId2;    /* three letter ISO 639-2 language code */
         PSZ            pszPromptKeys;     /* specify a "*" if no check required   */
   } SYSLANGINFO, *PSYSLANGINFO;


// --- last entry must be a SYSLANGINFO competely set to zero
// --- always make sure that for entries with same language char
//     there may only be one entry without prompt keys and this must
//     be the last in the row
static   SYSLANGINFO    ausli[] = { {"0_", "en", "eng", "YNARI"}, /* english          / USA/UK      */
                                    {"D",  "da", "dan", "*"},     /* danish           / Denmark     */
                                    {"F",  "fr", "fra", "ONARI"}, /* french           / France      */
                                    {"F",  "fi", "fin", "*"},     /* finnish          / Finnland    */
                                    {"G",  "de", "deu", "*"},     /* german           / Germany     */
                                    {"H",  "nl", "ndl", "JNAHN"}, /* dutch            / Netherlands */
                                    {"H",  "ko", "kor", "*"},     /* korean           / Korea       */
                                    {"I",  "it", "ita", "*"},     /* italian          / Italy       */
                                    {"N",  "no", "nor", "*"},     /* norwegian        / Norway      */
                                    {"S",  "es", "esp", "*"},     /* spanish          / Spain       */
                                    {"W",  "sv", "sve", "*"},     /* swedish          / Sweden      */
                                    {"J",  "jp", "jap", "*"},     /* japanese         / Japan       */
                                    {"P",  "pt", "ptg", "*"},     /* portugese        / Portugal    */
                                    {"B",  "pt", "ptb", "*"},     /* brazil.portugese / Brazil      */
                                    {"C",  "fr", "frc", "*"},     /* canadian french  / Canada      */
                                    {"T",  "zh", "twn", "*"},    /* taiwanese        / Taiwan      */
                                    {"O",  "pl", "plk", "*"},     /* polish (Warp 4)  / Poland      */
                                    {"0",  "pl", "plk", "TNAPI"}, /* polish (Warp 3)  / Poland      */
//                                  {"0",  "hu", "hun", "?????"}, /* hungarian        / Hungary     */
//                                  {"?",  "he", "heb", "?????"}, /* gebrew           / Israel      */
//                                  {"?",  "ar", "ara", "?????"}, /* arab             / ------      */
//                                  {"?",  "zh", "cht", "?????"}, /* simpl. chinese   / China       */
                                    {0}};

// ###########################################################################

/*
@@WtkIsWarp4@SYNTAX
This function tests, if the calling program is running under OS/2 WARP 4
or greater.

@@WtkIsWarp4@RETURN
OS/2 WARP 4 indicator.
:parml compact.
:pt.TRUE
:pd.Program is running under OS/2 WARP 4 or greater. This includes eComStation.
:pt.FALSE
:pd.Program is running under OS/2 below WARP 4
:eparml.

@@WtkIsWarp4@REMARKS
This function is mostly needed to distinct between OS/2 WARP 3 and OS/2 WARP 4
because of the differences in
:ul.
:li.the layout of settings notebook pages of WPS objects, especially the notebook buttons.
:p.
See also
:sl compact.
:li.:link reftype=hd refid=WtkRelocateNotebookpageControls.WtkRelocateNotebookpageControls:elink.
:esl.
:li.font names and sizes being used in dialogs
:eul.
:p.
If executed under eComStation, WtkIsWarp4 will return TRUE, indicating OS/2 Warp 4. In order
to determine, wether it is executed under OS/2 Warp compared to eComStation, use the functions
:link reftype=hd refid=WtkQueryOperatingSystem.WtkQueryOperatingSystem:elink.
or
:link reftype=hd refid=WtkIseComStation.WtkIseComStation:elink. and
:link reftype=hd refid=WtkIsOS2.WtkIsOS2:elink..
@@
*/

BOOL  APIENTRY WtkIsWarp4( VOID)
{

         BOOL           fResult = FALSE;
         APIRET         rc = NO_ERROR;
         ULONG          aulOsVersion[ 2];

do
   {
   // get os version
   rc = DosQuerySysInfo( QSV_VERSION_MAJOR,
                         QSV_VERSION_MINOR,
                         &aulOsVersion,
                         sizeof( aulOsVersion));
   if (rc != NO_ERROR)
      break;

   fResult = (( aulOsVersion[ VERSION_MAJOR] >= 20) &&
              ( aulOsVersion[ VERSION_MINOR] >= 40));

   } while (FALSE);

return fResult;
}

// ---------------------------------------------------------------------------

/*
@@WtkQueryOperatingSystem@SYNTAX
This function determines under which operating system (OS/2 Warp or eComStation)
it is executed,

@@WtkQueryOperatingSystem@RETURN
Operating system indicator
:parml compact.
:pt.WTK_OSTYPE_ECS
:pd.Program is running under eComStation.
:pt.WTK_OSTYPE_OS2
:pd.Program is running under OS/2.
:eparml.

@@WtkQueryOperatingSystem@REMARKS
This function is called by
:link reftype=hd refid=WtkIseComStation.WtkIseComStation:elink. and
:link reftype=hd refid=WtkIsOS2.WtkIsOS2:elink..

@@
*/

BOOL APIENTRY WtkQueryOperatingSystem( VOID)
{

         APIRET         rc = NO_ERROR;
         PSZ            p;

static   BOOL           fInitialized = FALSE;
static   ULONG          ulOsType = WTK_OSTYPE_OS2;

         ULONG          ulBootDrive;
         CHAR           szCheckFile[ _MAX_PATH];

         HMODULE        hmodSecDll = NULLHANDLE;

do
   {
   // already determined ?
   if (fInitialized)
      break;

#if USE_ECS_SECURITY_DLL
   do
      {

               CHAR           szError[ 9];

      static   PSZ            pszSecDllName = "SECURIT2";
      static   PSZ            pszSecProcName = "KeyGetRegisteredName";
               PFN            pfn;

      // load dll
      memset( szError, 0, sizeof( szError));
      rc = DosLoadModule( szError, sizeof( szError), pszSecDllName, &hmodSecDll);
      if (rc != NO_ERROR)
         break;

#if CHECK_FOR_FUNCTIONS
      // lookup for at least one interface
      rc = DosQueryProcAddr( hmodSecDll, 0, pszSecProcName, (PFN*) &pfn);
      if (rc != NO_ERROR)
         break;
#endif

      ulOsType = WTK_OSTYPE_ECS;

      } while (FALSE);

      // cleanup
      if (hmodSecDll)
         DosFreeModule( hmodSecDll);

#else
   // check environment var
   p = getenv( "OS");
   if ((p) && (!strcmp( p, "ecs"))
      ulOsType = WTK_OSTYPE_ECS;

   // hack for eCS V1.0: check for file wisemachine.fit
   // in root directory of boot drive
   DosQuerySysInfo( QSV_BOOT_DRIVE, QSV_BOOT_DRIVE, &ulBootDrive, sizeof(ULONG));
   sprintf( szCheckFile, "%c:\\wisemachine.fit", (CHAR) ulBootDrive + 'A' - 1);
   if (FileExists( szCheckFile, FALSE))
      ulOsType = WTK_OSTYPE_ECS;
#endif

   // don't check again
   fInitialized = TRUE;

   } while (FALSE);

return ulOsType;
}


// ---------------------------------------------------------------------------

/*
@@WtkIseComStation@SYNTAX
This function tests, if the calling program is running under eComStation.

@@WtkIseComStation@RETURN
eComStation indicator.
:parml compact.
:pt.TRUE
:pd.Program is running under eComStation.
:pt.FALSE
:pd.Program is running under OS/2.
:eparml.

@@WtkIseComStation@REMARKS
This function is the counterpart of the API
:link reftype=hd refid=WtkIsOS2.WtkIsOS2:elink..

@@
*/

BOOL APIENTRY WtkIseComStation( VOID)
{
return (WtkQueryOperatingSystem() == WTK_OSTYPE_ECS);
}

// ---------------------------------------------------------------------------

/*
@@WtkIsOS2@SYNTAX
This function tests, if the calling program is running under eComStation.

@@WtkIsOS2@RETURN
eComStation indicator.
:parml compact.
:pt.TRUE
:pd.Program is running under OS/2
:pt.FALSE
:pd.Program is running under eComStation
:eparml.

@@WtkIsOS2@REMARKS
This function is the counterpart of the API
:link reftype=hd refid=WtkIseComStation.WtkIseComStation:elink..

@@
*/

BOOL APIENTRY WtkIsOS2( VOID)
{
return (WtkQueryOperatingSystem() == WTK_OSTYPE_OS2);
}

// ---------------------------------------------------------------------------

/*
@@WtkQueryBootDrive@SYNTAX
This function returns the ASCII value of the drive, where the operating
system was started from.

@@WtkQueryBootDrive@RETURN
ASCII value of the letter from the drive, where the operating system was started from.

@@WtkQueryBootDrive@REMARKS
This function is intended as an easier way to query the boot drive compared to the
conventiuonal method using :hp2.DosQuerySysValue:ehp2. Using :hp2.WtkQueryBootDrive:ehp2.
instead allows to embedd the query for a boot drive in another function call,
like for example&colon.
:xmp.
sprintf( szSyslevelFilename, "?&colon.\\OS2\\SYSTEM\\SYSLEVEL.OS2", WtkQueryBootDrive());
:exmp.
.br
:color fc=red.
Note however that when using file related functions of the :hp2.Workplace Shell
Toolkit:ehp2. this is not required, as all of these allow to specify :hp2.?&colon.:ehp2.
as a placeholder for the boot drive within file- and/or pathname specifications.
:color fc=default.
@@
*/

CHAR APIENTRY WtkQueryBootDrive( VOID)
{
         CHAR           chResult = 'C';
         APIRET         rc = NO_ERROR;
         ULONG          ulBootDrive;

do
   {
   // get os version
   rc = DosQuerySysInfo( QSV_BOOT_DRIVE,
                         QSV_BOOT_DRIVE,
                         &ulBootDrive,
                         sizeof( ulBootDrive));
   if (rc != NO_ERROR)
      break;

   // make it ASCII value
   chResult = ulBootDrive + 'A' - 1;

   } while (FALSE);

return chResult;
}

// ---------------------------------------------------------------------------

/*
@@WtkQuerySystemLanguage@SYNTAX
This function returns a pointer to a ASCIIZ ISO 639 language code
according to the language of your operation system, determined from 
the SYSLEVEL.OS2 file.
:p.
For downwards compatibility, the formerly available function
:hp2.WtkQuerySysLanguage:ehp2., returning only the
three-letter ISO 639-2 language code, is still supported. It 
calls WtkQuerySystemLanguage with the parameter ulIdType set to 
WTK_LANGUAGEID_639_2.

@@WtkQuerySystemLanguage@PARM@ulIdType@in
The type of the language code to be returned.
:parml compact.
:pt.WTK_LANGUAGEID_639_1
:pd.returns the two-letter language code according to ISO 639-1
:pt.WTK_LANGUAGEID_639_2
:pd.returns the three-letter language code according to ISO 639-2
:eparml.

@@WtkQuerySystemLanguage@RETURN
Pointer to a ASCIIZ ISO 639 language code, as determined by the SYSLEVEL.OS2 file.
:p.
If for any reason the language cannot be determined properly,
:hp2.en:ehp2. or :hp2.eng:ehp2. is returned for the english language as default,
according to the value specified as parameter ulIdType.

@@WtkQuerySystemLanguage@REMARKS
This function determines the language from the OS/2 syslevel file
of your operating system, by reading the language character of the CSD
identifier. As the language is coded in one character only within CSD ids,
and this character must not necessarily be unique to one language, this
function in some cases also checks the OS/2 system message file in order to
uniquely identify the language of the operating system.
:note.
:ul compact.
:li. In order to determine the locale default language,
use :link reftype=hd refid=WtkQueryLocaleLanguage.WtkQueryLocaleLanguage:elink. instead.
:color fc=red.
:li.If the user has  applied a fixpak of another language, the syslevel file
may well reflect the language of the fixpak instead of the base operating
system. In such a case of course the language code for this language is returned,
but the system will then use this language for many dialogs anyway, so this
won't hurt much.
:color fc=default.
:eul.
:p.
The following ISO 639-1/-2 language codes are currently returned&colon.
:parml compact tsize=12 break=none.
:pt.en/eng
:pd.us/uk english
:pt.da/dan
:pd.danish
:pt.de/deu
:pd.german
:pt.es/esp
:pd.spanish
:pt.fi/fin
:pd.finnish
:pt.fr/fra
:pd.french
:pt.fr/frc
:pd.canadian french
:pt.it/ita
:pd.italian
:pt.jp/jap
:pd.japanese
:pt.ko/kor
:pd.korean
:pt.nl/ndl
:pd.dutch
:pt.no/nor
:pd.norwegian
:pt.pl/plk
:pd.polish
:pt.pt/ptb
:pd.brazilian portugese
:pt.pt/ptg
:pd.portugese
:pt.sv/sve
:pd.swedish
:pt.zh/twn
:pd.taiwanese
:eparml.

@@
*/

PSZ APIENTRY WtkQuerySystemLanguage(  ULONG ulIdType)
{
         PSZ            pszResult = "eng";
         APIRET         rc = NO_ERROR;
         PSZ            pszLanguage = NULL;
         PSZ            p;
         CHAR           chLanguage = '_';
         PSZ            pszLanguageChar;

         CHAR           szSyslevelFile[ _MAX_PATH];

         HFILE          hfileSyslevel = -1;
         ULONG          ulAction;
         ULONG          ulBytesRead;
         ULONG          ulFilePtr;

         PSYSLANGINFO   psli;
         BOOL           fIdFound = FALSE;
         CHAR           szPromptKeys[ 20];

do
   {
   // open syslevel file
   sprintf(  szSyslevelFile, "%c:\\os2\\install\\syslevel.os2", WtkQueryBootDrive());
   rc = DosOpen( szSyslevelFile, &hfileSyslevel, &ulAction, 0, 0,
                 OPEN_ACTION_FAIL_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS,
                 OPEN_SHARE_DENYNONE | OPEN_ACCESS_READONLY,
                 0);
   if (rc != NO_ERROR)
      break;

   // read one byte from the csd level information
   DosChgFilePtr( hfileSyslevel, 46, FILE_BEGIN, &ulFilePtr);
   rc = DosRead( hfileSyslevel, &chLanguage, sizeof( chLanguage), &ulBytesRead);
   if ((rc != NO_ERROR) || (ulBytesRead != sizeof( chLanguage)))
      break;

   // get prompt keys for further language check ...
   memset( szPromptKeys, 0, sizeof( szPromptKeys));
   rc = DosGetMessage( NULL, 0, szPromptKeys, sizeof( szPromptKeys), 0, "OSO001.MSG", &ulBytesRead);
   if (rc != NO_ERROR)
      break;

   // ... and prepare for comparison by removing whitespace
   p = szPromptKeys;
   while (*p)
      {
      if (*p <= 32)
         {
         strcpy( p, p + 1);
         continue;
         }
      p++;
      }

   // search language in list
   psli = ausli;
   fIdFound = FALSE;
   while (psli->pszLanguageChars)
      {
      do
         {
         // is language character included here ?
         if (!strchr( psli->pszLanguageChars, chLanguage))
            break;

         // is it required to check the system message file ?
         if (*psli->pszPromptKeys == '*')
            {
            // no, ID is already valid here
            fIdFound = TRUE;
            break;
            }

         if (!strcmp( psli->pszPromptKeys, szPromptKeys))
            {
            // prompt keys match, ID is valid
            fIdFound = TRUE;
            break;
            }


         } while (FALSE);

#ifdef DEBUG
      {
               CHAR           szTestLanguage[ 16];

      if (PrfQueryProfileString( HINI_USER, "WPSTK", "Debug_SystemLanguage", NULL,
                                 szTestLanguage, sizeof( szTestLanguage)))
         {
         pszResult = WtkTranslateLanguageCode( szTestLanguage, ulIdType);
         break;
         }

      }
#endif

      // return new value
      if (fIdFound)
         {
         switch (ulIdType)
            {
            case WTK_LANGUAGEID_639_1:
               pszResult = psli->pszLanguageId1;
               break;

            case WTK_LANGUAGEID_639_2:
               pszResult = psli->pszLanguageId2;
               break;

            case WTK_LANGUAGEID_639_1C:
               pszResult = WtkTranslateLanguageCode( psli->pszLanguageId2, WTK_LANGUAGEID_639_1C);
               break;
            }
         break;
         }

      // check next item
      psli++;
      }


   } while (FALSE);

// cleanup
DosClose( hfileSyslevel);
return pszResult;
}

// ---- maintain lib/DLL entry for the outdated API
//      WtkQuerySysLanguage, replaced by WtkQuerySystemLanguage 

// - diable the macro defined in wtkusys.h
#undef WtkQuerySysLanguage

PSZ APIENTRY WtkQuerySysLanguage( VOID)
{
return WtkQuerySystemLanguage( WTK_LANGUAGEID_639_2);
}


// ---------------------------------------------------------------------------

/*
@@WtkReboot@SYNTAX
This function reboots the operating system.

@@WtkReboot@RETURN
Return Code.
:p.
WtkReadFile return codes of the following functions
:ul compact.
:li.DosOpen
:li.DosDevIOCtl
:eul.

@@WtkReboot@REMARKS
This function returns only if the call was not successful.
Otherwise the system is just rebooted.
:p.
:note.
:ul compact.
:li.:color fc=red.:hp2.No application shudown is performed, instead
all unsaved data is lost !:ehp2.:color fc=default.
Only the filesystems are shutdown properly, just avoiding a :hp2.chkdsk:ehp2.
on the next system startup.
:eul.
@@
*/

#define DOS_CONTROL_DRIVER  "\\DEV\\DOS$"
#define CATEGORY_DOSSYS 0xD5
#define FUNCTION_REBOOT 0xAB

APIRET APIENTRY WtkReboot( VOID)
{
         APIRET         rc = NO_ERROR;
         HFILE          hfDosControlDriver = -1;
         ULONG          ulAction;

         BYTE           bParm = 0;
         ULONG          ulParmLen = 0;
         ULONG          ulDataLen = 0;

do
   {
   // open driver
   rc = DosOpen( DOS_CONTROL_DRIVER,
                 &hfDosControlDriver,
                 &ulAction,
                 0L,
                 FILE_NORMAL,
                 FILE_OPEN, OPEN_SHARE_DENYNONE | OPEN_ACCESS_READWRITE,
                 0L);
   if (rc != NO_ERROR)
      break;

   // send reboot command (this should not return at all ;-) )
   ulParmLen = 0;
   rc = DosDevIOCtl( hfDosControlDriver,
                     CATEGORY_DOSSYS,
                     FUNCTION_REBOOT,
                     NULL, 0, &ulParmLen,
                     NULL, 0, &ulDataLen);

   } while (FALSE);

// cleanup (playing safe here...)
DosClose(hfDosControlDriver);
return rc;
}

