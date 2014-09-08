/****************************** Module Header ******************************\
*
* Module Name: wtkulmd.c
*
* Source for module language utility functions
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: wtkulmd.c,v 1.28 2009-11-18 22:18:14 cla Exp $
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
#define INCL_ERRORS
#include <os2.h>

#include "wtkulmd.h"
#include "wtkusys.h"
#include "wtkuloc.h"
#include "wtkumod.h"
#include "wtkufil.h"
#include "wpstk.ih"

#ifndef DosQueryModFromEIP
APIRET APIENTRY  DosQueryModFromEIP(HMODULE *phMod,
                                     ULONG *pObjNum,
                                     ULONG BuffLen,
                                     PCHAR pBuff,
                                     ULONG *pOffset,
                                     ULONG Address);
#endif

// --- language code definitions for a language, including memory for variant lists

typedef struct _LANGDEF
   {
         CHAR           szLanguageId1[ 3];
         CHAR           szLanguage2Variants[ _MAX_PATH];
         CHAR           szLanguage1CVariants[ _MAX_PATH];
   } LANGDEF, *PLANGDEF;

// --- language code transform table

typedef struct _LANGINFO
   {
         PSZ            pszLanguageId1;    /* two letter ISO 639-1 language code                   */
         PSZ            pszLanguageId2;    /* three letter ISO 639-2 language code                 */
         PSZ            pszLocaleName;     /* two letter ISO 639-1 language code with country code */
   } LANGINFO, *PLANGINFO;

// language codes as supported by the eCS locale API
static   LANGINFO       auli[] = { { "ar", "ara", "ar_AA"}, // Arabic - ?
                                   { "ar", "ara", "ar_AE"}, // Arabic - ?
                                   { "ar", "ara", "ar_BH"}, // Arabic - ?
                                   { "ar", "ara", "ar_DZ"}, // Arabic - ?
                                   { "ar", "ara", "ar_EG"}, // Arabic - ?
                                   { "ar", "ara", "ar_JO"}, // Arabic - ?
                                   { "ar", "ara", "ar_KW"}, // Arabic - ?
                                   { "ar", "ara", "ar_LB"}, // Arabic - ?
                                   { "ar", "ara", "ar_MA"}, // Arabic - ?
                                   { "ar", "ara", "ar_OM"}, // Arabic - ?
                                   { "ar", "ara", "ar_QA"}, // Arabic - ?
                                   { "ar", "ara", "ar_SA"}, // Arabic - ?
                                   { "ar", "ara", "ar_SY"}, // Arabic - ?
                                   { "ar", "ara", "ar_TN"}, // Arabic - ?
                                   { "ar", "ara", "ar_YE"}, // Arabic - ?
                                   { "be", "bel", "be_BY"}, // Belarusian
                                   { "bg", "bgr", "bg_BG"}, // Bulgarian
                                   { "ca", "cat", "ca_ES"}, // Catalan
                                   { "cs", "csy", "cs_CZ"}, // Czech
                                   { "da", "dan", "da_DK"}, // Danish - Denmark
                                   { "de", "deu", "de_DE"}, // German - Germany
                                   { "de", "deu", "de_AT"}, // German - Austria
                                   { "de", "deu", "de_LU"}, // German-  Luxembourg
                                   { "de", "deu", "de_LI"}, // German - Lichtenstein
                                   { "de", "des", "de_CH"}, // German - Switzerland
                                   { "el", "ell", "el_GR"}, // Greek - Greece
                                   { "en", "eng", "en_GB"}, // English - United Kingdom
                                   { "en", "enu", "en_US"}, // English - United states
                                   { "en", "eng", "en_IE"}, // English - Ireland
                                   { "en", "eng", "en_ZA"}, // English - ?
                                   { "en", "ena", "en_AU"}, // English - Australia
                                   { "en", "enc", "en_CA"}, // English - Canada
                                   { "en", "enb", "en_BE"}, // English - Belgium
                                   { "es", "esp", "es_ES"}, // Spanish - Spain
                                   { "es", "esp", "es_AR"}, // Spanish - Argentina
                                   { "es", "esp", "es_BO"}, // Spanish - Bolivia
                                   { "es", "esp", "es_CL"}, // Spanish - ?
                                   { "es", "esp", "es_CO"}, // Spanish - ?
                                   { "es", "esp", "es_CR"}, // Spanish - ?
                                   { "es", "esp", "es_DO"}, // Spanish - ?
                                   { "es", "esp", "es_EC"}, // Spanish - Ecuador
                                   { "es", "esp", "es_GT"}, // Spanish - Guatemala
                                   { "es", "esp", "es_HN"}, // Spanish - ?
                                   { "es", "esp", "es_LA"}, // Spanish - ?
                                   { "es", "esp", "es_MX"}, // Spanish - Mexico
                                   { "es", "esp", "es_NI"}, // Spanish - Nicaragua
                                   { "es", "esp", "es_PA"}, // Spanish - ?
                                   { "es", "esp", "es_PE"}, // Spanish - ?
                                   { "es", "esp", "es_PY"}, // Spanish - ?
                                   { "es", "esp", "es_SV"}, // Spanish - ?
                                   { "es", "esp", "es_US"}, // Spanish - United States
                                   { "es", "esp", "es_UY"}, // Spanish - Uruguay
                                   { "es", "esp", "es_VE"}, // Spanish - Venezuela
                                   { "et", "eta", "et_EE"}, // Estonian
                                   { "eu", "eus", "eu_ES"}, // Basque
                                   { "fa", "far", "fa_IR"}, // Farsi - Iran
                                   { "fi", "fin", "fi_FI"}, // Finnish
                                   { "fr", "fra", "fr_FR"}, // French - France
                                   { "fr", "frb", "fr_BE"}, // French - Belgium
                                   { "fr", "frb", "fr_LU"}, // French - Luxembourg
                                   { "fr", "frc", "fr_CA"}, // French - Canada
                                   { "fr", "frs", "fr_CH"}, // French - Switzerland
                                   { "he", "heb", "he_IL"}, // Hebrew - Israel
                                   { "he", "heb", "iw_IL"}, // Hebrew - ?
                                   { "hr", "hrv", "hr_HR"}, // Croatian
                                   { "hr", "shl", "hr_SP"}, // Romanian
                                   { "hu", "hun", "hu_HU"}, // Hungarian
                                   { "in", "ind", "in_ID"}, // Indonesian
                                   { "is", "isl", "is_IS"}, // Icelandic
                                   { "it", "ita", "it_IT"}, // Italian - Italy
                                   { "it", "ita", "it_CH"}, // Italian - Switzerland
                                   { "ja", "jap", "ja_JP"}, // Japanese - Japan
                                   { "ja", "jpn", "ja_JP"}, // Japanese *** additional IBM code jpn
                                   { "ko", "kor", "ko_KR"}, // Korean
                                   { "lt", "lta", "lt_LT"}, // Lithuanian
                                   { "lv", "lva", "lv_LV"}, // Latvian
                                   { "mk", "mkd", "mk_MK"}, // Macedonian
                                   { "nl", "nld", "nl_NL"}, // Dutch - Netherlands
                                   { "nl", "nlb", "nl_BE"}, // Dutch - Belgium
                                   { "nl", "ndl", "nl_NL"}, // Dutch *** additional IBM code ndl
                                   { "no", "nor", "no_NO"}, // Norwegian
                                   { "pl", "plk", "pl_PL"}, // Polish
                                   { "pl", "pol", "pl_PL"}, // Polish *** additional IBM code pol
                                   { "pt", "ptg", "pt_PT"}, // Portugese
                                   { "pt", "ptb", "pt_BR"}, // Portugese - Brazil
                                   { "ro", "rom", "ro_RO"}, // Romanian
                                   { "ru", "rus", "ru_RU"}, // Russian
                                   { "sh", "shl", "sh_BA"}, // Serbo-Croatian
                                   { "sk", "sky", "sk_SK"}, // Slovak
                                   { "sl", "slo", "sl_SI"}, // Slovene
                                   { "sq", "sqi", "sq_AL"}, // Albanian
                                   { "sr", "shc", "sr_SP"}, // Serbian (Cyrillic)
                                   { "sv", "sve", "sv_FI"}, // Swedish
                                   { "sv", "sve", "sv_SE"}, // Swedish - Finnland
                                   { "th", "tha", "th_TH"}, // Thai
                                   { "tr", "tkr", "tr_TR"}, // Turkish
                                   { "uk", "ukr", "uk_UA"}, // Ukrainian
                                   { "vi", "vie", "vi_VN"}, // Vietnamese
                                   { "zh", "cht", "zh_CN"}, // Chinese - China
                                   { "zh", "chs", "zh_HK"}, // Chinese - Hongkong
                                   { "zh", "chs", "zh_SG"}, // Chinese - ?
                                   { "zh", "twn", "zh_TW"}, // Chinese *** additional IBM code twn for Taiwan
                                   { 0}};

