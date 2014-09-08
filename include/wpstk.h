/****************************** Module Header ******************************\
*
* Module Name: wpstk.h
*
* Include file for the version function of the WPS Toolkit
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: wpstk.h,v 1.12 2008-10-15 16:43:17 cla Exp $
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

#ifndef WTK_INCLUDED
#error including wpstk.h, include wtk instead!
#endif

#ifndef WTKVERSION_INCLUDED
#define WTKVERSION_INCLUDED WPS Toolkit version function

#ifdef __cplusplus
      extern "C" {
#endif

#define WPSTK_VERSION  "1.7.0"

APIRET APIENTRY WtkQueryVersion( PSZ pszBuffer, ULONG ulBuflen);

#ifdef __cplusplus
        }
#endif

#endif /* WTKVERSION_INCLUDED */

