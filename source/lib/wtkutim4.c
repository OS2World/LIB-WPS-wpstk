/****************************** Module Header ******************************\
*
* Module Name: wtkutim.c
*
* Source for time related helper functions.
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: wtkutim4.c,v 1.2 2003-04-24 14:33:32 cla Exp $
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

#define ENDOFSTRING(s) (s+strlen(s))

// ---------------------------------------------------------------------------

static APIRET APIENTRY __QueryDateTimeStamp( PDATETIME pdt, ULONG ulStampType, PSZ pszBuffer, ULONG ulBuflen)
{
         APIRET         rc = NO_ERROR;
         DATETIME       dt;

         COUNTRYCODE    cc         = {0};
         COUNTRYINFO    ci         = {0};
         ULONG          ulInfoLen;

         CHAR           szTimeStamp[ 32];

do
   {
   // check parms
   if (!pszBuffer)
      {
      rc= ERROR_INVALID_PARAMETER;
      break;
      }

   // init vars
   szTimeStamp[ 0] = 0;

   // we need the separators ...
   rc = DosQueryCtryInfo( sizeof( ci), &cc, &ci, &ulInfoLen);
   if (rc != NO_ERROR)
      break;

   // ... and current time, if needed
   if (!pdt)
      {
      rc = DosGetDateTime( &dt);
      if (rc != NO_ERROR)
         break;
      pdt = &dt;
      }

   // handle SORTED* and NLSDATE here
   switch( ulStampType)
      {
      case WTK_TIMESTAMP_SORTEDDATETIME:
         sprintf( szTimeStamp, "%04d%02d%02d%02d%02d%02d",
                  pdt->year,  pdt->month,   pdt->day,
                  pdt->hours, pdt->minutes, pdt->seconds);
         break;

      case WTK_TIMESTAMP_SORTEDDATE:
         sprintf( szTimeStamp, "%04d%02d%02d",
                  pdt->year,  pdt->month,   pdt->day);
         break;

      case WTK_TIMESTAMP_SORTEDTIME:
         sprintf( szTimeStamp, "%02d%02d%02d",
                  pdt->hours, pdt->minutes, pdt->seconds);
         break;


      case WTK_TIMESTAMP_NLSDATETIME:
      case WTK_TIMESTAMP_NLSDATE:
         switch (ci.fsDateFmt)
            {
            case 1: // dd/mm/yyyy
               sprintf( ENDOFSTRING( szTimeStamp),
                        "%d%s%d%s%d",
                        pdt->day,   ci.szDateSeparator,
                        pdt->month, ci.szDateSeparator,
                        pdt->year);
               break;

            case 2: // yyyy/mm/dd
               sprintf( ENDOFSTRING( szTimeStamp),
                       "%d%s%d%s%d",
                       pdt->year,  ci.szDateSeparator,
                       pdt->month, ci.szDateSeparator,
                       pdt->day);
               break;

            default: // mm/dd/yy
               sprintf( ENDOFSTRING( szTimeStamp),
                       "%d%s%d%s%d",
                       pdt->month, ci.szDateSeparator,
                       pdt->day,   ci.szDateSeparator,
                       pdt->year);
               break;


            } // switch (ci.fsDateFmt)

         break; // case WTK_TIMESTAMP_NLSDATETIME: case WTK_TIMESTAMP_NLSDATE:

      } // switch( ulStampType)


   // handle NLSTIME stuff in a second switch
   switch( ulStampType)
      {
      case WTK_TIMESTAMP_NLSDATETIME:
         strcat( szTimeStamp, " ");
         // fallthru !!!

      case WTK_TIMESTAMP_NLSTIME:
         switch (ci.fsTimeFmt)
            {
            case 1: // 24 hour
               sprintf( ENDOFSTRING( szTimeStamp),
                        "%d%s%02d%s%02d",
                        pdt->hours,   ci.szTimeSeparator,
                        pdt->minutes, ci.szTimeSeparator,
                        pdt->seconds);
               break;

            default: // 12 hour with am/pm
               {
                        BOOL           fIsPM;
                        UCHAR          hours;

               fIsPM = (pdt->hours > 12);
               hours = (fIsPM) ? pdt->hours - 12 : pdt->hours;
               sprintf( ENDOFSTRING( szTimeStamp),
                       "%d%s%02d%s%02d %s",
                        hours,        ci.szTimeSeparator,
                        pdt->minutes, ci.szTimeSeparator,
                        pdt->seconds,
                        (fIsPM) ? "pm" : "am");
               }
               break;

            } // switch (ci.fsTimeFmt)

         break; // case WTK_TIMESTAMP_NLSDATETIME: case WTK_TIMESTAMP_NLSTIME:

      } // switch( ulStampType)

   // is stamp tpye not valid ?
   if ( szTimeStamp[ 0] == 0)
      {
      rc= ERROR_INVALID_PARAMETER;
      break;
      }

   // does buffer fit ?
   if (strlen( szTimeStamp) + 1 > ulBuflen)
      {
      rc= ERROR_BUFFER_OVERFLOW;
      break;
      }

   // hand over result
   strcpy( pszBuffer, szTimeStamp);


   } while (FALSE);

return rc;

}

// ---------------------------------------------------------------------------

/*
@@WtkQueryDateTimeStamp@SYNTAX
This function determines a timestamp according to the values
of a DATETIME structure.

@@WtkQueryDateTimeStamp@PARM@pdt@in
Address of the DATETIME structure.

@@WtkQueryDateTimeStamp@PARM@ulStampType@in
Type of timestamp.

:parml.
:pt.WTK_TIMESTAMP_SORTEDDATETIME
:pd.returns a sorted date and timestamp in yyyymmddhhmmss
:pt.WTK_TIMESTAMP_SORTEDDATE
:pd.returns a sorted datestamp in yyyymmdd
:pt.WTK_TIMESTAMP_SORTEDTIME
:pd.returns a sorted timestamp in hhmmss
:pt.WTK_TIMESTAMP_NLSDATETIME
:pd.returns a NLS compliant date and timestamp
:pt.WTK_TIMESTAMP_NLSDATE
:pd.returns a NLS compliant datestamp
:pt.WTK_TIMESTAMP_NLSTIME
:pd.returns a NLS compliant timestamp
:eparml.

@@WtkQueryDateTimeStamp@PARM@pszBuffer@out
The address of a buffer, into which the
timestamp is returned.

@@WtkQueryDateTimeStamp@PARM@ulBuflen@in
The length, in bytes, of the buffer described by :hp1.pszBuffer:ehp1..
A buffer of 32 bytes of size should fit for all purposes.


@@WtkQueryDateTimeStamp@RETURN
Return Code.
:p.
WtkQueryDateTimeStamp returns one of the following return codes&colon.

:parml compact break=none.
:pt.0
:pd.NO_ERROR
:pt.87
:pd.ERROR_INVALID_PARAMETER
:pt.111
:pd.ERROR_BUFFER_OVERFLOW
:eparml.
:p.
or return codes of the following functions
:ul compact.
:li.DosQueryCtryInfo
:eul.

@@WtkQueryDateTimeStamp@REMARKS
none

@@
*/

