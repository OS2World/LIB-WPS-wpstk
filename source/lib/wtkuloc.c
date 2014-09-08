/****************************** Module Header ******************************\
*
* Module Name: wtkuloc.c
*
* Source for locale helper functions
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2007
*
* $Id: wtkuloc.c,v 1.12 2009-11-18 22:18:14 cla Exp $
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
#include <unidef.h>
#include <uconv.h>
#include <callconv.h>

#include "wtkuloc.h"
#include "wtkulmd.h"
#include "wpstk.ih"

// ==========================================================================
//     wrappers for required functions of LIBUNI.DLL
// ==========================================================================

typedef int CALLCONV FNUNISTRFROMUCS( 
																			UconvObject   co,        /* I  - Conversion object           */
																			char        * target,    /* O  - Buffer for converted string */
																			UniChar     * source,    /* I  - String to convert           */
																			int           len);      /* I  - Bytes in target buffer      */
typedef FNUNISTRFROMUCS *PFNUNISTRFROMUCS;

int CALLCONV UniStrFromUcs(
																			UconvObject   co,        /* I  - Conversion object           */
																			char        * target,    /* O  - Buffer for converted string */
																			UniChar     * source,    /* I  - String to convert           */
																			int           len)       /* I  - Bytes in target buffer      */
{
static   PFNUNISTRFROMUCS pfnNUNISTRFROMUCS = NULL;

if (!pfnNUNISTRFROMUCS)
   pfnNUNISTRFROMUCS =
      (PFNUNISTRFROMUCS) __loadDllFunc( "LIBUNI", "UniStrFromUcs");

if (pfnNUNISTRFROMUCS)
   return pfnNUNISTRFROMUCS(   
																			  co,        /* I  - Conversion object           */
																			  target,    /* O  - Buffer for converted string */
																			  source,    /* I  - String to convert           */
																			  len);      /* I  - Bytes in target buffer      */
else
   return ULS_OTHER;
}
// --------------------------------------------------------------------------

typedef int CALLCONV FNUNIFREEUCONVOBJECT( UconvObject   uobj );
typedef FNUNIFREEUCONVOBJECT *PFNUNIFREEUCONVOBJECT;

int CALLCONV UniFreeUconvObject(  UconvObject   uobj )  
                                   
{
static   PFNUNIFREEUCONVOBJECT pfnNUNIFREEUCONVOBJECT = NULL;

if (!pfnNUNIFREEUCONVOBJECT)
   pfnNUNIFREEUCONVOBJECT =
      (PFNUNIFREEUCONVOBJECT) __loadDllFunc( "LIBUNI", "UniFreeUconvObject");

if (pfnNUNIFREEUCONVOBJECT)
   return pfnNUNIFREEUCONVOBJECT(   uobj ); 
else
   return ULS_OTHER;
}
// --------------------------------------------------------------------------

typedef int CALLCONV FNUNICREATEUCONVOBJECT( UniChar     * code_set,   /* I  - Unicode name of uconv table */
																						 UconvObject * uobj  );    /* O  - Uconv object handle         */
typedef FNUNICREATEUCONVOBJECT *PFNUNICREATEUCONVOBJECT;

int CALLCONV UniCreateUconvObject(  UniChar     * code_set,   /* I  - Unicode name of uconv table */ 
																		UconvObject * uobj  )     /* O  - Uconv object handle         */ 
                                   
{
static   PFNUNICREATEUCONVOBJECT pfnNUNICREATEUCONVOBJECT = NULL;

if (!pfnNUNICREATEUCONVOBJECT)
   pfnNUNICREATEUCONVOBJECT =
      (PFNUNICREATEUCONVOBJECT) __loadDllFunc( "LIBUNI", "UniCreateUconvObject");

if (pfnNUNICREATEUCONVOBJECT)
   return pfnNUNICREATEUCONVOBJECT(   code_set,   /* I  - Unicode name of uconv table */ 
																		 * uobj  );    /* O  - Uconv object handle         */ 
else
   return ULS_OTHER;
}
// --------------------------------------------------------------------------

