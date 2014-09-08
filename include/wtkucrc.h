/****************************** Module Header ******************************\
*
* Module Name: wtkucrc.h
*
* include file for CRC related helper functions
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: wtkucrc.h,v 1.3 2002-11-22 22:21:28 cla Exp $
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

#ifndef WTKUTLCRC_INCLUDED
#define WTKUTLCRC_INCLUDED CRC related helper functions

#ifdef __cplusplus
      extern "C" {
#endif

/*** prototypes for calculating CRC of data in memory *******************/
/*** initialize pulCRC32 with -1 on first call **************************/
APIRET APIENTRY WtkCalcMemCRC32( PVOID pvData, ULONG ulDatalen, PULONG pulCRC32);

/*** prototypes for calculating CRC of files ****************************/
APIRET APIENTRY WtkCalcFileCRC32( PSZ pszFilename, PULONG pulCRC32);

#ifdef __cplusplus
        }
#endif

#endif /* WTKUTLCRC_INCLUDED */

