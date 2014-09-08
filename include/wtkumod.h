/****************************** Module Header ******************************\
*
* Module Name: wtkumod.h
*
* Include file for module helper functions (EXE and DLL)
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: wtkumod.h,v 1.6 2005-10-23 18:28:34 cla Exp $
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

#ifndef WTKUTLMODULE_INCLUDED
#define WTKUTLMODULE_INCLUDED Module helper functions (EXE and DLL)

#ifdef __cplusplus
      extern "C" {
#endif

/*** prototypes for module information functions ***************************/
APIRET APIENTRY WtkGetModuleInfo( PFN pfn, PHMODULE phmod, PSZ pszBuffer, ULONG ulBuflen);
APIRET APIENTRY WtkGetModulePath( PFN pfn, PSZ pszBuffer, ULONG ulBuflen);
HMODULE APIENTRY WtkGetModuleHandle( PFN pfn);

/*** prototypes for determining files / filenames **************************/
APIRET APIENTRY WtkGetPackageFilename( PFN pfnMod, PSZ pszFileMaskPath,
                                       PSZ pszBuffer, ULONG ulBuflen);
APIRET APIENTRY WtkAssemblePackageFilename( PFN pfn, PSZ pszSubdir, PSZ pszFilename,
                                            PSZ pszFileext, PSZ pszBuffer, ULONG ulBuflen);


/*** prototypes for loading modules from a pah *****************************/
APIRET APIENTRY WtkLoadModules( PFN pfnMod, PHMODULE pahmod,
                                PULONG pulCount, PSZ pszFileMaskPath);
APIRET APIENTRY WtkFreeModules( PHMODULE pahmod, ULONG ulCount);

#ifdef __cplusplus
        }
#endif

#endif /* WTKUTLMODULE_INCLUDED */

