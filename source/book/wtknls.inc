..******************************* Module Header *******************************
..*
..* Module Name: wtknls.inc
..*
..* Hypertext/2 include file for programmers guide section on
..* NLS related functions
..*
..* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
..*
..* $Id: wtknls.inc,v 1.7 2007-02-16 22:09:50 cla Exp $
..*
..* ===========================================================================
..*
..* This file is part of the WPS Toolkit package and is free software.  You can
..* redistribute it and/or modify it under the terms of the GNU Library General
..* Public License as published by the Free Software Foundation, in version 2
..* as it comes in the "COPYING.LIB" file of the WPS Toolkit main distribution.
..* This library is distributed in the hope that it will be useful, but WITHOUT
..* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
..* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
..* License for more details.
..*
..*****************************************************************************

.. ----------------------------------------
.1 NLS related functions
.. ----------------------------------------
.an IDPNL_APINLS
.
[=TOPICS]
.su V30 1

.. ----------------------------------------
.2 NLS related functions Overview
.. ----------------------------------------
.an IDPNL_APINLS_OVERVIEW
.
The National Language Support (NLS) related functions of the [=TITLE] greatly
ease the handling of language specific files, such as
.ul
- determining the [.IDPNL_APINLS_LANGCODE ISO 639 code] of the
  [.IDPNL_APINLS_GETLANG language] to be used, either 
  the [.IDPNL_APINLS_GETLANG_LOCALE user selected language]
  or the [.IDPNL_APINLS_GETLANG_SYS system language]
- locating or even loading language specific files of an application, where the 
  API automatically
  - determines the path where the application resides
  - determines the directory where the application executable resides
  - searches a path of pathname specifications for a given file for either the
    system language and optional for a [.IDPNL_APINLS_GETLANG_DEFAULT default or fallback language].

.. ----------------------------------------
.2 ISO 639 language codes
.. ----------------------------------------
.an IDPNL_APINLS_LANGCODE
.res 3000
.
The ISO 639 definition of language codes are used 
to identify and determine a language. This specification
is separated into two parts, defining two and tree-letter
codes.

[=TOPICS]
.su H50

.. ----------------------------------------
.3 ISO 639-1 two-letter language codes
.. ----------------------------------------
.an IDPNL_APINLS_LANGCODE_ISO639-1
.
The two-letter languages codes of the ISO 639-1 standard
are enlisted together in the code list of the 
[.IDPNL_APINLS_LANGCODE_ISO639-2 ISO 639-2 Registration authority].

