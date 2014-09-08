/****************************** Module Header ******************************\
*
* Module Name: wtkuioc.h
*
* include file for device IO control helper functions
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: wtkuioc.h,v 1.3 2007-02-07 22:06:47 cla Exp $
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

#ifndef WTKUTLIOCTRL_INCLUDED
#define WTKUTLIOCTRL_INCLUDED Device IO control helper functions

#ifdef __cplusplus
      extern "C" {
#endif

/*** prototypes for opening devices ****************************************/
APIRET APIENTRY WtkOpenDevice( PSZ pszName, PHFILE phdevice, ULONG ulOpenMode);
#define WTK_OPENDEVICE_SHARED      0x0000
#define WTK_OPENDEVICE_EXCLUSIVE   0x0001
#define WTK_OPENDEVICE_NOCACHE     0x0002
#define WTK_OPENDEVICE_BLOCKDEVICE 0x0004 /* implicitely set, when pszName specifies drive */

/*** prototypes for performing an I/o Control transaction ******************/
APIRET APIENTRY WtkDevIOCtl( PSZ pszName, ULONG ulOpenMode,
                             ULONG ulCategory, ULONG ulFunction,
                             PVOID pvParams, PULONG pcbParmLen,
                             PVOID pvData, PULONG pcbDataLen);

#ifdef __cplusplus
        }
#endif

#endif /* WTKUTLIOCTRL_INCLUDED */

