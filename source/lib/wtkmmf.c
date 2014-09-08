/****************************** Module Header ******************************\
*
* Module Name: wtkmmf.c
*
* Source for memory mapped file manager functions.
*
* This code bases on the MMF library by Sergey I. Yevtushenko
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2002
*
* $Id: wtkmmf.c,v 1.35 2008-10-24 00:54:30 cla Exp $
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
#include <time.h>

#include <sys/fmutex.h>

#define INCL_DOS
#define INCL_ERRORS
#include <os2.h>

#include "wtkufil.h"
#include "wtkutim.h"
#include "wtkmmf.h"
#include "wpstk.ih"

#ifdef DEBUG
#include <stdarg.h>
inline static void debug_printf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
}
#define DPRINTF(p) debug_printf p
#else
#define DPRINTF(p) do {} while(0)
#endif

// --- defines for detailed debug messages
#ifdef DEBUG
#define DEBUG_DUMPHANDLERACTIONS 0
#define DEBUG_DUMPALLOCACTIONS   0
#define DEBUG_DUMPLOCATEENTRY    0
#endif

#if DEBUG_DUMPHANDLERACTIONS
#define DPRINTF_HANDLERACTION(p)  debug_printf p
#else
#define DPRINTF_HANDLERACTION(p)  do {} while(0)
#endif

#if DEBUG_DUMPALLOCACTIONS
#define DPRINTF_ALLOCACTION(p)  debug_printf p
#else
#define DPRINTF_ALLOCACTION(p)  do {} while(0)
#endif

#if DEBUG_DUMPLOCATEENTRY
#define DPRINTF_DUMPLOCATEENTRY(p)  debug_printf p
#else
#define DPRINTF_DUMPLOCATEENTRY(p)  do {} while(0)
#endif

#define MAX(a,b)        (a > b ? a : b)
#define MIN(a,b)        (a < b ? a : b)

// internal MMF defines
#define MMF_MAXINSTANCES     128

// define masks for bit groups
#define MMF_MASK_ACCESS    0x0000000F
#define MMF_MASK_UPDATE    0x000000F0
#define MMF_MASK_MEMORY    0x00000F00
#define MMF_MASK_OPENMODE  0x000F0000
#define MMF_MASK_ALLOC     0x0F000000

// macros to examine entry flags
#define MMFENTRY_USED(p)               (p->pvData != NULL)

#define MMFENTRY_ACCESS(p)               (p->ulFlags & MMF_MASK_ACCESS)
#define MMFENTRY_ACCESS_READONLY(p)      (MMFENTRY_ACCESS( p) == MMF_ACCESS_READONLY)
#define MMFENTRY_ACCESS_WRITEONLY(p)     (MMFENTRY_ACCESS( p) == MMF_ACCESS_WRITEONLY)
#define MMFENTRY_ACCESS_READWRITE(p)     (MMFENTRY_ACCESS( p) == MMF_ACCESS_READWRITE)

#define MMFENTRY_UPDATE(p)               (p->ulFlags & MMF_MASK_UPDATE)
#define MMFENTRY_UPDATE_KEEPMEM(p)       (MMFENTRY_UPDATE( p) == MMF_UPDATE_KEEPMEM)
#define MMFENTRY_UPDATE_RELEASEMEM(p)    (MMFENTRY_UPDATE( p) == MMF_UPDATE_RELEASEMEM)

#define MMFENTRY_MEMORY(p)               (p->ulFlags & MMF_MASK_MEMORY)
#define MMFENTRY_MEMORY_READWRITE(p)     (MMFENTRY_MEMORY( p) == MMF_MEMORY_READWRITE)
#define MMFENTRY_MEMORY_READONLY(p)      (MMFENTRY_MEMORY( p) == MMF_MEMORY_READONLY)
#define MMFENTRY_MEMORY_EXECUTE(p)       (MMFENTRY_MEMORY( p) == MMF_MEMORY_EXECUTE)

#define MMFENTRY_ALLOC(p)                (p->ulFlags & MMF_MASK_ALLOC)
#define MMFENTRY_ALLOC_LOWMEM(p)         (MMFENTRY_ALLOC( p) == MMF_ALLOC_LOWMEM)
#define MMFENTRY_ALLOC_HIGHMEM(p)        (MMFENTRY_ALLOC( p) == MMF_ALLOC_HIGHMEM)


#pragma pack(1)

typedef struct _MMFENTRY
  {
         ULONG          ulFlags;
         HFILE          hfile;
         BOOL           fFileOpened;
         PVOID          pvData;
         ULONG          ulMaxSize;
         ULONG          ulCurrentSize;
         BOOL           fModified;
         DATETIME       dtModified;
         CHAR           szFile[ CCHMAXPATH];
#ifdef DEBUG
         CHAR           szDesc[ CCHMAXPATH];
#endif
         ULONG          ulOffset;
  } MMFENTRY, *PMMFENTRY;

typedef struct _MMF
   {
         EXCEPTIONREGISTRATIONRECORD errh;
         ULONG          ulEntryCount;
         PMMFENTRY      apmmfentry;
         PID            pid;
         TID            tid;
   } MMF, *PMMF;


// global data
static   PMMF           apmmf[ MMF_MAXINSTANCES];
static   _fmutex        fmtx_apmmf = _FMUTEX_INITIALIZER;
static   BOOL           fInitialized = FALSE;

// -----------------------------------------------------------------------------

static BOOL _areOpenFlagsValid( ULONG ulOpenFlags)
{
         BOOL           fValid = TRUE;

switch (ulOpenFlags & MMF_MASK_ACCESS)
   {
   case MMF_ACCESS_READONLY:
   case MMF_ACCESS_WRITEONLY:
   case MMF_ACCESS_READWRITE:
      break;

   default:
      fValid = FALSE;
      break;

   } // switch (ulOpenFlags & MMF_MASK_ACCESS)

switch (ulOpenFlags & MMF_MASK_UPDATE)
   {
   case MMF_UPDATE_RELEASEMEM:
   case MMF_UPDATE_KEEPMEM:
      break;

   default:
      fValid = FALSE;
      break;

   } // switch (ulOpenFlags & MMF_MASK_UPDATE)

switch (ulOpenFlags & MMF_MASK_MEMORY)
   {
   case MMF_MEMORY_READWRITE:
   case MMF_MEMORY_READONLY:
   case MMF_MEMORY_EXECUTE:
      break;

   default:
      fValid = FALSE;
      break;

   } // switch (ulOpenFlags & MMF_MASK_MEMORY)



switch (ulOpenFlags & MMF_MASK_OPENMODE)
   {
   case MMF_OPENMODE_OPENFILE:
   case MMF_OPENMODE_RESETFILE:
      break;

   default:
      fValid = FALSE;
      break;

   } // switch (ulOpenFlags & MMF_MASK_OPENMODE)

switch (ulOpenFlags & MMF_MASK_ALLOC)
   {
   case MMF_ALLOC_LOWMEM:
   case MMF_ALLOC_HIGHMEM:
      break;

   default:
      fValid = FALSE;
      break;

   } // switch (ulOpenFlags & MMF_MASK_ALLOC)

return fValid;
}

// #############################################################################

// prototypes used here
static PMMF __locateMMFHandler( VOID);

static PMMFENTRY __locateMMFEntry( PMMF pmmf, PVOID addr)
{
         ULONG          i;
         PMMFENTRY      pmmfeResult = NULL;
         PMMFENTRY      pmmfe;

if (!pmmf)
   pmmf = __locateMMFHandler();

DPRINTF_DUMPLOCATEENTRY(( "MMF: LOCATE: search buffer for address 0x%08p\n", addr));

if (pmmf)
   for (i = 0, pmmfe = pmmf->apmmfentry;
        i < pmmf->ulEntryCount;
        i++, pmmfe++)
      {
      if (MMFENTRY_USED( pmmfe))
         {
         DPRINTF_DUMPLOCATEENTRY(( "MMF: LOCATE:   check buffer 0x%08p len 0x%08p end at 0x%08p \n",
                                   pmmfe->pvData,
                                   pmmfe->ulMaxSize,
                                   (ULONG)pmmfe->pvData + pmmfe->ulMaxSize - 1));
         if (((ULONG) pmmfe->pvData <= (ULONG) addr) &&
             (((ULONG) pmmfe->pvData + pmmfe->ulMaxSize) > (ULONG) addr))
            {
            pmmfeResult = pmmfe;
            break;
            }
         }
      }

#ifdef DEBUG
if (pmmfeResult)
   DPRINTF_DUMPLOCATEENTRY(( "MMF: LOCATE: found buffer 0x%08p len 0x%08p\n",
                             pmmfeResult->pvData,
                             pmmfeResult->ulMaxSize));
else
   DPRINTF_DUMPLOCATEENTRY(( "MMF: LOCATE: no matching buffer found\n"));
#endif

return pmmfeResult;
}

// -----------------------------------------------------------------------------

static PMMFENTRY __locateFreeMMFEntry( PMMF pmmf)
{
         ULONG          i;
         PMMFENTRY      pmmfeResult = NULL;
         PMMFENTRY      pmmfe;

if (pmmf)
   for (i = 0, pmmfe = pmmf->apmmfentry;
        i < pmmf->ulEntryCount;
        i++, pmmfe++)
      {
      if (!MMFENTRY_USED( pmmfe))
         {
         pmmfeResult = pmmfe;

         // set file handle to unused
         // so that a DosClose on cleanup does not hurt
         pmmfe->hfile = -1;
         break;
         }
      }
return pmmfeResult;
}

// -----------------------------------------------------------------------------

static VOID __destroyMMFEntry( PMMFENTRY pmmfe)
{
         APIRET         rc = NO_ERROR;
         FILESTATUS3    fs3;
         time_t         timeModified;

if ((pmmfe) && (MMFENTRY_USED( pmmfe)))
   {
   // cleanup all data related to the file
   if ((pmmfe->fFileOpened) && (pmmfe->hfile))
      {

      // close file
      DosClose( pmmfe->hfile);

      // if file was modified, set timestamp
      if ((pmmfe->fModified) && (rc == NO_ERROR) && pmmfe->szFile[0]!=0)
         {
         rc = DosQueryPathInfo( pmmfe->szFile, FIL_STANDARD,
                                &fs3, sizeof( fs3));
         if (rc == NO_ERROR)
            {
            // convert DATETIME to FDATE and FTIME
            if ((WtkDateTimeToTime( &pmmfe->dtModified, &timeModified)) &&
                (WtkTimeToFDateTime( &timeModified,
                                     &fs3.fdateLastWrite,
                                     &fs3.ftimeLastWrite)))
   
               {
               DosSetPathInfo( pmmfe->szFile, FIL_STANDARD,
                               &fs3, sizeof( fs3), 0);
               }
            }
         }
      }

   // free buffer
   if (pmmfe->pvData)
      {
      rc = DosFreeMem( pmmfe->pvData);
      DPRINTF_ALLOCACTION(( "MMF: ALLOCATE: free buffer 0x%08p len 0x%08p rc=%u\n", 
                            pmmfe->pvData,  pmmfe->ulMaxSize, rc));
      }
   memset( pmmfe, 0, sizeof( MMFENTRY));
   }

return;
}

// #############################################################################

static PMMF __locateMMFHandler( VOID)
{
         ULONG          i;
         PMMF           pmmfResult = NULL;

         PMMF           pmmf;
         PPIB           ppib;
         PTIB           ptib;

// get process and thread id
_fmutex_request(&fmtx_apmmf,0);
DosGetInfoBlocks( &ptib,&ppib);
for (i = 0; i < MMF_MAXINSTANCES; i++)
   {
   pmmf = apmmf[ i];
   if ((pmmf)                         &&
       (pmmf->pid == ppib->pib_ulpid) &&
       (pmmf->tid == ptib->tib_ptib2->tib2_ultid))
      {
      pmmfResult = pmmf;
      DPRINTF(("__locateMMFHandler found #%d -> %08x\n", i, pmmf));
      }
   }
_fmutex_release(&fmtx_apmmf);

return pmmfResult;
}

// -----------------------------------------------------------------------------

// prototype used here
static ULONG APIENTRY __pageFaultHandler( PEXCEPTIONREPORTRECORD p1,
                                          PEXCEPTIONREGISTRATIONRECORD p2,
                                          PCONTEXTRECORD p3, PVOID  pv);

static PMMF __createNewMMFHandler( ULONG ulMaxFiles)
{
         APIRET         rc = NO_ERROR;
         ULONG          i;
         PMMF           pmmfNew = NULL;

         PPIB           ppib;
         PTIB           ptib;

// zero files not allowed
if (!ulMaxFiles)
   return NULL;

// get process and thread id
_fmutex_request(&fmtx_apmmf,0);
DosGetInfoBlocks( &ptib,&ppib);
for (i = 0; i < MMF_MAXINSTANCES; i++)
   {
   if (!apmmf[ i])
      {
      // get memory for data struct
      pmmfNew = malloc( sizeof( MMF));
      DPRINTF(("__createNewMMFHandler free #%d -> %08x\n", i, pmmfNew));
      if (pmmfNew)
         {
         apmmf[ i] = pmmfNew;
         memset( pmmfNew, 0, sizeof( MMF));
         pmmfNew->pid = ppib->pib_ulpid;
         pmmfNew->tid = ptib->tib_ptib2->tib2_ultid;

         // get memory for MMF entries
         pmmfNew->apmmfentry = calloc( ulMaxFiles, sizeof( MMFENTRY));
         if (!pmmfNew->apmmfentry)
            {
            rc = ERROR_NOT_ENOUGH_MEMORY;
            break;
            }
         else
            {
            // initialize
            pmmfNew->ulEntryCount = ulMaxFiles;
            memset( &pmmfNew->errh, 0, sizeof( pmmfNew->errh));
            pmmfNew->errh.ExceptionHandler = (ERR) __pageFaultHandler;
            rc = DosSetExceptionHandler( &pmmfNew->errh);
            if (rc != NO_ERROR)
               break;

            }

         // we are done
         break;
         }
      }
   }

// cleanup
if (rc != NO_ERROR)
   if (pmmfNew) free( pmmfNew);
_fmutex_release(&fmtx_apmmf);

return pmmfNew;
}

// -----------------------------------------------------------------------------

static APIRET __destroyMMFHandler( PMMF pmmf)
{
         APIRET         rc = NO_ERROR;
         ULONG          i, j;

_fmutex_request(&fmtx_apmmf,0);
if (pmmf)
   {
   rc = ERROR_INVALID_HANDLE;
   for (i = 0; i < MMF_MAXINSTANCES; i++)
      {
      if (apmmf[ i] == pmmf)
         {
         // release all memory objects
         for (j = 0; j < pmmf->ulEntryCount; j++)
            {
            __destroyMMFEntry( pmmf->apmmfentry + j);
            }

         // release handler
         rc = DosUnsetExceptionHandler( &pmmf->errh);
         if (rc != NO_ERROR)
            break;

         // reset handler data
         //memset( pmmf, 0, sizeof( MMF));
         DPRINTF(("__createNewMMFHandler destroy #%d -> %08x\n", i, pmmf));
         free( pmmf->apmmfentry);
         free( pmmf);
         apmmf[ i] = NULL;
         rc = NO_ERROR;
         break;
         }
      }
   } // if (pmmf)
_fmutex_release(&fmtx_apmmf);

return rc;
}

// #############################################################################

#define EXCEPTION_NUM   p1->ExceptionNum
#define EXCEPTION_TYPE  p1->ExceptionInfo[ 0]
#define EXCEPTION_ADDR  ((PVOID) p1->ExceptionInfo[ 1])

#define PAG_ADDR_MASK   0xFFFFF000
#define PAG_PERM_MASK   (PAG_READ | PAG_WRITE | PAG_EXECUTE)
#define PAG_SIZE        4096

static ULONG APIENTRY __pageFaultHandler( PEXCEPTIONREPORTRECORD p1,
                                          PEXCEPTIONREGISTRATIONRECORD p2,
                                          PCONTEXTRECORD p3, PVOID  pv)
{
         ULONG          ulExceptNum = EXCEPTION_NUM;
         ULONG          ulExceptType = EXCEPTION_TYPE;
         PVOID          pvExceptAddress = EXCEPTION_ADDR;

if ((ulExceptNum == XCPT_ACCESS_VIOLATION)  &&
    ((ulExceptType & XCPT_EXECUTE_ACCESS) ||
     (ulExceptType & XCPT_WRITE_ACCESS)   ||
     (ulExceptType & XCPT_READ_ACCESS)))
   {
            PMMFENTRY      pmmfe = 0;
            PVOID          pvPage = 0;
            APIRET         rc  = NO_ERROR;
            ULONG          ulPageFlags = 0;
            ULONG          ulPageSize = PAG_SIZE;

            ULONG          ulFilePtr = 0;
            ULONG          ulPageOffset;
            ULONG          ulAccessOffset;
            ULONG          ulNewSize;

            PSZ            pszViolationType;
            PSZ            pszReqAccessType;
            ULONG          ulReqPageFlags = 0;
   
   // determine type of exception from bit mask
   if (ulExceptType & XCPT_EXECUTE_ACCESS)
      {
      // this exception should not occur, as read access is sufficient
      // for allowing execution, but we implement it for safety
      pszViolationType = "EXECUTE";
      pszReqAccessType = "executable";
      ulReqPageFlags   = PAG_READ | PAG_WRITE | PAG_EXECUTE;
      }
   else if (ulExceptType & XCPT_WRITE_ACCESS)
      {
      pszViolationType = "WRITE";
      pszReqAccessType = "readwrite";
      ulReqPageFlags   = PAG_READ | PAG_WRITE;
      }
   else if (ulExceptType & XCPT_READ_ACCESS)
      {
      pszViolationType = "READ";
      pszReqAccessType = "readonly";
      ulReqPageFlags   = PAG_READ;
      }

   DPRINTF_HANDLERACTION(( "MMF: HANDLER: XCPT_%s_ACCESS at 0x%08x\n",
                           pszViolationType, pvExceptAddress));

   // ------------------------------------------------------

   // address must be of one of our address ranges
   pmmfe = __locateMMFEntry( NULL, pvExceptAddress);
   if(!pmmfe)
      {
      DPRINTF(( "MMF: HANDLER: error: not my memory at 0x%08x\n", pvExceptAddress));
      return XCPT_CONTINUE_SEARCH;
      }

   // ------------------------------------------------------

   // handle certain violation types only if matching the memory mode flags 

   // allow write access for all modes except readonly
   if ((ulExceptType & XCPT_WRITE_ACCESS) &&
       (MMFENTRY_MEMORY_READONLY( pmmfe)))
      {
      DPRINTF(( "MMF: HANDLER: error: keep write access denied for buffer %s\n", pmmfe->szDesc));
      return XCPT_CONTINUE_SEARCH;
      }

   // allow execute access for execute mode only
   if ((ulExceptType & XCPT_EXECUTE_ACCESS) &&
       (!MMFENTRY_MEMORY_EXECUTE( pmmfe)))
      {
      DPRINTF(( "MMF: HANDLER: error: keep execute access denied for buffer %s\n", pmmfe->szDesc));
      return XCPT_CONTINUE_SEARCH;
      }

#if 1
   // promote read flag if page is read/write
   if (ulReqPageFlags==PAG_READ &&
       (MMFENTRY_MEMORY_READWRITE( pmmfe)))
      {
      ulReqPageFlags=PAG_READ|PAG_WRITE;
      }
#endif

   // ------------------------------------------------------

   // determine page address
   pvPage = (PVOID)((ULONG)pvExceptAddress & PAG_ADDR_MASK);

   // query commit and permission flags of affected page
   rc = DosQueryMem( pvPage, &ulPageSize, &ulPageFlags);
   if (rc != NO_ERROR)
      {
      DPRINTF(( "MMF: HANDLER: error: cannot query memory flags rc=%u\n", rc));
      return XCPT_CONTINUE_SEARCH;
      }

   // determine access position in memory regarded to the base pointer
   ulAccessOffset = (PSZ) pvExceptAddress - (PSZ) pmmfe->pvData;

   // determine page position in memory regarded to the base pointer
   ulPageOffset = (PSZ) pvPage - (PSZ) pmmfe->pvData;
   DPRINTF_HANDLERACTION(( "MMF: HANDLER:   examining page at 0x%08x offset 0x%08x %s\n",
                           pvPage, ulPageOffset, pmmfe->szDesc));

   // ------------------------------------------------------

   // if page is not committed, commit it first
   // and read file contents to it

   if (!(ulPageFlags & PAG_COMMIT))
      {
      // commit memory and  temporarily allow writing so
      // that file contents can be loaded
      rc = DosSetMem( pvPage, PAG_SIZE, PAG_COMMIT | PAG_WRITE);
      if (rc != NO_ERROR)
         {
         DPRINTF(( "MMF: HANDLER: error: cannot commit memory rc=%u\n", rc));
         return XCPT_CONTINUE_SEARCH;
         }
      else
         DPRINTF_HANDLERACTION(( "MMF: HANDLER:   commit memory\n", pvPage));

      // read data from file if it is a file area
      if (pmmfe->hfile)
         {
         // if memory is not beyond current file size, read from file
         if (ulPageOffset < pmmfe->ulCurrentSize)
            {
            // set file position
            rc = DosSetFilePtr( pmmfe->hfile,
                                ulPageOffset + pmmfe->ulOffset,
                                FILE_BEGIN,
                                &ulFilePtr);
            if (rc != NO_ERROR)
               {
               DPRINTF(( "MMF: HANDLER: error: cannot set file position 0x%08x, rc=%u\n",
                          ulPageOffset, rc));
               return XCPT_CONTINUE_SEARCH;
               }

            // rc is NO_ERROR if file is smaller than ulPageOffset,
            // so lets check the result
            if (ulPageOffset + pmmfe->ulOffset == ulFilePtr)
               {
               // read page from disk into buffer
               rc = DosRead( pmmfe->hfile,
                             pvPage,
                             PAG_SIZE,
                             &ulFilePtr);
               if (rc != NO_ERROR)
                  {
                  DPRINTF(( "MMF: HANDLER: error: cannot read file, rc=%u\n", rc));
                  return XCPT_CONTINUE_SEARCH;
                  }

               DPRINTF_HANDLERACTION(( "MMF: HANDLER:   read file offset 0x%08x\n", ulFilePtr));

               } // if (ulPageOffset == ulFilePtr)

            } // if (ulPageOffset < pmmfe->ulCurrentSize)

         } // if (pmmfe->hfile)

      } // if (!(ulPageFlags & PAG_COMMIT))

   // ------------------------------------------------------

   // on access violation due to missing access rights
   // set required page flags
   if (ulReqPageFlags)
      {
      // set new flags
      rc = DosSetMem( pvPage, PAG_SIZE, ulReqPageFlags);
      if (rc != NO_ERROR)
         {
         DPRINTF(( "MMF: HANDLER: error: cannot set memory to %s, rc=%u\n",
                   pszReqAccessType, rc));
         return XCPT_CONTINUE_SEARCH;
         }
      else
         DPRINTF_HANDLERACTION(( "MMF: HANDLER:   set memory to %s\n",
                                 pszReqAccessType));

      } // if (ulReqPageFlags)

   // ------------------------------------------------------

   // if necessary, extend current file size to end of new page

   if ((!ulAccessOffset) && (!pmmfe->ulCurrentSize))
      // at least make the result one page large
      ulNewSize = ulPageOffset + PAG_SIZE;
   else if (ulAccessOffset >= pmmfe->ulCurrentSize)
      // extend to end of page
      ulNewSize =  ulPageOffset + PAG_SIZE;
   else
      // do nothing otherwise
      ulNewSize = 0;

   if (ulNewSize)
      {
      // set new size, but don't exceed maximum size
      pmmfe->ulCurrentSize = MIN( ulNewSize, pmmfe->ulMaxSize);
      DPRINTF_HANDLERACTION(( "MMF: HANDLER:   adjust currrent size to 0x%08x\n",
                              pmmfe->ulCurrentSize));
      }

   // exception handled successfully
   return XCPT_CONTINUE_EXECUTION;
   }

// exception not handled, pass to following handlers
return XCPT_CONTINUE_SEARCH;
}

// -----------------------------------------------------------------------------

#ifdef DEBUG
VOID __dumpMMF(  PMMF pmmf)
{
         ULONG          i;
         ULONG          ulCount = 0;
         PPIB           ppib;
         PTIB           ptib;
         PMMFENTRY      pmmfe;

// is handle valid for this thread ?
if (pmmf != __locateMMFHandler())
   return;

// try to initialize
if (!fInitialized)
   {
   DPRINTF(( "MMF: DUMP: not initialized\n"));
   return;
   }

DosGetInfoBlocks( &ptib,&ppib);
DPRINTF(( "\n"));
DPRINTF(( "MMF: DUMP entries for pid %u tid: %u:\n"
        "-------------------------------------\n",
        ppib->pib_ulpid, ptib->tib_ptib2->tib2_ultid));

for (i = 0, pmmfe = pmmf->apmmfentry;
     i < pmmf->ulEntryCount;
     i++, pmmfe++)
   {
   if (MMFENTRY_USED( pmmfe))
      {
      DPRINTF(( "%u: memory at %p, size %u, handle %u, target: %s\n",
                i, pmmfe->pvData, pmmfe->ulMaxSize, pmmfe->hfile, pmmfe->szDesc));
      ulCount++;
      }
   }
DPRINTF(( "%u of %u entries used\n\n", ulCount, pmmf->ulEntryCount));
return;
}
#endif

// -----------------------------------------------------------------------------

static VOID __initialize( VOID)
{
do
   {
   if (fInitialized)
      break;

   // init handle table
   memset( apmmf, 0, sizeof( apmmf));

   // we're done
   fInitialized = TRUE;

   } while (FALSE);

return;
}

// -----------------------------------------------------------------------------

#define MEMATTR_ALLOC_LOWMEM   (PAG_READ | OBJ_TILE)
#define MEMATTR_ALLOC_HIGHMEM  (PAG_READ | OBJ_ANY)

static APIRET __allocateBuffer( PVOID *ppvData, ULONG ulMaxSize, BOOL fAllocHighMem)
{
         APIRET         rc = NO_ERROR;

do
   {
   // check parms
   if (!ppvData)
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // allocate memory
   rc = DosAllocMem( ppvData, ulMaxSize,
                     (fAllocHighMem) ?
                       MEMATTR_ALLOC_HIGHMEM :
                       MEMATTR_ALLOC_LOWMEM);

   // if high mem failed, then try low mem
   if ((fAllocHighMem) && (rc != NO_ERROR))
      rc = DosAllocMem( ppvData, ulMaxSize, MEMATTR_ALLOC_LOWMEM);

   } while (FALSE);

DPRINTF_ALLOCACTION(( "MMF: ALLOCATE: allocate buffer 0x%08p len 0x%08p, rc=%u\n", *ppvData, ulMaxSize, rc));
return rc;
}

// ===========================================================================

/*
@@WtkInitializeMmf@SYNTAX
This function initializes MMF support for the current thread and returns
the handle of either a newly created or already existing MMF manager.

@@WtkInitializeMmf@PARM@phmmf@out
The address of a variable receiving the handle to the MMF manager.
:p.
If a MMF manager already exists, WtkInitializeMmf will return
ERROR_ACCESS_DENIED . However, the handle to the existing manager
will be returned in phmmf.
:p.
The handle is only valid within the thread having created it. Moreover,
it may not be used in other modules of the application being linked to
abother copy of the Workplace Shell library.

@@WtkInitializeMmf@PARM@ulMaxFiles@in
Number of memory mapped files to support for this thread.

@@WtkInitializeMmf@RETURN
Return Code.
:p.
WtkInitializeMmf returns one of the following return codes&colon.

:parml compact break=none.
:pt.0
:pd.NO_ERROR
:pt.5
:pd.ERROR_ACCESS_DENIED
:pt.8
:pd.ERROR_NOT_ENOUGH_MEMORY
:pt.87
:pd.ERROR_INVALID_PARAMETER
:eparml.

@@WtkInitializeMmf@REMARKS
This function registers the
:link reftype=hd viewport refid=G_WTKMMF_XHANDLER.MMF exception handler:elink.
for to support memory mapped files. When leaving the function that called WtkInitializeMmf,
:link reftype=hd viewport refid=WtkTerminateMmf.WtkTerminateMmf:elink.
must be called to deregister the exception handler again, especially if other
exception handler are used beside the one for MMF support. Otherwise unpredictable
results (most often simply protection violations may occurr).


@@
*/

