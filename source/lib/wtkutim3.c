/****************************** Module Header ******************************\
*
* Module Name: wtkutim.c
*
* Source for time related helper functions.
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: wtkutim3.c,v 1.2 2003-04-24 14:33:32 cla Exp $
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
@@WtkCDateTimeToTime@SYNTAX
This function converts the contents of a CDATE and a CTIME stucture to
a time value used by the compiler runtime.

@@WtkCDateTimeToTime@PARM@pcdate@in
Address of the CDATE structure.

@@WtkCDateTimeToTime@PARM@pctime@in
Address of the CTIME structure.

@@WtkCDateTimeToTime@PARM@ptime@out
Address of the resulting time value.

@@WtkCDateTimeToTime@RETURN
Success indicator.
:parml compact.
:pt.TRUE
:pd.Conversion successful.
:pt.FALSE
:pd.Conversion not successful or invalid parameters.
:eparml.

@@WtkCDateTimeToTime@REMARKS
This function does not check any of the values of the
CDATE or CTIME structures before converting.

@@
*/

BOOL APIENTRY WtkCDateTimeToTime( PCDATE pcdate, PCTIME pctime,  time_t* ptime)
{
         BOOL           fResult = FALSE;
         APIRET         rc = NO_ERROR;
         struct tm      tm;

         time_t         ltime;
         struct tm     *ptm;
do
   {
   // check parms
   if ((!pcdate) || (!pctime) || (!ptime))
      break;

   // fill buffer and convert cvalue
   memset( &tm, 0, sizeof( struct tm));
   tm.tm_mday = pcdate->day;
   tm.tm_mon  = pcdate->month - 1;
   tm.tm_year = pcdate->year - 1900;
   tm.tm_sec  = pctime->seconds;
   tm.tm_min  = pctime->minutes;
   tm.tm_hour = pctime->hours;
   *ptime = mktime( &tm);
   fResult = (*ptime != -1);

   } while (FALSE);

return fResult;

}

// ---------------------------------------------------------------------------

/*
@@WtkTimeToCDateTime@SYNTAX
This function either converts a time value used by the compiler
runtime to CDATE and CTIME stuctures or fills it with values
of current date and time.

@@WtkTimeToCDateTime@PARM@ptime@in
Address of the time value.
:p.
In order to use the current time,
leave this parameter NULL.

@@WtkTimeToCDateTime@PARM@pcdate@out
Address of the CDATE structure.
:p.
If the setup of a CDATE structure is not required,
leave this parameter NULL.

@@WtkTimeToCDateTime@PARM@pctime@out
Address of the CTIME structure.
:p.
If the setup of a CTIME structure is not required,
leave this parameter NULL.

@@WtkTimeToCDateTime@RETURN
Success indicator.
:parml compact.
:pt.TRUE
:pd.Conversion successful.
:pt.FALSE
:pd.Conversion not successful or invalid parameters.
:eparml.

@@WtkTimeToCDateTime@REMARKS
This function uses the localtime function and thus
modifies the runtime memory being used for reporting
the result.

@@
*/

BOOL APIENTRY WtkTimeToCDateTime( time_t* ptime, PCDATE pcdate, PCTIME pctime)
{
         BOOL           fResult = FALSE;
         APIRET         rc = NO_ERROR;
         struct tm     *ptm;
         time_t         time_local;
do
   {
   // check parms
   if ((!pcdate) || (!pctime))
      break;

   // convert time value
   if (ptime)
      time_local = *ptime;
   else
      time( &time_local);

   ptm = localtime( &time_local);
   if (!ptm)
      break;

   // fill buffer and convert cvalue
   memset( pcdate, 0, sizeof( PCDATE));
   memset( pctime, 0, sizeof( PCTIME));
   pcdate->day      = ptm->tm_mday;
   pcdate->month    = ptm->tm_mon + 1;
   pcdate->year     = ptm->tm_year + 1900;
   pctime->hours    = ptm->tm_hour;
   pctime->minutes  = ptm->tm_min;
   pctime->seconds  = ptm->tm_sec;
   fResult = TRUE;

   } while (FALSE);

return fResult;

}

// ---------------------------------------------------------------------------

