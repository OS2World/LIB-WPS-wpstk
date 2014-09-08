/****************************** Module Header ******************************\
*
* Module Name: wtkmmf.h
*
* include file for memory mapped files manager functions.
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: wtkmmf.h,v 1.9 2008-10-22 23:39:58 cla Exp $
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

#ifndef WTKMMF_INCLUDED
#define WTKMMF_INCLUDED Memory Mapped Files manager functions

#ifdef __cplusplus
      extern "C" {
#endif

/* flags for MmfAlloc parm ulOpenFlags           */
/* NOTE:                                         */
/*  - for all except MMF_ACCESS_READWRITE,       */
/*    only write by others is denied, otherwise  */
/*    both read and write by others is denied    */
/*  - when using MMF_ACCESS_READONLY, the buffer */
/*    still can be written, but the file cannot  */
/*    be updated                                 */

#define MMF_ACCESS_READONLY     0x00000000
#define MMF_ACCESS_WRITEONLY    0x00000001
#define MMF_ACCESS_READWRITE    0x00000002

#define MMF_UPDATE_RELEASEMEM   0x00000000
#define MMF_UPDATE_KEEPMEM      0x00000010

#define MMF_MEMORY_READWRITE    0x00000000
#define MMF_MEMORY_READONLY     0x00000100
#define MMF_MEMORY_EXECUTE      0x00000200

#define MMF_OPENMODE_OPENFILE   0x00000000
#define MMF_OPENMODE_RESETFILE  0x00010000

#define MMF_ALLOC_LOWMEM        0x00000000
#define MMF_ALLOC_HIGHMEM       0x01000000

/* some sizes for usage with MmfAlloc parameter ulMaxSize */
#define MMF_MAXSIZE_KB             1024
#define MMF_MAXSIZE_MB          1048576

/* special NULL filename for MmfAlloc parameter pszFilename */
#define MMF_FILE_INMEMORY       NULL


/* define a handle type */
typedef LHANDLE HMMF;
typedef HMMF *PHMMF;

/* data structures */
typedef struct _MMFINFO {
  ULONG          cbSize;         /* size of stucture                             */
  HMMF           hmmf;           /* handle to MMF manager for current thread     */
  ULONG          ulFiles;        /* file buffers currently in use                */
  ULONG          ulMaxFiles;     /* file buffers requested with WtkInitializeMmf */
  ULONG          ulInstances;    /* open MMF Manager instances                   */
  ULONG          ulMaxInstances; /* maximum number of MMF Manager instances      */
} MMFINFO, *PMMFINFO;


/* prototypes */
APIRET APIENTRY WtkInitializeMmf( PHMMF phmmf, ULONG ulMaxFiles);
APIRET APIENTRY WtkTerminateMmf( HMMF hmmf);
APIRET APIENTRY WtkGetMmfInfo( PMMFINFO pmi);

APIRET APIENTRY WtkAllocMmf( HMMF hmmf, PVOID *ppvdata, PSZ pszFilename, ULONG ulOpenFlags, ULONG ulMaxSize);
APIRET APIENTRY WtkAllocMmfFile( HMMF hmmf, PVOID *ppvdata, HFILE hfile, ULONG ulOpenFlags, ULONG ulMaxSize);
APIRET APIENTRY WtkAllocMmfFileEx( HMMF hmmf, PVOID *ppvdata, HFILE hfile, ULONG ulOpenFlags, ULONG ulMaxSize, ULONG ulOffset);
APIRET APIENTRY WtkFreeMmf( HMMF hmmf, PVOID pvData);
APIRET APIENTRY WtkFreeMmfFile( HMMF hmmf, PVOID pvData);
APIRET APIENTRY WtkUpdateMmf( HMMF hmmf, PVOID pvData);
APIRET APIENTRY WtkRevertMmf( HMMF hmmf, PVOID pvData);

APIRET APIENTRY WtkSetMmfSize( HMMF hmmf, PVOID pvData, ULONG ulNewSize);
APIRET APIENTRY WtkQueryMmfSize( HMMF hmmf, PVOID pvData, PULONG pulSize);

APIRET APIENTRY WtkCommitMmf( HMMF hmmf, PVOID pvData, ULONG ulSize);


#ifdef __cplusplus
        }
#endif

#endif /* WTKMMF_INCLUDED */

