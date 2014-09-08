/****************************** Module Header ******************************\
*
* Module Name: wtkproc.c
*
* Source for process related functions.
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2002
*
* $Id: wtkproc.c,v 1.12 2009-07-03 20:45:27 cla Exp $
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
* ===========================================================================
*
\***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INCL_DOS
#define INCL_WIN
#define INCL_ERRORS
#include <os2.h>

#include "wtkproc.h"
#include "wpstk.ih"

// definitions for dynamic linkage of undefined functions
typedef APIRET (APIENTRY16 FNDOSGETPRTY)(USHORT usScope, PUSHORT pusPriority, USHORT pid);
typedef FNDOSGETPRTY *PFNDOSGETPRTY;

typedef APIRET (APIENTRY16 FNDOSQPROCSTATUS)(PVOID pBuf, USHORT cbBuf);
typedef FNDOSQPROCSTATUS *PFNDOSQPROCSTATUS;

typedef APIRET (APIENTRY FNDOSQUERYSYSSTATE)(ULONG EntityList, ULONG EntityLevel, PID pid,
                                    TID tid, PVOID pDataBuf, ULONG cbBuf);
typedef FNDOSQUERYSYSSTATE *PFNDOSQUERYSYSSTATE;


// original prototypes and import definitions
// not used because not all compilers support pragma import
// USHORT APIENTRY16 DosGetPrty( USHORT usScope, PUSHORT pusPriority, USHORT pid);
// #pragma import (DosGetPrty, "DOSGETPRTY", "DOSCALLS", 9)
//
//
// USHORT APIENTRY16 DosQProcStatus(PVOID pBuf, USHORT cbBuf);
// #pragma import (DosQProcStatus, "DOSQPROCSTATUS", "DOSCALLS", 154)
//
// APIRET  APIENTRY DosQuerySysState( ULONG EntityList, ULONG EntityLevel, PID pid,
//                                   TID tid, PVOID pDataBuf, ULONG cbBuf);
// #pragma import (DosQuerySysState, "DOSQUERYSYSSTATE", "DOSCALLS", 368)

// internal debug macros
#ifdef DEBUG
#define DUMPHANDLE(h) printf( "%s(%u): dump handle in %s\n"                \
                              "handle   :%p\n"                             \
                              "procstat :%p\n"                             \
                              "CRC:     :%u\n",                            \
                              __FILE__,                                    \
                              __LINE__,                                    \
                              __FUNCTION__,                                \
                              h,                                           \
                              (h) ? ((PPROCSTATBUF) h)->pps   : (PVOID)-1, \
                              (h) ? ((PPROCSTATBUF) h)->ulCrc : -1)

#define DUMPPROCESS(p) printf( "%s(%u): dump process in %s\n"              \
                              "type          : %p\n"                       \
                              "process id    : %p\n"                       \
                              "parent id     : %p\n"                       \
                              "session type  : %p\n"                       \
                              "status        : %p\n"                       \
                              "session id    : %p\n"                       \
                              "module handle : %p\n"                       \
                              "reserved1     : %p\n"                       \
                              "reserved2     : %p\n"                       \
                              "reserved3     : %p\n"                       \
                              "reserved4     : %p\n",                      \
                              __FILE__,                                    \
                              __LINE__,                                    \
                              __FUNCTION__,                                \
                              p->ulType,                                   \
                              p->usProcessId,                              \
                              p->usParentId,                               \
                              p->ulSessionType,                            \
                              p->ulStatus,                                 \
                              p->ulSessionId,                              \
                              p->usModuleHandle,                           \
                              p->reserved1,                                \
                              p->reserved2,                                \
                              p->reserved3,                                \
                              p->reserved4);
#else

#define DUMPHANDLE(h)
#define DUMPPROCESS(p)
#endif

// convert macro
#define CONVERT(fp) (*((unsigned *) &(fp) + 1) = QSsel)
#define PTRNEXT(ptr, ofs)  ((void *) ((char *) (ptr) + (ofs)))

// memory chunks for process ulStatus list
#define MEMORY_BLOCKSIZE 0x4000

#pragma pack(1)

// ---------------------------------------------------------------------------

typedef struct _THREAD
{
         ULONG          ulType;
         USHORT         usThreadId;
         USHORT         usSlotId;
         ULONG          ulBlockId;
         ULONG          ulPriority;
         ULONG          ulSysTime;
         ULONG          ulUserTime;
         UCHAR          ulStatus; // see TSTAT_* below
         UCHAR          reserved1;
         USHORT         reserved2;
} THREAD, *PTHREAD;

// ---------------------------------------------------------------------------

typedef struct _SEMAPHORE
{
         struct _SEMAPHORE * psemaphoreNext;
         USHORT         usOwner;
         UCHAR          usFlags;
         UCHAR          usRefCount;
         UCHAR          usRequests;
         UCHAR          reserved1;
         ULONG          reserved2;
         USHORT         reserved3;
         USHORT         usIndex;
         USHORT         dummy;
         UCHAR          szName[1];
}_SEMAPHORE, *PSEMAPHORE;

// ---------------------------------------------------------------------------

typedef struct _SHMEM
{
         struct _SHMEM * pshmenNext;
         USHORT         usMemHandle;
         USHORT         usSelector;
         USHORT         usRefCount;
         UCHAR          szName[1];       /* varying */
} SHMEM, *PSHMEM;

// ---------------------------------------------------------------------------

typedef struct _MODULE
{
         struct _MODULE * pmoduleNext;
         USHORT         usModuleHandle;
         USHORT         usModuleType;
         ULONG          ulSubmoduleCount;
         PSHMEM         pashmem;
         ULONG          reserved;
         PSZ            pszName;
         USHORT         submodule[1];
} MODULE, *PMODULE;

// ---------------------------------------------------------------------------

