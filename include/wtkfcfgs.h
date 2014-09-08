/****************************** Module Header ******************************\
*
* Module Name: wtkfcfgs.h
*
* include file for access functions for CONFIG.SYS
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: wtkfcfgs.h,v 1.4 2005-06-12 18:13:06 cla Exp $
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

#ifndef WTKFILECFGSYS_INCLUDED
#define WTKFILECFGSYS_INCLUDED File access functions for CONFIG.SYS

#ifdef __cplusplus
      extern "C" {
#endif

#define WTK_CFGSYS_UPDATE_ADD          0
#define WTK_CFGSYS_UPDATE_DELETE       1

APIRET APIENTRY WtkUpdateConfigsys( PSZ pszUpdate, ULONG ulUpdateMode, PSZ pszBackupExtension);

#ifdef __cplusplus
        }
#endif

#endif /* WTKFILECFGSYS_INCLUDED */

