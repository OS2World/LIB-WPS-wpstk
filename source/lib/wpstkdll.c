/****************************** Module Header ******************************\
*
* Module Name: wpstkdll.c
*
* Dummy module for attaching the DLL runtime to the DLL
* This module is only used by the makefile for the IBM compilers
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: wpstkdll.c,v 1.1 2002-11-26 15:51:31 cla Exp $
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

#include <stdio.h>
#include <string.h>

#define INCL_ERRORS
#include <os2.h>


static BOOL __IsDll( VOID)
{
return TRUE;
}

