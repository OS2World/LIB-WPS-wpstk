/****************************** Module Header ******************************\
*
* Module Name: wtkutim.h
*
* include file for date and time related helper functions
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: wtkutim.h,v 1.4 2006-12-02 21:09:04 cla Exp $
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

#ifndef WTKUTLTIME_INCLUDED
#define WTKUTLTIME_INCLUDED Date and time related helper functions

#ifdef __cplusplus
      extern "C" {
#endif

// required includes
#include <time.h>

#ifdef INCL_DOSINCLUDED
#define INCL_DOSDATETIME
#include <bsedos.h>
#else
#define INCL_DOSDATETIME
#include <os2.h>
#endif

#ifndef INCL_WINSTDCNR_INCLUDED
#define INCL_WINSTDCNR
#include <pmstddlg.h>
#endif

/*** prototypes for DATETIME struct handling ****************************/
BOOL APIENTRY WtkSetDateTime( UCHAR uchDay, UCHAR uchMonth, USHORT usYear, UCHAR uchHours,
                              UCHAR uchMinutes, UCHAR uchSeconds, PDATETIME pdt);
BOOL APIENTRY WtkDateTimeToTime( PDATETIME pdt, time_t* ptime);
BOOL APIENTRY WtkTimeToDateTime( time_t* ptime, PDATETIME pdt);

/*** prototypes for FTIME FDATE struct handling *************************/
BOOL APIENTRY WtkSetFDateTime( UCHAR uchDay, UCHAR uchMonth, USHORT usYear, UCHAR uchHours,
                               UCHAR uchMinutes, UCHAR uchSeconds, PFDATE pfdate, PFTIME pftime);
BOOL APIENTRY WtkFDateTimeToTime( PFDATE pfdate, PFTIME pftime, time_t* ptime);
BOOL APIENTRY WtkTimeToFDateTime( time_t* ptime, PFDATE pfdate, PFTIME pftime);

/*** prototypes for CDATE/CTIME struct handling ****************************/
BOOL APIENTRY WtkSetCDateTime( UCHAR uchDay, UCHAR uchMonth, USHORT usYear, UCHAR uchHours,
                               UCHAR uchMinutes, UCHAR uchSeconds, PCDATE pcdate, PCTIME pctime);
BOOL APIENTRY WtkCDateTimeToTime( PCDATE pcdate, PCTIME pctime, time_t* ptime);
BOOL APIENTRY WtkTimeToCDateTime( time_t* ptime, PCDATE pcdate, PCTIME pctime);

/*** prototypes for getting a timestamp *********************************/
#define WTK_TIMESTAMP_SORTEDDATETIME 0
#define WTK_TIMESTAMP_SORTEDDATE     1
#define WTK_TIMESTAMP_SORTEDTIME     2
#define WTK_TIMESTAMP_NLSDATETIME    3
#define WTK_TIMESTAMP_NLSDATE        4
#define WTK_TIMESTAMP_NLSTIME        5
APIRET APIENTRY WtkQueryDateTimeStamp( PDATETIME pdt, ULONG ulStampType,
                                       PSZ pszBuffer, ULONG ulBuflen);
APIRET APIENTRY WtkQueryFDateTimeStamp( PFDATE pfdate, PFTIME pftime, ULONG ulStampType,
                                        PSZ pszBuffer, ULONG ulBuflen);
APIRET APIENTRY WtkQueryCDateTimeStamp( PCDATE pcdate, PCTIME pctime, ULONG ulStampType,
                                        PSZ pszBuffer, ULONG ulBuflen);

/*** prototypes for getting last write time from a file or directory ****/
APIRET APIENTRY WtkQueryFileDateTime( PDATETIME pdt, PSZ pszName);
APIRET APIENTRY WtkQueryFileFDateTime( PFDATE pfdate, PFTIME pftime, PSZ pszName);
APIRET APIENTRY WtkQueryFileCDateTime( PCDATE pcdate, PCTIME pctime, PSZ pszName);

#ifdef __cplusplus
        }
#endif

#endif /* WTKUTLTIME_INCLUDED */