// --------------------------------------------------------------------------

// returned memory must be freed by caller
static PLANGDEF _getLangDef( PSZ pszLanguageCode)
{
         PLANGDEF       pld = NULL;

if (pszLanguageCode)
   {
   pld = malloc( sizeof( LANGDEF));
   memset( pld, 0, sizeof( LANGDEF));

   strcpy( pld->szLanguageId1, WtkTranslateLanguageCode( pszLanguageCode, WTK_LANGUAGEID_639_1));
   WtkQueryLanguageVariants( pszLanguageCode, WTK_LANGUAGEID_639_2,
                             pld->szLanguage2Variants, sizeof( pld->szLanguage2Variants));
   WtkQueryLanguageVariants( pszLanguageCode, WTK_LANGUAGEID_639_1C,
                             pld->szLanguage1CVariants, sizeof( pld->szLanguage1CVariants));
   }

return pld;
}

// --------------------------------------------------------------------------

#define GETCODE(c,t)          _getcode( c, t,NULL, 0)
#define GETVARIANTS(c,t,b)    _getcode( c, t, b, sizeof( b))
#define GETVARIANTSP(c,t,p,s) _getcode( c, t, p, s)
static PSZ _getcode( PSZ pszLanguage, ULONG ulIdType,
                     PSZ pszBuffer, ULONG ulBuflen)
{
         APIRET         rc = NO_ERROR;
         PSZ            pszResult;

         PLANGINFO      pli;
         PSZ            pszMainVariant;
         BOOL           fQueryVariants = FALSE;
         CHAR           szVariants[ _MAX_PATH];

do
   {
   switch (ulIdType)
      {
      case WTK_LANGUAGEID_639_1:  pszResult = "en";    break;
      case WTK_LANGUAGEID_639_1C: pszResult = "en_GB"; break;
      case WTK_LANGUAGEID_639_2:  pszResult = "eng";   break;
      }

   // check parms
   if (!pszLanguage)
      break;

   // variants to be queried ?
   if (pszBuffer)
      {
      // search on using the two-letter base language code
      pszLanguage    = GETCODE( pszLanguage, WTK_LANGUAGEID_639_1);
      pszMainVariant = GETCODE( pszLanguage, ulIdType);
      strcpy( szVariants, pszMainVariant);
      fQueryVariants = TRUE;
      }

   // set result pointer to value in language table
   rc = ERROR_FILE_NOT_FOUND;
   pli = auli;
   memset( szVariants, 0, sizeof( szVariants));
   while (pli->pszLanguageId1)
      {
      // have we found the the language code ?
      if ((!stricmp( pszLanguage, pli->pszLanguageId1)) ||
          (!stricmp( pszLanguage, pli->pszLanguageId2)) ||
          (!stricmp( pszLanguage, pli->pszLocaleName)))
         {
         // we have found the language
         rc = NO_ERROR;

         // return primary language code
         switch (ulIdType)
            {
            case WTK_LANGUAGEID_639_1:  pszResult = pli->pszLanguageId1; break;
            case WTK_LANGUAGEID_639_1C: pszResult = pli->pszLocaleName;  break;
            case WTK_LANGUAGEID_639_2:  pszResult = pli->pszLanguageId2; break;
            }

         // return language variants
         if (fQueryVariants)
            {
            // append cureent language code to comma separated variant list,
            // if not already included
            if (!strstr( szVariants, pszResult))
               {
               if (szVariants[ 0])
                  strcat( szVariants, ",");
               strcat( szVariants, pszResult);
               }

            // always report main variant as function result
            pszResult = pszMainVariant;
            }
         else
            // we are done, if no variants are checked for
            break;
         }

      // next item in table
      pli++;
      }

   // return language variants
   if (fQueryVariants)
      {
      // check buflen
      if (strlen( szVariants) + 1 > ulBuflen)
         {
         rc = ERROR_BUFFER_OVERFLOW;
         break;
         }

      // hand over result
      strcpy( pszBuffer, szVariants);
      }

   } while (FALSE);

return pszResult;
}

