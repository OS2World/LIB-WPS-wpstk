/****************************** Module Header ******************************\
*
* Module Name: wtkucrc.c
*
* Source for CRC related helper functions.
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: wtkucrc.c,v 1.7 2008-02-03 22:17:24 cla Exp $
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
* This code is taken from the crc.zip package from hobbes.
* Only the CRC32 code is being incorporated here. Further
* this code is modified, so that the CRC is not calculated
* from a file, but from a memory buffer.
*
* WtkCalcMemCRC32 is designed to calculate a CRC32 within
* multiple calls, so the first initialization of the CRC
* value with -1 has to be done by the caller of WtkCalcMemCRC32.
*
* Excerpt from crc.c:
*
*   CRC 0.1, Public domain
*   Radim Kolar 2:423/66.111@FidoNet.Org
*
\***************************************************************************/

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INCL_DOS
#define INCL_WINSTDCNR
#define INCL_ERRORS
#include <os2.h>

#include "wtkucrc.h"
#include "wpstk.ih"

// global vars
static   ULONG          aulCrc32Tab[256];
static   BOOL           fTabInitialized = FALSE;

// ---------------------------------------------------------------------------

static VOID _CrcTabInit( VOID)
{
         INT            i,n;
         ULONG          ulCRC32;

for (i=0; i<256; i++)
   {
   ulCRC32=i;
   for (n=1; n < 9; n++)
      {
      if (ulCRC32 & 1)
         ulCRC32 = (ulCRC32 >> 1) ^ /* XOR */ 0xedb88320;
      else
         ulCRC32 = ulCRC32 >> 1;
      }
   aulCrc32Tab[i] = ulCRC32;
   }

fTabInitialized = TRUE;
}

// ---------------------------------------------------------------------------

/*
@@WtkCalcMemCRC32@SYNTAX
This function calculates a 32-bit CRC for a memory buffer.
A CRC can also be calculated in multiple calls.
.br
This code has been adapted from Radim Kolars crc.c of CRC V0.1.

@@WtkCalcMemCRC32@PARM@pvData@in
Address of the buffer, for which the 32-bit CRC is to be calculated.

@@WtkCalcMemCRC32@PARM@ulDatalen@in
The length, in bytes, of the buffer described by :hp1.pvData:ehp1..

@@WtkCalcMemCRC32@PARM@pulCRC32@inout
The address of a variable for the starting value for the CRC
calculation and storage for the resulting 32-bit CRC.

:parml break=none.
:pt.Input
:pd.starting value for CRC calculation. On a first call, set this value to -1.
On consecutive calls, leave the value as returned by the previous call to
WtkCalcMemCRC32.
:pt.Output
:pd.resulting CRC value
:eparml.

@@WtkCalcMemCRC32@RETURN
Return Code.
:p.
WtkCalcMemCRC32 returns one of the following return codes&colon.
:parml compact break=none.
:pt.0
:pd.NO_ERROR
:pt.87
:pd.ERROR_INVALID_PARAMETER
:pt.111
:pd.ERROR_BUFFER_OVERFLOW
:pt.123
:pd.ERROR_INVALID_NAME
:eparml.

@@WtkCalcMemCRC32@REMARKS
When WtkCalcMemCRC32 is called to calculate a 32-bit CRC for one
memory buffer, the variable pointed to by pulCRC32 has to be set to -1.
:p.
If the CRC is to be calculated for multiple memory buffers by calling
WtkCalcMemCRC32 several times, the variable pointed to by pulCRC32 has
to be set to -1 for the first call only. On consecutive calls, leave
the value as returned by the previous call to WtkCalcMemCRC32.

@@
*/

