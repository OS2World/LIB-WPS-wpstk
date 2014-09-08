/****************************** Module Header ******************************\
*
* Module Name: wtkfslvl.h
*
* include file for access functions for syslevel files
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: wtkfslvl.h,v 1.8 2008-02-08 20:25:09 cla Exp $
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

#ifndef WTKFILESYSLEVEL_INCLUDED
#define WTKFILESYSLEVEL_INCLUDED File access functions for Syslevel files

#ifdef __cplusplus
      extern "C" {
#endif

#pragma pack(1)

/* file handle type and structures */

typedef LHANDLE HSYSLEVEL;
typedef HSYSLEVEL *PHSYSLEVEL;

#define WTK_SYSLEVEL_MAXLEN_COMPID  9
#define WTK_SYSLEVEL_MAXLEN_NAME   79
#define WTK_SYSLEVEL_MAXLEN_PREFIX  2
#define WTK_SYSLEVEL_MAXLEN_CSDNUM  4
#define WTK_SYSLEVEL_MAXLEN_APPTYPE 8

/* external data structure                 */
/* NOTE:                                   */
/*  - read/write version fields in hex     */
/*  - use the WTK_SYSLEVEL_MAXLEN_* with   */
/*    strncpy() to properly copy strings   */
/*    to string fields (see sample !)      */

typedef struct _SYSLEVELINFO {
  ULONG          cbSize;
  USHORT         usSysId;
  CHAR           szComponentId[ WTK_SYSLEVEL_MAXLEN_COMPID + 1];
  CHAR           szName[ WTK_SYSLEVEL_MAXLEN_NAME + 1];
  BYTE           bVersionMajor;
  BYTE           bVersionMinor;
  BYTE           bVersionRefresh;
  CHAR           szCsdPrefix[ WTK_SYSLEVEL_MAXLEN_PREFIX + 1];
  CHAR           chCsdLanguage;
  CHAR           szCurrentCsd[ WTK_SYSLEVEL_MAXLEN_CSDNUM + 1];
  CHAR           szPreviousCsd[ WTK_SYSLEVEL_MAXLEN_CSDNUM + 1];
  USHORT         usDate;
  CHAR           szAppType[ WTK_SYSLEVEL_MAXLEN_APPTYPE + 1];
   } SYSLEVELINFO, *PSYSLEVELINFO;

// define for backwards copatibility
// in former versions the scructure was named SYLEVELINFO
typedef SYSLEVELINFO SYLEVELINFO;
typedef SYSLEVELINFO *PSYLEVELINFO;

APIRET APIENTRY WtkOpenSyslevel( PSZ pszName, PHSYSLEVEL phsl);
APIRET APIENTRY WtkCloseSyslevel( HSYSLEVEL hsl, ULONG ulUpdateMode);
#define WTK_SYSLEVEL_CLOSE_DISCARD    0
#define WTK_SYSLEVEL_CLOSE_UPDATE     1

APIRET APIENTRY WtkQuerySyslevelInfo( HSYSLEVEL hsl, PSYSLEVELINFO psli);
APIRET APIENTRY WtkSetSyslevelInfo( HSYSLEVEL hsl, ULONG ulUpdateFlags, PSYSLEVELINFO psli);
#define WTK_SYSLEVEL_UPDATE_ALL                       -1
#define WTK_SYSLEVEL_UPDATE_SYSID             0x00000001
#define WTK_SYSLEVEL_UPDATE_COMPONENTID       0x00000002
#define WTK_SYSLEVEL_UPDATE_NAME              0x00000004
#define WTK_SYSLEVEL_UPDATE_VERSION           0x00000008
#define WTK_SYSLEVEL_UPDATE_CSDPREFIX         0x00000010
#define WTK_SYSLEVEL_UPDATE_CSDLANGUAGE       0x00000020
#define WTK_SYSLEVEL_UPDATE_CURRENTCSD        0x00000040
#define WTK_SYSLEVEL_UPDATE_PREVIOUSCSD       0x00000080
#define WTK_SYSLEVEL_UPDATE_DATE              0x00000010
#define WTK_SYSLEVEL_UPDATE_APPTYPE           0x00000020

#pragma pack()

#ifdef __cplusplus
        }
#endif

#endif /* WTKFILESYSLEVEL_INCLUDED */