// ---------------------------------------------------------------------------

#define CODE_LANG2   0
#define CODE_LANG3   1
#define CODE_COUNTRY 2

// pszDir must end with backslash
static APIRET _getLanguageFile( PSZ pszBuffer, ULONG ulBufLen, PSZ pszDir, PSZ pszMask,
                                PLANGDEF pldSystem, PLANGDEF pldLocale, PLANGDEF pldDefault)
{
         APIRET         rc = NO_ERROR;
         ULONG          i;

         BOOL           fRelativePath;
         ULONG          ulEntryLen;
         CHAR           szEntryMask[ _MAX_PATH];

#define VARDELIMITER '%'
         PSZ            pszVarStart;
         ULONG          ulVarLen;
         PSZ            pszSource;
         PSZ            pszTarget;
         BOOL           fFormatValid;

         ULONG          ulVarIsoType;
         CHAR           chVarLangType;

         PLANGDEF       pldUsed = NULL;

         CHAR           szVariants[ _MAX_PATH];
         BOOL           fLanguageVariantsUsed = FALSE;
         BOOL           fCountryVariantsUsed  = FALSE;
         PSZ            pszVariant;

#define MAXPARMS 32
         ULONG          ulParmCode;
         ULONG          aulParmCode[ MAXPARMS];
         PSZ            apszParm[ MAXPARMS];
         ULONG          ulParmCount = 0;
         CHAR           szCodeLang2[ 6];
         CHAR           szCodeLang3[ 6];
         CHAR           szCodeLangCountry[ 12];
         PSZ            pszCodeCountry;

         va_list        arg_ptr;

do
   {
   // determine if relative path is given
   fRelativePath = ((*pszMask != '\\') &&
                    (!strchr( pszMask, ':')));

   // if relative path is given, prepend it to filemask
   memset( szEntryMask, 0, sizeof( szEntryMask));
   if (fRelativePath)
      strcpy( szEntryMask, pszDir);
   else
      *szEntryMask = 0;

   // now scan through mask
   pszSource = pszMask;
   pszTarget = szEntryMask + strlen( szEntryMask);
   pszVarStart = strchr( pszSource, VARDELIMITER);
   fLanguageVariantsUsed = FALSE;
   fCountryVariantsUsed = FALSE;

   while (pszVarStart)
      {
      chVarLangType = *(pszVarStart + 1);

      // take care for ISO type, according to length specification
      fFormatValid = TRUE;
      switch (chVarLangType)
         {
         case '2':
            // read next char as language type
            chVarLangType = *(pszVarStart + 2);

            // determine var len
            ulVarLen = 3;

            // determine language code type
            ulParmCode = CODE_LANG2;
            break;

         case '3':
            // read next char as language type
            chVarLangType = *(pszVarStart + 2);

            // determine var len
            ulVarLen = 3;

            // determine language code type
            ulParmCode = CODE_LANG3;

            // make sure to search variants for languages
            fLanguageVariantsUsed = TRUE;
            break;

         case 'S':
         case 's':
         case 'L':
         case 'l':
         case 'D':
         case 'd':
            // determine var len
            ulVarLen = 2;

            // determine language code type
            ulParmCode = CODE_LANG3;

            // make sure to search variants for languages
            fLanguageVariantsUsed = TRUE;
            break;
            break;

         case 'c':
            // determine var len
            ulVarLen = 2;

            // determine language code type
            ulParmCode = CODE_COUNTRY;

            // make sure to search variants for countries
            fCountryVariantsUsed = TRUE;
            break;

         default:
            fFormatValid = FALSE;
            break;

         } // switch (chVarLangType)

      // take care for language type
      switch (chVarLangType)
         {
         case 'S':
         case 's':
            if (!pldUsed)
               pldUsed = pldSystem;
            break;

         case 'L':
         case 'l':
            if (!pldUsed)
               pldUsed = pldLocale;
            break;

         case 'D':
         case 'd':
            if (!pldUsed)
               pldUsed = pldDefault;
            break;

         default:
            fFormatValid = FALSE;
            break;

         } // switch (chVarLangType)

      // check for error
      if ((!fFormatValid) || (!pldUsed))
         {
         rc = ERROR_BAD_FORMAT;
         break;
         }

      // insert
      if (fFormatValid)
         {
         // calculate required length
         ulEntryLen = pszVarStart - pszSource + 2;
         if (strlen( szEntryMask) + ulEntryLen + 1 > ulBufLen)
            {
            // buffer does not fit
            rc = ERROR_BUFFER_OVERFLOW;
            break;
            }

         // copy up to var
         memcpy( pszTarget, pszSource, pszVarStart - pszSource);

         // store parm definition
         if (ulParmCount >= MAXPARMS)
            {
            rc = ERROR_NOT_ENOUGH_MEMORY;
            break;
            }
         aulParmCode[ ulParmCount] = ulParmCode;
         ulParmCount++;

         // append placeholder for language
         strcat( pszTarget, "%s");

         // adjust pointers
         pszSource  = pszVarStart + ulVarLen;
         pszTarget += strlen( pszTarget);

         } // if (fFormatValid)

      // search next var
      pszVarStart = strchr( pszVarStart + 1, VARDELIMITER);

      } // while (pszVarStart)

   // append remaining part of source
   if (strlen( szEntryMask) + strlen( pszSource) + 1 > ulBufLen)
      {
      // buffer does not fit
      rc = ERROR_BUFFER_OVERFLOW;
      break;
      }
   strcat( szEntryMask, pszSource);

   // ---------------------- check for language requested -----------------

   if (pldUsed)
      {
      // ---- now search for all variants
      //      if country codes are used, we must loop through
      //      all variants for the countries, instead of those
      //      for the three-letter language codes
      strcpy( szVariants, (fCountryVariantsUsed) ?
                 pldUsed->szLanguage1CVariants :
                 pldUsed->szLanguage2Variants);

      // append variants of default language
      if ((pldDefault) && (pldUsed != pldDefault) &&
          (strcmp( pldUsed->szLanguageId1, pldDefault->szLanguageId1)))
         {
         strcat( szVariants, ",");
         strcat( szVariants, (fCountryVariantsUsed) ?
                    pldDefault->szLanguage1CVariants :
                    pldDefault->szLanguage2Variants);
         }

      //loop all variants
      pszVariant = strtok( szVariants, ",");
      rc = ERROR_FILE_NOT_FOUND;
      while (pszVariant)
         {

         // determine
         strcpy( szCodeLang2,       GETCODE( pszVariant, WTK_LANGUAGEID_639_1));
         strcpy( szCodeLang3,       GETCODE( pszVariant, WTK_LANGUAGEID_639_2));
         strcpy( szCodeLangCountry, GETCODE( pszVariant, WTK_LANGUAGEID_639_1C));
         pszCodeCountry = strchr( szCodeLangCountry, '_') + 1;

         // setup parameter for vsprintf
         memset( apszParm, 0, sizeof( apszParm));
         for (i = 0; i < ulParmCount; i++)
            {
            switch (aulParmCode[ i])
               {
               case CODE_LANG2:   apszParm[ i] = szCodeLang2;    break;
               case CODE_LANG3:   apszParm[ i] = szCodeLang3;    break;
               case CODE_COUNTRY: apszParm[ i] = pszCodeCountry; break;
               }
            }

         // assemble filename
#ifdef __WATCOMC__
         // Watcom defines:        typedef char *va_list[1];
         arg_ptr[0] = (PSZ)apszParm;
#else
         // other compiles define: typedef char *va_list;
         arg_ptr = (va_list)apszParm;
#endif
         vsprintf( pszBuffer, szEntryMask, arg_ptr);

         // check if file exists
         if (WtkFileExists( pszBuffer))
            {
            rc = NO_ERROR;
            break;
            }

         // if no variants to be searched for, exit loop
         if ((!fLanguageVariantsUsed) && (!fCountryVariantsUsed))
            break;

         // next variant
         pszVariant = strtok( NULL, ",");
         }
      }
   else // if (pldUsed)
      {
      // no language code requested, so just check the name provided

      // check buffer
      if (strlen( szEntryMask) + 1 > ulBufLen)
         {
         // buffer does not fit
         rc = ERROR_BUFFER_OVERFLOW;
         break;
         }

      // hand over result
      strcpy( pszBuffer, szEntryMask);

      // check if file exists
      rc = WtkFileExists( pszBuffer) ? NO_ERROR : ERROR_FILE_NOT_FOUND;
      }

   } while (FALSE);

return rc;
}