APIRET APIENTRY WtkInitializeMmf( PHMMF phmmf, ULONG ulMaxFiles)
{

         APIRET         rc = NO_ERROR;
         PMMF           pmmf = NULL;
do
   {
   // check parms
   if (!phmmf)
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // setup
   __initialize();

   // if a handlerr is already registered, break with error
   pmmf = __locateMMFHandler();
   if (pmmf)
      {
      // report handle anyway
      *phmmf = (HMMF)pmmf;

      DPRINTF_ALLOCACTION(( "MMF: initialize: handler already exists for pid %u tid %u\n",
                            pmmf->pid, pmmf->tid));
      rc = ERROR_ACCESS_DENIED;
      break;
      }

   // --------------------------------------------------------

   // create a new entry
   pmmf = __createNewMMFHandler( ulMaxFiles);
   if (!pmmf)
      {
      rc = ERROR_NOT_ENOUGH_MEMORY;
      break;
      }

   // report pointer as handle
   *phmmf = (HMMF)pmmf;

   } while (FALSE);

return rc;
}

// ---------------------------------------------------------------------------

/*
@@WtkTerminateMmf@SYNTAX
This function deinitializes MMF support for the current thread.

@@WtkTerminateMmf@PARM@hmmf@in
Handle to the MMF manager.

@@WtkTerminateMmf@RETURN
Return Code.
:p.
WtkTerminateMmf returns one of the following return codes&colon.

:parml compact break=none.
:pt.0
:pd.NO_ERROR
:pt.87
:pd.ERROR_INVALID_PARAMETER
:eparml.

@@WtkTerminateMmf@REMARKS
This function deregisters the exception handler for to support memory mapped files.
Make sure that you always WtkTerminateMmf before exiting the function that has called
:link reftype=hd viewport refid=WtkInitializeMmf.WtkInitializeMmf:elink., especially
if other exception handler are used beside the one for MMF support. Otherwise
unpredictable results (most often simply protection violations may occurr).

@@
*/