APIRET APIENTRY WtkCalcMemCRC32( PVOID pvData, ULONG ulDatalen, PULONG pulCRC32)
{
         APIRET         rc = NO_ERROR;
         ULONG          i;
         ULONG          ulCRC;    // pass -1 as *pulCRC32 on first entry
         PCHAR          pchChar;
         USHORT         usChar;

do
   {
   // check parm
   if ((pvData   == NULL) ||
       (pulCRC32 == NULL))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }
   ulCRC = *pulCRC32;
   if (ulCRC != -1)
      ulCRC = ulCRC ^ 0xffffffffL;


   // initialize
   if (!fTabInitialized)
      _CrcTabInit();

   // calc CRC32
   for (i = 0, pchChar = pvData; i < ulDatalen; i++, pchChar++)
     {
     usChar = *pchChar;
     ulCRC =aulCrc32Tab[(ulCRC ^ usChar) & 0xFF] ^ ((ulCRC>>8) & 0x00ffffffL);
     }
   ulCRC = ulCRC ^ 0xffffffffL;

   // hand over result
   *pulCRC32 = ulCRC;

   } while (FALSE);

return rc;

}

// ---------------------------------------------------------------------------

/*
@@WtkCalcFileCRC32@SYNTAX
This function calculates a 32-bit CRC for a given file.

@@WtkCalcFileCRC32@PARM@pszFilename@in
Address of the ASCIIZ filename, for which the 32-bit CRC is to be calculated.
:p.
The path name may contain :hp2.?&colon.:ehp2. for the drive in order to
read the file from the boot drive.

@@WtkCalcFileCRC32@PARM@pulCRC32@out
The address of a variable, into which the 32-bit CRC for the file
is returned.

@@WtkCalcFileCRC32@RETURN
Return Code.
:p.
WtkCalcFileCRC32 returns one of the following return codes&colon.
:parml compact break=none.
:pt.0
:pd.NO_ERROR
:pt.8
:pd.ERROR_NOT_ENOUGH_MEMORY
:pt.87
:pd.ERROR_INVALID_PARAMETER
:eparml.
:p.
or return codes of the following functions
:ul compact.
:li.DosOpen
:eul.

@@WtkCalcFileCRC32@REMARKS
This function supports boot drive recognition.
Specify :hp2.?&colon.:ehp2. for the drive in order to
read the file from the boot drive.

@@
*/

APIRET APIENTRY WtkCalcFileCRC32( PSZ pszFilename, PULONG pulCRC32)
{
         APIRET         rc = NO_ERROR;
         ULONG          i;
         ULONG          ulCRC = -1;

         PVOID          pvData = NULL;
         ULONG          ulDatalen = 0x1000;

         CHAR           szFilename[ _MAX_PATH];
         HFILE          hfile = -1;
         ULONG          ulAction;
         ULONG          ulBytesRead;

do
   {
   // check parm
   if ((pszFilename == NULL) ||
       (pulCRC32    == NULL))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // initialize
   if (!fTabInitialized)
      _CrcTabInit();

   // get memory for buffer
   pvData = malloc( ulDatalen);
   if (!pvData)
      {
      rc = ERROR_NOT_ENOUGH_MEMORY;
      break;
      }

   // copy name and replace ?: with bootdrive
   strcpy( szFilename, pszFilename);
   __PatchBootDrive( szFilename);

   // open file
   rc = DosOpen( szFilename,
                 &hfile,
                 &ulAction,
                 0, 0,
                 OPEN_ACTION_FAIL_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS,
                 OPEN_FLAGS_FAIL_ON_ERROR | OPEN_FLAGS_SEQUENTIAL | OPEN_SHARE_DENYWRITE,
                 NULL);
   if (rc != NO_ERROR)
      break;

   // read data and calculate CRC
   ulBytesRead = -1;
   while (TRUE)
      {
      // read next chunk ...
      rc = DosRead( hfile,
                    pvData,
                    ulDatalen,
                    &ulBytesRead);

      // ... and take it into account for the CRC
      if ((rc == NO_ERROR) && (ulBytesRead != 0))
         WtkCalcMemCRC32( pvData, ulBytesRead, &ulCRC);
      else
         break;
      }

   // hand over result
   *pulCRC32 = ulCRC;

   } while (FALSE);

// cleanup
DosClose( hfile);
if (pvData) free( pvData);
return rc;

}

