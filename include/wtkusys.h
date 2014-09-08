/****************************** Module Header ******************************\
*
* Module Name: wtksys.h
*
* include file for system helper functions
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: wtkusys.h,v 1.9 2009-11-18 22:18:14 cla Exp $
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

#ifndef WTKUTLSYSTEM_INCLUDED
#define WTKUTLSYSTEM_INCLUDED System helper functions

#ifdef __cplusplus
      extern "C" {
#endif

/*** prototypes for system configuration functions *************************/
BOOL APIENTRY WtkIsWarp4( VOID);

// distinct between OS/2 Warp and eComStation
ULONG APIENTRY WtkQueryOperatingSystem( VOID);
#define WTK_OSTYPE_OS2 0x0000
#define WTK_OSTYPE_ECS 0x0001

BOOL APIENTRY WtkIsOS2( VOID);
BOOL APIENTRY WtkIseComStation( VOID);

CHAR APIENTRY WtkQueryBootDrive( VOID);

PSZ APIENTRY WtkQuerySystemLanguage( ULONG ulIdType);
#ifndef WTK_LANGUAGEID_639_1
#define WTK_LANGUAGEID_639_1     0x0000
#define WTK_LANGUAGEID_639_1C    0x0001
#define WTK_LANGUAGEID_639_2     0x0002
#define WTK_LANGUAGENAME_ENGLISH 0x1000
#define WTK_LANGUAGENAME_NATIVE  0x2000
#endif
// maintain source compatibility for WtkQuerySysLanguage
// (binary compatibility is maintained within wtkusys.c)
#define WtkQuerySysLanguage()  WtkQuerySystemLanguage( WTK_LANGUAGEID_639_2)

APIRET APIENTRY WtkReboot( VOID);


#ifdef __cplusplus
        }
#endif

#endif /* WTKUTLSYSTEM_INCLUDED */