typedef int CALLCONV FNUNICREATELOCALEOBJECT( int locale_spec_type,
                                              const void *locale_spec,
                                              LocaleObject *locale_object_ptr);
typedef FNUNICREATELOCALEOBJECT *PFNUNICREATELOCALEOBJECT;

int CALLCONV UniCreateLocaleObject( int locale_spec_type,
                                    const void *locale_spec,
                                    LocaleObject *locale_object_ptr)
{
static   PFNUNICREATELOCALEOBJECT pfnNUNICREATELOCALEOBJECT = NULL;

if (!pfnNUNICREATELOCALEOBJECT)
   pfnNUNICREATELOCALEOBJECT =
      (PFNUNICREATELOCALEOBJECT) __loadDllFunc( "LIBUNI", "UniCreateLocaleObject");

if (pfnNUNICREATELOCALEOBJECT)
   return pfnNUNICREATELOCALEOBJECT( locale_spec_type, locale_spec,
                                     locale_object_ptr);
else
   return ULS_OTHER;
}
// --------------------------------------------------------------------------

typedef int CALLCONV FNUNIFREELOCALEOBJECT( LocaleObject locale_object);
typedef FNUNIFREELOCALEOBJECT *PFNUNIFREELOCALEOBJECT;

int CALLCONV UniFreeLocaleObject( LocaleObject locale_object)
{
static   PFNUNIFREELOCALEOBJECT pfnUNIFREELOCALEOBJECT = NULL;

if (!pfnUNIFREELOCALEOBJECT)
    pfnUNIFREELOCALEOBJECT =
      (PFNUNIFREELOCALEOBJECT) __loadDllFunc( "LIBUNI", "UniFreeLocaleObject");

if (pfnUNIFREELOCALEOBJECT)
   return pfnUNIFREELOCALEOBJECT( locale_object);
else
   return ULS_OTHER;
}

// --------------------------------------------------------------------------

typedef int CALLCONV FNUNIQUERYLOCALEITEM( const LocaleObject locale_object,
                                           LocaleItem item,
                                           UniChar **info_item_addr_ptr);
typedef FNUNIQUERYLOCALEITEM *PFNUNIQUERYLOCALEITEM;

int CALLCONV UniQueryLocaleItem( const LocaleObject locale_object,
                                 LocaleItem item,
                                 UniChar **info_item_addr_ptr)
{
static   PFNUNIQUERYLOCALEITEM  pfnUNIQUERYLOCALEITEM= NULL;

if (!pfnUNIQUERYLOCALEITEM)
   pfnUNIQUERYLOCALEITEM =
      (PFNUNIQUERYLOCALEITEM) __loadDllFunc( "LIBUNI", "UniQueryLocaleItem");

if (pfnUNIQUERYLOCALEITEM)
   return pfnUNIQUERYLOCALEITEM( locale_object, item, info_item_addr_ptr);
else
   return ULS_OTHER;
}

// --------------------------------------------------------------------------

typedef int CALLCONV FNUNIFREEMEM(  void *memory_ptr);
typedef FNUNIFREEMEM *PFNUNIFREEMEM;

int CALLCONV UniFreeMem( void *memory_ptr)
{
static   PFNUNIFREEMEM pfnUNIFREEMEM = NULL;

if (!pfnUNIFREEMEM)
   pfnUNIFREEMEM =
      (PFNUNIFREEMEM) __loadDllFunc( "LIBUNI", "UniFreeMem");

if (pfnUNIFREEMEM)
   return pfnUNIFREEMEM( memory_ptr);
else
   return ULS_OTHER;
}

// --------------------------------------------------------------------------

typedef int CALLCONV FNUNISTRLEN( const UniChar *ucs1 );
typedef FNUNISTRLEN *PFNUNISTRLEN;

