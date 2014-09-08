/****************************** Module Header ******************************\
*
* Module Name: wtkumd5.h
*
* include file for MD5 related helper functions
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: wtkumd5.h,v 1.1 2008-02-03 21:52:18 cla Exp $
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

#ifndef WTKUTLMD5_INCLUDED
#define WTKUTLMD5_INCLUDED MD5 related helper functions

#ifdef __cplusplus
      extern "C" {
#endif

typedef struct _MD5DIGEST {
  UCHAR          achDigest[16];
} MD5DIGEST, *PMD5DIGEST;

/*** prototype for calculating MD5 of data in memory ********************/
APIRET APIENTRY WtkCalcMemMD5( PVOID pvData, ULONG ulDatalen, PMD5DIGEST pmd5d);

/*** prototype for calculating MD5 of files *****************************/
APIRET APIENTRY WtkCalcFileMD5( PSZ pszFilename, PMD5DIGEST pmd5d);

/*** prototype for converting digest to a string ************************/
APIRET APIENTRY WtkMD5DigestToStr( PMD5DIGEST pmd5d, PSZ pszBuffer, ULONG ulBuflen);

#ifdef __cplusplus
        }
#endif

#endif /* WTKUTLMD5_INCLUDED */