/*
@@WtkSetCDateTime@SYNTAX
This function sets up a CDATE and/or CTIME structure in a single API call.

@@WtkSetCDateTime@PARM@uchDay@in
Day of the month, using values 1 through 31.

@@WtkSetCDateTime@PARM@uchMonth@in
Month of the year, using values 1 through 12.

@@WtkSetCDateTime@PARM@usYear@in
Year, using values in a four digit format.

@@WtkSetCDateTime@PARM@uchHours@in
Hour, using values 0 through 23.

@@WtkSetCDateTime@PARM@uchMinutes@in
Minute, using values 0 through 59.

@@WtkSetCDateTime@PARM@uchSeconds@in
Second, using values 0 through 59.

@@WtkSetCDateTime@PARM@pcdate@in
Address of the CDATE structure to set up.
:p.
If the setup of a CDATE structure is not required,
leave this parameter NULL.

@@WtkSetCDateTime@PARM@pctime@in
Address of the CTIME structure to set up.
:p.
If the setup of a CTIME structure is not required,
leave this parameter NULL.

@@WtkSetCDateTime@RETURN
Success indicator.
:parml compact.
:pt.TRUE
:pd.Conversion successful.
:pt.FALSE
:pd.Conversion not successful or invalid parameters.
:eparml.

@@WtkSetCDateTime@REMARKS
This function does not check any of the date or time values.
It only fails, if both pointers to the CDATE and CTIME structure are NULL.

@@
*/

BOOL APIENTRY WtkSetCDateTime( UCHAR uchDay, UCHAR uchMonth, USHORT usYear, UCHAR uchHours, UCHAR uchMinutes, UCHAR uchSeconds, PCDATE pcdate, PCTIME pctime)
{
         BOOL           fResult = FALSE;
do
   {
   // check parms
   if ((!pcdate) && (!pctime))
      break;

   // set date
   if (pcdate)
      {
      pcdate->day   = uchDay;
      pcdate->month = uchMonth;
      pcdate->year  = usYear;
      }

   // set time
   if (pctime)
      {
      pctime->hours   = uchHours;
      pctime->minutes = uchMinutes;
      pctime->seconds = uchSeconds;
      }

   fResult = TRUE;

   } while (FALSE);

return fResult;

}

// ---------------------------------------------------------------------------

/*
@@WtkQueryFileCDateTime@SYNTAX
This function stores the last write date and time
info of a file or directory in a CDATE and CTIME structure.

@@WtkQueryFileCDateTime@PARM@pcdate@out
Address of the CDATE structure to setup.

@@WtkQueryFileCDateTime@PARM@pctime@out
Address of the CTIME structure to setup.

@@WtkQueryFileCDateTime@PARM@pszName@in
Address of the ASCIIZ path name of the file to be examined.
:p.
The path name may contain :hp2.?&colon.:ehp2. for the drive in order to
open the file or directory on the boot drive.

@@WtkQueryFileCDateTime@RETURN
Return Code.
:p.
WtkQueryFileCDateTime returns one of the following return codes&colon.

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

@@WtkQueryFileCDateTime@REMARKS
This function supports boot drive recognition.
Specify :hp2.?&colon.:ehp2. for the drive in order to
open the file or directory on the boot drive.

@@
*/

APIRET APIENTRY WtkQueryFileCDateTime( PCDATE pcdate, PCTIME pctime, PSZ pszName)
{
         APIRET         rc = NO_ERROR;
         DATETIME       dt;

         CHAR           szName[ _MAX_PATH];
         FILESTATUS3    fs3;

do
   {
   // check parms
   if ((!pcdate) || (!pctime) || (!pszName))
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
   memset( pcdate, 0, sizeof( PCDATE));
   memset( pctime, 0, sizeof( PCTIME));
   pcdate->day     = fs3.fdateLastWrite.day;
   pcdate->month   = fs3.fdateLastWrite.month;
   pcdate->year    = fs3.fdateLastWrite.year + 1980;
   pctime->hours   = fs3.ftimeLastWrite.hours;
   pctime->minutes = fs3.ftimeLastWrite.minutes;
   pctime->seconds = fs3.ftimeLastWrite.twosecs * 2;

   } while (FALSE);

return rc;

}