// ---------------------------------------------------------------------------

/*
@@WtkGetNlsPackageFilename@SYNTAX
This function determines a fully qualified pathname for a language specific
file stored in a path relative to the pathname of a calling DLL,
matching the language of the system or a fallback language.

@@WtkGetNlsPackageFilename@PARM@pfnMod@in
Address of a function residing in the calling DLL code or NULL.
:p.
This parameter determines if the path of the calling DLL or the executable
is used as a base path. If this parameter is NULL, the path of the
executable is used as a base for the resulting filename.
:p.
For easy code maintenance, specifying :hp2.(PFN)__FUNCTION__:ehp2.
is always valid.

@@WtkGetNlsPackageFilename@PARM@pszDefaultLanguage@in
Address of the optional ASCIIZ default language in the two- or three
letter ISO 639 format.
:p.
The language specified here is used if no file can be found for
either
:ul compact.
:li.the user or system language or, if specified,
:li.the language specified by the environment variable determined
by the parameter :hp1.pszEnvVar:ehp1..
:eul.
:p.
The language specified here should be the fallback language. If no
fallback language is desired, specify :hp2.NULL:ehp2. instead.

@@WtkGetNlsPackageFilename@PARM@pszEnvVar@in
Address of the ASCIIZ name of the environment variable to be used
instead of the system and the locale language.
:p.
The value of the environment variable must comply to the
:link reftype=hd res=3000.two- or three letter ISO 639-1 or ISO 639-2 format:elink.
for a language identifier.

@@WtkGetNlsPackageFilename@PARM@pszFileMaskPath@in
Address of the ASCIIZ name of a path with file entries,
including placeholders for a a language code in the
:link reftype=hd res=3000.two- or three letter ISO 639-1 or ISO 639-2 format:elink..
:p.
Each entry of the path consists of a path relatively to the
path of the executing module, and a filenname including
:ul compact.
:li.a %l as a place holder for the ISO 639 language code
of the user-defined language set in the default locale
object
:li.a %s as a place holder for the ISO 639 language code
of the system language, like defined by the SYSLEVEL.OS2 file.
:li.a %d as a place holder for the ISO 639 language code
of the language specified in the parameter pszDefaultLanguage
:eul.
:p.
In order to let the API include either the two- or three-letter
ISO 639-1 or ISO 639-2 language code, include length parameters within
the placholders such as %2s, %3s, %2l, %3l, %2d or %3d accordingly. If no
length parameter is specified, the three-letter ISO 639-2 language code
is inserted.
:p.
Examples of valid placeholders:
:ul compact.
:li."myapp%l.inf;inf\myapp%l.inf".
:li."myapp%s.inf;inf\myapp%s.inf".
:li."myapp%2l.inf;inf\myapp%2l.inf".
:li."myapp%3s.inf;inf\myapp%3s.inf".
:eul.

@@WtkGetNlsPackageFilename@PARM@pszBuffer@out
Address of a buffer, where the full qualified filename
is being returned.

@@WtkGetNlsPackageFilename@PARM@ulBuflen@in
The length, in bytes, of the buffer described by :hp1.pszBuffer:ehp1..

@@WtkGetNlsPackageFilename@RETURN
Return Code.
:p.
WtkGetNlsPackageFilename returns one of the following return codes&colon.

:parml compact break=none.
:pt.0
:pd.NO_ERROR
:pt.2
:pd.ERROR_FILE_NOT_FOUND
:pt.87
:pd.ERROR_INVALID_PARAMETER
:pt.111
:pd.ERROR_BUFFER_OVERFLOW
:eparml.
:p.
or return codes of the following functions
:ul compact.
:li. WtkGetModuleInfo
:li. DosQueryPathInfo
:eul.

@@WtkGetNlsPackageFilename@REMARKS
This function is called by
:link reftype=hd refid=WtkLoadNlsResourceModule.WtkLoadNlsResourceModule:elink.
and
:link reftype=hd refid=WtkLoadNlsInfFile.WtkLoadNlsInfFile:elink.
in order to find the language specific file to be loaded.
:p.
When calling this function, all entries within the file path specified with
parameter :hp1.pszFileMaskPath:ehp1. are
:ol compact.
:li.appended to the path of the executing module
:li.searched with replacing %s (first found is returned) by
:ol compact.
:li.the language identifier specified by the environment variable specified
by the parameter :hp1.pszEnvVar:ehp1., or if not specified
:li.the identifier specifiying the system language (determined by the
:hp2.Workplace Shell Toolkit:ehp2.).
:li.the identifier specified by the parameter :hp1.pszDefaultLanguage:ehp1.
:eol.
:eol.
@@
*/

