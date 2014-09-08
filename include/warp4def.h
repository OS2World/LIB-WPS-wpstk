/****************************** Module Header ******************************\
*
* Module Name: warp4def.h
*
* Include file for compiling this library with the WARP 3 toolkit
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: warp4def.h,v 1.4 2006-12-04 21:25:44 cla Exp $
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

#ifndef WTKWARP4DEF_INCLUDED
#define WTKWARP4DEF_INCLUDED

#ifdef __cplusplus
      extern "C" {
#endif

/* ------------------------------------------------------------------ */

/* some definitions of Warp 4 Toolkit not included in Open Watcom headers */
#ifdef __WATCOMC__
#ifndef MAKEP
#define MAKEP(sel,off) ((void *)(void * _Seg16)((sel) << 16 | (off)))
#endif

#ifndef WinShowControl
#define WinShowControl(hwndDlg, id, fShow) \
WinShowWindow(WinWindowFromID(hwndDlg, id), fShow)
#endif

#ifndef OBJSTYLE_LOCKEDINPLACE
#define OBJSTYLE_LOCKEDINPLACE  0x00020000
#endif

#endif

/* ------------------------------------------------------------------ */

#ifdef __cplusplus
        }
#endif

#endif /* WTKWARP4DEF_INCLUDED */

