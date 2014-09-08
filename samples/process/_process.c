/****************************** Module Header ******************************\
*
* Module Name: _process.c
*
* process functions related sample
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: _process.c,v 1.8 2008-10-04 23:41:29 cla Exp $
*
* ===========================================================================*
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

#define INCL_ERRORS
#define INCL_DOS
#include <os2.h>

#define INCL_WTKPROCESS
#include <wtk.h>

// -----------------------------------------------------------------------------

#define PRINTSEPARATOR printf("\n------------------------------------------\n")

int main ( int argc, char *argv[])
{

         APIRET         rc = NO_ERROR;
         HPROCSTAT      hp = NULLHANDLE;

         ULONG          i;
         PPID           ppidList = NULL;
         BOOL           fUpdated = FALSE;
         ULONG          ulMaxLevel = 0;
         ULONG          ulTotalPidCount;
         ULONG          ulMaxThreads;

         PROCESSINFO    pi;
         PPID           ppid;

         STARTDATA      sd;
         ULONG          ulSessionId;
         PID            pid;

static   PSZ            pszExeName = "PMSHELL.EXE";
static   PSZ            pszDetailsHeader = 
                           "pid  threads executable\n"
                           "---- ------- ---------------------------------------------\n";
static   PSZ            pszDetailsMask =
                           "%04x %4u    %s\n";

do
   {
   // --------------------------------------------------------------------------------

   PRINTSEPARATOR;

   printf( "initialize\n");
   printf( "- WtkInitializeProcessStatus: initialize support for process status\n");
   rc = WtkInitializeProcessStatus( &hp, 0);
   if (rc != NO_ERROR)
      {
      printf( " WtkInitializeMmf: error: cannot initialize MMF support, rc=%u\n", rc);
      break;
      }

   // --------------------------------------------------------------------------------

   PRINTSEPARATOR;
   printf( "- WtkRefreshProcessStatus: query current process status\n");
   rc = WtkRefreshProcessStatus( hp, WTK_PROCSTATUS_REFRESH_ALWAYS, &fUpdated);
   if (rc != NO_ERROR)
      {
      printf( "WtkRefreshProcessStatus error: cannot query current process status, rc=%u\n", rc);
      break;
      }
   printf( "- retrieved current process status successfully\n");



   // --------------------------------------------------------------------------------

   PRINTSEPARATOR;

   printf( "- WtkQueryProcessStatusMaxLevel: query maximum depth of process status list\n");
   rc = WtkQueryProcessStatusMaxLevel( hp, &ulMaxLevel);
   if (rc != NO_ERROR)
      {
      printf( "WtkQueryProcessStatusMaxLevel error: cannot query maximum depth of process status, rc=%u\n", rc);
      break;
      }
   printf( "- maximum depth of process status is: %u\n", ulMaxLevel);


   // --------------------------------------------------------------------------------

   PRINTSEPARATOR;
   printf( "- WtkEnumProcessChilds/WtkEnumProcessThreads: query process/thread details\n"
           "\n"
           "%s", pszDetailsHeader);

   // get list of all processes
   rc = WtkEnumProcessChilds( hp, WTK_PROCESS_ENUMALL, NULL, &ulTotalPidCount);
   if (rc != NO_ERROR)
      break;

   // get memory for complete list
   ppidList  = malloc( sizeof( PID) * ulTotalPidCount);
   if (!ppidList)
      break;
   memset( ppidList, 0, sizeof( PID) * ulTotalPidCount);

   // get all process ids
   WtkEnumProcessChilds( hp, WTK_PROCESS_ENUMALL, ppidList, &ulTotalPidCount);

   // get max length of all items
   for (i = 0,  ppid = ppidList; i < ulTotalPidCount; i++, ppid++)
      {
      // get name of process
      rc = WtkQueryProcessDetails( hp, *ppid, &pi, NULL);
      if (rc != NO_ERROR)
         continue;

      // determine threads
      ulMaxThreads = 0;
      rc = WtkEnumProcessThreads( hp, pi.pid, NULL, &ulMaxThreads);

      // show pid and name of process
      printf( pszDetailsMask,
              pi.pid,
              ulMaxThreads,
              (pi.szFullname[ 0]) ? pi.szFullname : pi.szProcname);

      }

   // --------------------------------------------------------------------------------

   PRINTSEPARATOR;
   printf( "- WtkQueryProcess: query process/thread details for %s\n"
           "\n"
           "%s", pszExeName, pszDetailsHeader);

   // query details for the two instances of PMSHELL
   i = 0;
   do
      {
      // query details
      rc = WtkQueryProcess( hp, pszExeName, i, &pi);
      if (rc == NO_ERROR)
         // show pid and name of process
         printf( pszDetailsMask,
                 pi.pid,
                 ulMaxThreads,
                 (pi.szFullname[ 0]) ? pi.szFullname : pi.szProcname);

      // next instance
      i++;

      } while (rc == NO_ERROR);
    

   // --------------------------------------------------------------------------------

   PRINTSEPARATOR;

   // start test session
   printf( "- starting separate session with EPM.EXE\n");
   memset( &sd, 0, sizeof( sd));
   sd.Length   =  sizeof( sd);
   sd.Related  = SSF_RELATED_INDEPENDENT;
   sd.Related  = SSF_RELATED_CHILD;
   sd.FgBg     = SSF_FGBG_BACK;
   sd.PgmTitle = "WPSTK sampe test session";
   sd.PgmName  = "EPM.EXE";
   ulSessionId = 0;
   pid         = 0;
   rc = DosStartSession( &sd, &ulSessionId, &pid);
   if (rc != NO_ERROR)
      {
      printf( "error: cannot start session, rc=%u\n", rc);
      break;
      }
   printf( "- started session, pid is 0x%04X\n", pid);


   // get process details right from process list
   printf( "- WtkQueryProcessDetails: query process details for pid\n");
   memset( &pi, 0, sizeof( PROCESSINFO));
   rc = WtkQueryProcessDetails( NULLHANDLE, pid, &pi, NULL);
   if (rc != NO_ERROR)
      continue;
   printf( "  fullname: %s\n", pi.szFullname);

   printf( "- WtkSelectProcess: select session\n");
   rc = WtkSelectProcess( NULLHANDLE, pid);
   if (rc != NO_ERROR)
      {
      printf( "WtkSelectProcess error: session could not be selected, rc=%u\n", rc);
      break;
      }
   printf( "- selected session successfully\n");

   // wait for some time
   DosSleep( 500);

   printf( "- WtkKillProcess: kill session\n");
   WtkRefreshProcessStatus( hp, WTK_PROCSTATUS_REFRESH_ALWAYS, &fUpdated);
   rc = WtkKillProcess( hp, pid, 0);
   if (rc != NO_ERROR)
      {
      printf( "WtkKillProcess error: session could not be selected, rc=%u\n", rc);
      break;
      }
   printf( "- session was killed\n");

   // --------------------------------------------------------------------------------

   PRINTSEPARATOR;


   } while (FALSE);

// cleanup
printf( "cleanup\n");
if (ppidList) free( ppidList);
if (hp)
   {
   printf( "- WtkTerminateProcessStatus: terminate support for process status\n");
   WtkTerminateProcessStatus( hp);
   }
printf( "\n");

return rc;
}

