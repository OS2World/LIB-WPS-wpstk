/****************************** Module Header ******************************\
*
* Module Name: _pmfddlg.h
*
* PM helper functions sample - file/directory dialog related code header
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2004
*
* $Id: _pmfddlg.h,v 1.1 2008-12-22 18:12:52 cla Exp $
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

#ifndef _PMFDDLG_H
#define _PMFDDLG_H

/* prototypes */
ULONG LaunchDirDlg( HWND hwnd, HWND hwndBubbleHelp,
                    BOOL fPreload, BOOL fLoadCustomDialog);
ULONG LaunchFileDlg( HWND hwnd, HWND hwndBubbleHelp);

#endif // _PMFDDLG_H