APIRET APIENTRY WtkTerminateMmf( HMMF hmmf)
{
         APIRET         rc = NO_ERROR;
         PMMF           pmmf = (PMMF) hmmf;

do
   {
   // check parms
   if (!hmmf)
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // destroy handler
   rc = __destroyMMFHandler( pmmf);
   } while (FALSE);

// cleanup
return rc;
}

// ---------------------------------------------------------------------------

/*
@@WtkAllocMmf@SYNTAX
This function allocates a dynamic memory buffer either for a file or plain
memory access.

@@WtkAllocMmf@PARM@hmmf@in
Handle to the MMF manager.
:p.
For applications being linked statically to the 
Workplace Shell Toolkit, MMF handles are only valid in the
same aplication part (main executable or DLL) where being ceated.
See also the :link reftype=hd viewport res=2001.MMF code linking considerations:elink..

@@WtkAllocMmf@PARM@*ppvdata@out
Pointer to a variable receiving the pointer to the allocated address space.
:p.
Despite of the file access mode specified in parameter ulOpenFlags,
the buffer always can be read and written.

@@WtkAllocMmf@PARM@pszFilename@in
Address of the ASCIIZ path name or :hp2.MMF_FILE_INMEMORY:ehp2..
:p.
If no file is specified, :hp2.MMF_FILE_INMEMORY:ehp2.
(equals NULL) will allocate a dynamic memory buffer only.
:p.
The path name may contain :hp2.?&colon.:ehp2. for the drive in order to
open the file on the boot drive.
:p.
The name may not include wildcards.

@@WtkAllocMmf@PARM@ulOpenFlags@in
Specify one of the following file access modes:
:parml.
:pt.MMF_ACCESS_READONLY
:pd.open file in readonly mode, denying write by others (default).
:pt.MMF_ACCESS_WRITEONLY
:pd.open file in writeonly mode, denying write by others
:pt.MMF_ACCESS_READWRITE
:pd.open file in readwrite mode, denying read and write by others
:eparml.
:p.
Specify one of the following file open modes:
:parml.
:pt.MMF_OPENMODE_OPENFILE
:pd.open existing file or create new one
:pt.MMF_OPENMODE_RESETFILE
:pd.reset existing file or create new one
:eparml.
:p.
Specify one of the following memory access modes:
:p.
:pt.MMF_MEMORY_READWRITE
:pd.allocate readwrite memory (default)
:pt.MMF_MEMORY_READONLY
:pd.allocate readonly memory
:pt.MMF_MEMORY_EXECUTE
.pd.allocate readwrite memory with execute rights
:eparml.
:p.
Specify one of the following update modes:
:parml.
:pt.MMF_UPDATE_RELEASEMEM
:pd.release memory on update (default)
:pt.MMF_UPDATE_KEEPMEM
:pd.keep memory on update
:eparml.
:p.
Specify the following memory allocation flag:
:parml.
:pt.MMF_ALLOC_LOWMEM
:pd.allocates the file buffer in memory below 512 MB (default)
:pt.MMF_ALLOC_HIGHMEM
:pd.allocates the file buffer in high memory above 512 MB. If that fails,
the buffer is allocated in low memory. Do not specify this flag if
the memory is to be accessed by APIs being still implemented in 16 bit code
or calling other APIs calling 16-bit APIs, and thus
:link reftype=hd viewport res=2000.not supporting high memory:elink..
:eparml.

@@WtkAllocMmf@PARM@ulMaxSize@in
Maximum possible size of the dynamic memory in bytes.
:p.
This maximum size cannot be changed later.

@@WtkAllocMmf@RETURN
Return Code.
:p.
WtkAllocMmf returns one of the following return codes&colon.

:parml compact break=none.
:pt.0
:pd.NO_ERROR
:pt.4
:pd.ERROR_TOO_MANY_OPEN_FILES
:pt.6
:pd.ERROR_INVALID_HANDLE
:pt.87
:pd.ERROR_INVALID_PARAMETER
:pt.111
:pd.ERROR_BUFFER_OVERFLOW
:eparml.
:p.
or return codes of the following functions
:ul compact.
:li.DosOpen
:li.DosAllocMem
:eul.

@@WtkAllocMmf@REMARKS
This function supports boot drive recognition.
Specify :hp2.?&colon.:ehp2. for the drive in order to
open the file on the boot drive.
:p.
The file is kept open until
:link reftype=hd viewport refid=WtkFreeMmf.WtkFreeMmf:elink.
is called to release the provided memory buffer again. Until that
reading or writing to that file by other processes is denied according to
the access flags specified for parameter
:link reftype=hd viewport refid=ulOpenFlags_WtkAllocMmf.ulOpenFlags:elink..

@@
*/