// ------------------------------------------------

APIRET APIENTRY WtkGetNlsPackageFilename( PFN pfnMod, PSZ pszDefaultLanguage, PSZ pszEnvVar,
                                          PSZ pszFileMaskPath, PSZ pszBuffer, ULONG ulBuflen)
{
         APIRET         rc = NO_ERROR;

         PSZ            pszEnvLanguage     = NULL;
         PSZ            pszSystemLanguage  = NULL;
         PSZ            pszLocaleLanguage  = NULL;

         PLANGDEF       pldSystem  = NULL;
         PLANGDEF       pldLocale  = NULL;
         PLANGDEF       pldDefault = NULL;

         PPIB           ppib;
         PTIB           ptib;

         CHAR           szDir[ _MAX_PATH];
         CHAR           szFile[ _MAX_PATH];

         CHAR           szMaskEntry[ _MAX_PATH];
         PSZ            pszMaskEntry;
         PSZ            p;
         CHAR           szVariants[ 128];

do
   {
   // check parms
   if ((!pszFileMaskPath) ||
       (!pszBuffer))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // NOTE: we now translate all codes to the 639-1 code plus country identifier
   // this code is the most specific one !!!

   // check for default language
   pszDefaultLanguage = WtkTranslateLanguageCode( pszDefaultLanguage, WTK_LANGUAGEID_639_1C);
   if (rc != NO_ERROR)
      break;

   // check env var or system language
   if ((pszEnvVar) && (*pszEnvVar))
      pszEnvLanguage = getenv( pszEnvVar);
   if (pszEnvLanguage)
      {
      pszEnvLanguage = WtkTranslateLanguageCode( pszEnvLanguage, WTK_LANGUAGEID_639_1C);

      // use always this language !
      pszSystemLanguage = pszEnvLanguage;
      pszLocaleLanguage = pszEnvLanguage;
      }
   else
      {
      // determine languages of system and locale
      // for the second code, use the transform
      pszSystemLanguage  = WtkQuerySystemLanguage( WTK_LANGUAGEID_639_1C);
      pszLocaleLanguage  = WtkQueryLocaleLanguage( WTK_LANGUAGEID_639_1C);
      }

    // get gefinitions of languages in question
    pldSystem  = _getLangDef( pszSystemLanguage);
    pldLocale  = _getLangDef( pszLocaleLanguage);
    pldDefault = _getLangDef( pszDefaultLanguage);

   // determine directory of executable or DLL
   if (!pfnMod)
      {
      DosGetInfoBlocks( &ptib, &ppib);
      rc = DosQueryModuleName( ppib->pib_hmte, sizeof( szDir), szDir);
      }
   else
      {
               HMODULE        hmod;
               ULONG          ulObjNum;
               ULONG          ulOffs;

      rc = DosQueryModFromEIP( &hmod, &ulObjNum, sizeof( szDir), szDir, &ulOffs, (ULONG)pfnMod);
      rc = DosQueryModuleName( hmod, sizeof( szDir), szDir);
      }
   if (rc != NO_ERROR)
      break;
   // cut off filename from path
   strcpy( strrchr( szDir, '\\') + 1, "");

   // now process all file path entries
   rc = ERROR_FILE_NOT_FOUND;
   pszMaskEntry = pszFileMaskPath;
   while (*pszMaskEntry)
      {
      // isolate entry
      memset( szMaskEntry, 0, sizeof( szMaskEntry));
      p = strchr( pszMaskEntry, ';');
      if (p)
         {
         strncpy( szMaskEntry, pszMaskEntry, p - pszMaskEntry);
         pszMaskEntry += strlen( szMaskEntry) + 1;
         }
      else
         {
         strcpy( szMaskEntry, pszMaskEntry);
         pszMaskEntry += strlen( szMaskEntry);
         }

      //  check for a file matching any of the lanuages (or one of its variants)
      rc = _getLanguageFile( szFile, sizeof( szFile), szDir, szMaskEntry,
                             pldSystem, pldLocale, pldDefault);
      if (rc == NO_ERROR)
         // found a file !!!
         break;

      // for debugging purposes, place the mask into the target buffer
      // memset( pszBuffer, 0, ulBuflen);
      // strncpy( pszBuffer, szFile, ulBuflen - 1);

      } // while (*pszMaskEntry)

   if (rc != NO_ERROR)
      break;

   // hand over result, query full pathname
   rc = DosQueryPathInfo( szFile, FIL_QUERYFULLNAME, pszBuffer, ulBuflen);

   } while (FALSE);


// cleanup
if (pldSystem)  free( pldSystem);
if (pldLocale)  free( pldLocale);
if (pldDefault) free( pldDefault);
return rc;
}