typedef struct _PROCESS
{
         ULONG          ulType;
         PTHREAD        pathread;
         USHORT         usProcessId;
         USHORT         usParentId;
         ULONG          ulSessionType;
         ULONG          ulStatus;      //* see STAT_* below
         ULONG          ulSessionId;
         USHORT         usModuleHandle;
         USHORT         usThreadCount;
         ULONG          reserved1;
         ULONG          reserved2;
         USHORT         usSemCount;
         USHORT         dlls;
         USHORT         usShrMemCount;
         USHORT         reserved3;
         PSEMAPHORE     pasem;
         PMODULE        pamodule;
         PSHMEM         pashmem;
         ULONG          reserved4;
} PROCESS, *PPROCESS;

// ---------------------------------------------------------------------------

typedef struct _COUNTTABLE {
         ULONG          ulThreadCount;
         ULONG          ulProcessCount;
         ULONG          ulModuleCount;
} COUNTTABLE, *PCOUNTTABLE;

// ---------------------------------------------------------------------------

typedef struct _PROCSTAT
{
         PCOUNTTABLE    pct;
         PPROCESS       pprocess;
         PSEMAPHORE     psemaphore;
         ULONG          unknown1;
         PSHMEM         pshmem;
         PMODULE        pmodule;
         ULONG          unknown2;
         ULONG          unknown3;
} PROCSTAT, *PPROCSTAT;

// ----------------------------------------------------------------------------

typedef struct _PROCSTATBUF
{
         PPROCSTAT      pps;
         ULONG          ulBufsize;       // last bufsize used
         ULONG          ulCrc;           // checksum over all pids
         ULONG          ulProcessCount;  // count of processes
         ULONG          ulListLevelCount;// count of child levels
         ULONG          ulReserveMem;    // size of reserved memory per process in list
         PVOID          pvPrivateMem;    // reserved memory;
} PROCSTATBUF, *PPROCSTATBUF;

// ----------------------------------------------------------------------------

#define SYSSTATE_QUERYDATA_PROCESS         0x00000001
#define SYSSTATE_QUERYDATA_SEMAPHORE       0x00000002
#define SYSSTATE_QUERYDATA_MODULE          0x00000004
#define SYSSTATE_QUERYDATA_FILE            0x00000008
#define SYSSTATE_QUERYDATA_NAMEDSHAREDMEM  0x00000010


// ###########################################################################

static int _strrcmp( const char *string1, const char *string2)
{
         char          *p1, *p2;
         int           len1;
         int           len2;
         int           count;
         int           result;

// check parms
if ((!string1) && (!string2))
   return 0;
if ((!string1) || (!string2))
   return (string1 > string2);

// get maximum compare length
len1 = strlen( string1);
len2 = strlen( string2);
if ((!len1) && (!len2))
   return 0;
count = len1;
if (len2 < count) count = len2;

// set pointer to end of strings and compare
p1 = (char *) string1 + strlen( string1) - 1;
p2 = (char *) string2 + strlen( string2) - 1;
do
   {
   result = *p2 - *p1;
   if (result != 0)
      break;
   count--; p1--; p2--;
   } while (count);

return result;
}

// ###########################################################################

// encapsulate call to kernel
APIRET _queryProcessStatus( PVOID pvData, ULONG ulBufsize, PID pid)
{
         APIRET         rc = NO_ERROR;

         HMODULE        hmod;
         ULONG          ulFlags;

static   BOOL           fInitialized = FALSE;
static   PFNDOSQPROCSTATUS   pfnDosQProcStatus = NULL;
static   PFNDOSQUERYSYSSTATE pfnDosQuerySysState = NULL;

do
   {
   // get pointer to 32 or 16 bit version
   if (!fInitialized)
      {
      // get module handle
      rc = DosQueryModuleHandle( "DOSCALLS", &hmod);
      if (rc != NO_ERROR)
         break;

      // load 32 bit version API DosQuerySysState first
      rc = DosQueryProcAddr( hmod, 368, NULL, (PFN *) &pfnDosQuerySysState);
      if (rc != NO_ERROR)
         {
         // try 16 bit version DosQProcStat as fallback
         // for older Warp 3 versions
         rc = DosQueryProcAddr( hmod, 154, NULL, (PFN*) &pfnDosQProcStatus);
         }

      if (rc != NO_ERROR)
         break;

      // ok, we can use the API now
      fInitialized = TRUE;
      }

   // use either 32-bit or 16-bit API
   if (pfnDosQuerySysState)
      {
      if (pid)
         ulFlags = SYSSTATE_QUERYDATA_PROCESS;
      else
         ulFlags = SYSSTATE_QUERYDATA_PROCESS         |
                   SYSSTATE_QUERYDATA_SEMAPHORE       |
                   SYSSTATE_QUERYDATA_MODULE          |
                   SYSSTATE_QUERYDATA_NAMEDSHAREDMEM;
      rc = (pfnDosQuerySysState)( ulFlags, 0, pid, 0, pvData, ulBufsize);
      }
   else if (pfnDosQProcStatus)
       rc = (pfnDosQProcStatus)( pvData, ulBufsize);
   else
       rc = ERROR_INVALID_NAME;

   } while (FALSE);

return rc;
}


// ----------------------------------------------------------------------------

static VOID _getModuleName( HMODULE hmod,
                            PSZ pszProcessName, ULONG ulProcessNameMaxlen,
                            PSZ pszFullname, ULONG ulFullNameMaxlen,
                            PID pidParent, ULONG ulSessionType)

