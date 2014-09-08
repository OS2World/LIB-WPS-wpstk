/****************************** Module Header ******************************\
*
* Module Name: wtkutim.c
*
* Source for time related helper functions.
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: wtkutim2.c,v 1.4 2008-10-23 20:10:06 cla Exp $
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
@@WtkFDateTimeToTime@SYNTAX
This function converts the contents of a FDATE and a FTIME stucture to
a time value used by the compiler runtime.

@@WtkFDateTimeToTime@PARM@pfdate@in
Address of the FDATE structure.

@@WtkFDateTimeToTime@PARM@pftime@in
Address of the FTIME structure.

@@WtkFDateTimeToTime@PARM@ptime@out
Address of the resulting time value.

@@WtkFDateTimeToTime@RETURN
Success indicator.
:parml compact.
:pt.TRUE
:pd.Conversion successful.
:pt.FALSE
:pd.Conversion not successful or invalid parameters.
:eparml.

@@WtkFDateTimeToTime@REMARKS
This function does not check any of the values of the
FDATE or FTIME structures before converting.

@@
*/

BOOL APIENTRY WtkFDateTimeToTime( PFDATE pfdate, PFTIME pftime, time_t* ptime)
{
         BOOL           fResult = FALSE;
         APIRET         rc = NO_ERROR;
         struct tm      tm;
do
   {
   // check parms
   if ((!pfdate) || (!pftime) || (!ptime))
      break;

   // fill buffer and convert cvalue
   memset( &tm, 0, sizeof( struct tm));
   tm.tm_mday = pfdate->day;
   tm.tm_mon  = pfdate->month - 1;
   tm.tm_year = pfdate->year + 80;  // tm.tm_year has base 1900, pfdate->year has base 1980
   tm.tm_hour = pftime->hours;
   tm.tm_min  = pftime->minutes;
   tm.tm_sec  = pftime->twosecs * 2;
   *ptime = mktime( &tm);
   fResult = (*ptime != -1);

   } while (FALSE);

return fResult;

}

// ---------------------------------------------------------------------------

/*
@@WtkTimeToFDateTime@SYNTAX
This function either converts a time value used by the compiler
runtime to FDATE and FTIME stuctures or fills it with values
of current date and time.

@@WtkTimeToFDateTime@PARM@ptime@in
Address of the time value.
:p.
In order to use the current time,
leave this parameter NULL.

@@WtkTimeToFDateTime@PARM@pfdate@out
Address of the FDATE structure.
:p.
If the setup of a FDATE structure is not required,
leave this parameter NULL.

@@WtkTimeToFDateTime@PARM@pftime@out
Address of the FTIME structure.
:p.
If the setup of a FDATE structure is not required,
leave this parameter NULL.

@@WtkTimeToFDateTime@RETURN
Success indicator.
:parml compact.
:pt.TRUE
:pd.Conversion successful.
:pt.FALSE
:pd.Conversion not successful or invalid parameters.
:eparml.

@@WtkTimeToFDateTime@REMARKS
This function uses the localtime function and thus
modifies the runtime memory being used for reporting
the result.

@@
*/

BOOL APIENTRY WtkTimeToFDateTime( time_t* ptime, PFDATE pfdate, PFTIME pftime)
{
         BOOL           fResult = FALSE;
         APIRET         rc = NO_ERROR;
         struct tm     *ptm;
         time_t         time_local;
do
   {
   // check parms
   if ((!pfdate) && (!pftime))
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
   if (pfdate)
      {
      memset( pfdate, 0, sizeof( FDATE));
      pfdate->day      = ptm->tm_mday;
      pfdate->month    = ptm->tm_mon + 1;
      pfdate->year     = ptm->tm_year - 80; // pfdate->year has base 1980, ptm->tm_year has base 1900
      }
   if (pftime)
      {
      memset( pftime, 0, sizeof( FTIME));
      pftime->hours    = ptm->tm_hour;
      pftime->minutes  = ptm->tm_min;
      pftime->twosecs  = ptm->tm_sec / 2;
      }
   fResult = TRUE;

   } while (FALSE);

return fResult;

}

// ---------------------------------------------------------------------------