APIRET APIENTRY WtkAllocMmf( HMMF hmmf, PVOID *ppvdata, PSZ pszFilename, ULONG ulOpenFlags, ULONG ulMaxSize)
{
         APIRET         rc = NO_ERROR;
         PMMF           pmmf = (PMMF) hmmf;

         ULONG          ulAction;
         ULONG          fsOpenFlags;
         ULONG          fsOpenMode;

         HFILE          hfile = -1;
         FILESTATUS3    fs3;
         ULONG          ulCurrentSize = 0;
         PVOID          pvData = NULL;
         PMMFENTRY      pmmfe;

do
   {
   // check parms
   if ((!hmmf)     ||
       (!ppvdata)  ||
       (!ulMaxSize)||
       (!_areOpenFlagsValid( ulOpenFlags)))
      {  
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // is handle valid for this thread ?
   if (pmmf != __locateMMFHandler())
      {
      rc = ERROR_INVALID_HANDLE;
      break;
      }

   // adapt open mode and open flags
   switch (ulOpenFlags & MMF_MASK_ACCESS)
      {
      case MMF_ACCESS_READONLY:     fsOpenMode = OPEN_ACCESS_READONLY  | OPEN_SHARE_DENYWRITE;     break;
      case MMF_ACCESS_WRITEONLY:    fsOpenMode = OPEN_ACCESS_WRITEONLY | OPEN_SHARE_DENYWRITE;     break;
      case MMF_ACCESS_READWRITE:    fsOpenMode = OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYREADWRITE; break;

      default:
         rc = ERROR_INVALID_PARAMETER;
         break;
      }

   switch (ulOpenFlags & MMF_MASK_OPENMODE)
      {
      case MMF_OPENMODE_OPENFILE:  fsOpenFlags = OPEN_ACTION_OPEN_IF_EXISTS;    break;
      case MMF_OPENMODE_RESETFILE: fsOpenFlags = OPEN_ACTION_REPLACE_IF_EXISTS; break;

      default:
         rc = ERROR_INVALID_PARAMETER;
         break;
      }
   if (rc != NO_ERROR)
      break;


   // locate free entry in table
   pmmfe = __locateFreeMMFEntry( pmmf);
   if(!pmmfe)
      {
      rc = ERROR_TOO_MANY_OPEN_FILES;
      break;
      }

   // use a link to a file
   if ((pszFilename) && (*pszFilename))
      {

      // determine fullname of file (replaces ?: with bootdrive as well)
      WtkQueryFullname( pszFilename, pmmfe->szFile, sizeof( pmmfe->szFile));
#ifdef DEBUG
      strcpy( pmmfe->szDesc, pmmfe->szFile);
#endif

      // open file
      rc = DosOpen( pmmfe->szFile,
                    &hfile,
                    &ulAction,
                    0L,
                    FILE_ARCHIVED | FILE_NORMAL,
                    OPEN_ACTION_CREATE_IF_NEW | fsOpenFlags,
                    OPEN_FLAGS_NOINHERIT | OPEN_FLAGS_FAIL_ON_ERROR | fsOpenMode,
                    NULL);
      if (rc != NO_ERROR)
         return rc;

      // query size, may not be larger than the requested buffer size
      rc = DosQueryFileInfo( hfile, FIL_STANDARD,  &fs3, sizeof( fs3));
      if (rc != NO_ERROR)
         break;
      ulCurrentSize = fs3.cbFile;
      if (ulMaxSize  < ulCurrentSize)
         {
         rc = ERROR_BUFFER_OVERFLOW;
         break;
         }

      } // if ((pszFilename) && (*pszFilename))

   // allocate memory for the file
   rc = __allocateBuffer( &pvData, ulMaxSize, (ulOpenFlags & MMF_ALLOC_HIGHMEM));
   if (rc != NO_ERROR)
      break;

   // setup handle data
   pmmfe->ulFlags       = ulOpenFlags;
   pmmfe->hfile         = hfile;
   pmmfe->fFileOpened   = TRUE;
   pmmfe->pvData        = pvData;
   pmmfe->ulMaxSize     = ulMaxSize;
   pmmfe->ulCurrentSize = ulCurrentSize;
   pmmfe->ulOffset      = 0;


   *ppvdata = pvData;

#ifdef DEBUG
   // copy filename to description
   strcpy( pmmfe->szDesc, pmmfe->szFile);

   // syntax-check macros for flag filtering here
   ulOpenFlags = MMFENTRY_ACCESS_READONLY( pmmfe);
   ulOpenFlags = MMFENTRY_ACCESS_WRITEONLY( pmmfe);
   ulOpenFlags = MMFENTRY_ACCESS_READWRITE( pmmfe);

   ulOpenFlags = MMFENTRY_UPDATE_KEEPMEM( pmmfe);
   ulOpenFlags = MMFENTRY_UPDATE_RELEASEMEM( pmmfe);

   ulOpenFlags = MMFENTRY_MEMORY_READWRITE( pmmfe);
   ulOpenFlags = MMFENTRY_MEMORY_READONLY( pmmfe);
   ulOpenFlags = MMFENTRY_MEMORY_EXECUTE( pmmfe);

   ulOpenFlags = MMFENTRY_ALLOC_LOWMEM( pmmfe);
   ulOpenFlags = MMFENTRY_ALLOC_HIGHMEM( pmmfe);

#endif


   } while (FALSE);

// cleanup
if (rc != NO_ERROR)
   DosClose( hfile);

return rc;
}

// ---------------------------------------------------------------------------

/*
@@WtkAllocMmfFile@SYNTAX
This function allocates a dynamic memory buffer for an open file.

@@WtkAllocMmfFile@PARM@hmmf@in
Handle to the MMF manager.
:p.
For applications being linked statically to the 
Workplace Shell Toolkit, MMF handles are only valid in the
same aplication part (main executable or DLL) where being ceated.
See also the :link reftype=hd viewport res=2001.MMF code linking considerations:elink..

@@WtkAllocMmfFile@PARM@*ppvdata@out
Pointer to a variable receiving the pointer to the allocated address space.
:p.
Despite of the file access mode specified in parameter ulOpenFlags,
the buffer always can be read and written.

@@WtkAllocMmfFile@PARM@hfile@in
Handle to the open file.
:note.
:ul compact.
:li.You must not modify the file with the original filehandle
while having it mapped by WtkAllocMmfFile!
:eul.

@@WtkAllocMmfFile@PARM@ulOpenFlags@in
Specify one of the following file access modes.
They must match the access flags of the filehandle!
:parml.
:pt.MMF_ACCESS_READONLY
:pd.open file in readonly mode, denying write by others (default).
:pt.MMF_ACCESS_WRITEONLY
:pd.open file in writeonly mode, denying write by others
:pt.MMF_ACCESS_READWRITE
:pd.open file in readwrite mode, denying read and write by others
:eparml.
:p.
Specify one of the following memory access modes:
:p.
:pt.MMF_MEMORY_READWRITE
:pd.allocate readwrite memory (default)
:pt.MMF_MEMORY_READONLY
:pd.allocate readonly memory
:pt.MMF_MEMORY_EXECUTE
.pd.allocate readwrite memory with execute rights
:eparml.
:p.
Specify one of the following update modes:
:parml.
:pt.MMF_UPDATE_RELEASEMEM
:pd.release memory on update (default)
:pt.MMF_UPDATE_KEEPMEM
:pd.keep memory on update
:eparml.
:p.
Specify the following memory allocation flag:
:parml.
:pt.MMF_ALLOC_LOWMEM
:pd.allocates the file buffer in memory below 512 MB (default)
:pt.MMF_ALLOC_HIGHMEM
:pd.allocates the file buffer in high memory above 512 MB. If that fails,
the buffer is allocated in low memory. Do not specify this flag if
the memory is to be accessed by APIs being still implemented in 16 bit code
or calling other APIs calling 16-bit APIs, and thus
:link reftype=hd viewport res=2000.not supporting high memory:elink..
:eparml.

@@WtkAllocMmfFile@PARM@ulMaxSize@in
Maximum possible size of the dynamic memory in bytes.
:p.
This maximum size cannot be changed later.

@@WtkAllocMmfFile@RETURN
Return Code.
:p.
WtkAllocMmfFile returns one of the following return codes&colon.

:parml compact break=none.
:pt.0
:pd.NO_ERROR
:pt.5
:pd.ERROR_ACCESS_DENIED
:pt.6
:pd.ERROR_INVALID_HANDLE
:pt.8
:pd.ERROR_NOT_ENOUGH_MEMORY
:pt.87
:pd.ERROR_INVALID_PARAMETER
:pt.111
:pd.ERROR_BUFFER_OVERFLOW
:eparml.
:p.
or return codes of the following functions
:ul compact.
:li.DosOpen
:li.DosAllocMem
:eul.

@@WtkAllocMmfFile@REMARKS
Make sure that you don't modify the file with the original filehandle
while having it mapped by WtkAllocMmfFile, otherwise this will
lead to unpredictable results, if you use
:link reftype=hd viewport refid=WtkUpdateMmf.WtkUpdateMmf:elink..
to update it later.
:p.
The file is kept open until
:link reftype=hd viewport refid=WtkFreeMmfFile.WtkFreeMmfFile:elink.
is called to release the provided memory buffer again. Until that
reading or writing to that file by other processes is denied according to
the access flags specified for parameter
:link reftype=hd viewport refid=ulOpenFlags_WtkAllocMmf.ulOpenFlags:elink..

@@
*/


APIRET APIENTRY WtkAllocMmfFile( HMMF hmmf, PVOID *ppvdata, HFILE hfile, ULONG ulOpenFlags, ULONG ulMaxSize)
{
        return WtkAllocMmfFileEx( hmmf, ppvdata, hfile, ulOpenFlags, ulMaxSize, 0);
}

/*
 * This is the same as WtkAllocMmfFile but takes an additional argument ulOffset that specifies
 * the offset from the beginning of the file where the mapped region of ulMaxSize bytes starts. The
 * offset must be aligned on a page boundary.
 */

APIRET APIENTRY WtkAllocMmfFileEx( HMMF hmmf, PVOID *ppvdata, HFILE hfile, ULONG ulOpenFlags, ULONG ulMaxSize, ULONG ulOffset)
{
         APIRET         rc = NO_ERROR;
         PMMF           pmmf = (PMMF) hmmf;

         ULONG          ulFileMode;
         ULONG          ulAccessFlag;

         ULONG          ulCurrentSize = 0;
         FILESTATUS3    fs3;
         PVOID          pvData = NULL;
         PMMFENTRY      pmmfe;
         HFILE          hfile2 = (HFILE) -1;

do
   {
   // check parms
   if ((!hmmf)      ||
       (!ppvdata)   ||
       (!ulMaxSize) ||
       (!_areOpenFlagsValid( ulOpenFlags)))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // is handle valid for this thread ?
   if (pmmf != __locateMMFHandler())
      {
      rc = ERROR_INVALID_HANDLE;
      break;
      }

   // locate free entry in table
   pmmfe = __locateFreeMMFEntry( pmmf);
   if(!pmmfe)
      {
      rc = ERROR_TOO_MANY_OPEN_FILES;
      DPRINTF(("__locateFreeMMFEntry failed\n"));
      break;
      }

   // unix mmap() allows closing the file handle after mapping
   if (hfile != -1)
      {
      rc = DosDupHandle( hfile, &hfile2);
      if (rc != NO_ERROR)
         break;
      hfile = hfile2;
      }

   // get access flags for file handle and
   // adjust the access flags accordingly
   if (hfile != -1)
      {
      rc = DosQueryFHState( hfile, &ulFileMode);
      if (rc != NO_ERROR)
         break;
      }

   // examine access bits 
   // (don't mind the strange masking here... readonly sets no bit)
#if 1
   switch (ulFileMode & (OPEN_ACCESS_WRITEONLY | OPEN_ACCESS_READWRITE))
      {
      case OPEN_ACCESS_READONLY:  ulAccessFlag = MMF_ACCESS_READONLY;  break;
      case OPEN_ACCESS_WRITEONLY: ulAccessFlag = MMF_ACCESS_WRITEONLY; break;
      case OPEN_ACCESS_READWRITE: ulAccessFlag = ((ulOpenFlags&MMF_ACCESS_READWRITE) ? MMF_ACCESS_READWRITE : MMF_ACCESS_READONLY); break;
      }
   if ((ulOpenFlags & MMF_MASK_ACCESS) != ulAccessFlag)
      {
      rc = ERROR_ACCESS_DENIED;
      break;
      }
#endif

   // query file size
   if (hfile != -1)
      {
      rc = DosQueryFileInfo( hfile, FIL_STANDARD,  &fs3, sizeof( fs3));
      if (rc != NO_ERROR)
         break;
      ulCurrentSize = fs3.cbFile;
      }

   // offset must be not greater than the current file size
   if (ulCurrentSize < ulOffset)
       {
       rc = ERROR_INVALID_PARAMETER;
       break;
       }

   // allocate memory for the file
   rc = __allocateBuffer( &pvData, ulMaxSize, (ulOpenFlags & MMF_ALLOC_HIGHMEM));
   if (rc != NO_ERROR)
      break;

   // setup handle data
   pmmfe->ulFlags       = ulOpenFlags;
   pmmfe->hfile         = (hfile == -1 ? 0 : hfile);
   pmmfe->fFileOpened   = (hfile == -1 ? FALSE : TRUE);
   pmmfe->pvData        = pvData;
   pmmfe->ulMaxSize     = ulMaxSize;
   pmmfe->ulCurrentSize = ulCurrentSize - ulOffset;
   pmmfe->ulOffset      = ulOffset;

   *ppvdata = pvData;

#ifdef DEBUG
   sprintf( pmmfe->szDesc, "file handle %u\n", pmmfe->hfile);
#endif

   } while (FALSE);

return rc;
}

// ---------------------------------------------------------------------------

/*
@@WtkFreeMmf@SYNTAX
This function frees a dynamic memory buffer.

@@WtkFreeMmf@PARM@hmmf@in
Handle to the MMF manager.

@@WtkFreeMmf@PARM@pvData@in
Pointer to the allocated address space to be freed.

@@WtkFreeMmf@RETURN
Return Code.
:p.
WtkFreeMmf returns one of the following return codes&colon.

:parml compact break=none.
:pt.0
:pd.NO_ERROR
:pt.87
:pd.ERROR_INVALID_HANDLE
:pt.87
:pd.ERROR_INVALID_PARAMETER
:eparml.
:p.
or return codes of the following functions
:ul compact.
:li.DosFreeMem
:eul.

@@WtkFreeMmf@REMARKS
Calling WtkFreeMmf will not save unsaved data. For that
:link reftype=hd viewport refid=WtkUpdateMmf.WtkUpdateMmf:elink.
must be called.

@@
*/

APIRET APIENTRY WtkFreeMmf(  HMMF hmmf, PVOID pvData)
{
         APIRET         rc = NO_ERROR;
         PMMF           pmmf = (PMMF) hmmf;
         PMMFENTRY      pmmfe;

do
   {
   // check parms
   if (!pvData)
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // is handle valid for this thread ?
   if (pmmf != __locateMMFHandler())
      {
      rc = ERROR_INVALID_HANDLE;
      break;
      }

   // search entry
   pmmfe = __locateMMFEntry( pmmf, pvData);
   if (!pmmfe)
      {
      rc = ERROR_INVALID_HANDLE;
      break;
      }

   // cleanup all data related to the file
   __destroyMMFEntry( pmmfe);

   } while (FALSE);

return rc;
}

// ---------------------------------------------------------------------------

/*
@@WtkFreeMmfFile@SYNTAX
This function frees a dynamic memory buffer. It is equivalent to
:link reftype=hd viewport refid=WtkFreeMmf.WtkFreeMmf:elink.
and is implemented as a counterpart to
:link reftype=hd viewport refid=WtkAllocMmfFile.WtkAllocMmfFile:elink..

@@WtkFreeMmfFile@PARM@hmmf@in
Handle to the MMF manager.

@@WtkFreeMmfFile@PARM@pvData@in
Pointer to the allocated address space to be freed.

@@WtkFreeMmfFile@RETURN
Return Code.
:p.
WtkFreeMmfFile returns one of the following return codes&colon.

:parml compact break=none.
:pt.0
:pd.NO_ERROR
:pt.6
:pd.ERROR_INVALID_HANDLE
:pt.87
:pd.ERROR_INVALID_PARAMETER
:eparml.
:p.
or return codes of the following functions
:ul compact.
:li.DosFreeMem
:eul.

@@WtkFreeMmfFile@REMARKS
Calling WtkFreeMmfFile will not save unsaved data. For that
:link reftype=hd viewport refid=WtkUpdateMmf.WtkUpdateMmf:elink.
must be called.

@@
*/

APIRET APIENTRY WtkFreeMmfFile(  HMMF hmmf, PVOID pvData)
{
return WtkFreeMmf( hmmf, pvData);
}

// ---------------------------------------------------------------------------

/*
@@WtkUpdateMmf@SYNTAX
This function updates the file mapped to a memory buffer
by writing all modified pages to the file.

@@WtkUpdateMmf@PARM@hmmf@in
Handle to the MMF manager.

@@WtkUpdateMmf@PARM@pvData@in
Pointer to the allocated address space to be saved.

@@WtkUpdateMmf@RETURN
Return Code.
:p.
WtkUpdateMmf returns one of the following return codes&colon.

:parml compact break=none.
:pt.0
:pd.NO_ERROR
:pt.5
:pd.ERROR_ACCESS_DENIED
:pt.6
:pd.ERROR_INVALID_HANDLE
:pt.87
:pd.ERROR_INVALID_PARAMETER
:eparml.

@@WtkUpdateMmf@REMARKS
Only truly modified memory pages of the dynamic memory buffer are stored
(every memory page of 4KB in size that has been modified since the read from disk)
and then marked as updated.
:p.
Calling WtkUpdateMmf on readonly memory buffers, or memory buffers that hold data
from a file opened in readonly mode will return ERROR_ACCESS_DENIED.
:p.
If for the specified memory buffer the flag MMF_UPDATE_RELEASEMEM is
set, the buffer memory is decommitted, but  will be recommitted on access
by the
:link reftype=hd viewport refid=G_WTKMMF_XHANDLER.MMF exception handler:elink.,
when required.
:p.
When explicitely closing the file by a call to
:link reftype=hd viewport refid=WtkFeeMmf.WtkFreeMmf:elink. or
:link reftype=hd viewport refid=WtkFeeMmfFil.WtkFreeMmfFile:elink.,
or implicitely by a call to 
:link reftype=hd viewport refid=WtkTerminate.WtkTerminate:elink.,
the last modification timestamp of the file will be set to the
time of the last call to .

@@
*/

APIRET APIENTRY WtkUpdateMmf(  HMMF hmmf, PVOID pvData)
{
         APIRET         rc = NO_ERROR;
         PMMF           pmmf = (PMMF) hmmf;
         PMMFENTRY      pmmfe;

         PBYTE          pbArea = 0;
         ULONG          ulPos = 0;
         ULONG          ulPageFlags = 0;
         ULONG          ulPageSize = PAG_SIZE;

         BOOL           fFileWritten = FALSE;
         ULONG          ulFilePtr;
         ULONG          ulBytesToWrite;
         ULONG          ulBytesWritten;
         ULONG          ulPageOffset;

do
   {
   // check parms
   if (!pvData)
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // is handle valid for this thread ?
   if (pmmf != __locateMMFHandler())
      {
      rc = ERROR_INVALID_HANDLE;
      break;
      }

   // search entry
   pmmfe = __locateMMFEntry( pmmf, pvData);
   if (!pmmfe)
      {
      rc = ERROR_INVALID_HANDLE;
      break;
      }

   // don't allow update if it is not a file area at all
   if (!(pmmfe->hfile))
      {
      rc = ERROR_INVALID_HANDLE;
      break;
      }

   // don't allow update if readonly access
   if (MMFENTRY_ACCESS_READONLY( pmmfe))
      {
      rc = ERROR_ACCESS_DENIED;
      break;
      }

   // don't update if write requested for readonly memory
   if (MMFENTRY_MEMORY_READONLY( pmmfe))
      {
      rc = ERROR_ACCESS_DENIED;
      break;
      }

   // locate all regions which needs update, and actually update them
   for (pbArea = (PBYTE) pmmfe->pvData;
        ulPos < pmmfe->ulCurrentSize;
        ulPos += PAG_SIZE, pbArea += PAG_SIZE)
      {
      // determine flags of this page
      rc = DosQueryMem( pbArea, &ulPageSize, &ulPageFlags);
      if (rc != NO_ERROR)
         {
         DPRINTF(( "MMF: UPDATE: cannot query memory flags %s, rc=%u\n", pmmfe->szDesc, rc));
         break;
         }

      // if page has been written, update the respective file area
      if (ulPageFlags & PAG_WRITE)
         {
         // set file pointer
         ulPageOffset = (PSZ) pbArea - (PSZ) pmmfe->pvData;
         rc = DosSetFilePtr( pmmfe->hfile,
                             ulPageOffset + pmmfe->ulOffset,
                             FILE_BEGIN,
                             &ulFilePtr);
         if (rc != NO_ERROR)
            {
            DPRINTF(( "MMF: UPDATE: cannot set file position %s, rc=%u\n", pmmfe->szDesc, rc));
            break;
            }

         // write portion of memory, either a full page or
         // the remaining portion of it
         ulBytesToWrite = MIN( PAG_SIZE, ulPageOffset - pmmfe->ulCurrentSize);
         rc = DosWrite( pmmfe->hfile,
                        pbArea,
                        ulBytesToWrite,
                        &ulBytesWritten);
         if (rc != NO_ERROR)
            {
            DPRINTF(( "MMF: UPDATE: cannot write to file: %s, rc=%u\n", pmmfe->szDesc, rc));
            break;
            }
         if (ulBytesToWrite != ulBytesWritten)
            {
            DPRINTF(( "MMF: UPDATE: cannot write to file: %s, rc=%u\n", pmmfe->szDesc, rc));
            rc = ERROR_WRITE_FAULT;
            break;
            }
         fFileWritten = TRUE;

         // reset write flag, so that subsequent updates will not
         // update again for no reason
         DosSetMem( pbArea, PAG_SIZE, PAG_READ);

        } // if (ulPageFlags & PAG_WRITE)

      // release (== decommit) memory on update, if requested
      if (MMFENTRY_UPDATE_RELEASEMEM( pmmfe))
         DosSetMem( pbArea, PAG_SIZE, PAG_DECOMMIT);

      } // for (pbArea = (PBYTE) pmmfe->pvData; ...

   // set file size in case size was extended
   rc = DosSetFileSize( pmmfe->hfile,
                        pmmfe->ulCurrentSize + pmmfe->ulOffset);
   if (rc != NO_ERROR)
      {
      DPRINTF(( "MMF: UPDATE: cannot set file position %s, rc=%u\n", pmmfe->szDesc, rc));
      break;
      }

   // save timestamp
   if (fFileWritten)
      {
      pmmfe->fModified = TRUE;
      DosGetDateTime( &pmmfe->dtModified);
      }

   } while (FALSE);

return rc;
}

// ---------------------------------------------------------------------------

/*
@@WtkRevertMmf@SYNTAX
This function discards any changes to and releases the memory
attached to a memory buffer mapped to a file.

@@WtkRevertMmf@PARM@hmmf@in
Handle to the MMF manager.

@@WtkRevertMmf@PARM@pvData@in
Pointer to the allocated address space to be reverted.

@@WtkRevertMmf@RETURN
Return Code.
:p.
WtkRevertMmf returns one of the following return codes&colon.

:parml compact break=none.
:pt.0
:pd.NO_ERROR
:pt.1
:pd.ERROR_INVALID_FUNCTION
:pt.5
:pd.ERROR_ACCESS_DENIED
:pt.6
:pd.ERROR_INVALID_HANDLE
:pt.87
:pd.ERROR_INVALID_PARAMETER
:eparml.

@@WtkRevertMmf@REMARKS
When reverting the memory buffer, all committed memory is decommitted.
:p.
When WtkRevertMmf is called with the address of an in-memory buffer,
it returns ERROR_INVALID_FUNCTION, as a revert operation is not
possible without a mapped file.

@@
*/

APIRET APIENTRY WtkRevertMmf(  HMMF hmmf, PVOID pvData)
{
         APIRET         rc = NO_ERROR;
         PMMF           pmmf = (PMMF) hmmf;
         PMMFENTRY      pmmfe;

         PBYTE          pbArea  = 0;
         ULONG          ulPos  = 0;

do
   {
   // check parms
   if (!pvData)
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // is handle valid for this thread ?
   if (pmmf != __locateMMFHandler())
      {
      rc = ERROR_INVALID_HANDLE;
      break;
      }

   // search entry
   pmmfe = __locateMMFEntry( pmmf, pvData);
   if (!pmmfe)
      {
      rc = ERROR_INVALID_HANDLE;
      break;
      }

   // don't allow to revert if it is not a file area at all
   if (!(pmmfe->hfile))
      {
      rc = ERROR_INVALID_FUNCTION;
      break;
      }

   // decommit all pages in the range
   for (pbArea = (PBYTE) pmmfe->pvData;
        ulPos < pmmfe->ulCurrentSize;
        ulPos += PAG_SIZE, pbArea += PAG_SIZE)
      {
      DosSetMem( pbArea, PAG_SIZE, PAG_DECOMMIT);
      }

   } while (FALSE);

return rc;
}

// ---------------------------------------------------------------------------

/*
@@WtkSetMmfSize@SYNTAX
This function adjusts the size of a dynamic memory buffer.

@@WtkSetMmfSize@PARM@hmmf@in
Handle to the MMF manager.

@@WtkSetMmfSize@PARM@pvData@in
Pointer to the allocated address space to be adjusted in size.

@@WtkSetMmfSize@PARM@ulNewSize@in
New size of the memory buffer.
:p.
The new size cannot exceed the maximum size that was specified
when allocating the dynamic memory buffer with
:link reftype=hd viewport refid=WtkAllocMmf.WtkAllocMmf:elink..

@@WtkSetMmfSize@RETURN
Return Code.
:p.
WtkSetMmfSize returns one of the following return codes&colon.

:parml compact break=none.
:pt.0
:pd.NO_ERROR
:pt.6
:pd.ERROR_INVALID_HANDLE
:pt.87
:pd.ERROR_INVALID_PARAMETER
:pt.111
:pd.ERROR_BUFFER_OVERFLOW
:eparml.

@@WtkSetMmfSize@REMARKS
The new size cannot exceed the maximum size that was specified
when allocating the dynamic memory buffer with
:link reftype=hd viewport refid=WtkAllocMmf.WtkAllocMmf:elink..

@@
*/

APIRET APIENTRY WtkSetMmfSize(  HMMF hmmf, PVOID pvData, ULONG ulNewSize)
{
         APIRET         rc = NO_ERROR;
         PMMF           pmmf = (PMMF) hmmf;
         PMMFENTRY      pmmfe;

do
   {
   // check parms
   if (!pvData)
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // is handle valid for this thread ?
   if (pmmf != __locateMMFHandler())
      {
      rc = ERROR_INVALID_HANDLE;
      break;
      }

   // search entry
   pmmfe = __locateMMFEntry( pmmf, pvData);
   if (!pmmfe)
      {
      rc = ERROR_INVALID_HANDLE;
      break;
      }

   // check maximum size
   if (ulNewSize > pmmfe->ulMaxSize)
      {
      rc = ERROR_BUFFER_OVERFLOW;
      break;
      }

   // set new size
   pmmfe->ulCurrentSize = ulNewSize;

   } while (FALSE);

return rc;
}

// ---------------------------------------------------------------------------

/*
@@WtkQueryMmfSize@SYNTAX
This function queries the current size of a dynamic memory buffer.

@@WtkQueryMmfSize@PARM@hmmf@in
Handle to the MMF manager.

@@WtkQueryMmfSize@PARM@pvData@in
Pointer to the allocated address space to be queried.

@@WtkQueryMmfSize@PARM@pulSize@out
Address of a variable to receive the current size of the dynamic memory buffer.

@@WtkQueryMmfSize@RETURN
Return Code.
:p.
WtkQueryMmfSize returns one of the following return codes&colon.

:parml compact break=none.
:pt.0
:pd.NO_ERROR
:pt.5
:pd.ERROR_INVALID_HANDLE
:pt.87
:pd.ERROR_INVALID_PARAMETER
:eparml.

@@WtkQueryMmfSize@REMARKS
This function will return the size of the memory buffer
including the last read or written byte.

@@
*/


APIRET APIENTRY WtkQueryMmfSize( HMMF hmmf, PVOID pvData, PULONG pulSize)
{
         APIRET         rc = NO_ERROR;
         PMMF           pmmf = (PMMF) hmmf;
         PMMFENTRY      pmmfe;

do
   {
   // check parms
   if ((!pvData) ||
       (!pulSize))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }
   *pulSize = 0;

   // is handle valid for this thread ?
   if (pmmf != __locateMMFHandler())
      {
      rc = ERROR_INVALID_HANDLE;
      break;
      }

   // search entry
   pmmfe = __locateMMFEntry( pmmf, pvData);
   if (!pmmfe)
      {
      rc = ERROR_INVALID_HANDLE;
      break;
      }

   // hand over result
   *pulSize = pmmfe->ulCurrentSize;

   } while (FALSE);

return rc;
}