{
         APIRET         rc = NO_ERROR;
         CHAR           szExeName[ _MAX_PATH];
do
   {
   // check parms
   if ((!pszProcessName)      ||
       (!pszFullname))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }
   if ((!ulProcessNameMaxlen)      ||
       (!ulFullNameMaxlen))
      {
      rc = ERROR_BUFFER_OVERFLOW;
      break;
      }
   *pszProcessName = 0;
   *pszFullname     = 0;

   // handle processes without a module handle
   if (!hmod)
      {
      if (pidParent > 2)
         strcpy( szExeName, (ulSessionType == WTK_PROCTYPE_VDM) ? "DOS VDM" : "-unknown-");
      else
         strcpy( szExeName, "(kernel)");

      // hand over exename
      if (strlen( szExeName) + 1 > ulProcessNameMaxlen)
         {
         rc = ERROR_BUFFER_OVERFLOW;
         break;
         }
      strcpy( pszProcessName, szExeName);

      break;
      }

   // get module name
   rc = DosQueryModuleName( hmod,
                            sizeof( szExeName),
                            szExeName);
   if (rc != NO_ERROR)
      {
      strcpy( szExeName, "-zombie-");
      break;
      }

   // hand over executable name
   if (strlen( szExeName) + 1 > ulFullNameMaxlen)
      {
      rc = ERROR_BUFFER_OVERFLOW;
      break;
      }
   strcpy( pszFullname, szExeName);

   // eliminate path and extension and hand over process name
   strcpy( szExeName, strrchr( szExeName, '\\') + 1);
   strcpy( strrchr( szExeName, '.'), "");
   strlwr( szExeName);
   if (strlen( szExeName) + 1 > ulProcessNameMaxlen)
      {
      rc = ERROR_BUFFER_OVERFLOW;
      break;
      }
   strcpy( pszProcessName, szExeName);

   } while (FALSE);

return;

}

// ----------------------------------------------------------------------------

static APIRET _searchProcess( PPROCSTATBUF ppb, PID pid, PPROCESS *ppp)
{
         APIRET         rc = ERROR_FILE_NOT_FOUND;
         PPROCESS       pp;

do
   {
   for ( pp = ppb->pps->pprocess;
         pp->ulType != 3;             // not sure if there isn't another termination method
         pp = PTRNEXT( pp->pathread,  // next record behind all thread records
                        pp->usThreadCount * sizeof(THREAD))
       )
      {
      // compare pid
      if (pp->usProcessId == pid)
         {
         *ppp = pp;
         rc = NO_ERROR;
         break;
         }

      } // end for


   } while (FALSE);

return rc;
}

// ----------------------------------------------------------------------------

static APIRET _getProcessInfo( PID pid, PPROCESS pp)
{
         APIRET         rc = ERROR_FILE_NOT_FOUND;
         ULONG          ulBufsize;

         PPROCSTAT      pps;

do
   {
   if ((!pid) ||
       (!pp))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // get process info for specific PID
   // the system seems to use an array for 6 threads at least, we use 10
   ulBufsize = sizeof( PROCSTAT) + sizeof( COUNTTABLE) +
               (sizeof( PROCESS) + (10 *  sizeof( THREAD)));
   pps = malloc( ulBufsize);
   if (!pps)
      {
      rc = ERROR_NOT_ENOUGH_MEMORY;
      break;
      }

   // get list with one entry
   rc = _queryProcessStatus( pps, ulBufsize, pid);
   if (rc != NO_ERROR)
       break;

   // hand over process data
   memcpy( pp, pps->pprocess, sizeof( PROCESS));

   // overwrite pointers as they are invalid
   pp->pathread = NULL;
   pp->pasem    = NULL;
   pp->pamodule = NULL;
   pp->pashmem   = NULL;

   } while (FALSE);

// cleanup
if (pps) free( pps);
return rc;
}

// ----------------------------------------------------------------------------

static APIRET _searchProcessByName( PPROCSTATBUF ppb, PSZ pszExename, ULONG ulIndex, PPROCESS *ppp)
{
         APIRET         rc = ERROR_FILE_NOT_FOUND;
         PSZ            p;
         PPROCESS       pp;
         CHAR           szExecutableName[ _MAX_PATH];
         ULONG          ulEntryFound = 0;

do
   {

   if ((!pszExename) ||
       (!*pszExename))
      break;

   for ( pp = ppb->pps->pprocess;
         pp->ulType != 3;               // not sure if there isn't another termination method
         pp = PTRNEXT( pp->pathread,  // next record behind all thread records
                        pp->usThreadCount * sizeof(THREAD))
       )
      {
      // check module name
      DosQueryModuleName( pp->usModuleHandle,
                          sizeof( szExecutableName),
                          szExecutableName);
      if (_strrcmp( szExecutableName, pszExename) == 0)
         {
         // in case no path has been specified:
         // check that we did not find just the last part of a name
         if (strchr( pszExename, '\\') == 0)
            {
            p = &szExecutableName[ strlen( szExecutableName) - strlen( pszExename) - 1];
            if (*p != '\\')
               continue;
            }

         // check index
         if (ulIndex > ulEntryFound)
            ulEntryFound++;
         else
            {
            // we are done
            *ppp = pp;
            rc =  NO_ERROR;
            break;
            }
         }

      } // end for


   } while (FALSE);

return rc;
}

// ----------------------------------------------------------------------------

/*
@@WtkInitializeProcessStatus@SYNTAX
This function initializes process status handling.

@@WtkInitializeProcessStatus@PARM@php@out
The address of a variable containing the handle to the process status data.

@@WtkInitializeProcessStatus@PARM@ulReserveMem@in
Size of user memory to be attached to each entry in the process status list.
:p.
A pointer to this memory is returned by
:link reftype=hd viewport refid=WtkQueryProcessDetails.WtkQueryProcessDetails:elink.
with the parameter :hp1.ppbReservedMem:ehp1..
This memory is allocated and reset by each call to
:link reftype=hd viewport refid=WtkRefreshProcessStatus.WtkRefreshProcessStatus:elink.
and freed by
:link reftype=hd viewport refid=WtkTerminateProcessStatus.WtkTerminateProcessStatus:elink..
:p.
If zero is specified for this parameter, not private memory is being allocated.

@@WtkInitializeProcessStatus@RETURN
Return Code.
:p.
WtkInitializeProcessStatus returns one of the following return codes&colon.

:parml compact break=none.
:pt.0
:pd.NO_ERROR
:pt.8
:pd.ERROR_NOT_ENOUGH_MEMORY
:pt.87
:pd.ERROR_INVALID_PARAMETER
:eparml.

@@WtkInitializeProcessStatus@REMARKS
When calling this function,
:link reftype=hd viewport refid=WtkRefreshProcessStatus.WtkRefreshProcessStatus:elink.
is called implicitely after initialization, so that all process status functions can be
used right away.
@@
*/