/*
@@WtkSetFDateTime@SYNTAX
This function sets up a FDATE and/or FTIME structure
in a single API call.

@@WtkSetFDateTime@PARM@uchDay@in
Day of the month, using values 1 through 31.

@@WtkSetFDateTime@PARM@uchMonth@in
Month of the year, using values 1 through 12.

@@WtkSetFDateTime@PARM@usYear@in
Year, using values from 1980 up.

@@WtkSetFDateTime@PARM@uchHours@in
Hour, using values 0 through 23.

@@WtkSetFDateTime@PARM@uchMinutes@in
Minute, using values 0 through 59.

@@WtkSetFDateTime@PARM@uchSeconds@in
Second, using values 0 through 59.

@@WtkSetFDateTime@PARM@pfdate@out
Address of the FDATE structure to set up.
:p.
If the setup of a FDATE structure is not required,
leave this parameter NULL.

@@WtkSetFDateTime@PARM@pftime@out
Address of the FTIME structure to set up.
:p.
If the setup of a FTIME structure is not required,
leave this parameter NULL.

@@WtkSetFDateTime@RETURN
Success indicator.
:parml compact.
:pt.TRUE
:pd.Conversion successful.
:pt.FALSE
:pd.Conversion not successful or invalid parameters.
:eparml.

@@WtkSetFDateTime@REMARKS
This function does not check any of the date or time values.
It only fails, if both pointers to the FDATE and FTIME structure are NULL.
:p.
Due to limitations of the FAT filesystem
:ul compact.
:li. the FDATE structure can store and thus this function
can handle only dates from 1980 up.
:li. the FTIME structure can store and thus this function
can handle only seconds in two seconds steps. As a result,
the conversion via this function results in the loss of
one second, if an odd number of seconds is specified.
:eul.

@@
*/

BOOL APIENTRY WtkSetFDateTime( UCHAR uchDay, UCHAR uchMonth, USHORT usYear, UCHAR uchHours, UCHAR uchMinutes, UCHAR uchSeconds, PFDATE pfdate, PFTIME pftime)
{
         BOOL           fResult = FALSE;
do
   {
   // check parms
   if ((!pfdate) && (!pftime))
      break;

   // set time
   if (pfdate)
      {
      pfdate->day   = uchDay;
      pfdate->month = uchMonth;
      pfdate->year  = usYear - 1980;
      }

   if (pftime)
      {
      pftime->hours   = uchHours;
      pftime->minutes = uchMinutes;
      pftime->twosecs = uchSeconds / 2;
      }

   fResult = TRUE;

   } while (FALSE);

return fResult;

}

// ---------------------------------------------------------------------------

/*
@@WtkQueryFileFDateTime@SYNTAX
This function stores the last write date and time
info of a file or directory in a FDATE and FTIME structure.

@@WtkQueryFileFDateTime@PARM@pfdate@out
Address of the FDATE structure to setup.

@@WtkQueryFileFDateTime@PARM@pftime@out
Address of the FTIME structure to setup.

@@WtkQueryFileFDateTime@PARM@pszName@in
Address of the ASCIIZ path name of the file to be examined.
:p.
The path name may contain :hp2.?&colon.:ehp2. for the drive in order to
open the file or directory on the boot drive.

@@WtkQueryFileFDateTime@RETURN
Return Code.
:p.
WtkQueryFileFDateTime returns one of the following return codes&colon.

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

@@WtkQueryFileFDateTime@REMARKS
This function supports boot drive recognition.
Specify :hp2.?&colon.:ehp2. for the drive in order to
open the file or directory on the boot drive.

@@
*/

APIRET APIENTRY WtkQueryFileFDateTime( PFDATE pfdate, PFTIME pftime, PSZ pszName)
{
         APIRET         rc = NO_ERROR;
         DATETIME       dt;

         CHAR           szName[ _MAX_PATH];
         FILESTATUS3    fs3;

do
   {
   // check parms
   if ((!pfdate) || (!pftime) || (!pszName))
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

   // copy data
   memcpy( pfdate, &fs3.fdateLastWrite, sizeof( FDATE));
   memcpy( pftime, &fs3.ftimeLastWrite, sizeof( FTIME));

   } while (FALSE);

return rc;

}