size_t CALLCONV UniStrlen( const UniChar *ucs1)
{
static   PFNUNISTRLEN pfnUNISTRLEN = NULL;

if (!pfnUNISTRLEN)
   pfnUNISTRLEN =
      (PFNUNISTRLEN) __loadDllFunc( "LIBUNI", "UniStrlen");

if (pfnUNISTRLEN)
   return pfnUNISTRLEN( ucs1);
else
   return 0;
}

// ==========================================================================
//     end of wrappers
// ==========================================================================

// helper function to query a locale item and return it as single char str
static APIRET _queryLocaleStrItem( LocaleObject localeObject,
                                   UconvObject uconvObject,
                                   ULONG ulObjectItem, PSZ pszBuffer, ULONG ulBuflen)
{
         APIRET         rc = NO_ERROR;
         UniChar       *pucItem = NULL;

do
   {
   // check parms
   if ((!localeObject) ||
       (!pszBuffer))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // initialize
   memset( pszBuffer, 0, ulBuflen);

   // query item
   rc = UniQueryLocaleItem( localeObject, ulObjectItem, &pucItem);
   if (rc != NO_ERROR)
      break;

   // check size
   if (UniStrlen( pucItem) + 1 > ulBuflen)
      {
      rc = ERROR_BUFFER_OVERFLOW;
      break;
      }

   // convert unicode string
   rc = UniStrFromUcs( uconvObject, pszBuffer, pucItem, ulBuflen);
   if (rc != NO_ERROR)
      break;

   } while (FALSE);

// cleanup
if (pucItem) UniFreeMem( pucItem);
return rc;
}

// ---------------------------------------------------------------------------

/*
@@WtkQueryLocaleLanguage@SYNTAX
This function returns a pointer to an ASCIIZ ISO 639 language
code defined in the default locale object.

@@WtkQueryLocaleLanguage@PARM@ulIdType@in
The type of the language code to be returned.
:parml compact.
:pt.WTK_LANGUAGEID_639_1
:pd.returns the two-letter language code according to ISO 639-1
:pt.WTK_LANGUAGEID_639_2
:pd.returns the three-letter language code according to ISO 639-2
:pt.WTK_LANGUAGEAME_ENGLISH
:pd.returns the english name of the language
:pt.WTK_LANGUAGEAME_NATIVE
:pd.returns the native name of the language
:eparml.

@@WtkQueryLocaleLanguage@RETURN
Pointer to an ASCIIZ ISO 639 language code or language name defined in the default locale object.
:p.
If for any reason the language cannot be determined properly,
the values for the english language are returned as default,
according to the value specified as parameter ulIdType.

@@WtkQueryLocaleLanguage@REMARKS
The language is determined from the locale item LOCI_sLanguageID. The item
LOCI_sAbbrevLangName is never used, as this is not changed accordingly by the
API, if the item LOCI_sLanguageID gets changed.
:note.
:ul compact.
:li.In order to determine the system language instead, so the language of the operating
system, use :link reftype=hd refid=WtkQuerySystemLanguage.WtkQuerySystemLanguage:elink. instead.
:eul.
:p.
The following ISO 639-1/-2 language codes are currently supported by the Locale API&colon.
:parml compact tsize=12 break=none.
:pt.ar/ara
:pd.Arabic
:pt.be/bel
:pd.Belarusian
:pt.bg/bgr
:pd.Bulgarian
:pt.ca/cat
:pd.Catalan
:pt.cs/csy
:pd.Czech
:pt.da/dan
:pd.Danish
:pt.de/deu
:pd.German
:pt.el/ell
:pd.Greek
:pt.en/eng
:pd.English
:pt.es/esp
:pd.Spanish
:pt.et/eta
:pd.Estonian
:pt.eu/eus
:pd.Basque
:pt.fa/far
:pd.Farsi
:pt.fi/fin
:pd.Finnish
:pt.fr/fra
:pd.French
:pt.he/heb
:pd.Hebrew
:pt.hr/hrv
:pd.Croatian
:pt.hr/shl
:pd.Romanian
:pt.hu/hun
:pd.Hungarian
:pt.in/ind
:pd.Indonesian
:pt.is/isl
:pd.Icelandic
:pt.it/ita
:pd.Italian
:pt.ja/jap
:pd.Japanese
:pt.ko/kor
:pd.Korean
:pt.lt/lta
:pd.Lithuanian
:pt.lv/lva
:pd.Latvian
:pt.mk/mkd
:pd.Macedonian
:pt.nl/nld
:pd.Dutch
:pt.no/nor
:pd.Norwegian
:pt.pl/plk
:pd.Polish
:pt.pt/ptg
:pd.Portuguese
:pt.ro/rom
:pd.Romanian
:pt.ru/rus
:pd.Russian
:pt.sh/shl
:pd.Serbo-Croatian
:pt.sk/sky
:pd.Slovak
:pt.sl/slo
:pd.Slovene
:pt.sq/sqi
:pd.Albanian
:pt.sr/shc
:pd.Serbian (Cyrillic)
:pt.sv/sve
:pd.Swedish
:pt.th/tha
:pd.Thai
:pt.tr/tkr
:pd.Turkish
:pt.uk/ukr
:pd.Ukrainian
:pt.vi/vie
:pd.Vietnamese
:pt.zh/cht
:pd.Chinese
:eparml.

@@
*/

