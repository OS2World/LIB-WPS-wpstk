/****************************** Module Header ******************************\
*
* Module Name: _mmf.c
*
* memory mapped files manager sample
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: _mmf.c,v 1.15 2008-10-24 00:54:29 cla Exp $
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
#include <time.h>

#define INCL_ERRORS
#define INCL_DOSMISC
#define INCL_DOSPROCESS
#include <os2.h>

#define INCL_WTKMMF
#define INCL_WTKUTLFILE
#define INCL_WTKUTLTIME
#include <wtk.h>

// wait in secs before closing the tmp file
// do show that the file stamp is set to the timestamp
// of the update, not of the file close
// value must be 2 secs minimum !
#define WAIT_BEFORE_CLOSEFILE 2

// -----------------------------------------------------------------------------

typedef ULONG FNDEMOFUNC( ULONG ulParm1, ULONG ulParm2);
typedef FNDEMOFUNC *PFNDEMOFUNC;

static ULONG _demoFunc( ULONG ulParm1, ULONG ulParm2)
{
// don't call any other functions here as this would require fixups
// of all calls to any other routine !

// so we just calculate a bit !

return ulParm1 + ulParm2;
}

// -----------------------------------------------------------------------------

static time_t _printTimestamp2Secs( PSZ pszMessage)
{
         time_t         timeResult = 0;
         DATETIME       dt;

DosGetDateTime( &dt);

// adjust seconds to two seconds to 
// make it comparable to the seconds 
// of a file timestamp
dt.seconds = (dt.seconds / 2) * 2;

printf( "%s: %u.%02u.%02u\n", 
         pszMessage,
         dt.hours,
         dt.minutes,
         dt.seconds);
WtkDateTimeToTime( &dt, &timeResult);
return timeResult;
}

// -----------------------------------------------------------------------------

static time_t _printFileTimestamp( PSZ pszMessage, PSZ pszFile)
{
         time_t         timeResult = 0;
         APIRET         rc;
         FILESTATUS3    fs3;
rc = DosQueryPathInfo( pszFile, FIL_STANDARD, &fs3, sizeof( fs3));
if (rc == NO_ERROR)
   {
   printf( "%s: %u.%02u.%02u\n", 
            pszMessage,
            fs3.ftimeLastWrite.hours,
            fs3.ftimeLastWrite.minutes,
            fs3.ftimeLastWrite.twosecs * 2);
   WtkFDateTimeToTime( &fs3.fdateLastWrite, &fs3.ftimeLastWrite, &timeResult);
   }

return timeResult;
}

// -----------------------------------------------------------------------------

#define PRINTSEPARATOR printf("\n------------------------------------------\n")

int main ( int argc, char *argv[])
{

         APIRET         rc = NO_ERROR;
         ULONG          i;
         PSZ            p;

         HMMF           hmmf = NULLHANDLE;
         HMMF           hmmfCopy;
         MMFINFO        mi;

         CHAR           szFile[ _MAX_PATH];
         PSZ            pszMemory = NULL;
         PSZ            pszFileContents = NULL;
         ULONG          ulFilesize = 32 * MMF_MAXSIZE_KB;
         ULONG          ulCurrentSize;
         ULONG          ulBytesWritten;

         time_t         timeUpdate;
         time_t         timeFile;
         ULONG          ulTimeDiff;

         HFILE          hfile = NULLHANDLE;
         ULONG          ulAction;

         ULONG          ulCodeSize;
         PFNDEMOFUNC    pfn_demoFunc;

static   PSZ            pszFile = "\?:\\CONFIG.SYS";
static   PSZ            pszDummyText = "This is the WPSTK testcase for memory mapped files !\r\n";

#define FILEHEAD_MAXLINES 10

do
   {
   // --------------------------------------------------------------------------------

   PRINTSEPARATOR;

   // ----------  initialize ------------------------------

   printf( "initialize\n");
   printf( "- WtkInitializeMmf: initialize support for memory mapped files\n");
   rc = WtkInitializeMmf( &hmmf, 4);
   if (rc != NO_ERROR)
      {
      printf( " WtkInitializeMmf: error: cannot initialize MMF support, rc=%u\n", rc);
      break;
      }
   printf( "- WtkInitializeMmf: retry initialization, must yield error and return same handle\n");
   rc = WtkInitializeMmf( &hmmfCopy, 4);
   if (rc != ERROR_ACCESS_DENIED)
      {
      printf( " WtkInitializeMmf: error: reinitialization not rejected, must return rc=%u, returns rc=%u\n",
              ERROR_ACCESS_DENIED, rc);
      break;
      }
   if (hmmf != hmmfCopy)
      {
      printf( " WtkInitializeMmf: error: reinitialization does not return MMF handle\n");
      rc = ERROR_INVALID_DATA;
      break;
      }

   // --------------------------------------------------------------------------------

   PRINTSEPARATOR;

   printf( "Testing in-memory pseudo file\n");

   // ----------  create in-memory only file --------------

   rc = WtkAllocMmf( hmmf, (PVOID*) &pszMemory, MMF_FILE_INMEMORY,
                     MMF_ACCESS_READWRITE |
                     MMF_MEMORY_READWRITE |
                     MMF_ALLOC_HIGHMEM,
                     ulFilesize);
   printf( "- WtkAllocMmf: allocate in-memory buffer at 0x%08x\n", pszMemory);
   if (rc != NO_ERROR)
      {
      printf( " WtkAllocMmf:  error: cannot allocate in-memory buffer, rc=%u\n", rc);
      break;
      }

   // set memory to all hashmarks
   printf( "- access dynamic memory buffer, writing %u bytes\n", ulFilesize);
   memset( pszMemory, '#', ulFilesize);
   printf( "- no trap so far !\n");

   // --------------------------------------------------------------------------------

   PRINTSEPARATOR;

   printf( "Testing readwrite file access\n");

   // ----------  write dummy file --------------

   // allocate test file
   sprintf( szFile, "%s\\mmftest.txt", getenv( "TMP"));
   rc = WtkAllocMmf( hmmf, (PVOID*) &pszFileContents, szFile,
                     MMF_ACCESS_READWRITE   |
                     MMF_OPENMODE_RESETFILE |
                     MMF_MEMORY_READWRITE   |
                     MMF_UPDATE_RELEASEMEM  |
                     MMF_ALLOC_HIGHMEM,
                     ulFilesize);
   printf( "- WtkAllocMmf: allocate readwrite file buffer at 0x%08x for: %s\n", pszFileContents, szFile);
   if (rc != NO_ERROR)
      {
      printf( " WtkAllocMmf: error: cannot allocate memory mapped file %s, rc=%u\n", szFile, rc);
      break;
      }

   // access memory by copying the contents of the in-memory file
   // but leave out first 64 bytes
   printf( "- access dynamic memory buffer of file, writing %u bytes\n", ulFilesize - 64);
   strcpy( pszFileContents, pszDummyText);
   memcpy( pszFileContents + 64, pszMemory, ulFilesize - 64);
   printf( "- no trap so far !\n");

   // cut size to half of size
   ulFilesize /= 2;
   printf( "- WtkSetMmfSize: cut size to %u\n", ulFilesize);
   rc = WtkSetMmfSize( hmmf, pszFileContents, ulFilesize);
   if (rc != NO_ERROR)
      {
      printf( " WtkSetMmfSize: error: cannot set size of memory buffer for file %s, rc=%u\n", szFile, rc);
      break;
      }

   // query size of memory area
   rc = WtkQueryMmfSize( hmmf, pszFileContents, &ulCurrentSize);
   if (rc != NO_ERROR)
      {
      printf( " WtkQueryMmfSize: error: cannot query size of allocated memory for memory mapped file %s, rc=%u\n", szFile, rc);
      break;
      }
   printf( "- WtkQueryMmfSize: file area size is %u\n", ulCurrentSize);

   // update the file
   printf( "- WtkUpdateMmf: update file\n");
   rc = WtkUpdateMmf( hmmf, pszFileContents);
   if (rc != NO_ERROR)
      {
      printf( " WtkUpdateMmf: error: cannot update: %s\n", szFile);
      break;
      }
  timeUpdate = _printTimestamp2Secs( "- update at");

   // modify the file buffer - should reallocate the page
   // this is only visible in debug mode with DEBUG_DUMPHANDLERACTIONS
   // active in wtkmmf.c
   printf( "- modify buffer again after update having released memory\n");
   strcpy( pszFileContents, pszDummyText);

   // query file size with file API
   printf( "- WtkQueryFileSize: checked size of updated file is %u\n", WtkQueryFileSize( szFile));

   // --------------------------------------------------------------------------------

   PRINTSEPARATOR;

   printf( "Query MMF info\n");
   rc = WtkGetMmfInfo( &mi);
   if (rc != NO_ERROR)
      {
      printf( " WtkGetMmfInfo: error: cannot get MMF info, rc=%u\n", rc);
      break;
      }
   printf( "\n"
           "MMF handle:               0x%08x\n"
           "file bufffers in use:     %u\n"
           "max file buffers:         %u\n"
           "Manager Instances in use: %u\n"
           "max Manager Instances:    %u\n",
           mi.hmmf,
           mi.ulFiles,
           mi.ulMaxFiles,
           mi.ulInstances,
           mi.ulMaxInstances);


   // --------------------------------------------------------------------------------

   PRINTSEPARATOR;

   printf( "Release dynamic memory buffers used until here\n");

   // free memory buffer
   printf( "- WtkFreeMmf: free memory-only buffer\n");
   rc = WtkFreeMmf( hmmf, pszMemory);
   if (rc != NO_ERROR)
      {
      printf( " WtkFreeMmf: error: cannot free memory of memory-only buffer\n", szFile);
      break;
      }


   // wait for some secs before closing file
   printf( "- wait %u secs\n", WAIT_BEFORE_CLOSEFILE);
   fflush( stdout);
   DosSleep( WAIT_BEFORE_CLOSEFILE * 1000);

   // free memory and file again
   printf( "- WtkFreeMmf: free file buffer\n");
   rc = WtkFreeMmf( hmmf, pszFileContents);
   if (rc != NO_ERROR)
      {
      printf( " WtkFreeMmf: error: cannot free memory for: %s\n", szFile);
      break;
      }

   // filestamp must be near that of after update
   _printTimestamp2Secs( "- file close at");
   timeFile = _printFileTimestamp( "- filestamp is", szFile);
   printf( "- timestamps: update: 0x%08x file: 0x%08x\n", timeUpdate, timeFile);

   if (timeUpdate != timeFile)
      {
      printf( " error: update and file timestamp are not equal!\n");
      rc = ERROR_INVALID_DATA;
      break;
      }
   else
   printf( "  timestamps are equal as required\n");

   // --------------------------------------------------------------------------------

   PRINTSEPARATOR;

   printf( "Reading file\n");

   // ----------  read config sys file --------------

   // open up config.sys
   rc = WtkAllocMmf( hmmf, (PVOID*) &pszFileContents, pszFile, 
                     MMF_ACCESS_READONLY   |
                     MMF_OPENMODE_OPENFILE |
                     MMF_MEMORY_READONLY   |
                     MMF_ALLOC_HIGHMEM,
                     1024*1024);
   printf( "- WtkAllocMmf: allocate readonly file buffer at 0x%08x for %s\n", pszFileContents, pszFile);
   if (rc != NO_ERROR)
      {
      printf( " WtkAllocMmf: error: cannot allocate memory mapped file %s, rc=%u\n", pszFile, rc);
      break;
      }

   // display length of config.sys
   printf( "- strlen: contents of file is %u bytes long\n", strlen( pszFileContents));

   // free memory and file again
   printf( "- WtkFreeMmf: free file area\n");
   rc = WtkFreeMmf( hmmf, pszFileContents);
   if (rc != NO_ERROR)
      {
      printf( " WtkFreeMmf: error: cannot free memory for: %s\n", pszFile);
      break;
      }

   // --------------------------------------------------------------------------------

   PRINTSEPARATOR;

   printf( "mapping opened file\n");

   // ----------  open config sys file --------------

   WtkQueryFullname( pszFile, szFile, sizeof( szFile));
   printf( "- DosOpen: open file readonly: %s\n", szFile);
   // open the file and read it
   rc = DosOpen( szFile,
                 &hfile,
                 &ulAction,
                 0,
                 FILE_NORMAL,
                 OPEN_ACTION_FAIL_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS,
                 OPEN_ACCESS_READONLY | OPEN_SHARE_DENYWRITE,
                 (PEAOP2)NULL);
   if (rc != NO_ERROR)
      {
      printf( " DosOpen: error: cannot open file, rc=%u\n", rc);
      break;
      }

   // mapping open file with non-matching access mode
   rc = WtkAllocMmfFile( hmmf, (PVOID*) &pszFileContents, hfile,
                         MMF_ACCESS_READWRITE  |
                         MMF_OPENMODE_OPENFILE |
                         MMF_MEMORY_READWRITE  |
                         MMF_ALLOC_HIGHMEM,
                         1024*1024);
   printf( "- WtkAllocMmfFile: map open file with non-matching access mode, must yield error\n");
   if (rc != ERROR_ACCESS_DENIED )
      {
      printf( " WtkAllocMmfFile: error: unmatched access flag not rejected, must return rc=%u, returns rc=%u\n",
              ERROR_ACCESS_DENIED, rc);

      break;
      }
   printf( "  WtkAllocMmfFile yields rc=%u as expected\n", rc);


   // mapping open file
   rc = WtkAllocMmfFile( hmmf, (PVOID*) &pszFileContents, hfile,
                         MMF_ACCESS_READONLY   |
                         MMF_OPENMODE_OPENFILE |
                         MMF_MEMORY_READWRITE  |
                         MMF_ALLOC_HIGHMEM,
                         1024*1024);
   printf( "- WtkAllocMmfFile: map open file to buffer at 0x%08x\n", pszFileContents);
   if (rc != NO_ERROR)
      {
      printf( " WtkAllocMmfFile: error: cannot map file %s, rc=%u\n", szFile, rc);
      break;
      }

   // display length of config.sys
   printf( "- strlen: contents of file is %u bytes long\n", strlen( pszFileContents));

   // patch contents for display of head  of file
   printf( "- patch file buffer, strip off CR chars of first %u lines\n",
           FILEHEAD_MAXLINES);
   for (p = pszFileContents, i = 0;
        ((p) && (i < FILEHEAD_MAXLINES));
        i++)
       {
       // search end of line
       p = strchr( p, 13);
       if (p) p++;
       }
   // cut off after ten lines
   if (p)
      *(p + 1) = 0;

   // eliminate CR chars
   p = pszFileContents;
   while (*p)
      {
      if (*p == 13)
         strcpy( p, p + 1);
      p++;
      }

   // display head of config.sys
   printf( "- display %u first lines of file\n"
           ">>>>>>>>>>>\n"
           "%s\n"
           "<<<<<<<<<<<\n",
           FILEHEAD_MAXLINES,
           pszFileContents);

   // free memory again - this does not close the file !!!
   printf( "- WtkFreeMmfFile: free file area\n");
   rc = WtkFreeMmfFile( hmmf, pszFileContents);
   if (rc != NO_ERROR)
      {
      printf( " WtkFreeMmfFile: error: cannot free memory for: %s\n", szFile);
      break;
      }


   // close handle here
   printf( "- DosClose: close file again\n");
   rc = DosClose( hfile); 
   if (rc != NO_ERROR)
      {
      printf( " DosClose: error: cannot close file, rc=%u\n", rc);
      break;
      }
   hfile = NULLHANDLE;

   // --------------------------------------------------------------------------------

   PRINTSEPARATOR;

   printf( "creating execute buffer\n");

   // ----------  create memory-only buffer for testing execute access --------------
   rc = WtkAllocMmf( hmmf, (PVOID*) &pszMemory, MMF_FILE_INMEMORY,
                     MMF_MEMORY_EXECUTE | MMF_ALLOC_HIGHMEM, 100 * 4096);
   printf( "- WtkAllocMmf: allocate in-memory buffer at 0x%08x\n", pszMemory);
   if (rc != NO_ERROR)
      {
      printf( " WtkAllocMmf:  error: cannot allocate in-memory buffer, rc=%u\n", rc);
      break;
      }

   // copy code to buffer
   ulCodeSize = (PBYTE)&main - (PBYTE)&_demoFunc;
   memcpy( pszMemory, (PBYTE)&_demoFunc, ulCodeSize);

   // call function in buffer
   pfn_demoFunc = (PFNDEMOFUNC)pszMemory;
   printf( "- calling _demoFunc at: 0x%08x\n", pfn_demoFunc);
   i = pfn_demoFunc( 2, 3);

   // free memory buffer
   printf( "- WtkFreeMmf: free memory-only buffer\n");
   rc = WtkFreeMmf( hmmf, pszMemory);
   if (rc != NO_ERROR)
      {
      printf( " WtkFreeMmf: error: cannot free memory of memory-only buffer\n", szFile);
      break;
      }

   // --------------------------------------------------------------------------------

   PRINTSEPARATOR;

   // ----------  create very small file ------------------

   sprintf( szFile, "%s\\mmfsmall.txt", getenv( "TMP"));
   rc = WtkAllocMmf( hmmf, (PVOID*) &pszFileContents, szFile,
                     MMF_ACCESS_READWRITE   |
                     MMF_OPENMODE_RESETFILE |
                     MMF_MEMORY_READWRITE   |
                     MMF_UPDATE_RELEASEMEM  |
                     MMF_ALLOC_HIGHMEM,
                     strlen( pszDummyText));
   printf( "- WtkAllocMmf: allocate readwrite file buffer at 0x%08x for: %s\n", pszFileContents, szFile);
   if (rc != NO_ERROR)
      {
      printf( " WtkAllocMmf: error: cannot allocate memory mapped file %s, rc=%u\n", szFile, rc);
      break;
      }

   // write buffer
   strcpy( pszFileContents, pszDummyText);

   // update the file
   printf( "- WtkUpdateMmf: update file\n");
   rc = WtkUpdateMmf( hmmf, pszFileContents);
   if (rc != NO_ERROR)
      {
      printf( " WtkUpdateMmf: error: cannot update: %s\n", szFile);
      break;
      }

   // free memory and file again
   printf( "- WtkFreeMmf: free file buffer\n");
   rc = WtkFreeMmf( hmmf, pszFileContents);
   if (rc != NO_ERROR)
      {
      printf( " WtkFreeMmf: error: cannot free memory of readwrite file buffer\n", szFile);
      break;
      }

   } while (FALSE);

// cleanup
PRINTSEPARATOR;
printf( "cleanup\n");
if (hfile) DosClose( hfile);
if (hmmf)
   {
   printf( "- WtkTerminateMmf: terminate support for memory mapped files\n");
   WtkTerminateMmf( hmmf);
   }
printf( "\n");

return rc;
}
  
