/****************************** Module Header ******************************\
*
* Module Name: _nlstestinc.c
*
* NLS functions related sample include file
*
* ### NOTE: this file must be named *.c for checktest.cmd ###
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: _nlstestinc.c,v 1.10 2007-02-28 12:25:26 cla Exp $
*
* ===========================================================================*
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


#define DEFAULTLANGUAGE "eng"
#define ENVVARLANGUAGE  "_NLS_LANGUAGE"

#define RESMASKPATH     "_nls%l.nls;dll\\_nls%l.nls"
#define INFMASKPATH     "_nls%l.inf;inf\\_nls%l.inf"
#define RESFILEMASK     "nls%s.txt"

#define PRINTSEPARATOR  printf("\n------------------------------------------\n")

#define CALLED_BY_EXE   TRUE
#define CALLED_BY_DLL   FALSE

#define MAX_MODULES     16

#ifdef DEBUG
#define WTKDEBUG_SETSYSTEMLANGUAGE(c) PrfWriteProfileString( HINI_USER, "WPSTK", "Debug_SystemLanguage", c)
#define WTKDEBUG_SETLOCALELANGUAGE(c) PrfWriteProfileString( HINI_USER, "WPSTK", "Debug_LocaleLanguage", c)
#else
#define WTKDEBUG_SETSYSTEMLANGUAGE(c)
#define WTKDEBUG_SETLOCALELANGUAGE(c)
#endif

// -----------------------------------------------------------------------------

static PSZ _getUniqueDefaultLanguage( VOID)
{
         PSZ            pszResult = "eng";  // never let it fail completely
         ULONG          i;

         PSZ            pszSystemLanguage = WtkQuerySystemLanguage( WTK_LANGUAGEID_639_2);
         PSZ            pszLocaleLanguage = WtkQueryLocaleLanguage( WTK_LANGUAGEID_639_2);

         PSZ           *ppszTestDefaultLanguage;
         PSZ            apszTestDefaultLanguages[] = { "eng", "fra", "esp", "deu"};
#define MAX_TESTDEFAULTLANGUAGES (sizeof( apszTestDefaultLanguages) / sizeof( PSZ))

// check all the favourites
for (i = 0, ppszTestDefaultLanguage = apszTestDefaultLanguages;
     i < MAX_TESTDEFAULTLANGUAGES;
     i++, ppszTestDefaultLanguage++)
   {
   // may not be identical with system or local language
   if ((strcmp( *ppszTestDefaultLanguage, pszSystemLanguage)) &&
       (strcmp( *ppszTestDefaultLanguage, pszLocaleLanguage)))
      {
      pszResult = *ppszTestDefaultLanguage;
      break;
      }
   }

return pszResult;
}

// -----------------------------------------------------------------------------

static PSZ _getVariants( PSZ pszLanguageCode, ULONG ulIdType)
{
         APIRET         rc = NO_ERROR;
static   CHAR           szVariants[ _MAX_PATH];

memset( szVariants, 0, sizeof( szVariants));
WtkQueryLanguageVariants( pszLanguageCode, ulIdType, szVariants, sizeof( szVariants));
return szVariants;
}

// -----------------------------------------------------------------------------

static VOID _removeTestcaseFiles( VOID)
{

         APIRET         rc = NO_ERROR;
         HDIR           hdir = HDIR_CREATE;
         CHAR           szTestcaseFilemask[ _MAX_PATH];
         CHAR           szThisFile[ _MAX_PATH];

// assemble search mask
WtkGetModulePath( (PFN)__FUNCTION__, szTestcaseFilemask, sizeof( szTestcaseFilemask));
sprintf( &szTestcaseFilemask[ strlen( szTestcaseFilemask)], "\\" RESFILEMASK, "*");

// delete all files
while (rc == NO_ERROR)
   {
   // get next
   rc = WtkGetNextFile( szTestcaseFilemask, &hdir, szThisFile, sizeof( szThisFile));
   if (rc != NO_ERROR)
      break;

   // delete it
   DosDelete( szThisFile);
   }

return;
}

// -----------------------------------------------------------------------------

static APIRET _executeFileTestcase( PSZ pszTitle, PSZ pszCodeVar, 
                                    PSZ pszDefaultLanguage, PSZ pszProvidedLanguage,
                                    APIRET rcExpected)
{
         APIRET         rc = NO_ERROR;

         ULONG          ulBasePathLen;
         CHAR           chLangType;
         CHAR           szRc[ 6];
         CHAR           szFile[ _MAX_PATH];
         CHAR           szProvidedFile[ _MAX_PATH];
         CHAR           szSearchPath[ 64];
         PSZ            pszProvidedFileRel;
         PSZ            pszFoundFileRel;
         PSZ            pszRequestedLanguage;
         PSZ            psResult;

static   PSZ            pszOutputFormat = "%-4s  %-5s  %-5s  %-10s  %-10s  %2s  %-12s %s\n";
static   PSZ            pszFileMask = RESFILEMASK;
static   PSZ            pszFileContents = "This is a dummy file\n";

do
   {
   // print header
   if (!pszTitle)
      {
      printf( pszOutputFormat, "",     "def",   "reqst", "Provided",   "Searched",   "",    "",           "");
      printf( pszOutputFormat, "case", "Lang",  "Lang",  "File",       "File",       "rc",  "File found", "Result");
      printf( pszOutputFormat, "====", "=====", "=====", "==========", "==========", "==", "===========", "=======");
      break;
      }

   // exit if no language is given to be providede
   if (!pszProvidedLanguage)
      break;
   
   // create directory and provided file
   WtkGetModulePath( (PFN)__FUNCTION__, szProvidedFile, sizeof( szProvidedFile));
   strcat( szProvidedFile, "\\");
   ulBasePathLen = strlen( szProvidedFile);
   pszProvidedFileRel = szProvidedFile + ulBasePathLen;
   pszFoundFileRel    = szFile         + ulBasePathLen;

   sprintf( &szProvidedFile[ strlen( szProvidedFile)], pszFileMask, pszProvidedLanguage);
   WtkWriteFile( szProvidedFile, pszFileContents, strlen( pszFileContents), FALSE);

   // determine requested language
   chLangType = *(pszCodeVar + strlen( pszCodeVar) - 1);
   switch (chLangType)
      {
      case 's':
      case 'S':
         pszRequestedLanguage = WtkQuerySystemLanguage( WTK_LANGUAGEID_639_2);
         break;

      case 'l':
      case 'L':
         pszRequestedLanguage = WtkQueryLocaleLanguage( WTK_LANGUAGEID_639_2);
         break;
      }

   // assemble search path
   sprintf( szSearchPath, pszFileMask, pszCodeVar);

   // attempt to find the file
   rc = WtkGetNlsPackageFilename( (PFN)__FUNCTION__, pszDefaultLanguage, NULL,
                                  szSearchPath, szFile, sizeof( szFile));

   // output result
   _ltoa( rc, szRc, 10);
   printf( pszOutputFormat, pszTitle, pszDefaultLanguage, pszRequestedLanguage,
           pszProvidedFileRel, szSearchPath, szRc,
           (rc == NO_ERROR)   ? pszFoundFileRel : "<no file>",
           (rc == rcExpected) ? "Success" : "Error");

   } while (FALSE);

return rc;
}

// -----------------------------------------------------------------------------

static VOID _dumpLanguageInfo( PSZ pszType, PSZ pszLanguage)
{
printf( "                         %s language: %s/%s/%s\n", pszType,
                                                            WtkQueryLanguageVariants( pszLanguage, WTK_LANGUAGEID_639_1,  NULL, 0),
                                                            WtkQueryLanguageVariants( pszLanguage, WTK_LANGUAGEID_639_2,  NULL, 0),
                                                            WtkQueryLanguageVariants( pszLanguage, WTK_LANGUAGEID_639_1C, NULL, 0));
printf( "   variants of %s language ISO 639-2: %s\n",       pszType,
                                                            _getVariants( WtkQueryLanguageVariants( pszLanguage, WTK_LANGUAGEID_639_2,  NULL, 0),  WTK_LANGUAGEID_639_2));
printf( "variants of %s language ISO 639-1+cc: %s\n",       pszType,
                                                            _getVariants( WtkQueryLanguageVariants( pszLanguage, WTK_LANGUAGEID_639_1C,  NULL, 0), WTK_LANGUAGEID_639_1C));
printf( "\n");
}

// -----------------------------------------------------------------------------

static APIRET NlsExecuteTestcases( BOOL fExeCall)
{

         APIRET         rc = NO_ERROR;
         BOOL           fResult = FALSE;
         PSZ            p;
         ULONG          i;

         CHAR           szVersion[ 32];
         BOOL           fLibDebug = FALSE;
         PSZ            pszLibMode;

         PSZ            pszTestDefaultLanguage;
         PSZ            pszSystemLanguage;
         PSZ            pszSystemLanguageVariant;
         PSZ            pszLocaleLanguage;
         PSZ            pszLocaleLanguageVariant;

         PSZ            pszFormat;
         PSZ            pszDefaultLanguage;
         PSZ            pszEnvVarLanguage;
         PSZ            pszEnvValueLanguage;
         PSZ            pszResMaskPath;
         PSZ            pszInfMaskPath;

         HAB            hab = NULLHANDLE;
         CHAR           szFile[ _MAX_PATH];
         HMODULE        hmodRes = NULLHANDLE;
         CHAR           szString[ _MAX_PATH];

         CHAR           szModulePath[ _MAX_PATH];
         ULONG          ulModuleCount = 0;
         HMODULE        ahmod[ MAX_MODULES];
         PHMODULE       phmod;

static   PSZ            pszTestcaseSeparatorMask = 
            "\n"
            "**************************************************************************\n"
            "%s NLS testcases called from: %s\n"
            "**************************************************************************\n"
            "\n";
         PSZ            pszTestcaseCaller = (fExeCall) ?
                           "executable file" :
                           "dynamic link library";
static   PSZ            pszTestcaseLangMask = "Temporarily using %s as %s language\n";


// mark start of testcase
printf( pszTestcaseSeparatorMask, "Starting",  pszTestcaseCaller);

do
   {

   // check if library is in debug mode
   strcpy( szVersion,  "QUERYDEBUG");
   fLibDebug = (WtkQueryVersion( szVersion, sizeof( szVersion)) == NO_ERROR);
   pszLibMode = (fLibDebug) ? "debug" : "release";

   // print headers
   printf( "WtkGetNlsPackageFilename tescases for WPSTK library in %s mode\n"
           "\n",
           pszLibMode);
   
   if (fLibDebug)
      {
      // in debug mode, use languages with variants
      // that would prooftest the API best
      pszSystemLanguage        = "eng"; // english   - United Kingdom
      pszSystemLanguageVariant = "ena"; // english   - Australia
      pszLocaleLanguage        = "ptg"; // portugese - Portugal
      pszLocaleLanguageVariant = "ptb"; // portugese - Brazil
      pszTestDefaultLanguage   = "fra";

      // fake system and locale language
      // works only if lib is compiled in debug mode !
      WTKDEBUG_SETSYSTEMLANGUAGE( pszSystemLanguage);
      WTKDEBUG_SETLOCALELANGUAGE( pszLocaleLanguage);
      printf( pszTestcaseLangMask, pszSystemLanguage, "system");
      printf( pszTestcaseLangMask, pszLocaleLanguage, "locale");
      printf( "\n");
      }
   else
      {
      // in release mode, use a default language not matching 
      // the system or locale language
      pszSystemLanguage      = WtkQuerySystemLanguage( WTK_LANGUAGEID_639_2);
      pszLocaleLanguage      = WtkQueryLocaleLanguage( WTK_LANGUAGEID_639_2);
      pszTestDefaultLanguage = _getUniqueDefaultLanguage();

      // don't check for variants in release mode
      pszSystemLanguageVariant = NULL;
      pszLocaleLanguageVariant = NULL;
      }

   _executeFileTestcase( NULL, NULL,NULL, NULL, 0);

   // cleanup for safety
   _removeTestcaseFiles();

   // test cases for system language
   //                                    WPSTK      default language          provided language         rc expected
   //                                    code var
   rc = _executeFileTestcase( "sys1",    "%s",      pszTestDefaultLanguage, pszTestDefaultLanguage,   NO_ERROR);
   rc = _executeFileTestcase( "sys2",    "%s",      pszTestDefaultLanguage, "xyz",                    NO_ERROR);
   rc = _executeFileTestcase( "sys3",    "%s",      pszTestDefaultLanguage, pszSystemLanguageVariant, NO_ERROR);
   rc = _executeFileTestcase( "sys4",    "%s",      pszTestDefaultLanguage, pszSystemLanguage,        NO_ERROR);

   // test cases for locale language
   if (strcmp( pszSystemLanguage, pszLocaleLanguage))
      {
      rc = _executeFileTestcase( "loc1", "%l",   pszTestDefaultLanguage, pszTestDefaultLanguage,   NO_ERROR);
      rc = _executeFileTestcase( "loc2", "%l",   pszTestDefaultLanguage, "xyz",                    NO_ERROR);
      rc = _executeFileTestcase( "loc3", "%l",   pszTestDefaultLanguage, pszLocaleLanguageVariant, NO_ERROR);
      rc = _executeFileTestcase( "loc4", "%l",   pszTestDefaultLanguage, pszLocaleLanguage,        NO_ERROR);
      }
   else
      printf( "\nSkipped tests for locale language, being identical to the system language on your system\n");

   // cleanup
   _removeTestcaseFiles();

   // reset debug settings anyway
   WTKDEBUG_SETSYSTEMLANGUAGE( NULL);
   WTKDEBUG_SETLOCALELANGUAGE( NULL);

   // --------------------------------------------------------------------------------

   PRINTSEPARATOR;

   pszDefaultLanguage  = DEFAULTLANGUAGE;
   pszEnvVarLanguage   = ENVVARLANGUAGE;
   pszEnvValueLanguage = getenv( pszEnvVarLanguage);
   pszResMaskPath      = RESMASKPATH;
   pszInfMaskPath      = INFMASKPATH;

   printf( "\n");
   printf( " Current language settings:\n");
   printf( "\n");

   _dumpLanguageInfo( "default", pszDefaultLanguage);
   _dumpLanguageInfo( " system",  WtkQuerySystemLanguage( WTK_LANGUAGEID_639_2));
   _dumpLanguageInfo( " locale",  WtkQueryLocaleLanguage( WTK_LANGUAGEID_639_2));

   printf( "                                  env var: %s\n", pszEnvVarLanguage);
   printf( "                         value of env var: %s\n", (pszEnvValueLanguage) ? 
                                                              pszEnvValueLanguage : "<not set>");
   if (pszEnvValueLanguage)
      _dumpLanguageInfo( "env var", pszEnvValueLanguage);


   // --------------------------------------------------------------------------------

   PRINTSEPARATOR;

   printf( "\n");
   printf( "mask path for res modules: %s\n", pszResMaskPath);
   printf( "  mask path for INF files: %s\n", pszInfMaskPath);
   printf( "\n");
   printf( "- WtkLoadNlsResourceModule: load NLS specific resource file\n");

   rc = WtkLoadNlsResourceModule( (PFN)__FUNCTION__, &hmodRes, pszDefaultLanguage, pszEnvVarLanguage, pszResMaskPath);
   if (rc != NO_ERROR)
      {
      printf( " WtkLoadNlsResourceModule: error: cannot load NLS specific resource file. rc=%u\n", rc);
      break;
      }
   printf( "  - resource module handle: %u\n", hmodRes);
      
   // ------ ------ ------ ------ ------

   printf( "- load string from resource file\n");
   hab = WinInitialize( 0);
   if (hab)
      {
      if (!WinLoadString( hab, hmodRes, IDSTR_LANGUAGE_IDENTIFIER, sizeof( szString), szString))
         {
         rc = ERRORIDERROR( WinGetLastError( hab));
         printf( "error: cannot load string from NLS resource file. rc=%u/0x%04x\n", rc, rc);
         break;
         }
      printf( "  - loaded string is: %s\n", szString);
      }
   
   // ------ ------ ------ ------ ------

   PRINTSEPARATOR;

   printf( "- WtkLoadNlsInfFile: launch NLS specific INF file\n");

   rc = WtkLoadNlsInfFile( (PFN)__FUNCTION__, pszDefaultLanguage, pszEnvVarLanguage, pszInfMaskPath, "SAMPLEPAGE");
   if (rc != NO_ERROR)
      {
      printf( " WtkLoad(Mod)NlsInfFile: error: cannot load NLS specific INF file. rc=%u\n", rc);
      break;
      }
   printf( "  - loaded INF file successfully\n");

   // --------------------------------------------------------------------------------

   PRINTSEPARATOR;

   // modify the language mask, translate %l to *
   strcpy( szModulePath, RESMASKPATH);
   p = szModulePath;
   while (p)
      {
      p = strchr( p, '%');
      if (p)
         {
         strcpy( p, p + 1);
         *p = '*';
         }
      }

   // search and load all NLS DLLs
   printf( "- WtkLoadModules: load all available NLS files\n");
   printf( "    mask path for all res modules: %s\n", szModulePath);

   ulModuleCount = MAX_MODULES;
   rc = WtkLoadModules( (PFN)__FUNCTION__, ahmod, &ulModuleCount, szModulePath);
   if (rc != NO_ERROR)
      {
      printf( " WtkLoadModules: error: cannot load any NLS specific module. rc=%u\n", rc);
      break;
      }
   printf( "- loaded %u NLS modules\n", ulModuleCount);
   for (i = 0, phmod = ahmod; i < ulModuleCount; i++, phmod++)
      {
      DosQueryModuleName( *phmod, sizeof( szFile), szFile);
      printf( "  - %s\n", szFile);
      }

   } while (FALSE);

// mark end of testcase
printf( pszTestcaseSeparatorMask, "Completed",  pszTestcaseCaller);

// cleanup
if (ulModuleCount) WtkFreeModules( ahmod, ulModuleCount);
if (hmodRes) DosFreeModule( hmodRes);
if (hab) WinTerminate( hab);
return rc;
}