APIRET APIENTRY WtkQueryDateTimeStamp( PDATETIME pdt, ULONG ulStampType, PSZ pszBuffer, ULONG ulBuflen)
{
return __QueryDateTimeStamp(  pdt, ulStampType, pszBuffer, ulBuflen);
}

// ---------------------------------------------------------------------------

/*
@@WtkQueryFDateTimeStamp@SYNTAX
This function determines a timestamp according to the values
of a FDATE and FTIME structure.

@@WtkQueryFDateTimeStamp@PARM@pfdate@in
Address of the FDATE structure.

@@WtkQueryFDateTimeStamp@PARM@pftime@in
Address of the FTIME structure.

@@WtkQueryFDateTimeStamp@PARM@ulStampType@in
Type of timestamp.

:parml.
:pt.WTK_TIMESTAMP_SORTEDDATETIME
:pd.returns a sorted date and timestamp in yyyymmddhhmmss
:pt.WTK_TIMESTAMP_SORTEDDATE
:pd.returns a sorted datestamp in yyyymmdd
:pt.WTK_TIMESTAMP_SORTEDTIME
:pd.returns a sorted timestamp in hhmmss
:pt.WTK_TIMESTAMP_NLSDATETIME
:pd.returns a NLS compliant date and timestamp
:pt.WTK_TIMESTAMP_NLSDATE
:pd.returns a NLS compliant datestamp
:pt.WTK_TIMESTAMP_NLSTIME
:pd.returns a NLS compliant timestamp
:eparml.

@@WtkQueryFDateTimeStamp@PARM@pszBuffer@out
The address of a buffer, into which the
timestamp is returned.

@@WtkQueryFDateTimeStamp@PARM@ulBuflen@in
The length, in bytes, of the buffer described by :hp1.pszBuffer:ehp1..
A buffer of 32 bytes of size should fit for all purposes.


@@WtkQueryFDateTimeStamp@RETURN
Return Code.
:p.
WtkQueryFDateTimeStamp returns one of the following return codes&colon.

:parml compact break=none.
:pt.0
:pd.NO_ERROR
:pt.87
:pd.ERROR_INVALID_PARAMETER
:pt.111
:pd.ERROR_BUFFER_OVERFLOW
:eparml.
:p.
or return codes of the following functions
:ul compact.
:li.DosQueryCtryInfo
:eul.

@@WtkQueryFDateTimeStamp@REMARKS
none

@@
*/

