/****************************** Module Header ******************************\
*
* Module Name: wtkfbldl.h
*
* include file for access functions for buidlevel information
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2005
*
* $Id: wtkfbldl.h,v 1.1 2005-11-06 13:46:07 cla Exp $
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

#ifndef WTKFBLDL_INCLUDED
#define WTKFBLDL_INCLUDED File access functions for buildlevel information

#ifdef __cplusplus
      extern "C" {
#endif

typedef struct _BLDLEVEL {
  PSZ            pszVendor;
  PSZ            pszRevision;
  PSZ            pszDescription;
  CHAR           bData[ 1];
} BLDLEVEL, *PBLDLEVEL;

APIRET APIENTRY WtkGetBlSignature( PSZ pszExeFile, PBLDLEVEL pbl, ULONG ulBuflen);
APIRET APIENTRY WtkCheckBlSignature( PSZ pszExeFile, PSZ pszVendor,
                                     PSZ pszRevision, ULONG ulRevCheck,
                                     PSZ pszDescription);

/* flags for WtkCheckBlSignature - ulRevCheck */
#define WTK_BLREVCHECK_EQUAL             1
#define WTK_BLREVCHECK_GREATER           2
#define WTK_BLREVCHECK_LESSER            4
#define WTK_BLREVCHECK_GREATER_OR_EQUAL  (WTK_BLREVCHECK_GREATER | WTK_BLREVCHECK_EQUAL)
#define WTK_BLREVCHECK_LESSER_OR_EQUAL   (WTK_BLREVCHECK_LESSER  | WTK_BLREVCHECK_EQUAL)
#define WTK_BLREVCHECK_NOT_EQUAL         (WTK_BLREVCHECK_GREATER | WTK_BLREVCHECK_LESSER)

APIRET APIENTRY WtkSetBlSignature( PSZ pszExeFile, PBLDLEVEL pbl, ULONG ulOptions);
#define WTK_BLSET_VENDOR                 1
#define WTK_BLSET_REVISION               2
#define WTK_BLSET_DESCRIPTION            4
#define WTK_BLSET_ALL                   -1


#ifdef __cplusplus
        }
#endif

#endif /* WTKFBLDL_INCLUDED */