APIRET APIENTRY WtkInitializeProcessStatus( PHPROCSTAT php, ULONG ulReserveMem)
{

         APIRET         rc = NO_ERROR;
         PPROCSTATBUF   ppb = NULL;
do
   {
   // check parms
   if (!php)
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // get momory
   ppb = malloc( sizeof( PROCSTATBUF));
   if (!ppb)
      {
      rc = ERROR_NOT_ENOUGH_MEMORY;
      break;
      }
   memset( ppb, 0,  sizeof( PROCSTATBUF));
   ppb->ulReserveMem = ulReserveMem;
   ppb->ulBufsize    = MEMORY_BLOCKSIZE;

   // hand over result
   *php = (HPROCSTAT) ppb;

   // get the status for the first time
   rc = WtkRefreshProcessStatus( *php, WTK_PROCSTATUS_REFRESH_ALWAYS, NULL);

   } while (FALSE);

return rc;
}

// ----------------------------------------------------------------------------

/*
@@WtkTerminateProcessStatus@SYNTAX
This function terminates process status handling.

@@WtkTerminateProcessStatus@PARM@hp@in
Handle to the process status data.

@@WtkTerminateProcessStatus@RETURN
Return Code.
:p.
WtkTerminateProcessStatus returns one of the following return codes&colon.

:parml compact break=none.
:pt.0
:pd.NO_ERROR
:pt.87
:pd.ERROR_INVALID_PARAMETER
:eparml.

@@WtkTerminateProcessStatus@REMARKS
- none -
@@
*/

APIRET APIENTRY WtkTerminateProcessStatus( HPROCSTAT hp)
{

         APIRET         rc = NO_ERROR;
         PPROCSTATBUF   ppb = (PPROCSTATBUF) hp;
do
   {
   // check parms
   if (!hp)
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // free used memory
   if (ppb->pps)
      free( ppb->pps);
   memset( ppb, 0,  sizeof( PROCSTATBUF));
   free( ppb);

   } while (FALSE);

return rc;
}

// ----------------------------------------------------------------------------

/*
@@WtkRefreshProcessStatus@SYNTAX
This function refreshes previously rerieved process status data, if necessary.

@@WtkRefreshProcessStatus@PARM@hp@in
Handle to the process status data.

@@WtkRefreshProcessStatus@PARM@ulOptions@in
Flags to control the refresh behaviour.
:p.
Specify one of the following flags for to refresh the process data
:parml.
:pt.WTK_PROCSTATUS_REFRESH_ALWAYS
:pd.always refresh the process data
:pt.WTK_PROCSTATUS_REFRESH_LISTCHANGED
:pd.refresh the process data only if processes
have been removed or added. Any other changes within the
process status data are ignored.
:eparml.

@@WtkRefreshProcessStatus@PARM@pfUpdated@out
Pointer to a variable receiving a flag indicating wether the
process status data has been updated.
:p.
Specify NULL if the update flag is not required.

@@WtkRefreshProcessStatus@RETURN
Return Code.
:p.
WtkRefreshProcessStatus returns one of the following return codes&colon.

:parml compact break=none.
:pt.0
:pd.NO_ERROR
:pt.8
:pd.ERROR_NOT_ENOUGH_MEMORY
:pt.87
:pd.ERROR_INVALID_PARAMETER
:eparml.

@@WtkRefreshProcessStatus@REMARKS
A call to this function resets all user data attached to processes.
@@
*/

APIRET APIENTRY WtkRefreshProcessStatus( HPROCSTAT hp, ULONG ulOptions, PBOOL pfUpdated)
{
         APIRET         rc  = NO_ERROR;
         PPROCSTATBUF   ppb = (PPROCSTATBUF) hp;
         ULONG          i,j;

         PPROCSTAT      pps;
         PPROCESS       pp;
         PPROCESS       ppParent;

         ULONG          ulCrc;
         ULONG          ulProcessCount;
         BOOL           fUpdated = FALSE;
         ULONG          ulReservedMemLen;
         ULONG          ulLevel;

do
   {
   // check parms
   if (!hp)
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   do
      {
      // try to get the process ulStatus list
      pps = malloc( ppb->ulBufsize);
      if (pps  == NULL)
         {
         rc = ERROR_NOT_ENOUGH_MEMORY;
         break;
         }

      // now get ulStatus list
      rc = _queryProcessStatus( pps, ppb->ulBufsize, 0);

      // increase memory in case of error
      if (rc == ERROR_BUFFER_OVERFLOW)
         {
         free( pps);
         ppb->ulBufsize += MEMORY_BLOCKSIZE;
         }

      } while (rc == ERROR_BUFFER_OVERFLOW);

   if (rc != NO_ERROR)
     break;

   // calc crc
   ulCrc = 0;
   ulProcessCount = 0;
   for ( pp = pps->pprocess;
         pp->ulType != 3;               // not sure if there isn't another termination method
         pp = PTRNEXT( pp->pathread,  // next record behind all thread records
                        pp->usThreadCount * sizeof(THREAD))
       )
      {
      // store index in reserved field
      pp->reserved1 = ulProcessCount;

      // store ptr to parent in reserved2 field below, initialize here
      pp->reserved2 = 0;

      // count statistics
      ulCrc += pp->usProcessId;
      ulProcessCount++;

      }

   // quit if nothing has changed
   if ((ulOptions & WTK_PROCSTATUS_REFRESH_LISTCHANGED) &&
       (ulCrc ==  ppb->ulCrc))
      {
      free( pps);
      break;
      }

   // buffer already allocated ? free memory first
   if (ppb->pps)
      free( ppb->pps);
   if (ppb->pvPrivateMem)
      free( ppb->pvPrivateMem);

   // store values now
   ppb->pps            = pps;
   ppb->ulCrc          = ulCrc;
   ppb->ulProcessCount = ulProcessCount;

   if (ppb->ulReserveMem)
      {
      ulReservedMemLen      = ppb->ulReserveMem * ulProcessCount;
      ppb->pvPrivateMem   = malloc( ulReservedMemLen);
      memset( ppb->pvPrivateMem, 0, ulReservedMemLen);
      }

   fUpdated = TRUE;

   // code below this point may not lead to errors for this API

   // set pointer for parent processes
   ppb->ulListLevelCount = 0;
   for ( pp = pps->pprocess, i = 0;
         i < ppb->ulProcessCount;
         pp = PTRNEXT( pp->pathread,
                        pp->usThreadCount * sizeof(THREAD)), i++
       )
      {
      // don't search parent processes for kernel etc
      if (pp->usProcessId < 3)
         continue;

      // search parents
      for ( ppParent = pps->pprocess, j = 0;
            j < ppb->ulProcessCount;
            ppParent = PTRNEXT( ppParent->pathread,
                                ppParent->usThreadCount * sizeof(THREAD)), j++
          )
         {
         // store parent if found - use reserved2 field for this
         if (pp->usParentId == ppParent->usProcessId)
            {
            pp->reserved2 = (ULONG)ppParent;
            break;
            }
         }
      }

   // determine maximum tree depth of list
   for ( pp = pps->pprocess, i = 0;
         i < ppb->ulProcessCount;
         pp = PTRNEXT( pp->pathread,
                        pp->usThreadCount * sizeof(THREAD)), i++
       )
      {
      // init for this process
      ulLevel = 0;

      // don't search parent processes for kernel etc
      if (pp->usProcessId < 3)
         continue;

      // determine parent level for this process
      ppParent = (PPROCESS) pp->reserved2;
      while (ppParent)
         {
         ppParent = (PPROCESS) ppParent->reserved2;
         ulLevel++;
         }

      // save maximum
      ppb->ulListLevelCount = MAX( ppb->ulListLevelCount, ulLevel);

      }

   } while (FALSE);

// tell update ulStatus anyway
if (pfUpdated)
   *pfUpdated = fUpdated;
return rc;
}

