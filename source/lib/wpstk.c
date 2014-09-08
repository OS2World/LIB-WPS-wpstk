/****************************** Module Header ******************************\
*
* Module Name: wpstk.c
*
* Source for the version function of the WPS Toolkit
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: wpstk.c,v 1.12 2009-11-18 22:18:14 cla Exp $
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
#define INCL_DOS
#include <os2.h>

#include "wtk.h"
#include "wpstk.ih"

// ###########################################################################

/*
@@WtkQueryVersion@SYNTAX
This function returns the version information for the :hp2.Workplace Shell Toolkit:ehp2..

@@WtkQueryVersion@PARM@pszBuffer@out
The address of a buffer in into which the
version string is returned.

@@WtkQueryVersion@PARM@ulBuflen@in
The length, in bytes, of the buffer described by :hp1.pszBuffer:ehp1..

@@WtkQueryVersion@RETURN
Return Code.
:p.
WtkQueryVersion returns one of the following return codes&colon.

:parml compact break=none.
:pt.0
:pd.NO_ERROR
:pt.87
:pd.ERROR_INVALID_PARAMETER
:pt.111
:pd.ERROR_BUFFER_OVERFLOW
:eparml.

@@WtkQueryVersion@REMARKS
This function enables to ensure, that the WPS class is
using the same version of the WPS toolkit API, that it was compiled
with. This is normally not required for WPS class DLLs or applications,
where the WPS toolkit functions are statically linked to, but may be
important, where they are linked as a DLL.
:p.
In order to ensure, that your WPS DLL is using the same WPS Toolkit
API, as it was compiled with, compare the resulting string with
the value WPSTK_VERSION.

@@
*/

APIRET APIENTRY WtkQueryVersion( PSZ pszBuffer, ULONG ulBuflen)
{
         APIRET            rc = NO_ERROR;
static   PSZ               pszVersion = WPSTK_VERSION;

do
   {
   // check parms
   if (!pszBuffer)
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // does buffer fit ?
   if (strlen( pszVersion) + 1 > ulBuflen)
      {
      rc = ERROR_BUFFER_OVERFLOW;
      break;
      }

   // hand over result
   strcpy( pszBuffer, pszVersion);


   // is it an internal query to detect the debug version ?
   // return error for release version
#ifndef DEBUG
   if (!strcmp( pszBuffer, "QUERYDEBUG"))
      rc = ERROR_NOT_SUPPORTED;
#endif

   } while (FALSE);


return rc;

}
// ---------------------------------------------------------------------------

// helper

VOID __PatchBootDrive( PSZ pszName)
{
         ULONG          ulBootDrive;

// copy name and replace ?: with bootdrive
if (!(strncmp( pszName, "?:", 2)))
   {
   DosQuerySysInfo( QSV_BOOT_DRIVE, QSV_BOOT_DRIVE, &ulBootDrive, sizeof(ULONG));
   *pszName = (CHAR) ulBootDrive + 'A' - 1;
   }
return;
}

// ----------------------------------------------------------------------

PSZ __strip( PSZ string, CHAR ch)
{
 PSZ p = string;
 if (p)
    {
    while ((*p) && (*p == ch))
       { p++;}
    strcpy( string, p);
    }
 if ((p) && (*p))
    {
    p += strlen(p) - 1;
    while ((*p == ch) && (p >= string))
       {
       *p = 0;
       p--;
       }
    }

return string;
}

// ----------------------------------------------------------------------

PSZ __stripblanks( PSZ string)
{
return __strip( string, 32);
}

// ----------------------------------------------------------------------

PSZ __skipblanks( PSZ string)
{
 PSZ p = string;
 if ((p) && (*p))
    {
    while ((*p != 0) && (*p <= 32))
       { p++;}
    }

return p;
}

// ----------------------------------------------------------------------

