/****************************** Module Header ******************************\
*
* Module Name: _nlsdll.c
*
* NLS functions related DLL sample
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: _nlsdll.c,v 1.6 2007-02-27 23:45:29 cla Exp $
*
* ===========================================================================*
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
#include <stdlib.h>
#include <string.h>

#define INCL_ERRORS
#define INCL_DOS
#define INCL_WIN
#include <os2.h>

#define INCL_WTKUTLFILE
#define INLC_WTKUTLLOCALE
#define INCL_WTKUTLMODULE
#define INCL_WTKUTLNLSMODULE
#define INCL_WTKUTLSYSTEM
#include <wtk.h>

#include "_nls.rch"

// NOTE: the source include file must be named *.c for checktest.cmd
#include "_nlstestinc.c"

// -----------------------------------------------------------------------------

APIRET APIENTRY NlsDllTest( VOID)
{

         APIRET         rc = NO_ERROR;

// execute NLS testaces from within the DLL
rc = NlsExecuteTestcases( CALLED_BY_DLL);

// make sure to flush output buffer from the DLL runtime
// otherwise the output may get lost
fflush( stdout);

// cleanup
return rc;
}

