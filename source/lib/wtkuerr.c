/****************************** Module Header ******************************\
*
* Module Name: wtkuerr.c
*
* Source for error utility functions.
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: wtkuerr.c,v 1.3 2003-04-24 14:33:29 cla Exp $
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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INCL_DOS
#define INCL_WIN
#define INCL_WINSEI
#define INCL_ERRORS
#include <os2.h>

#include "wtkuerr.h"
#include "wpstk.ih"

// ###########################################################################

/*
@@WtkSetErrorInfo@SYNTAX
This function sets the error info, that can be obtained later
by WinGetLastError and WinGetErrorInfo.

@@WtkSetErrorInfo@PARM@rc@in
Return code to be used when creating the error information.
:p.
When rc is set to NO_ERROR (0) or PMHERR_USE_EXISTING_ERRORINFO (-1),
WtkSetErrorInfo takes no action.

@@WtkSetErrorInfo@RETURN
Error indicator.
:parml compact.
:pt.TRUE
:pd.rc was reporting NO_ERROR.
:pt.FALSE
:pd.rc was reporting an error.
:eparml.

@@WtkSetErrorInfo@REMARKS
WtkSetErrorInfo distincts between OS/2 control program errors
and errors of the presentation manager API, and sets the error info
according to that distinction.
:p.
When rc is set to NO_ERROR (0) or PMHERR_USE_EXISTING_ERRORINFO (-1),
WtkSetErrorInfo takes no action. This enables the caller to invoke this
function always without taking care, if an error occurred or not.
More, PMHERR_USE_EXISTING_ERRORINFO can preserve the error info of a
previous failed Win* API call, but still lets WtkSetErrorInfo return FALSE.
:p.
When the callers function returns BOOL, the following line would
automatically set the error info and return the appropriate value&colon.
:xmp.
return WtkSetErrorInfo( rc);
:exmp.

@@
*/

BOOL APIENTRY WtkSetErrorInfo( APIRET rc)
{

do
   {
   // do nothing if no error occurred
   if ((rc == NO_ERROR) || (rc == PMHERR_USE_EXISTING_ERRORINFO))
      break;

   // set the info, which can be retrieved by WinGetLastError/WinGetErrorInfo
   if (rc > PMERR_INVALID_HWND)
      WinSetErrorInfo( MAKEERRORID( SEVERITY_ERROR, rc), 0);
   else
      WinSetErrorInfo( MAKEERRORID( SEVERITY_ERROR, PMERR_DOS_ERROR), SEI_DOSERROR, (USHORT) rc);

   } while (FALSE);

return (rc == NO_ERROR);

}

