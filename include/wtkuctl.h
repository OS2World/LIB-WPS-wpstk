/****************************** Module Header ******************************\
*
* Module Name: wtkuctl.h
*
* include file for PM control helper functions
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: wtkuctl.h,v 1.5 2005-02-15 19:50:06 cla Exp $
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

#ifndef WTKUTLCONTROL_INCLUDED
#define WTKUTLCONTROL_INCLUDED PM control helper functions

#ifdef __cplusplus
      extern "C" {
#endif

/*** prototypes for general functions **************************************/
PSZ APIENTRY WtkQueryClassIndex( HWND hwnd);
BOOL APIENTRY WtkIsOfPublicPmClass( HWND hwnd, PSZ pszClassIndex, ULONG ulPrimaryWindowStyle);
PFNWP APIENTRY WtkGetDefaultWindowProcPtr( HWND hwnd);

/*** prototypes for specialized functions for certain window classes *******/
BOOL APIENTRY WtkInitializeNumSpinbuttonArray( HWND hwndSpinbutton, ULONG ulMinValue,
                                               ULONG ulMaxValue, ULONG ulStep);
LONG APIENTRY WtkQueryNumSpinbuttonIndex( HWND hwndSpinbutton, ULONG ulMinValue,
                                 ULONG ulMaxValue, ULONG ulStep, ULONG ulValue);

/*** prototypes for filling MLEs from diverse resources ********************/
BOOL WtkAddTextResourceToMLE( HWND hwnd, ULONG ulControlId,
                              HMODULE hmod,  ULONG ulResourceType, ULONG ulResId);

/*** subclass helper functions *******************************************/
BOOL APIENTRY WtkFilterEntryField( HWND hwndEntryField, PSZ pszValidChars,
                                   PSZ pszInvalidChars);

#ifdef __cplusplus
        }
#endif

#endif /* WTKUTLCONTROL_INCLUDED */