// ----------------------------------------------------------------------------

/*
@@WtkQueryProcessStatusMaxLevel@SYNTAX
This function determines the maximum depth of the process list.

@@WtkQueryProcessStatusMaxLevel@PARM@hp@in
Handle to the process status data.

@@WtkQueryProcessStatusMaxLevel@PARM@pulMaxLevel@out
Pointer to a variable receiving the number of levels in
the process status data.

@@WtkQueryProcessStatusMaxLevel@RETURN
Return Code.
:p.
WtkQueryProcessStatusMaxLevel returns one of the following return codes&colon.

:parml compact break=none.
:pt.0
:pd.NO_ERROR
:pt.87
:pd.ERROR_INVALID_PARAMETER
:eparml.

@@WtkQueryProcessStatusMaxLevel@REMARKS
This function is intended to help with creating graphical views of the
process status list.
@@
*/

APIRET APIENTRY WtkQueryProcessStatusMaxLevel( HPROCSTAT hp, PULONG pulMaxLevel)
{
         APIRET         rc  = NO_ERROR;
         PPROCSTATBUF   ppb = (PPROCSTATBUF) hp;

do
   {
   // check parms
   if ((!hp)        ||
       (!pulMaxLevel))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // hand over count
   *pulMaxLevel = ppb->ulListLevelCount;

   } while (FALSE);

return rc;
}

// ----------------------------------------------------------------------------

/*
@@WtkEnumProcessChilds@SYNTAX
This function enumerates the process ids of all childs for a
given process id or for all processes in the system.

@@WtkEnumProcessChilds@PARM@hp@in
Handle to the process status data.

@@WtkEnumProcessChilds@PARM@pidParent@in
Identifier of the parent process, for which the client processes
are to be enumerated.
:p.
Specify :hp2.WTK_PROCESS_ENUMALL:ehp2. as parent id to enumerate all
processes in the system.

@@WtkEnumProcessChilds@PARM@papid@out
Pointer to a buffer for to hold process identifiers for the child processes.
:p.
Specify :hp2.NULL:ehp2. to only query the number of childs for
the given parent process.

@@WtkEnumProcessChilds@PARM@pulMaxPid@inout
Pointer to a variable holding the number of process ids to be queried,
and receiving the number of child processes
of the given parent process.

@@WtkEnumProcessChilds@RETURN
Return Code.
:p.
WtkEnumProcessChilds returns one of the following return codes&colon.

:parml compact break=none.
:pt.0
:pd.NO_ERROR
:pt.2
:pd.ERROR_FILE_NOT_FOUND
:pt.87
:pd.ERROR_INVALID_PARAMETER
:pt.111
:pd.ERROR_BUFFER_OVERFLOW
:eparml.

@@WtkEnumProcessChilds@REMARKS
- none -
@@
*/

APIRET APIENTRY WtkEnumProcessChilds( HPROCSTAT hp, PID pidParent, PPID papid, PULONG pulMaxPid)
{
         APIRET         rc  = NO_ERROR;
         PPROCSTATBUF   ppb = (PPROCSTATBUF) hp;

         PPROCESS       pp;
         PPID           ppid;
         ULONG          ulCount = 0;

do
   {
   // check parms
   if ((!hp)        ||
       (!pulMaxPid))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   if (papid)
      {
      if (!*pulMaxPid)
         {
         rc = ERROR_INVALID_PARAMETER;
         break;
         }

      // init vars
      memset( papid, 0, sizeof( PID) * *pulMaxPid);
      }

   // loop all process data
   rc = NO_ERROR;
   for ( pp = ppb->pps->pprocess, ppid = papid;
         pp->ulType != 3;               // not sure if there isn't another termination method
         pp = PTRNEXT( pp->pathread,  // next record behind all thread records
                        pp->usThreadCount * sizeof(THREAD))
       )
      {
      // compare pid
      if ((pidParent == WTK_PROCESS_ENUMALL) || (pp->usParentId == pidParent))
         {
         if (ppid)
            {
            // make sure not to exceed the buffer
            if (ulCount == *pulMaxPid)
               {
               rc = ERROR_BUFFER_OVERFLOW;
               break;
               }

            // store pid
            *ppid = pp->usProcessId;
            ppid++;
            }

         // count anyway
         ulCount++;
         }

      } // end for

   // hand over count
   *pulMaxPid = ulCount;

   // break on error
   if (rc != NO_ERROR)
      break;

   // nothing found ?
   if (!ulCount)
      {
      rc = ERROR_FILE_NOT_FOUND;
      break;
      }

   } while (FALSE);

return rc;
}

