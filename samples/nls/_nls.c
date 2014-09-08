/****************************** Module Header ******************************\
*
* Module Name: _nls.c
*
* NLS functions related sample
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: _nls.c,v 1.8 2007-02-27 23:45:29 cla Exp $
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

#include "_nlsdll.h"
#include "_nls.rch"

// NOTE: the source include file must be named *.c for checktest.cmd
#include "_nlstestinc.c"

// -----------------------------------------------------------------------------


int main ( int argc, char *argv[])
{

         APIRET         rc = NO_ERROR;

         HMODULE        hmodDll = NULLHANDLE;
         CHAR           szError[ _MAX_PATH];
         CHAR           szDll[ _MAX_PATH];
         PFNDLLFUNC     pfnDllFunc;

do
   {
   // execute NLS testaces directly
   rc = NlsExecuteTestcases( CALLED_BY_EXE);

   // --------------------------------------------------------------------------------

   // determine DLL path
   rc = WtkGetModulePath( (PFN)__FUNCTION__, szDll, sizeof( szDll));
   if (rc != NO_ERROR)
      {
      printf( " WtkGetModulePath: error: cannot determine executable path. rc=%u\n", rc);
      break;
      }

   strcat( szDll, "\\"__DLLNAME__);
   printf( "Loading  Dynamic link library: %s\n", szDll);

   // load DLL
   memset( szError, 0, sizeof( szError));
   rc = DosLoadModule( szError, sizeof( szError), szDll, &hmodDll);
   if (rc != NO_ERROR)
      {
      printf( " DosLoadModule: error: cannot load dll " __DLLNAME__  " module %s, rc=%u\n", szError, rc);
      break;
      }
   rc = DosQueryProcAddr( hmodDll, 0, __FUNCNAME__, (PFN*) &pfnDllFunc);
   if (rc != NO_ERROR)
      {
      printf( " DosQueryProcAddr: error: cannot determine address of "  __FUNCNAME__ ", rc=%u\n", rc);
      break;
      }

   // make sure to flush output buffer from the EXE runtime
   // otherwise the output of the DLL bay be dislayed before
   fflush( stdout);

   // execute NLS testaces from within the DLL
   (pfnDllFunc)();

   } while (FALSE);

// cleanup
if (hmodDll) DosFreeModule( hmodDll);
return rc;
}