PSZ __getPMClassName( PSZ pszClassIndex)
{
         PSZ            pszClassName;
static   CHAR           szClassName[ 32];

switch ((ULONG) pszClassName)
   {
   case (ULONG) WC_FRAME:            pszClassName = "WC_FRAME";           break;
   case (ULONG) WC_COMBOBOX:         pszClassName = "WC_COMBOBOX";        break;
   case (ULONG) WC_BUTTON:           pszClassName = "WC_BUTTON";          break;
   case (ULONG) WC_MENU:             pszClassName = "WC_MENU";            break;
   case (ULONG) WC_STATIC:           pszClassName = "WC_STATIC";          break;
   case (ULONG) WC_ENTRYFIELD:       pszClassName = "WC_ENTRYFIELD";      break;
   case (ULONG) WC_LISTBOX:          pszClassName = "WC_LISTBOX";         break;
   case (ULONG) WC_SCROLLBAR:        pszClassName = "WC_SCROLLBAR";       break;
   case (ULONG) WC_TITLEBAR:         pszClassName = "WC_TITLEBAR";        break;
   case (ULONG) WC_MLE:              pszClassName = "WC_MLE";             break;
   case (ULONG) WC_APPSTAT:          pszClassName = "WC_APPSTAT";         break;
   case (ULONG) WC_KBDSTAT:          pszClassName = "WC_KBDSTAT";         break;
   case (ULONG) WC_PECIC:            pszClassName = "WC_PECIC";           break;
   case (ULONG) WC_DBE_KKPOPUP:      pszClassName = "WC_DBE_KKPOPUP";     break;
   case (ULONG) WC_SPINBUTTON:       pszClassName = "WC_SPINBUTTON";      break;
   case (ULONG) WC_CONTAINER:        pszClassName = "WC_CONTAINER";       break;
   case (ULONG) WC_SLIDER:           pszClassName = "WC_SLIDER";          break;
   case (ULONG) WC_VALUESET:         pszClassName = "WC_VALUESET";        break;
   case (ULONG) WC_NOTEBOOK:         pszClassName = "WC_NOTEBOOK";        break;
   case (ULONG) WC_PENFIRST:         pszClassName = "WC_PENFIRST";        break;
   case (ULONG) WC_PENLAST:          pszClassName = "WC_PENLAST";         break;
   case (ULONG) WC_MMPMFIRST:        pszClassName = "WC_MMPMFIRST";       break;
   case (ULONG) WC_CIRCULARSLIDER:   pszClassName = "WC_CIRCULARSLIDER";  break;
   case (ULONG) WC_MMPMLAST:         pszClassName = "WC_MMPMLAST";        break;
   case (ULONG) WC_PRISTDDLGFIRST:   pszClassName = "WC_PRISTDDLGFIRST";  break;
   case (ULONG) WC_PRISTDDLGLAST:    pszClassName = "WC_PRISTDDLGLAST";   break;
   case (ULONG) WC_PUBSTDDLGFIRST:   pszClassName = "WC_PUBSTDDLGFIRST";  break;
   case (ULONG) WC_PUBSTDDLGLAST:    pszClassName = "WC_PUBSTDDLGLAST";   break;

   default:
      sprintf( szClassName, "#%u", SHORT1FROMMP( pszClassIndex));
      pszClassName = szClassName;
      break;

   } // switch

return pszClassName;

}

// --------------------------------------------------------------------------

PFN __loadDllFunc( PSZ pszLibName, PSZ pszFuncName)
{
         PFN            pfnResult = NULL;
         APIRET         rc = NO_ERROR;
         CHAR           szError[ 32];
         HMODULE        hmod = NULLHANDLE;

do
   {
   // determine module handle
   rc = DosQueryModuleHandle( pszLibName, &hmod);
   if (rc != NO_ERROR)
      {
      // load module into memory
      // by design of the WPSTK library, the DLL cannot be freed afterwards
      // but we use it for few system DLLs only !
      rc = DosLoadModule( szError, sizeof( szError), pszLibName, &hmod);
      if (rc != NO_ERROR)
         break;
      }

   // determine procedure address
   rc = DosQueryProcAddr( hmod, 0, pszFuncName, &pfnResult);
   if (rc != NO_ERROR)
      {
      // the DLL is in memory, but not loaded for this process already - load it
      // by design of the WPSTK library, the DLL cannot be freed afterwards
      // but we use it for few system DLLs only !
      rc = DosLoadModule( szError, sizeof( szError), pszLibName, &hmod);
      if (rc != NO_ERROR)
         break;

      // once again, try to get func ptr
      rc = DosQueryProcAddr( hmod, 0, pszFuncName, &pfnResult);
      }

   if (rc != NO_ERROR)
      break;

   } while (FALSE);

return pfnResult;
}