// ---------------------------------------------------------------------------

/*
@@WtkLoadNlsResourceModule@SYNTAX
This function loads a language specific resource module
stored in a path relative to the pathname of a calling DLL,
matching the language of the system or a fallback language.

@@WtkLoadNlsResourceModule@PARM@pfnMod@in
Address of a function residing in the calling DLL code or NULL.
:p.
This parameter determines if the path of the calling DLL or the executable
is used as a base path. If this parameter is NULL, the path of the
executable is used as a base for the resulting filename.
:p.
For easy code maintenance, specifying :hp2.(PFN)__FUNCTION__:ehp2.
is always valid.

@@WtkLoadNlsResourceModule@PARM@phmod@out
Address of a variable receiving the module handle of the loaded
resource module.

@@WtkLoadNlsResourceModule@PARM@pszDefaultLanguage@in
Address of the optional ASCIIZ default language in the two- or three
letter ISO 639 format.
:p.
The language specified here is used if no file can be found for
either
:ul compact.
:li.the system language or, if specified,
:li.the language specified by the environment variable determined
by the parameter :hp1.pszEnvVar:ehp1..
:eul.
:p.
The language specified here should be the fallback language. If no
fallback language is desired, specify :hp2.NULL:ehp2. instead.

@@WtkLoadNlsResourceModule@PARM@pszEnvVar@in
Address of the ASCIIZ name of the environment variable to be used
instead of the system language.
:p.
The value of the environment variable must comply to the
:link reftype=hd res=3000.two- or three letter ISO 639-1 or ISO 639-2 format:elink.
for a language identifier.

@@WtkLoadNlsResourceModule@PARM@pszFileMaskPath@in
Address of the ASCIIZ name of a path with file entries,
including placeholders for a a language code in the
:link reftype=hd res=3000.two- or three letter ISO 639-1 or ISO 639-2 format:elink..
:p.
Each entry of the path consists of a path relatively to the
path of the executing module, and a filenname including
:ul compact.
:li.a %l as a place holder for the ISO 639 language code
of the user-defined language set in the default locale
object
:li.a %s as a place holder for the ISO 639 language code
of the system language, like defined by the SYSLEVEL.OS2 file.
:li.a %d as a place holder for the ISO 639 language code
of the language specified in the parameter pszDefaultLanguage
:eul.
:p.
In order to let the API include either the two- or three-letter
ISO 639-1 or ISO 639-2 language code, include length parameters within
the placholders such as %2s, %3s, %2l, %3l, %2d or %3d accordingly. If no
length parameter is specified, the three-letter ISO 639-2 language code
is inserted.
:p.
Examples of valid placeholders:
:ul compact.
:li."myapp%l.inf;inf\myapp%l.inf".
:li."myapp%s.inf;inf\myapp%s.inf".
:li."myapp%2l.inf;inf\myapp%2l.inf".
:li."myapp%3s.inf;inf\myapp%3s.inf".
:eul.


@@WtkLoadNlsResourceModule@RETURN
Return Code.
:p.
WtkLoadNlsResourceModule returns one of the following return codes&colon.

:parml compact break=none.
:pt.0
:pd.NO_ERROR
:pt.2
:pd.ERROR_FILE_NOT_FOUND
:pt.87
:pd.ERROR_INVALID_PARAMETER
:pt.111
:pd.ERROR_BUFFER_OVERFLOW
:eparml.
:p.
or return codes of the following functions
:ul compact.
:li. WtkGetNlsPackageFilename
:li. DosLoadModule
:eul.

@@WtkLoadNlsResourceModule@REMARKS
This function calls
:link reftype=hd refid=WtkGetNlsPackageFilename.WtkGetNlsPackageFilename:elink.
to find the language specific resource module to be loaded.
:p.
When calling this function, all entries within the file path specified with
parameter :hp1.pszFileMaskPath:ehp1. are
:ol compact.
:li.appended to the path of the executing module
:li.searched with replacing %s (first found is returned) by
:ol compact.
:li.the language identifier specified by the environment variable specified
by the parameter :hp1.pszEnvVar:ehp1., or if not specified
:li.the identifier specifiying the system language (determined by the
:hp2.Workplace Shell Toolkit:ehp2.).
:li.the identifier specified by the parameter :hp1.pszDefaultLanguage:ehp1.
:eol.
:eol.
:p.
In order to load a resource file being stored
relative to the pathname of a calling executable, call
:link reftype=hd refid=WtkLoadNlsResourceModule.WtkLoadNlsResourceModule:elink.
instead.
@@
*/

