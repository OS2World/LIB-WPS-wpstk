/****************************** Module Header ******************************\
*
* Module Name: wtkfbmp.h
*
* include file for access functions for bitmap files
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: wtkfbmp.h,v 1.2 2005-06-12 18:13:06 cla Exp $
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

#ifndef WTKFILEBMP_INCLUDED
#define WTKFILEBMP_INCLUDED File access functions for bitmap files

#ifdef __cplusplus
      extern "C" {
#endif

APIRET APIENTRY WtkLoadBitmapFromFile( HPS hps, PSZ pszFile, PHBITMAP phbmp,
                                       ULONG ulWidth, ULONG ulHeight, BOOL fForceSize);

#ifdef __cplusplus
        }
#endif

#endif /* WTKFILEBMP_INCLUDED */

