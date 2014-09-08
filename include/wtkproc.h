/****************************** Module Header ******************************\
*
* Module Name: wtkproc.h
*
* include file for process related functions.
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: wtkproc.h,v 1.9 2009-07-03 20:45:26 cla Exp $
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

#ifndef WTKPROCESS_INCLUDED
#define WTKPROCESS_INCLUDED Process related functions

#ifdef __cplusplus
      extern "C" {
#endif

#pragma pack(1)

/* define a handle type */
typedef LHANDLE HPROCSTAT;
typedef HPROCSTAT *PHPROCSTAT;


/* prototypes */

/* -- structure for process info */
typedef struct _PROCESSINFO {
  PID            pid;                     /* process identifier               */
  PID            pidParent;               /* parent process identifier        */
  HMODULE        hmod;                    /* handle of executable             */
  ULONG          ulSessionId;             /* session identifier               */
  ULONG          ulSessionType;           /* session type WTK_PROCTYPE_*      */
  ULONG          ulStatus;                /* process status WTK_PROCSTATUS_*  */
  ULONG          ulThreadCount;           /* number of threads                */
  PSZ            pszExename;              /* filename of executable           */
  CHAR           szFullname[ CCHMAXPATH]; /* full pathname of executable      */
  CHAR           szProcname[ CCHMAXPATH]; /* lowercase basename of executable */
} PROCESSINFO, *PPROCESSINFO;

#define WTK_ISKERNELPROCESS(p)  (p <= 1)

#define WTK_PROCTYPE_FULLSCREEN 0x00
#define WTK_PROCTYPE_VDM        0x01
#define WTK_PROCTYPE_WINDOW     0x02
#define WTK_PROCTYPE_PM         0x03
#define WTK_PROCTYPE_DETACHED   0x04

#define WTK_PROCSTATUS_EXITLIST 0x01
#define WTK_PROCSTATUS_EXIT1    0x02
#define WTK_PROCSTATUS_EXITALL  0x04
#define WTK_PROCSTATUS_PARSTAT  0x10
#define WTK_PROCSTATUS_SYNCH    0x20
#define WTK_PROCSTATUS_DYING    0x40
#define WTK_PROCSTATUS_EMBRYO   0x80

/* -- structure for thread info */
typedef struct _THREADINFO {
  ULONG          ulThreadId;  /* thread identifier                    */
  ULONG          ulPriority;  /* current priority                     */
  ULONG          ulStatus;    /* status of thread WTK_TRHEADSTATUS_*  */
} THREADINFO, *PTHREADINFO;

#define WTK_TRHEADSTATUS_READY   1
#define WTK_TRHEADSTATUS_BLOCKED 2
#define WTK_TRHEADSTATUS_RUNNING 5
#define WTK_TRHEADSTATUS_LOADED  9

/* prototypes */
APIRET APIENTRY WtkInitializeProcessStatus( PHPROCSTAT php, ULONG ulReserveMem);

APIRET APIENTRY WtkTerminateProcessStatus( HPROCSTAT hp);
APIRET APIENTRY WtkRefreshProcessStatus( HPROCSTAT hp, ULONG ulOptions, PBOOL pfUpdated);
#define WTK_PROCSTATUS_REFRESH_ALWAYS      0
#define WTK_PROCSTATUS_REFRESH_LISTCHANGED 1
APIRET APIENTRY WtkQueryProcessStatusMaxLevel( HPROCSTAT hp, PULONG pulMaxLevel);

APIRET APIENTRY WtkEnumProcessChilds( HPROCSTAT hp, PID pidParent, PPID papid, PULONG pulMaxPid);
#define WTK_PROCESS_ENUMALL  (ULONG)-1
APIRET APIENTRY WtkEnumProcessThreads( HPROCSTAT hp, PID pid, PTHREADINFO pati, PULONG pulMaxThreads);
APIRET APIENTRY WtkQueryProcessDetails( HPROCSTAT hp, PID pid, PPROCESSINFO ppi, PPVOID ppbReservedMem);

APIRET APIENTRY WtkKillProcess( HPROCSTAT hp, PID pid, BOOL fIncludeChildren);
APIRET APIENTRY WtkSelectProcess( HWND hwnd, PID pid);

APIRET APIENTRY WtkQueryProcess( HPROCSTAT hp, PSZ pszExename, ULONG ulIndex, PPROCESSINFO ppi);
APIRET APIENTRY WtkWaitProcess( HPROCSTAT hp, PSZ pszExename, ULONG ulWait, PULONG pulMaxWait);

#pragma pack()

#ifdef __cplusplus
        }
#endif

#endif /* WTKPROCESS_INCLUDED */

