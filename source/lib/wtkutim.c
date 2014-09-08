/****************************** Module Header ******************************\
*
* Module Name: wtkutim.c
*
* Source for time related helper functions.
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: wtkutim.c,v 1.6 2003-04-24 14:33:32 cla Exp $
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
#include <time.h>

#define INCL_DOS
#define INCL_WINSTDCNR
#define INCL_ERRORS
#include <os2.h>

#include "wtkutim.h"
#include "wpstk.ih"

// ---------------------------------------------------------------------------

/*
@@WtkDateTimeToTime@SYNTAX
This function converts the contents of a DATETIME stucture to
a time value used by the compiler runtime.

@@WtkDateTimeToTime@PARM@pdt@in
Address of the DATETIME structure.

@@WtkDateTimeToTime@PARM@ptime@out
Address of the resulting time value.

@@WtkDateTimeToTime@RETURN
Success indicator.
:parml compact.
:pt.TRUE
:pd.Conversion successful.
:pt.FALSE
:pd.Conversion not successful or invalid parameters.
:eparml.

@@WtkDateTimeToTime@REMARKS
This function does not check any of the values of the
DATETIME structure
before converting.

@@
*/

BOOL APIENTRY WtkDateTimeToTime( PDATETIME pdt, time_t* ptime)
{
         BOOL           fResult = FALSE;
         struct tm      tm;
do
   {
   // check parms
   if ((!ptime) || (!pdt))
      break;

   // fill buffer and convert cvalue
   memset( &tm, 0, sizeof( struct tm));
   tm.tm_mday = pdt->day;
   tm.tm_mon  = pdt->month - 1;
   tm.tm_year = pdt->year - 1900;
   tm.tm_hour = pdt->hours;
   tm.tm_min  = pdt->minutes;
   tm.tm_sec  = pdt->seconds;
   *ptime = mktime( &tm);
   fResult = (*ptime != -1);

   } while (FALSE);

return fResult;

}

// ---------------------------------------------------------------------------

/*
@@WtkTimeToDateTime@SYNTAX
This function either converts a time value used by the compiler
runtime to a DATETIME stucture or fills it with values
of current date and time.

@@WtkTimeToDateTime@PARM@ptime@in
Address of the time value to be converted.
:p.
In order to use the current time,
leave this parameter NULL.

@@WtkTimeToDateTime@PARM@pdt@out
Address of the DATETIME structure to set up.

@@WtkTimeToDateTime@RETURN
Success indicator.
:parml compact.
:pt.TRUE
:pd.Conversion successful.
:pt.FALSE
:pd.Conversion not successful or invalid parameters.
:eparml.

@@WtkTimeToDateTime@REMARKS
When converting a given time value, this function uses
the localtime function and thus modifies the runtime
memory being used for reporting the result.

@@
*/

BOOL APIENTRY WtkTimeToDateTime( time_t* ptime, PDATETIME pdt)

{
         BOOL           fResult = FALSE;
         struct tm     *ptm;
         time_t*       ptimeCheck;

do
   {
   // check parms
   if (!pdt)
      break;

   if (ptime)
      {
      // convert time value
      ptm = localtime( ptime);
      if (!ptm)
         break;

      // fill buffer and convert cvalue
      memset( pdt, 0, sizeof( DATETIME));
      pdt->day     = ptm->tm_mday;
      pdt->month   = ptm->tm_mon + 1;
      pdt->year    = ptm->tm_year + 1900;
      pdt->weekday = ptm->tm_wday;
      pdt->hours   = ptm->tm_hour;
      pdt->minutes = ptm->tm_min;
      pdt->seconds = ptm->tm_sec;
      }
   else
      DosGetDateTime( pdt);

   fResult = TRUE;

   } while (FALSE);

return fResult;

}

// ---------------------------------------------------------------------------

