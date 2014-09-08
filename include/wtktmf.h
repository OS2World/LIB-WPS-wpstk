/****************************** Module Header ******************************\
*
* Module Name: wtktmf.h
*
* include file for text message file functions.
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: wtktmf.h,v 1.1 2002-11-22 21:10:43 cla Exp $
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

#ifndef WTKTMF_INCLUDED
#define WTKTMF_INCLUDED Text Message File functions

#ifdef __cplusplus
      extern "C" {
#endif

APIRET APIENTRY WtkGetTextMessage( PCHAR* pTable, ULONG cTable,
                                   PBYTE pbBuffer, ULONG cbBuffer,
                                   PSZ pszMessageName, PSZ pszFile,
                                   PULONG pcbMsg);

#ifdef __cplusplus
        }
#endif

#endif /* WTKTMF_INCLUDED */