APIRET APIENTRY WtkQueryFDateTimeStamp(  PFDATE pfdate, PFTIME pftime, ULONG ulStampType, PSZ pszBuffer, ULONG ulBuflen)
{
         APIRET         rc = NO_ERROR;
         DATETIME       dt;
         time_t         timeTmp;
do
   {
   // convert to ctime
   if ((!WtkFDateTimeToTime( pfdate, pftime, &timeTmp)) ||
       (! WtkTimeToDateTime( &timeTmp, &dt)))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // create the stamp
   rc = __QueryDateTimeStamp(  &dt, ulStampType, pszBuffer, ulBuflen);

   } while (FALSE);

return rc;

}

// ---------------------------------------------------------------------------

/*
@@WtkQueryCDateTimeStamp@SYNTAX
This function determines a timestamp according to the values
of a CDATE and CTIME structure.

@@WtkQueryCDateTimeStamp@PARM@pcdate@in
Address of the CDATE structure.

@@WtkQueryCDateTimeStamp@PARM@pctime@in
Address of the CTIME structure.

@@WtkQueryCDateTimeStamp@PARM@ulStampType@in
Type of timestamp.

:parml.
:pt.WTK_TIMESTAMP_SORTEDDATETIME
:pd.returns a sorted date and timestamp in yyyymmddhhmmss
:pt.WTK_TIMESTAMP_SORTEDDATE
:pd.returns a sorted datestamp in yyyymmdd
:pt.WTK_TIMESTAMP_SORTEDTIME
:pd.returns a sorted timestamp in hhmmss
:pt.WTK_TIMESTAMP_NLSDATETIME
:pd.returns a NLS compliant date and timestamp
:pt.WTK_TIMESTAMP_NLSDATE
:pd.returns a NLS compliant datestamp
:pt.WTK_TIMESTAMP_NLSTIME
:pd.returns a NLS compliant timestamp
:eparml.

@@WtkQueryCDateTimeStamp@PARM@pszBuffer@out
The address of a buffer, into which the
timestamp is returned.

@@WtkQueryCDateTimeStamp@PARM@ulBuflen@in
The length, in bytes, of the buffer described by :hp1.pszBuffer:ehp1..
A buffer of 32 bytes of size should fit for all purposes.


@@WtkQueryCDateTimeStamp@RETURN
Return Code.
:p.
WtkQueryCDateTimeStamp returns one of the following return codes&colon.

:parml compact break=none.
:pt.0
:pd.NO_ERROR
:pt.87
:pd.ERROR_INVALID_PARAMETER
:pt.111
:pd.ERROR_BUFFER_OVERFLOW
:eparml.
:p.
or return codes of the following functions
:ul compact.
:li.DosQueryCtryInfo
:eul.

@@WtkQueryCDateTimeStamp@REMARKS
none

@@
*/

APIRET APIENTRY WtkQueryCDateTimeStamp(  PCDATE pcdate, PCTIME pctime, ULONG ulStampType, PSZ pszBuffer, ULONG ulBuflen)
{
         APIRET         rc = NO_ERROR;
         DATETIME       dt;
         time_t         timeTmp;
do
   {
   // convert to ctime
   if ((!WtkCDateTimeToTime( pcdate, pctime, &timeTmp)) ||
       (! WtkTimeToDateTime( &timeTmp, &dt)))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // create the stamp
   rc = __QueryDateTimeStamp(  &dt, ulStampType, pszBuffer, ulBuflen);

   } while (FALSE);

return rc;

}