The two-letter language code is most often (and so in eComStation)
used in conjunction with a country code, so that a language can be
associated to multiple countries, or also multiple languages to one country.
For determining the language this is not of importance though.
See the  the following web pages for more informations:
.ul compact
- [http://www.iso.org/iso/en/prods-services/iso3166ma/02iso-3166-code-lists/list-en1.html ISO 3166 - English country names and code elements]
- [http://www.ietf.org/rfc/rfc3066.txt Request for comment 3066 - Tags for the Identification of Languages]

.. ----------------------------------------
.3 ISO 639-2 three-letter language codes
.. ----------------------------------------
.an IDPNL_APINLS_LANGCODE_ISO639-2
.
The official site of the ISO 639-2 Registration Authority 
is [http://www.loc.gov/standards/iso639-2/], there you can 
view and download the current list of both two- and three letter
language codes of ISO 639-1 and 639-2 format.

Note that for many languages, due to historic reasons, there is
a bibliograhpic code (in english language) and a terminology 
code (in native language). With the OS/2 and ecomStation Locale 
API the terminology codes are used.

[=NOTE]
.ul compact
- The Locale Objects of the Internationalization Toolkit included 
  in *OS/2* and *eComStation* support a subset of the languages defined
  in ISO 639.

.. ----------------------------------------
.2 determining the language to be used
.. ----------------------------------------
.an IDPNL_APINLS_GETLANG
.
If an application supports several languages, it is good practise to launch
an application with the language best suitable for the user. In most cases
this is the language selected by the user, in other cases it may be the
language of the operating system. 

[=TOPICS]
.su H50

.. ----------------------------------------
.3 Determining the user selected language
.. ----------------------------------------
.an IDPNL_APINLS_GETLANG_LOCALE
.
For normal enduser applications it makes sense to select the language that was chosen by
the user on installation of the operating system. This is especially important if the
operating system is not available in the language of the user, but a given enduser
application is.

Herefore the API [.WtkQueryLocaleLanguage WtkQueryLocaleLanguage] supports you in
querying the system language and returning a two- or three-letter language code
in the ISO 639 format. 

[=NOTE]
.ul compact
- The Locale Objects of the Internationalization Toolkit included 
  in *OS/2* and *eComStation* support a subset of the languages defined
  in ISO 639.
  See the [.RM_WtkQueryLocaleLanguage Remarks on WtkQueryLocaleLanguage]
  for a list of the languages supported by the Locale API.

.. ----------------------------------------
.3 determining the system language
.. ----------------------------------------
.an IDPNL_APINLS_GETLANG_SYS
.
For system near applications and utilities it may make sense to select the language 
of the operating system. This may be the case if it is particular of interest
that an application seamlessly fits into the language of the operating system, 
as for example system near applications and installers.

Herefore the API [.WtkQuerySystemLanguage WtkQuerySystemLanguage] supports you in
querying the system language and returning a two- or three-letter language code
in the ISO 639 format.

[=NOTE]
.ul compact
- The system language is determined from the SYSLEVEL.OS2 file within the 
  \OS2\INSTALL directory. Since within that file the language is coded in 
  a single character of the CSD identifier, only a small subset of the
  languages of ISO 639 can be supported by this.
- See the [.RM_WtkQuerySystemLanguage Remarks on WtkQuerySystemLanguage]
  for a list of the languages reckognized by this API.

.. ----------------------------------------
.3 selecting the default or fallback language
.. ----------------------------------------
.an IDPNL_APINLS_GETLANG_DEFAULT
.
For systems using a language not supported by your application,
it is recommended to support a default or fallback language, which
should be understandable by most of the users of your application.

Depending on your user community, either the english, french or
spanish language may be a good choice, as they are widely spread.

.. ----------------------------------------
.3 converting between language codes of ISO 639-1 and ISO 639-2
.. ----------------------------------------
.an IDPNL_APINLS_GETLANG_TRANSLATECODE
.
When having determined an ISO 639 language code in either the two- or
three-letter variant of [.IDPNL_APINLS_LANGCODE ISO 639-1 and ISO 639-2],
it can be converted to the other variant using
[.WtkTranslateLanguageCode WtkTranslateLanguageCode].

.. ----------------------------------------
.3 problems with using the country id for language determination
.. ----------------------------------------
.an IDPNL_APINLS_GETLANG_COUNTRY
.
A widely made assumption is that the country identifier set for the system
would be a good indicator for the preferred or best suitable language,
but this in certain cases is not true.

Just think of countries with more than one official language, such as
Switzerland and Belgium. Here the country identifier cannot be used to 
uniquely determine the language to be used.

.. ----------------------------------------
.2 determining a file of the application
.. ----------------------------------------
.an IDPNL_APINLS_DETERMINEFILE
.
An application including National Language Support (NLS) needs to 
locate resource-, help, and information files for the supported language.

These are often (or at least mostly should be) installed within a directory
tree belonging to the application package (don't litter system directories,
if not absolutely necessary, we are no windoze programmers, are we ?).
So it is about 
.ul compact
- determining the directory where the application exeutable resides,
  or determine a base installation path relative to this (e.g. when having
  the executable in the bin subdirectory of an application package)
- possibly append a subdirectory and surely append a filename to it,
  taking in account that
  - the filename has to include a language identifier (like the
    recommended three-letter language identifier in the ISO 639 format).
  - the file possibly must be searched for several languages, namely
    the system language, and, if not available, a default or fallback
    language

Moreover, it is good practise to exepect language specific files not only
in specific subdirectories of the application location, but also in the
directory of the executable itself. Therefore it may well be useful to search
more than directory.

All this can easily be done with the API [.WtkGetNlsPackageFilename WtkGetNlsPackageFilename],
which allows you to define:
.ul compact
- a [.IDPNL_APINLS_GETLANG_DEFAULT default or fallback language] (optional)
- the name of an environment variable, which can be used to override
  the [.IDPNL_APINLS_GETLANG_SYS system language] from outside of the 
  application (optional)
- a path for filename entries, allowing to search the same file
  in several directories and for the system and/or default/fallback
  language 
.
  Each entry of the path consists of a path relatively to the
  path of the executing module, and a filenanme including a %s
  as a placeholder for a language identifier,
  like e.g. "myapp%s.dll;inf\myapp%s.dll".

[=NOTE]
.ul compact
-  Two more APIs are available to automatically detect and load 
   language specific [.IDPNL_APINLS_LOADRES resource modules] or
   [.IDPNL_APINLS_LOADINF INF files]. For all other file types
   you have to use *WtkGetNlsPackageFilename* instead.

.. ----------------------------------------
.2 loading language specific resource files
.. ----------------------------------------
.an IDPNL_APINLS_LOADRES
.
In order to directly load language specific resource modules,
use the API [.WtkLoadNlsResourceModule WtkLoadNlsResourceModule]
or [.WtkLoadModNlsResourceModule WtkLoadModNlsResourceModule], 
depending on whether the module to load is to be determined by the
path of the calling executable or the path of a calling DLL.

This will in turn call [.WtkGetNlsPackageFilename WtkGetNlsPackageFilename]
in order to find the resource file suitable either to the
[.IDPNL_APINLS_GETLANG_LOCALE user selected language], the
[.IDPNL_APINLS_GETLANG_SYS language of your system] or 
a [.IDPNL_APINLS_GETLANG_DEFAULT default or fallback language].

.. ----------------------------------------
.2 loading language specific INF files
.. ----------------------------------------
.an IDPNL_APINLS_LOADINF
.
In order to directly load a language specific INF file into
the INF file viewer (optionally with a given topic)
use the API [.WtkLoadNlsInfFile WtkLoadNlsInfFile] or
[.WtkLoadModNlsInfFile WtkLoadModNlsInfFile],
depending on whether the INF file to load is to be determined by the
path of the calling executable or the path of a calling DLL.

This will in turn call [.WtkGetNlsPackageFilename WtkGetNlsPackageFilename]
in order to find the resource file suitable to the
[.IDPNL_APINLS_GETLANG_SYS language of your system] or 
a [.IDPNL_APINLS_GETLANG_DEFAULT default or fallback language].

