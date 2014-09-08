/****************************** Module Header ******************************\
*
* Module Name: settcb.h
*
* include file for callback function of test class of
* settings and details manager WPS sample
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: settcb.h,v 1.2 2002-07-12 14:32:46 cla Exp $
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

#ifndef SETTCB_H
#define SETTCB_H

BOOL SettingsCallbackProc( ULONG ulAction, PVOID pvData, PVOID somSelf, PVOID somThis);

#endif // SETTCB_H