// ----------------------------------------------------------------------------

/*
@@WtkEnumProcessThreads@SYNTAX
This function enumerates details of all threads for a given process.

@@WtkEnumProcessThreads@PARM@hp@in
Handle to the process status data.

@@WtkEnumProcessThreads@PARM@pid@in
Identifier of the process, for which the thread details
are to be enumerated.

@@WtkEnumProcessThreads@PARM@pati@out
Pointer to a buffer for to receive details data for the threads of
the specified process.
:p.
Specify :hp2.NULL:ehp2. to only query the number of threads of
the given process.

@@WtkEnumProcessThreads@PARM@pulMaxThreads@inout
Pointer to a variable receiving the number of threads
of the given process.

@@WtkEnumProcessThreads@RETURN
Return Code.
:p.
WtkEnumProcessThreads returns one of the following return codes&colon.

:parml compact break=none.
:pt.0
:pd.NO_ERROR
:pt.87
:pd.ERROR_INVALID_PARAMETER
:pt.111
:pd.ERROR_BUFFER_OVERFLOW
:eparml.

@@WtkEnumProcessThreads@REMARKS
- none -
@@
*/

APIRET APIENTRY WtkEnumProcessThreads( HPROCSTAT hp, PID pid, PTHREADINFO pati, PULONG pulMaxThreads)
{
         APIRET         rc  = NO_ERROR;
         PPROCSTATBUF   ppb = (PPROCSTATBUF) hp;

         ULONG          i;
         PPROCESS       pp;
         PTHREAD        pt;

         PTHREADINFO    pti;

do
   {
   // check parms
   if ((!hp)        ||
       (!pulMaxThreads))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }
   if (pati)
      memset( pati, 0, sizeof( THREADINFO) * (*pulMaxThreads));

   // search process
   rc = _searchProcess( ppb, pid, &pp);
   if (rc != NO_ERROR)
     break;

   // only count requested ?
   if (!pati)
      {
      *pulMaxThreads = pp->usThreadCount;
      break;
      }

   // does buffer fit ?
   if (pp->usThreadCount > *pulMaxThreads)
      {
      rc = ERROR_BUFFER_OVERFLOW;
      break;
      }
   *pulMaxThreads = pp->usThreadCount;

   // fill buffer
   for (i = 0, pti = pati, pt = PTRNEXT( pp->pathread, 0);
           i < pp->usThreadCount;
           i++, pti++, pt = PTRNEXT( pt, sizeof(THREAD)))
      {
      pti->ulThreadId  = (ULONG) pt->usThreadId;
      pti->ulPriority  =         pt->ulPriority;
      pti->ulStatus    = (ULONG) pt->ulStatus;
      }

   } while (FALSE);

return rc;
}


// ---------------------------------------------------------------------

/*
@@WtkQueryProcessDetails@SYNTAX
This function retrieves details data for a given process, specified by a process identifier (PID).
It can be used with or without taking a process list snapshot before.

@@WtkQueryProcessDetails@PARM@hp@in
Handle to the process status data or zero.
:p.
If zero is specified, the process data is directly queried directly
from the process list, instead from a previously taken process list snapshot.

@@WtkQueryProcessDetails@PARM@pid@in
Identifier of the process, for which the details data
is to be retrieved.

@@WtkQueryProcessDetails@PARM@ppi@out
Pointer to a buffer for to receive details data for the
specified process.
:p.
Specify :hp2.NULL:ehp2. to only query the pointer to user memory.

@@WtkQueryProcessDetails@PARM@ppbReservedMem@out
Pointer to a variable holding a pointer to application data for
the specified process.
:p.
This parameter is ignored if zero is specified as hp.
:p.
The private memory for all processes is allocated and reset by each call to
:link reftype=hd viewport refid=WtkRefreshProcessStatus.WtkRefreshProcessStatus:elink.
and freed by
:link reftype=hd viewport refid=WtkTerminateProcessStatus.WtkTerminateProcessStatus:elink..
In order to use this user memory, the size of user memory per process must be specified
on the call to :hp2.WtkInitializeProcessStatus:ehp2. with the parameter
:hp1.ulReserveMem:ehp1..
:p.
This is useful for storing a pointer to application data,
which can be accessed by a subsequent call to :hp2.WtkQueryProcessDetails:ehp2..
:p.
NULL may be specified for this parameter, if the pointer to private memory is not
required. If no private memory has been requested when calling
:link reftype=hd viewport refid=WtkInitializeProcessStatus.WtkInitializeProcessStatus:elink.,
NULL is returned.

@@WtkQueryProcessDetails@RETURN
Return Code.
:p.
WtkQueryProcessDetails returns one of the following return codes&colon.

:parml compact break=none.
:pt.0
:pd.NO_ERROR
:pt.87
:pd.ERROR_INVALID_PARAMETER
:eparml.

@@WtkQueryProcessDetails@REMARKS
If the PID of a process is not known, but the name of the program executable,
all running instances of that program can be queried with calls to
:link reftype=hd viewport refid=WtkQueryProcess.WtkQueryProcess:elink..
@@
*/

