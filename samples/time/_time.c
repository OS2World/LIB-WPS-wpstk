/****************************** Module Header ******************************\
*
* Module Name: _time.c
*
* time functions sample
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: _time.c,v 1.5 2004-04-15 21:18:47 cla Exp $
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
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define INCL_ERRORS
#include <os2.h>

#define INCL_WTKUTLTIME
#define INCL_WTKUTLMODULE
#include <wtk.h>

// -----------------------------------------------------------------------------

#define PRINTSEPARATOR printf("\n------------------------------------------\n")


int main ( int argc, char *argv[])
{

         APIRET         rc = NO_ERROR;
         BOOL           fError;

         CDATE          cdateCnr;
         CTIME          ctimeCnr;

         FTIME          ftime;
         FDATE          fdate;
         CHAR           szTimestamp[ 32];

         CHAR           szExeFile[ _MAX_PATH];
         CHAR           szExePath[ _MAX_PATH];
         DATETIME       dt;

         time_t         timeFileD;
         time_t         timeFileF;
         time_t         timeFileC;
         time_t         timeCurrent;

do
   {

   // ----------------------------------------------------------------------

   PRINTSEPARATOR;

   // try out some stamps with current time
   printf( "\nsome timestamps of current time:\n");
   do
      {
      rc = WtkQueryDateTimeStamp( NULL, WTK_TIMESTAMP_SORTEDDATETIME, szTimestamp, sizeof( szTimestamp));
      if (rc != NO_ERROR)
         break;
      printf( "sorted date/time: %s\n", szTimestamp);

      rc = WtkQueryDateTimeStamp( NULL, WTK_TIMESTAMP_SORTEDDATE, szTimestamp, sizeof( szTimestamp));
      if (rc != NO_ERROR)
         break;
      printf( "sorted date     : %s\n", szTimestamp);

      rc = WtkQueryDateTimeStamp( NULL, WTK_TIMESTAMP_SORTEDTIME, szTimestamp, sizeof( szTimestamp));
      if (rc != NO_ERROR)
         break;
      printf( "sorted time     : %s\n", szTimestamp);

      rc = WtkQueryDateTimeStamp( NULL, WTK_TIMESTAMP_NLSDATETIME, szTimestamp, sizeof( szTimestamp));
      if (rc != NO_ERROR)
         break;
      printf( "NLS date/time   : %s\n", szTimestamp);

      rc = WtkQueryDateTimeStamp( NULL, WTK_TIMESTAMP_NLSDATE, szTimestamp, sizeof( szTimestamp));
      if (rc != NO_ERROR)
         break;
      printf( "NLS date        : %s\n", szTimestamp);

      rc = WtkQueryDateTimeStamp( NULL, WTK_TIMESTAMP_NLSTIME, szTimestamp, sizeof( szTimestamp));
      if (rc != NO_ERROR)
         break;
      printf( "NLS time        : %s\n", szTimestamp);

      } while (FALSE);

   if (rc != NO_ERROR)
      {
      printf( "\nWtkQueryDateTimeStamp: cannot set cdate/ctime to current time.\n");
      break;
      }

   // ======================================================================

   PRINTSEPARATOR;

   // get current time into container date time
   if (!WtkTimeToCDateTime( NULL, &cdateCnr, &ctimeCnr))
      {
      printf( "\nWtkSetCDateTimeToCurrent: cannot set CDATE/CTIME to current time.\n");
      break;
      }
   else
      printf( "\nWtkSetCDateTimeToCurrent: set CDATE/CTIME to current time.\n");


   rc = WtkQueryCDateTimeStamp( &cdateCnr, &ctimeCnr, WTK_TIMESTAMP_NLSDATETIME, szTimestamp, sizeof( szTimestamp));
   if (rc != NO_ERROR)
      {
      printf( "\nWtkQueryCDateTimeStamp: cannot get timestamp from CDATE/CTIME.\n");
      break;
      }
   else
      printf( "\nWtkQueryCDateTimeStamp: timestamp from CDATE/CTIME is %s.\n", szTimestamp);

   // ----------------------------------------------------------------------

   PRINTSEPARATOR;

   // get current time into file date time
   if (!WtkTimeToFDateTime( NULL, &fdate, &ftime))
      {
      printf( "\nWtkSetFDateTimeToCurrent: cannot set FDATE/FTIME to current time.\n");
      break;
      }
   else
      printf( "\nWtkSetFDateTimeToCurrent: set FDATE/FTIME to current time.\n");

   rc = WtkQueryFDateTimeStamp( &fdate, &ftime, WTK_TIMESTAMP_NLSDATETIME, szTimestamp, sizeof( szTimestamp));
   if (rc != NO_ERROR)
      {
      printf( "\nWtkQueryFDateTimeStamp: cannot get timestamp from FDATE/FTIME.\n");
      break;
      }
   else
      printf( "\nWtkQueryFDateTimeStamp: timestamp from FDATE/FTIME is %s.\n", szTimestamp);

   // ======================================================================

   PRINTSEPARATOR;

   // get current time
   time( &timeCurrent);

   // get fullname of our exe file
   rc = WtkGetModuleInfo( (PFN) main, NULL, szExeFile, sizeof( szExeFile));
   if (rc == NO_ERROR)
      printf( "\nWtkGetModuleInfo: exe file is: %s\n", szExeFile);
   else
      {
      printf( "\nWtkGetModuleInfo: cannot get name of our exefile.\n");
      break;
      }

   // get path of our exe file
   rc = WtkGetModulePath( (PFN) main, szExePath, sizeof( szExePath));
   if (rc == NO_ERROR)
      printf( "\nWtkGetModulePath: exe directory is: %s\n", szExePath);
   else
      {
      printf( "\nWtkGetModulePath: cannot get dir of our exefile.\n");
      break;
      }

   // --- try it with DATETIME ---------------------------

   PRINTSEPARATOR;

   rc = WtkQueryFileDateTime( &dt, szExeFile);
   if (rc != NO_ERROR)
      {
      printf( "\nWtkQueryFileDateTime: cannot get timestamp from our exefile.\n");
      break;
      }
   else
      printf( "\nWtkQueryFileDateTime:  got timestamp from our exefile into DATETIME.\n");

   rc = WtkQueryDateTimeStamp( &dt, WTK_TIMESTAMP_NLSDATETIME, szTimestamp, sizeof( szTimestamp));
   if (rc != NO_ERROR)
      {
      printf( "\nWtkQueryDateTimeStamp: cannot get timestamp from DATETIME.\n");
      break;
      }
   else
      printf( "\nWtkQueryDateTimeStamp: timestamp from DATETIME is %s.\n", szTimestamp);


   if (!WtkDateTimeToTime( &dt, &timeFileD))
      {
      printf( "\nWtkDateTimeToTime: cannot convert DATETIME to time.\n");
      break;
      }
   else
      printf( "\nWtkDateTimeToTime: converted DATETIME to time, file is %u seconds old.\n",
               timeCurrent - timeFileD);


   // try it with FDATE FTIME ---------------------------

   PRINTSEPARATOR;

   rc = WtkQueryFileFDateTime( &fdate, &ftime, szExeFile);
   if (rc != NO_ERROR)
      {
      printf( "\nWtkQueryFileFDateTime: cannot get timestamp from our exefile.\n");
      break;
      }
   else
      printf( "\nWtkQueryFileFDateTime:  got timestamp from our exefile into FDATE/FTIME.\n");

   rc = WtkQueryFDateTimeStamp( &fdate, &ftime, WTK_TIMESTAMP_NLSDATETIME, szTimestamp, sizeof( szTimestamp));
   if (rc != NO_ERROR)
      {
      printf( "\nWtkQueryFDateTimeStamp: cannot get timestamp from FDATE/FTIME.\n");
      break;
      }
   else
      printf( "\nWtkQueryFDateTimeStamp: timestamp from FDATE/FTIME is %s.\n", szTimestamp);

   if (!WtkFDateTimeToTime( &fdate, &ftime, &timeFileF))
      {
      printf( "\nWtkFDateTimeToTime: cannot convert FDATE/FTIME to time.\n");
      break;
      }
   else
      printf( "\nWtkFDateTimeToTime: converted FDATE/FTIME to time, file is %u seconds old.\n",
               timeCurrent - timeFileF);

   if (! (timeFileD == timeFileF))
      {
      printf( "\nerror: timestamp not equal, datetime %u fdate/ftime %u\n", timeFileD, timeFileF);
      rc = ERROR_INVALID_DATA;
      break;
      }

   // try it with CDATE CTIME ---------------------------

   PRINTSEPARATOR;

   rc = WtkQueryFileCDateTime( &cdateCnr, &ctimeCnr, szExeFile);
   if (rc != NO_ERROR)
      {
      printf( "\nWtkQueryFileCDateTime: cannot get timestamp from our exefile.\n");
      break;
      }
   else
      printf( "\nWtkQueryFileCDateTime:  got timestamp from our exefile into CDATE/CTIME.\n");

   rc = WtkQueryCDateTimeStamp( &cdateCnr, &ctimeCnr, WTK_TIMESTAMP_NLSDATETIME, szTimestamp, sizeof( szTimestamp));
   if (rc != NO_ERROR)
      {
      printf( "\nWtkQueryCDateTimeStamp: cannot get timestamp from CDATE/CTIME.\n");
      break;
      }
   else
      printf( "\nWtkQueryCDateTimeStamp: timestamp from FDATE/FTIME is %s.\n", szTimestamp);

   if (!WtkCDateTimeToTime( &cdateCnr, &ctimeCnr, &timeFileC))
      {
      printf( "\nWtkCDateTimeToTime: cannot convert CDATE/CTIME to time.\n");
      break;
      }
   else
      printf( "\nWtkCDateTimeToTime: converted CDATE/CTIME to time, file is %u seconds old.\n",
               timeCurrent - timeFileC);

   if (! (timeFileD == timeFileC))
      {
      printf( "\nerror: timestamp not equal, datetime %u cdate/ctime %u\n", timeFileD, timeFileC);
      rc = ERROR_INVALID_DATA;
      break;
      }

   // ======================================================================

   PRINTSEPARATOR;

   // convert time to datetime - cannot fail here
   WtkTimeToDateTime( &timeCurrent, &dt);
   WtkDateTimeToTime( &dt, &timeFileD);
   if (timeFileD != timeCurrent)
      {
      printf( "\nerror: WtkTimeToDateTime / WtkDateTimeToTime does not match: %u != %u\n", timeFileD, timeCurrent);
      rc = ERROR_INVALID_DATA;
      break;
      }
   else
      printf( "\nWtkTimeToDateTime / WtkDateTimeToTime matches\n");

   // ======================================================================

   PRINTSEPARATOR;

#define TIMEVALUES  17, 7, 1997, 22, 41, 44
   fError = FALSE;

   // test set routines
   WtkSetDateTime( TIMEVALUES, &dt);
   WtkDateTimeToTime( &dt, &timeFileD);
   if (timeFileD == (ULONG) -1)
      {
      printf( "\nerror: in WtkSetDateTime\n");
      fError = TRUE;
      }
   else
      printf( "\nWtkSetDateTime/WtkDateTimeToTime reports: %u\n", timeFileD);

   WtkSetFDateTime( TIMEVALUES, &fdate, &ftime);
   WtkFDateTimeToTime( &fdate, &ftime, &timeFileF);
   if (timeFileF == (ULONG) -1)
      {
      printf( "\nerror: in WtkSetFDateTime\n");
      fError = TRUE;
      }
   else
      printf( "\nWtkSetFDateTime/WtkFDateTimeToTime reports: %u\n", timeFileF);

   WtkSetCDateTime( TIMEVALUES, &cdateCnr, &ctimeCnr);
   WtkCDateTimeToTime( &cdateCnr, &ctimeCnr, &timeFileC);
   if (timeFileC == (ULONG) -1)
      {
      printf( "\nerror: in WtkSetCDateTime\n");
      fError = TRUE;
      }
   else
      printf( "\nWtkSetCDateTime/WtkCDateTimeToTime reports: %u\n", timeFileC);

   if (fError)
      {
      rc = ERROR_INVALID_DATA;
      break;
      }

   if ((timeFileD == timeFileF) && (timeFileF == timeFileC))
       printf( "\nWtkSet*Time: all reported values match\n");
    else
       printf( "\nerror: WtkSet*Time: mo match\n");


   } while (FALSE);

return rc;
}