APIRET APIENTRY WtkLoadNlsResourceModule( PFN pfnMod, PHMODULE phmod, PSZ pszDefaultLanguage,
                                          PSZ pszEnvVar, PSZ pszFileMaskPath)
{
         APIRET         rc = NO_ERROR;

         CHAR           szLanguageModule[ _MAX_PATH];
         CHAR           szError[ 20];

do
   {
   // determine filename
   rc = WtkGetNlsPackageFilename( pfnMod, pszDefaultLanguage, pszEnvVar, pszFileMaskPath,
                                     szLanguageModule, sizeof( szLanguageModule));
   if (rc != NO_ERROR)
      break;

   // load resource file
   rc = DosLoadModule( szError, sizeof( szError), szLanguageModule, phmod);
   if (rc != NO_ERROR)
      break;

   } while (FALSE);

return rc;
}

// ---------------------------------------------------------------------------

/*
@@WtkLoadNlsInfFile@SYNTAX
This function loads a language specific INF file
stored in a path relative to the pathname of a calling DLL,
matching the language of the system or a fallback language.

@@WtkLoadNlsInfFile@PARM@pfnMod@in
Address of a function residing in the calling DLL code or NULL.
:p.
This parameter determines if the path of the calling DLL or the executable
is used as a base path. If this parameter is NULL, the path of the
executable is used as a base for the resulting filename.
:p.
For easy code maintenance, specifying :hp2.(PFN)__FUNCTION__:ehp2.
is always valid.

@@WtkLoadNlsInfFile@PARM@pszDefaultLanguage@in
Address of the optional ASCIIZ default language in the two- or three
letter ISO 639 format.
:p.
The language specified here is used if no file can be found for
either
:ul compact.
:li.the system language or, if specified,
:li.the language specified by the environment variable determined
by the parameter :hp1.pszEnvVar:ehp1..
:eul.
:p.
The language specified here should be the fallback language. If no
fallback language is desired, specify :hp2.NULL:ehp2. instead.

@@WtkLoadNlsInfFile@PARM@pszEnvVar@in
Address of the ASCIIZ name of the environment variable to be used
instead of the system language.
:p.
The value of the environment variable must comply to the
:link reftype=hd res=3000.two- or three letter ISO 639-1 or ISO 639-2 format:elink.
for a language identifier.

@@WtkLoadNlsInfFile@PARM@pszFileMaskPath@in
Address of the ASCIIZ name of a path with file entries,
including placeholders for a a language code in the
:link reftype=hd res=3000.two- or three letter ISO 639-1 or ISO 639-2 format:elink..
:p.
Each entry of the path consists of a path relatively to the
path of the executing module, and a filenname including
:ul compact.
:li.a %l as a place holder for the ISO 639 language code
of the user-defined language set in the default locale
object
:li.a %s as a place holder for the ISO 639 language code
of the system language, like defined by the SYSLEVEL.OS2 file.
:li.a %d as a place holder for the ISO 639 language code
of the language specified in the parameter pszDefaultLanguage
:eul.
:p.
In order to let the API include either the two- or three-letter
ISO 639-1 or ISO 639-2 language code, include length parameters within
the placholders such as %2s, %3s, %2l, %3l, %2d or %3d accordingly. If no
length parameter is specified, the three-letter ISO 639-2 language code
is inserted.
:p.
Examples of valid placeholders:
:ul compact.
:li."myapp%l.inf;inf\myapp%l.inf".
:li."myapp%s.inf;inf\myapp%s.inf".
:li."myapp%2l.inf;inf\myapp%2l.inf".
:li."myapp%3s.inf;inf\myapp%3s.inf".
:eul.

@@WtkLoadNlsInfFile@PARM@pszTopic@in
Address of the ASCIIZ name of the topic to be viewed.
:p.
This parameter is optional and may be :hp2.NULL:ehp2..

@@WtkLoadNlsInfFile@RETURN
Return Code.
:p.
WtkLoadNlsInfFile returns one of the following return codes&colon.

:parml compact break=none.
:pt.0
:pd.NO_ERROR
:eparml.
:p.
or return codes of the following functions
:ul compact.
:li. WtkGetNlsPackageFilename
:eul.

@@WtkLoadNlsInfFile@REMARKS
This function calls
:link reftype=hd refid=WtkGetNlsPackageFilename.WtkGetNlsPackageFilename:elink.
to find the language specific INF file to be viewed.
:p.
When calling this function, all entries within the file path specified with
parameter :hp1.pszFileMaskPath:ehp1. are
:ol compact.
:li.appended to the path of the executing module
:li.searched with replacing %s (first found is returned) by
:ol compact.
:li.the language identifier specified by the environment variable specified
by the parameter :hp1.pszEnvVar:ehp1., or if not specified
:li.the identifier specifiying the system language (determined by the
:hp2.Workplace Shell Toolkit:ehp2.).
:li.the identifier specified by the parameter :hp1.pszDefaultLanguage:ehp1.
:eol.
:eol.
:p.
In order to load an INF file being stored
relative to the pathname of a calling executable, call
:link reftype=hd refid=WtkLoadNlsInfFile.WtkLoadNlsInfFile:elink.
instead.
@@
*/