APIRET APIENTRY WtkQueryProcessDetails( HPROCSTAT hp, PID pid, PPROCESSINFO ppi,
                                        PPVOID ppbReservedMem)
{
         APIRET         rc  = NO_ERROR;
         PSZ            p;
         PPROCSTATBUF   ppb = (PPROCSTATBUF) hp;

         PPROCESS       pp;
         PROCESS        process;

do
   {
   // check parms
   if (!pid)
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   if (hp)
   // loop all process data
      rc = _searchProcess( ppb, pid, &pp);
   else
      // get process info directly from process tree
      {
      rc = _getProcessInfo( pid, &process);
      pp = &process;
      }
   if (rc != NO_ERROR)
      break;

   if (ppi)
      {
      memset( ppi, 0, sizeof( PROCESSINFO));

      // determine name
      _getModuleName( pp->usModuleHandle,
                      ppi->szProcname,
                      sizeof( ppi->szProcname),
                      ppi->szFullname,
                      sizeof( ppi->szFullname),
                      pp->usParentId,
                      pp->ulSessionType);

      ppi->pid            = pp->usProcessId;
      ppi->pidParent      = pp->usParentId;
      ppi->hmod           = pp->usModuleHandle;
      ppi->ulSessionId    = pp->ulSessionId;
      ppi->ulSessionType  = pp->ulSessionType;
      ppi->ulStatus       = pp->ulStatus;
      ppi->ulThreadCount  = pp->usThreadCount;

      // store pointer to executable name
      ppi->pszExename = ppi->szFullname;
      if (*(ppi->pszExename))
         {
         p = strrchr( ppi->pszExename, '\\');
         if (p)
            ppi->pszExename = p + 1;
         }

      }

   // return also pointer to private mem stored in reserved value
   if ((hp) && (ppbReservedMem))
      {
      if (ppb->ulReserveMem)
         //                base pointer                + (sizeof mem per process      * process index)
         *ppbReservedMem = ((PBYTE) ppb->pvPrivateMem) + (ppb->ulReserveMem           * pp->reserved1);
      else
         *ppbReservedMem = NULL;
      }

   } while (FALSE);

return rc;
}

// ----------------------------------------------------------------------------

/*
@@WtkKillProcess@SYNTAX
This function kills a process or a process tree.

@@WtkKillProcess@PARM@hp@in
Handle to the process status data.

@@WtkKillProcess@PARM@pid@in
Identifier of the process, which is to be killed.

@@WtkKillProcess@PARM@fIncludeChildren@in
Flag for to kill also all child processes.

@@WtkKillProcess@RETURN
Return Code.
:p.
WtkKillProcess returns one of the following return codes&colon.

:parml compact break=none.
:pt.0
:pd.NO_ERROR
:pt.87
:pd.ERROR_INVALID_PARAMETER
:eparml.
:p.
or return codes of the following functions
:ul compact.
:li.DosKillProcess
:eul.

@@WtkKillProcess@REMARKS
- none -
@@
*/

APIRET APIENTRY WtkKillProcess( HPROCSTAT hp, PID pid, BOOL fIncludeChildren)
{

               APIRET         rc = NO_ERROR;
               ULONG          i;

               ULONG          ulPidCount;
               PPID           ppidList = NULL;
               PPID           ppid;

do
   {

   if (fIncludeChildren)
      {
      // get list of all child processes
      rc = WtkEnumProcessChilds( hp, pid, NULL, &ulPidCount);
      if ((rc == NO_ERROR) && (ulPidCount))
         {
         // get memory for complete list
         ppidList  = malloc( sizeof( PID) * ulPidCount);
         if (!ppidList)
            break;
         memset( ppidList, 0, sizeof( PID) * ulPidCount);

         // get all process ids
         WtkEnumProcessChilds( hp, pid, ppidList, &ulPidCount);

         // go through all items and check the point clicked to
         for (i = 0,  ppid = ppidList; i < ulPidCount; i++, ppid++)
            {
            // kill childs first
            WtkKillProcess( hp, *ppid, TRUE);
            }
         }

      // don't use rc of last enum call
      rc = NO_ERROR;

      } // if (fIncludeChildren)

   // now kill this process
   rc = DosKillProcess(  DKP_PROCESS, pid);

   } while (FALSE);

// cleanup
if (ppidList)
   free( ppidList);
return rc;
}

// ----------------------------------------------------------------------------

/*
@@WtkSelectProcess@SYNTAX
This function switches to a specified process. The
task list of the Presentation Manager is used to find the
session identifier.
:p.
Calling this function requires the existance of a message queue.

@@WtkSelectProcess@PARM@hwnd@in
Window handle of the application that is to be selected.
:p.
Specify :hp2.NULLHANDLE:ehp2. if no specific window of an application is
to be selected. If a window handle is specified, it must be related to the
process specified by the parameter :hp1.pid:ehp1..

@@WtkSelectProcess@PARM@pid@in
Identifier of the process, which is to be selected.

@@WtkSelectProcess@RETURN
Return Code.
:p.
WtkSelectProcess returns one of the following return codes&colon.

:parml compact break=none.
:pt.0
:pd.NO_ERROR
:pt.2
:pd.ERROR_FILE_NOT_FOUND
:pt.8
:pd.ERROR_NOT_ENOUGH_MEMORY
:pt.87
:pd.ERROR_INVALID_PARAMETER
:eparml.
:p.
or return codes of the following functions
:ul compact.
:li.DosKillProcess
:eul.

@@WtkSelectProcess@REMARKS
- none -
@@
*/

APIRET APIENTRY WtkSelectProcess( HWND hwnd, PID pid)
{
return WinSwitchToProgram( WinQuerySwitchHandle( NULLHANDLE, pid));
}

// ----------------------------------------------------------------------------

