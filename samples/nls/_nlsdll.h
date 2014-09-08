/****************************** Module Header ******************************\
*
* Module Name: _nlsdll.h
*
* NLS functions related DLL sample header
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: _nlsdll.h,v 1.2 2006-08-18 23:31:24 cla Exp $
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

#ifndef _NLSDLL_INCLUDED
#define _NLSDLL_INCLUDED

#ifdef __cplusplus
      extern "C" {
#endif

// definitions for dynamic linkage
typedef APIRET (FNDLLFUNC)(VOID);
typedef FNDLLFUNC *PFNDLLFUNC;
#define __DLLNAME__   "_NLSDLL.DLL"
#define __FUNCNAME__  "NlsDllTest"

APIRET APIENTRY NlsDllTest( VOID);

#ifdef __cplusplus
        }
#endif

#endif /* _NLSDLL_INCLUDED */

