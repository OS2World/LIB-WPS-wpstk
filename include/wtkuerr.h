/****************************** Module Header ******************************\
*
* Module Name: wtkerr.h
*
* include file for error helper functions
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: wtkuerr.h,v 1.2 2002-11-22 22:21:28 cla Exp $
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

#ifndef WTKUTLERROR_INCLUDED
#define WTKUTLERROR_INCLUDED Error helper functions

#ifdef __cplusplus
      extern "C" {
#endif

/*** prototypes for PM error functions *************************************/
BOOL APIENTRY WtkSetErrorInfo( APIRET rc);
#define PMHERR_USE_EXISTING_ERRORINFO -1

#ifdef __cplusplus
        }
#endif

#endif /* WTKUTLERROR_INCLUDED */

