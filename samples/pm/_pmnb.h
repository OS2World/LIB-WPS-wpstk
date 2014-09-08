/****************************** Module Header ******************************\
*
* Module Name: _pmnb.h
*
* PM helper functions sample - notebook control related code header
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2004
*
* $Id: _pmnb.h,v 1.2 2008-12-22 18:02:08 cla Exp $
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

#ifndef _PMNB_H
#define _PMNB_H

#define TYPE_DEFNOTEBOOK         0
#define TYPE_OKNOTEBOOK          1

/* prototypes */
ULONG LaunchNotebook( HWND hwnd, HWND hwndBubbleHelp, PSZ pszHelpFile, ULONG ulNotebookType);

#endif // _PMNB_H

