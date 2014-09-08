/****************************** Module Header ******************************\
*
* Module Name: wtkubdat.h
*
* include file for XBIN2OBJ access to binary data
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2003
*
* $Id: wtkubdat.h,v 1.3 2005-06-12 18:13:07 cla Exp $
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

#ifndef WTKUTLXBIN2OBJ_INCLUDED
#define WTKUTLXBIN2OBJ_INCLUDED XBIN2OBJ access to binary data

#ifdef __cplusplus
      extern "C" {
#endif

#pragma pack(1)

// data structures for 16- and 32-bit memory model


typedef struct _BINDATA16 {
  USHORT         cbSize;
  BYTE           bData[1];
} BINDATA16, *PBINDATA16;

typedef struct _BINDATA32 {
  ULONG          cbSize;
  BYTE           bData[1];
} BINDATA32, *PBINDATA32;


#ifdef __32BIT__
typedef BINDATA32  BINDATA;
typedef PBINDATA32 PBINDATA;
#else
typedef BINDATA16  BINDATA;
typedef PBINDATA16 PBINDATA;
#endif

#pragma pack()

#ifdef __cplusplus
        }
#endif

#endif /* WTKUTLXBIN2OBJ_INCLUDED */