// ---------------------------------------------------------------------------

/*
@@WtkGetMmfInfo@SYNTAX
This function retreieves information about the MMF Manager
instance for the current thread, if initialized.

@@WtkGetMmfInfo@PARM@pmi@out
The address of a buffer receiving the MMF Manager info.

@@WtkGetMmfInfo@RETURN
Return Code.
:p.
WtkGetMmfInfo returns one of the following return codes&colon.

:parml compact break=none.
:pt.0
:pd.NO_ERROR
:pt.2
:pd.ERROR_FILE_NOT_FOUND
:pt.87
:pd.ERROR_INVALID_PARAMETER
:eparml.

@@WtkGetMmfInfo@REMARKS
The MMF Manager info can only be retrieved if an instance was
created for the current thread by a previous call to 
:link reftype=hd viewport refid=WtkInitializeMmf.WtkInitializeMmf:elink..
:p.


@@
*/

APIRET APIENTRY WtkGetMmfInfo( PMMFINFO pmi)
{
         APIRET         rc = NO_ERROR;
         ULONG          i;
         PMMF           pmmf;
         PMMFENTRY      pmmfe;

do
   {
   // check parms
   if (!pmi)
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // determine handler for this thread
   pmmf = __locateMMFHandler();
   if (!pmmf)
      {
      rc = ERROR_FILE_NOT_FOUND;
      break;
      }

   // return data
   memset( pmi, 0, sizeof( MMFINFO));
   pmi->cbSize         = sizeof( MMFINFO);
   pmi->hmmf           = (HMMF) pmmf;
   pmi->ulMaxFiles     = pmmf->ulEntryCount;
   pmi->ulMaxInstances = MMF_MAXINSTANCES;

   // determine file buffer entries being used
   for (i = 0, pmmfe = pmmf->apmmfentry, pmi->ulFiles = 0;
        i < pmmf->ulEntryCount;
        i++, pmmfe++)
      {
      if (MMFENTRY_USED( pmmfe))
         pmi->ulFiles++;
      }

   // determine thread entries being used
   _fmutex_request(&fmtx_apmmf,0);
   for (i = 0, pmi->ulInstances = 0;
        i < MMF_MAXINSTANCES;
        i++, pmmf++)
      {
      pmmf = apmmf[ i];
      if ((pmmf) && (pmmf->apmmfentry))
         pmi->ulInstances++;
      }
   _fmutex_release(&fmtx_apmmf);

   } while (FALSE);

return rc;
}