/*
@@WtkSetDateTime@SYNTAX
This function sets up a DATETIME structure in a single API call.

@@WtkSetDateTime@PARM@uchDay@in
Day of the month, using values 1 through 31.

@@WtkSetDateTime@PARM@uchMonth@in
Month of the year, using values 1 through 12.

@@WtkSetDateTime@PARM@usYear@in
Year, using values from 1900 up.

@@WtkSetDateTime@PARM@uchHours@in
Hour, using values 0 through 23.

@@WtkSetDateTime@PARM@uchMinutes@in
Minute, using values 0 through 59.

@@WtkSetDateTime@PARM@uchSeconds@in
Second, using values 0 through 59.

@@WtkSetDateTime@PARM@pdt@out
Address of the DATETIME structure to set up.

@@WtkSetDateTime@RETURN
Success indicator.
:parml compact.
:pt.TRUE
:pd.Successful
:pt.FALSE
:pd.Not successful (due to invalid parameter).
:eparml.

@@WtkSetDateTime@REMARKS
This function does not check any of the date or time values.
It only fails, if the provided pointer to the
DATETIME structure is NULL.

@@
*/

BOOL APIENTRY WtkSetDateTime( UCHAR uchDay, UCHAR uchMonth, USHORT usYear, UCHAR uchHours, UCHAR uchMinutes, UCHAR uchSeconds, PDATETIME pdt)
{
         BOOL           fResult = FALSE;
do
   {
   // check parms
   if (!pdt)
      break;

   // setup struct
   memset( pdt, 0, sizeof( DATETIME));
   pdt->day     = uchDay;
   pdt->month   = uchMonth;
   pdt->year    = usYear;
   pdt->hours   = uchHours;
   pdt->minutes = uchMinutes;
   pdt->seconds = uchSeconds;
   fResult = TRUE;

   } while (FALSE);

return fResult;

}

// ---------------------------------------------------------------------------

/*
@@WtkQueryFileDateTime@SYNTAX
This function stores the last write date and time
info of a file or directory in a DATETIME structure.

@@WtkQueryFileDateTime@PARM@pdt@out
Address of the DATETIME structure to setup.

@@WtkQueryFileDateTime@PARM@pszName@in
Address of the ASCIIZ path name of the file to be examined.
:p.
The path name may contain :hp2.?&colon.:ehp2. for the drive in order to
open the file or directory on the boot drive.

@@WtkQueryFileDateTime@RETURN
Return Code.
:p.
WtkQueryFileDateTime returns one of the following return codes&colon.

:parml compact break=none.
:pt.0
:pd.NO_ERROR
:pt.87
:pd.ERROR_INVALID_PARAMETER
:eparml.
:p.
or return codes of the following functions
:ul compact.
:li.DosQueryPathInfo
:eul.

@@WtkQueryFileDateTime@REMARKS
This function supports boot drive recognition.
Specify :hp2.?&colon.:ehp2. for the drive in order to
open the file or directory on the boot drive.

@@
*/

APIRET APIENTRY WtkQueryFileDateTime( PDATETIME pdt, PSZ pszName)
{
         APIRET         rc = NO_ERROR;
         DATETIME       dt;

         CHAR           szName[ _MAX_PATH];
         FILESTATUS3    fs3;

do
   {
   // check parms
   if ((!pdt) || (!pszName))
      {
      rc= ERROR_INVALID_PARAMETER;
      break;
      }

   // copy name and replace ?: with bootdrive
   strcpy( szName, pszName);
   __PatchBootDrive( szName);

   // search entry
   rc = DosQueryPathInfo( szName,
                          FIL_STANDARD,
                          &fs3,
                          sizeof( fs3));
   if (rc != NO_ERROR)
      break;

   // setup struct
   memset( pdt, 0, sizeof( DATETIME));
   pdt->day     = fs3.fdateLastWrite.day;
   pdt->month   = fs3.fdateLastWrite.month;
   pdt->year    = fs3.fdateLastWrite.year + 1980;
   pdt->hours   = fs3.ftimeLastWrite.hours;
   pdt->minutes = fs3.ftimeLastWrite.minutes;
   pdt->seconds = fs3.ftimeLastWrite.twosecs * 2;

   } while (FALSE);

return rc;

}

