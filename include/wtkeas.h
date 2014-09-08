/****************************** Module Header ******************************\
*
* Module Name: wtkeas.h
*
* include file for extended attributes manager functions.
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: wtkeas.h,v 1.4 2002-11-22 22:21:27 cla Exp $
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

#ifndef WTKEAS_INCLUDED
#define WTKEAS_INCLUDED Extended Attributes manager functions

#ifdef __cplusplus
      extern "C" {
#endif

/*** handle types used by EA manager ***************************************/
typedef LHANDLE HEA;
typedef HEA *PHEA;

#define HEA_CREATE (HEA) -1

/*** EA type wildcard for WtkFindNextEaValue *******************************/
#define WTK_EAT_ANY     -1

/*** get size of all values of an EA ***************************************/
APIRET APIENTRY WtkQueryEaSize( PSZ pszName, PSZ pszEaName, PULONG pulSize);

/*** read complete ea into memory and scan it ******************************/
APIRET APIENTRY WtkReadEa( PSZ pszName, PSZ pszEaName, PHEA phea);
APIRET APIENTRY WtkFindFirstEaValue( HEA hea, PULONG pulEaType, PSZ pszValue, PULONG pulBuflen);
APIRET APIENTRY WtkFindNextEaValue( HEA hea, PULONG pulEaType, PSZ pszValue, PULONG pulBuflen);

/*** create new EA list in memory ******************************************/
APIRET APIENTRY WtkCreateEa( PSZ pszName, PSZ pszEaName, PHEA phea);

/*** append a value ********************************************************/
APIRET APIENTRY WtkAppendEaValue( HEA hea, ULONG ulMultiType, ULONG ulEaType, PBYTE pbValue,  ULONG ulValuelen);

/*** write EA to disk ******************************************************/
APIRET APIENTRY WtkSaveEa( HEA hea, BOOL fWriteThru, BYTE fEA);

/*** destroy EA in memory **************************************************/
APIRET APIENTRY WtkCloseEa( HEA hea);

/*** WtkGetNextFile alike API, does WtkReadEa/WtkFind*EaValue/WtkCloseEa in one API */
APIRET APIENTRY WtkGetNextEaValue( PSZ pszName, PSZ pszEaName, PULONG pulEaType, PHEA phea, PSZ pszValue, PULONG pulBuflen);

/*** read/appendWrite EA in one step ***************************************/
APIRET APIENTRY WtkPutEaValue( PSZ pszName, PSZ pszEaName, ULONG ulMultiType, ULONG ulEaType, PBYTE pbValue,  ULONG ulValuelen, BOOL fWriteThru, BYTE fEA);

/*** simplified APIs to read/write exatcly one string EA********************/
APIRET APIENTRY WtkWriteStringEa( PSZ pszName, PSZ pszEaName, PSZ pszEaValue);
APIRET APIENTRY WtkReadStringEa( PSZ pszName, PSZ pszEaName, PSZ pszEaValue, PULONG pulBuflen);

#ifdef __cplusplus
        }
#endif

#endif /* WTKEAS_INCLUDED */

