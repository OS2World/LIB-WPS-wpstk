/****************************** Module Header ******************************\
*
* Module Name: wtkurgx.h
*
* include file for regular expression support functions
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2003
*
* $Id: wtkurgx.h,v 1.3 2003-09-10 13:35:36 cla Exp $
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

#ifndef WTKUTLREGEXP_INCLUDED
#define WTKUTLREGEXP_INCLUDED Regular expression support functions

#ifdef __cplusplus
      extern "C" {
#endif

BOOL APIENTRY WtkIsRegularExpressionValid( PSZ pszExpression);
APIRET APIENTRY WtkMatchRegularExpression( PSZ pszExpression, PSZ pszText,
                                           PSZ pszBuffer, ULONG ulBuflen);
APIRET APIENTRY WtkSubstRegularExpressionMatch( PSZ pszExpression, PSZ pszText,
                                                PSZ pszReplacePattern,
                                                PSZ pszBuffer, ULONG ulBuflen);

#ifdef __cplusplus
        }
#endif

#endif /* WTKUTLREGEXP_INCLUDED */