APIRET APIENTRY WtkCommitMmf( HMMF hmmf, PVOID pvData, ULONG ulSize)
{
         APIRET         rc = NO_ERROR;
         PMMF           pmmf = (PMMF) hmmf;
         PMMFENTRY      pmmfe;
int i, fstart;

do
   {
   // check parms
   if ((!pvData) ||
       (!ulSize))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // is handle valid for this thread ?
   if (pmmf != __locateMMFHandler())
      {
      rc = ERROR_INVALID_HANDLE;
      break;
      }

   // search entry
   pmmfe = __locateMMFEntry( pmmf, pvData);
   if (!pmmfe)
      {
      rc = ERROR_INVALID_HANDLE;
      break;
      }

   // offset to file start position
   fstart = pvData - pmmfe->pvData;

   for( i = 0; i<ulSize; i+= 4096)
   {
      ULONG ulPageSize = PAG_SIZE;
      ULONG ulPageFlags = 0, ulPageFlags2 = 0;
      ULONG ulFilePtr = 0;

      // query commit and permission flags of affected page
      rc = DosQueryMem( pvData+i, &ulPageSize, &ulPageFlags);
      if (rc != NO_ERROR) {
         DPRINTF(( "WtkCommitMmf pvdata=0x%08x DosQueryMem failed rc=%d\n", pvData+i, rc));
         continue;
      }

      // allow writing
      if ((ulPageFlags & PAG_COMMIT) == 0)
         ulPageFlags2 |= PAG_COMMIT;
      if ((ulPageFlags & PAG_WRITE) == 0)
         ulPageFlags2 |= PAG_WRITE;
      if (ulPageFlags2 != 0) {
         rc = DosSetMem( pvData+i, PAG_SIZE, ulPageFlags2);
         if (rc != NO_ERROR) {
            DPRINTF(( "WtkCommitMmf pvdata=0x%08x DosSetMem failed rc=%d\n", pvData+i, rc));
            continue;
         }
      }

      // set file position
      rc = DosSetFilePtr( pmmfe->hfile, fstart + i, FILE_BEGIN, &ulFilePtr);
      // read page from disk into buffer
      rc = DosRead( pmmfe->hfile, pvData + i, PAG_SIZE, &ulFilePtr);

      // set proper access flag
      ulPageFlags = (MMFENTRY_ACCESS_READONLY(pmmfe) ? PAG_READ : PAG_READ|PAG_WRITE);
      rc = DosSetMem( pvData + i, PAG_SIZE, ulPageFlags);
      if (rc != NO_ERROR) {
         DPRINTF(( "WtkCommitMmf pvdata=0x%08x DosSetMem flags=%x failed rc=%d\n", pmmfe->pvData+i, ulPageFlags, rc));
         continue;
      }

   }

   } while (FALSE);

return rc;
}