APIRET APIENTRY WtkLoadNlsInfFile( PFN pfnMod, PSZ pszDefaultLanguage, PSZ pszEnvVar,
                                   PSZ pszFileMaskPath, PSZ pszTopic)
{
         APIRET         rc = NO_ERROR;
         HMODULE        hmod = NULLHANDLE;


         CHAR           szInf[ _MAX_PATH];

         STARTDATA      stdata;
         CHAR           szParms[2 *  _MAX_PATH];
         ULONG          ulSession;
         PID            pid;
do
   {

   // determine INF
   rc = WtkGetNlsPackageFilename( pfnMod, pszDefaultLanguage, pszEnvVar,
                                  pszFileMaskPath, szInf, sizeof( szInf));
   if (rc != NO_ERROR)
      break;

   // launch INF viewer - ignore error
   strcpy( szParms, szInf);
   if (pszTopic)
      sprintf( &szParms[ strlen( szParms)], " \"%s\"", pszTopic);
   memset( &stdata, 0, sizeof( stdata));
   stdata.Length    = sizeof( stdata);
   stdata.PgmName   = "VIEW.EXE";
   stdata.PgmInputs = szParms;
   DosStartSession( &stdata, &ulSession, &pid);

   } while (FALSE);

return rc;
}

// ---------------------------------------------------------------------------

/*
@@WtkTranslateLanguageCode@SYNTAX
This function translates an ASCIIZ ISO 639 language code into the
specified target format of two- or three-letter code according
to ISO 639-1 or ISO 639-2.

@@WtkTranslateLanguageCode@PARM@pszLanguageCode@in
The address of an ASCIIZ ISO 639 language code.

@@WtkTranslateLanguageCode@PARM@ulIdType@in
The type of the language code to be returned.
:parml compact.
:pt.WTK_LANGUAGEID_639_1
:pd.returns the two-letter language code according to ISO 639-1
:pt.WTK_LANGUAGEID_639_1C
:pd.returns the two-letter language code according to ISO 639-1,
with the country code appended according to the RFC 3066.
:pt.WTK_LANGUAGEID_639_2
:pd.returns the three-letter language code according to ISO 639-2
:eparml.

@@WtkTranslateLanguageCode@RETURN
Pointer to an ASCIIZ ISO 639 language code matching the language
of the provided language code.
:p.
Only languages supported by the locale API are translated. If a non-supported
language code is provided, the appropriate language code for the english
language is returned.

@@WtkTranslateLanguageCode@REMARKS
- none -

@@
*/

PSZ APIENTRY WtkTranslateLanguageCode( PSZ pszLanguageCode, ULONG ulIdType)
{
return GETCODE( pszLanguageCode, ulIdType);
}

// ---------------------------------------------------------------------------

/*
@@WtkQueryLanguageVariants@SYNTAX
This function returns a comma separated list of ASCIIZ ISO 639-2 language codes of
all variants matching a given two- or three-letter code according
to ISO 639-1 or ISO 639-2.

@@WtkQueryLanguageVariants@PARM@pszLanguageCode@in
The address of an  ASCIIZ two- or three-letter code according to
ISO 639-1 or ISO 639-2.

@@WtkQueryLanguageVariants@PARM@ulIdType@in
The type of the language code to be returned.
:parml compact.
:pt.WTK_LANGUAGEID_639_1C
:pd.returns the two-letter language code according to ISO 639-1,
with the country code appended according to the RFC 3066.
:pt.WTK_LANGUAGEID_639_2
:pd.returns the three-letter language code according to ISO 639-2
:eparml.
For WTK_LANGUAGEID_639_1, for obvious reason no language variants can be returned,
so instead the variants are returned for WTK_LANGUAGEID_639_1C.

@@WtkQueryLanguageVariants@PARM@pszBuffer@out
Address of a buffer receiving the variant list.
:p.
If the language code specified in the parameter :hp1.pszLanguageCode:ehp1.
is a three-letter code according to ISO 639-2, this is included in the
returned variant list.

@@WtkQueryLanguageVariants@PARM@ulBuflen@in
The length, in bytes, of the buffer described by :hp1.pszBuffer:ehp1..

@@WtkQueryLanguageVariants@RETURN
Pointer to an ASCIIZ ISO 639 language code matching the language
of the provided language code.
:p.
Only languages and its variants supported by the locale API are translated.
If a non-supported language code is provided, the appropriate language code
and its variants for the english language are returned.

@@WtkQueryLanguageVariants@REMARKS
- None -
@@
*/

PSZ APIENTRY WtkQueryLanguageVariants( PSZ pszLanguageCode, ULONG ulIdType, PSZ pszBuffer, ULONG ulBuflen)
{

         PSZ            pszResult = pszLanguageCode;

if (ulIdType == WTK_LANGUAGEID_639_1)
   ulIdType = WTK_LANGUAGEID_639_1C;

switch (ulIdType)
  {
  // convert language ids
  case WTK_LANGUAGEID_639_1:
  case WTK_LANGUAGEID_639_1C:
  case WTK_LANGUAGEID_639_2:
    pszResult = GETVARIANTSP( pszLanguageCode, ulIdType, pszBuffer, ulBuflen);
    break;

  // don't touch any other values
  default:
    break;
  }

return pszResult;
}