PSZ APIENTRY WtkQueryLocaleLanguage( ULONG ulIdType)
{
         PSZ            pszResult = "";
         APIRET         rc = NO_ERROR;

         LocaleObject   localeObject   = NULL;
         UconvObject    uconvObject    = NULL;
         ULONG          ulObjectItemId = 0;
static   CHAR           szLanguage[ 8];


do
   {
   // determine what to query
   switch (ulIdType)
      {

      case WTK_LANGUAGEID_639_1:
      case WTK_LANGUAGEID_639_1C:
      case WTK_LANGUAGEID_639_2:
         ulObjectItemId = LOCI_sLanguageID;
         break;

      case WTK_LANGUAGENAME_ENGLISH:
         ulObjectItemId = LOCI_sEngLanguage;
         break;

      case WTK_LANGUAGENAME_NATIVE:
         ulObjectItemId = LOCI_sLanguage;
         break;
      }
   if (!ulObjectItemId)
      break;

   // setup default
   pszResult = WtkQueryLanguageVariants( NULL, 0, NULL, 0);

#ifdef DEBUG
      {
               CHAR           szTestLanguage[ 16];

      if (PrfQueryProfileString( HINI_USER, "WPSTK", "Debug_LocaleLanguage", NULL,
                                 szTestLanguage, sizeof( szTestLanguage)))
         {
         pszResult = WtkTranslateLanguageCode( szTestLanguage, ulIdType);
         break;
         }

      }
#endif

   // get conversion object
   rc = UniCreateUconvObject( (UniChar *)L"", &uconvObject);
   if (rc != NO_ERROR)
      break;

   // get locale object
   rc = UniCreateLocaleObject( UNI_UCS_STRING_POINTER, (UniChar *)L"", &localeObject);
   if (rc != NO_ERROR)
      break;

   // get language
   rc =  _queryLocaleStrItem( localeObject, uconvObject, ulObjectItemId,
                              szLanguage, sizeof( szLanguage));
   if (rc != NO_ERROR)
      break;

   // transform to language code of correct length,
   // and also always return a pointer to an entry in the table
   pszResult = WtkQueryLanguageVariants( szLanguage, ulIdType, NULL, 0);

   } while (FALSE);

// cleanup
if (localeObject) UniFreeLocaleObject( localeObject);
if (uconvObject)  UniFreeUconvObject( uconvObject);
return pszResult;
}

