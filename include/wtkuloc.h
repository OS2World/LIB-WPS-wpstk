/****************************** Module Header ******************************\
*
* Module Name: wtkuloc.h
*
* include file for locale helper functions
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2007
*
* $Id: wtkuloc.h,v 1.6 2009-11-18 22:18:14 cla Exp $
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

#ifndef WTKUTLLOCALE_INCLUDED
#define WTKUTLLOCALE_INCLUDED Locale helper functions

#ifdef __cplusplus
      extern "C" {
#endif

PSZ APIENTRY WtkQueryLocaleLanguage( ULONG ulIdType);
#ifndef WTK_LANGUAGEID_639_1
#define WTK_LANGUAGEID_639_1     0x0000
#define WTK_LANGUAGEID_639_1C    0x0001
#define WTK_LANGUAGEID_639_2     0x0002
#define WTK_LANGUAGENAME_ENGLISH 0x1000
#define WTK_LANGUAGENAME_NATIVE  0x2000
#endif

#ifdef __cplusplus
        }
#endif

#endif /* WTKUTLLOCALE_INCLUDED */