/*
@@WtkQueryProcess@SYNTAX
This function retrieves details data for a running instance of a given process,
searched by the name of the program executable.

@@WtkQueryProcess@PARM@hp@in
Handle to the process status data.

@@WtkQueryProcess@PARM@pszExename@in
Pointer to the name of the program executable
:p.
Speficy either only the name (like "MYPROG.EXE") or the fully qualified pathname
of the executable (like "D&colon.\DIR\MYPROG.EXE").

@@WtkQueryProcess@PARM@ulIndex@in
Index number for the instance of the queried process.
:p.
Specifies the number of the running instance of the specified executable, where
0 represents the first running instance, 1 the second, and so on.

@@WtkQueryProcess@PARM@ppi@out
Pointer to a buffer for to receive details data for the
specified process.

@@WtkQueryProcess@RETURN
Return Code.
:p.
WtkQueryProcessDetails returns one of the following return codes&colon.

:parml compact break=none.
:pt.0
:pd.NO_ERROR
:pt.87
:pd.ERROR_INVALID_PARAMETER
:eparml.

@@WtkQueryProcess@REMARKS
If only the PID of a process is known, the process details can be queried with a call to
:link reftype=hd viewport refid=WtkQueryProcessDetails.WtkQueryProcessDetails:elink..
@@
*/

APIRET APIENTRY WtkQueryProcess( HPROCSTAT hp, PSZ pszExeName, ULONG ulIndex, PPROCESSINFO ppi)
{
         APIRET         rc  = NO_ERROR;
         PSZ            p;
         PPROCSTATBUF   ppb = (PPROCSTATBUF) hp;
         PPROCESS       pp;

do
   {
   // check parms
   if ((!hp)          ||
       (!pszExeName)  ||
       (!*pszExeName) ||
       (!ppi))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // loop all process data
   rc = _searchProcessByName( ppb, pszExeName, ulIndex, &pp);
   if (rc != NO_ERROR)
      break;

   if (ppi)
      {
      memset( ppi, 0, sizeof( PROCESSINFO));

      // determine name
      _getModuleName( pp->usModuleHandle,
                      ppi->szProcname,
                      sizeof( ppi->szProcname),
                      ppi->szFullname,
                      sizeof( ppi->szFullname),
                      pp->usParentId,
                      pp->ulSessionType);

      ppi->pid            = pp->usProcessId;
      ppi->pidParent      = pp->usParentId;
      ppi->hmod           = pp->usModuleHandle;
      ppi->ulSessionId    = pp->ulSessionId;
      ppi->ulSessionType  = pp->ulSessionType;
      ppi->ulStatus       = pp->ulStatus;
      ppi->ulThreadCount  = pp->usThreadCount;

      // store pointer to executable name
      ppi->pszExename = ppi->szFullname;
      if (*(ppi->pszExename))
         {
         p = strrchr( ppi->pszExename, '\\');
         if (p)
            ppi->pszExename = p + 1;
         }

      }

   } while (FALSE);

return rc;
}

// ----------------------------------------------------------------------------

/*
@@WtkWaitProcess@SYNTAX
This function retrieves details data for a running instance of a given process,
searched by the name of the program executable.

@@WtkWaitProcess@PARM@hp@in
Handle to the process status data or NULLHANDLE.
:p.
If NULLHANDLE is specified, the process data is obtained internally by WtkWaitProcess.

@@WtkWaitProcess@PARM@pszExename@in
Pointer to the name of the program executable
:p.
Speficy either only the name (like "MYPROG.EXE") or the fully qualified pathname
of the executable (like "D&colon.\DIR\MYPROG.EXE").

@@WtkWaitProcess@PARM@ulWait@in
Period of time in milliseconds that WtkWaitProcess should wait before querying the
process status data, or zero.
:p.
This value must be zero to not wait for the process, or greater or equal to
the value of ulMaxWait.

@@WtkWaitProcess@PARM@pulMaxWait@in
Pointer to a variable containing the period of time in milliseconds that
WtkWaitProcess should wait as a maximum, if the process is not in memory.
:p.
This value must be smaller or equal to the value of ulWait. The value gets
decreased with every wait, and is set to zero, if the process did not appear.
NULL may be secified, if ulWait is zero.

@@WtkWaitProcess@RETURN
Return Code.
:p.
WtkWaitProcessDetails returns one of the following return codes&colon.

:parml compact break=none.
:pt.0
:pd.NO_ERROR
:pt.2
:pd.ERROR_FILE_NOT_FOUND
:pt.87
:pd.ERROR_INVALID_PARAMETER
:eparml.

@@WtkWaitProcess@REMARKS
WtkWaitProcess can be used as well to query if a process is in memory,
by not waiting for it, setting ulWait to zero and pulMaxWait to NULL.
@@
*/

APIRET APIENTRY WtkWaitProcess( HPROCSTAT hp, PSZ pszExeName, ULONG ulWait, PULONG pulMaxWait)
{
         APIRET         rc  = NO_ERROR;
         PSZ            p;
         BOOL           fDataAllocated = FALSE;
         PPROCSTATBUF   ppb = (PPROCSTATBUF) hp;
         PROCESSINFO    pi;
         ULONG          ulZeroWait = 0;

do
   {
   // check parms
   if ((!pszExeName)  ||
       (!*pszExeName) ||
       ((pulMaxWait) &&
        (*pulMaxWait < ulWait)))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }
   // play safe with the pulMaxWait ptr
   if (!pulMaxWait)
      pulMaxWait = &ulZeroWait;


   // allocate process data on our own, if not provided
   if (!hp)
      {
      rc = WtkInitializeProcessStatus( &hp, 0);
      if (rc != NO_ERROR)
         break;
      ppb = (PPROCSTATBUF) hp;
      fDataAllocated = TRUE;
      }

   do
      {
      // check for the executable in memory
      rc = WtkQueryProcess( hp, pszExeName, 0, &pi);
      if (rc == NO_ERROR)
         break;

      // exit if no wait requested or max time elapsed
      if ((!ulWait) ||
          (*pulMaxWait < ulWait))
         {
         *(pulMaxWait) = 0;
         rc = ERROR_FILE_NOT_FOUND;
         break;
         }

      // spend some time
      DosSleep( ulWait);
      *(pulMaxWait) -= ulWait;

      // get new status
      WtkRefreshProcessStatus( hp, WTK_PROCSTATUS_REFRESH_LISTCHANGED, NULL);

      } while (TRUE);

   } while (FALSE);

// cleanup
if (fDataAllocated) WtkTerminateProcessStatus( hp);
return rc;
}

