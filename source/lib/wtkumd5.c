/****************************** Module Header ******************************\
*
* Module Name: wtkumd5.c
*
* Source for MD5 related helper functions.
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2008
*
* $Id: wtkumd5.c,v 1.2 2008-02-03 22:45:04 cla Exp $
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
* This code is taken from the md5_os2.zip package from hobbes.
* Only the md5.c/.h is incorporated here. Modified parts are marked
* with "CHANGED".
*
\***************************************************************************/

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INCL_DOS
#define INCL_ERRORS
#include <os2.h>

#include "wtkumd5.h"
#include "wtkufil.h"
#include "wpstk.ih"

// ###########################################################################
// source: md5_os2.zip
// name:   src/md5.h
// date:   02.10.1991 (DD.MM.YYYY)
// time:   03:46      (24h)
// MD5:    391d7cd72454c243c485c1d90cc95637 md5.h
// ###########################################################################

/*
 ***********************************************************************
 ** md5.h -- header file for implementation of MD5                    **
 ** RSA Data Security, Inc. MD5 Message-Digest Algorithm              **
 ** Created: 2/17/90 RLR                                              **
 ** Revised: 12/27/90 SRD,AJ,BSK,JT Reference C version               **
 ** Revised (for MD5): RLR 4/27/91                                    **
 **   -- G modified to have y&~z instead of y&z                       **
 **   -- FF, GG, HH modified to add in last register done             **
 **   -- Access pattern: round 2 works mod 5, round 3 works mod 3     **
 **   -- distinct additive constant for each step                     **
 **   -- round 4 added, working mod 7                                 **
 ***********************************************************************
 */

/*
 ***********************************************************************
 ** Copyright (C) 1990, RSA Data Security, Inc. All rights reserved.  **
 **                                                                   **
 ** License to copy and use this software is granted provided that    **
 ** it is identified as the "RSA Data Security, Inc. MD5 Message-     **
 ** Digest Algorithm" in all material mentioning or referencing this  **
 ** software or this function.                                        **
 **                                                                   **
 ** License is also granted to make and use derivative works          **
 ** provided that such works are identified as "derived from the RSA  **
 ** Data Security, Inc. MD5 Message-Digest Algorithm" in all          **
 ** material mentioning or referencing the derived work.              **
 **                                                                   **
 ** RSA Data Security, Inc. makes no representations concerning       **
 ** either the merchantability of this software or the suitability    **
 ** of this software for any particular purpose.  It is provided "as  **
 ** is" without express or implied warranty of any kind.              **
 **                                                                   **
 ** These notices must be retained in any copies of any part of this  **
 ** documentation and/or software.                                    **
 ***********************************************************************
 */

/* typedef a 32-bit type */
typedef unsigned long int UINT4;

/* Data structure for MD5 (Message-Digest) computation */
typedef struct {
  UINT4 i[2];                   /* number of _bits_ handled mod 2^64 */
  UINT4 buf[4];                                    /* scratch buffer */
  unsigned char in[64];                              /* input buffer */
  unsigned char digest[16];     /* actual digest after MD5Final call */
} MD5_CTX;

// CHANGED: CLA:20080203
// - modified prototypes to include the parameters
// - made MD5 functions static
static void MD5Init( MD5_CTX *mdContext);
static void MD5Update( MD5_CTX *mdContext, unsigned char *inBuf, unsigned int inLen);
static void MD5Final( MD5_CTX *mdContext);

/*
 ***********************************************************************
 ** End of md5.h                                                      **
 ******************************** (cut) ********************************
 */


// ###########################################################################
// source: md5_os2.zip
// name:   src/md5.c
// date:   02.10.1991 (DD.MM.YYYY)
// time:   03:46      (24h)
// MD5:    30313b8196ce58ff45ddce51e3669042 md5.c
// ###########################################################################

/*
 ***********************************************************************
 ** md5.c -- the source code for MD5 routines                         **
 ** RSA Data Security, Inc. MD5 Message-Digest Algorithm              **
 ** Created: 2/17/90 RLR                                              **
 ** Revised: 1/91 SRD,AJ,BSK,JT Reference C ver., 7/10 constant corr. **
 ***********************************************************************
 */

/*
 ***********************************************************************
 ** Copyright (C) 1990, RSA Data Security, Inc. All rights reserved.  **
 **                                                                   **
 ** License to copy and use this software is granted provided that    **
 ** it is identified as the "RSA Data Security, Inc. MD5 Message-     **
 ** Digest Algorithm" in all material mentioning or referencing this  **
 ** software or this function.                                        **
 **                                                                   **
 ** License is also granted to make and use derivative works          **
 ** provided that such works are identified as "derived from the RSA  **
 ** Data Security, Inc. MD5 Message-Digest Algorithm" in all          **
 ** material mentioning or referencing the derived work.              **
 **                                                                   **
 ** RSA Data Security, Inc. makes no representations concerning       **
 ** either the merchantability of this software or the suitability    **
 ** of this software for any particular purpose.  It is provided "as  **
 ** is" without express or implied warranty of any kind.              **
 **                                                                   **
 ** These notices must be retained in any copies of any part of this  **
 ** documentation and/or software.                                    **
 ***********************************************************************
 */

// #include "md5.h" // CHANGED: CLA:20080203

/*
 ***********************************************************************
 **  Message-digest routines:                                         **
 **  To form the message digest for a message M                       **
 **    (1) Initialize a context buffer mdContext using MD5Init        **
 **    (2) Call MD5Update on mdContext and M                          **
 **    (3) Call MD5Final on mdContext                                 **
 **  The message digest is now in mdContext->digest[0...15]           **
 ***********************************************************************
 */

// CHANGED: CLA:20080203
// modified prototype to include the parameters
/* forward declaration */
static void Transform (UINT4 *buf, UINT4 *in);

static unsigned char PADDING[64] = {
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/* F, G, H and I are basic MD5 functions */
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

/* ROTATE_LEFT rotates x left n bits */
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

/* FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4 */
/* Rotation is separate from addition to prevent recomputation */
#define FF(a, b, c, d, x, s, ac) \
  {(a) += F ((b), (c), (d)) + (x) + (UINT4)(ac); \
   (a) = ROTATE_LEFT ((a), (s)); \
   (a) += (b); \
  }
#define GG(a, b, c, d, x, s, ac) \
  {(a) += G ((b), (c), (d)) + (x) + (UINT4)(ac); \
   (a) = ROTATE_LEFT ((a), (s)); \
   (a) += (b); \
  }
#define HH(a, b, c, d, x, s, ac) \
  {(a) += H ((b), (c), (d)) + (x) + (UINT4)(ac); \
   (a) = ROTATE_LEFT ((a), (s)); \
   (a) += (b); \
  }
#define II(a, b, c, d, x, s, ac) \
  {(a) += I ((b), (c), (d)) + (x) + (UINT4)(ac); \
   (a) = ROTATE_LEFT ((a), (s)); \
   (a) += (b); \
  }

/* The routine MD5Init initializes the message-digest context
   mdContext. All fields are set to zero.
 */
// CHANGED: CLA:20080203
// - made MD5 functions static
static void MD5Init (mdContext)
MD5_CTX *mdContext;
{
  mdContext->i[0] = mdContext->i[1] = (UINT4)0;

  /* Load magic initialization constants.
   */
  mdContext->buf[0] = (UINT4)0x67452301;
  mdContext->buf[1] = (UINT4)0xefcdab89;
  mdContext->buf[2] = (UINT4)0x98badcfe;
  mdContext->buf[3] = (UINT4)0x10325476;
}

/* The routine MD5Update updates the message-digest context to
   account for the presence of each of the characters inBuf[0..inLen-1]
   in the message whose digest is being computed.
 */
// CHANGED: CLA:20080203
// - made MD5 functions static
static void MD5Update (mdContext, inBuf, inLen)
MD5_CTX *mdContext;
unsigned char *inBuf;
unsigned int inLen;
{
  UINT4 in[16];
  int mdi;
  unsigned int i, ii;

  /* compute number of bytes mod 64 */
  mdi = (int)((mdContext->i[0] >> 3) & 0x3F);

  /* update number of bits */
  if ((mdContext->i[0] + ((UINT4)inLen << 3)) < mdContext->i[0])
    mdContext->i[1]++;
  mdContext->i[0] += ((UINT4)inLen << 3);
  mdContext->i[1] += ((UINT4)inLen >> 29);

  while (inLen--) {
    /* add new character to buffer, increment mdi */
    mdContext->in[mdi++] = *inBuf++;

    /* transform if necessary */
    if (mdi == 0x40) {
      for (i = 0, ii = 0; i < 16; i++, ii += 4)
        in[i] = (((UINT4)mdContext->in[ii+3]) << 24) |
                (((UINT4)mdContext->in[ii+2]) << 16) |
                (((UINT4)mdContext->in[ii+1]) << 8) |
                ((UINT4)mdContext->in[ii]);
      Transform (mdContext->buf, in);
      mdi = 0;
    }
  }
}

/* The routine MD5Final terminates the message-digest computation and
   ends with the desired message digest in mdContext->digest[0...15].
 */
// CHANGED: CLA:20080203
// - made MD5 functions static
static void MD5Final (mdContext)
MD5_CTX *mdContext;
{
  UINT4 in[16];
  int mdi;
  unsigned int i, ii;
  unsigned int padLen;

  /* save number of bits */
  in[14] = mdContext->i[0];
  in[15] = mdContext->i[1];

  /* compute number of bytes mod 64 */
  mdi = (int)((mdContext->i[0] >> 3) & 0x3F);

  /* pad out to 56 mod 64 */
  padLen = (mdi < 56) ? (56 - mdi) : (120 - mdi);
  MD5Update (mdContext, PADDING, padLen);

  /* append length in bits and transform */
  for (i = 0, ii = 0; i < 14; i++, ii += 4)
    in[i] = (((UINT4)mdContext->in[ii+3]) << 24) |
            (((UINT4)mdContext->in[ii+2]) << 16) |
            (((UINT4)mdContext->in[ii+1]) << 8) |
            ((UINT4)mdContext->in[ii]);
  Transform (mdContext->buf, in);

  /* store buffer in digest */
  for (i = 0, ii = 0; i < 4; i++, ii += 4) {
    mdContext->digest[ii] = (unsigned char)(mdContext->buf[i] & 0xFF);
    mdContext->digest[ii+1] =
      (unsigned char)((mdContext->buf[i] >> 8) & 0xFF);
    mdContext->digest[ii+2] =
      (unsigned char)((mdContext->buf[i] >> 16) & 0xFF);
    mdContext->digest[ii+3] =
      (unsigned char)((mdContext->buf[i] >> 24) & 0xFF);
  }
}

/* Basic MD5 step. Transforms buf based on in.
 */
static void Transform (buf, in)
UINT4 *buf;
UINT4 *in;
{
  UINT4 a = buf[0], b = buf[1], c = buf[2], d = buf[3];

  /* Round 1 */
#define S11 7
#define S12 12
#define S13 17
#define S14 22
  FF ( a, b, c, d, in[ 0], S11, 3614090360); /* 1 */
  FF ( d, a, b, c, in[ 1], S12, 3905402710); /* 2 */
  FF ( c, d, a, b, in[ 2], S13,  606105819); /* 3 */
  FF ( b, c, d, a, in[ 3], S14, 3250441966); /* 4 */
  FF ( a, b, c, d, in[ 4], S11, 4118548399); /* 5 */
  FF ( d, a, b, c, in[ 5], S12, 1200080426); /* 6 */
  FF ( c, d, a, b, in[ 6], S13, 2821735955); /* 7 */
  FF ( b, c, d, a, in[ 7], S14, 4249261313); /* 8 */
  FF ( a, b, c, d, in[ 8], S11, 1770035416); /* 9 */
  FF ( d, a, b, c, in[ 9], S12, 2336552879); /* 10 */
  FF ( c, d, a, b, in[10], S13, 4294925233); /* 11 */
  FF ( b, c, d, a, in[11], S14, 2304563134); /* 12 */
  FF ( a, b, c, d, in[12], S11, 1804603682); /* 13 */
  FF ( d, a, b, c, in[13], S12, 4254626195); /* 14 */
  FF ( c, d, a, b, in[14], S13, 2792965006); /* 15 */
  FF ( b, c, d, a, in[15], S14, 1236535329); /* 16 */

  /* Round 2 */
#define S21 5
#define S22 9
#define S23 14
#define S24 20
  GG ( a, b, c, d, in[ 1], S21, 4129170786); /* 17 */
  GG ( d, a, b, c, in[ 6], S22, 3225465664); /* 18 */
  GG ( c, d, a, b, in[11], S23,  643717713); /* 19 */
  GG ( b, c, d, a, in[ 0], S24, 3921069994); /* 20 */
  GG ( a, b, c, d, in[ 5], S21, 3593408605); /* 21 */
  GG ( d, a, b, c, in[10], S22,   38016083); /* 22 */
  GG ( c, d, a, b, in[15], S23, 3634488961); /* 23 */
  GG ( b, c, d, a, in[ 4], S24, 3889429448); /* 24 */
  GG ( a, b, c, d, in[ 9], S21,  568446438); /* 25 */
  GG ( d, a, b, c, in[14], S22, 3275163606); /* 26 */
  GG ( c, d, a, b, in[ 3], S23, 4107603335); /* 27 */
  GG ( b, c, d, a, in[ 8], S24, 1163531501); /* 28 */
  GG ( a, b, c, d, in[13], S21, 2850285829); /* 29 */
  GG ( d, a, b, c, in[ 2], S22, 4243563512); /* 30 */
  GG ( c, d, a, b, in[ 7], S23, 1735328473); /* 31 */
  GG ( b, c, d, a, in[12], S24, 2368359562); /* 32 */

  /* Round 3 */
#define S31 4
#define S32 11
#define S33 16
#define S34 23
  HH ( a, b, c, d, in[ 5], S31, 4294588738); /* 33 */
  HH ( d, a, b, c, in[ 8], S32, 2272392833); /* 34 */
  HH ( c, d, a, b, in[11], S33, 1839030562); /* 35 */
  HH ( b, c, d, a, in[14], S34, 4259657740); /* 36 */
  HH ( a, b, c, d, in[ 1], S31, 2763975236); /* 37 */
  HH ( d, a, b, c, in[ 4], S32, 1272893353); /* 38 */
  HH ( c, d, a, b, in[ 7], S33, 4139469664); /* 39 */
  HH ( b, c, d, a, in[10], S34, 3200236656); /* 40 */
  HH ( a, b, c, d, in[13], S31,  681279174); /* 41 */
  HH ( d, a, b, c, in[ 0], S32, 3936430074); /* 42 */
  HH ( c, d, a, b, in[ 3], S33, 3572445317); /* 43 */
  HH ( b, c, d, a, in[ 6], S34,   76029189); /* 44 */
  HH ( a, b, c, d, in[ 9], S31, 3654602809); /* 45 */
  HH ( d, a, b, c, in[12], S32, 3873151461); /* 46 */
  HH ( c, d, a, b, in[15], S33,  530742520); /* 47 */
  HH ( b, c, d, a, in[ 2], S34, 3299628645); /* 48 */

  /* Round 4 */
#define S41 6
#define S42 10
#define S43 15
#define S44 21
  II ( a, b, c, d, in[ 0], S41, 4096336452); /* 49 */
  II ( d, a, b, c, in[ 7], S42, 1126891415); /* 50 */
  II ( c, d, a, b, in[14], S43, 2878612391); /* 51 */
  II ( b, c, d, a, in[ 5], S44, 4237533241); /* 52 */
  II ( a, b, c, d, in[12], S41, 1700485571); /* 53 */
  II ( d, a, b, c, in[ 3], S42, 2399980690); /* 54 */
  II ( c, d, a, b, in[10], S43, 4293915773); /* 55 */
  II ( b, c, d, a, in[ 1], S44, 2240044497); /* 56 */
  II ( a, b, c, d, in[ 8], S41, 1873313359); /* 57 */
  II ( d, a, b, c, in[15], S42, 4264355552); /* 58 */
  II ( c, d, a, b, in[ 6], S43, 2734768916); /* 59 */
  II ( b, c, d, a, in[13], S44, 1309151649); /* 60 */
  II ( a, b, c, d, in[ 4], S41, 4149444226); /* 61 */
  II ( d, a, b, c, in[11], S42, 3174756917); /* 62 */
  II ( c, d, a, b, in[ 2], S43,  718787259); /* 63 */
  II ( b, c, d, a, in[ 9], S44, 3951481745); /* 64 */

  buf[0] += a;
  buf[1] += b;
  buf[2] += c;
  buf[3] += d;
}

/*
 ***********************************************************************
 ** End of md5.c                                                      **
 ******************************** (cut) ********************************
 */


// ###########################################################################

/*
@@WtkCalcMemMD5@SYNTAX
This function calculates a MD5 checksum for a memory buffer.
It uses the :hp2.RSA Data Security, Inc. MD5 Message-Digest Algorithm:ehp2.

@@WtkCalcMemMD5@PARM@pvData@in
Address of the buffer, for which the MD5 checksum is to be calculated.

@@WtkCalcMemMD5@PARM@ulDatalen@in
The length, in bytes, of the buffer described by :hp1.pvData:ehp1..

@@WtkCalcMemMD5@PARM@pmd5d@out
The address of a buffer, into which the MD5 digest for the data
is returned.

@@WtkCalcMemMD5@RETURN
Return Code.
:p.
WtkCalcMemMD5 returns one of the following return codes&colon.
:parml compact break=none.
:pt.0
:pd.NO_ERROR
:pt.87
:pd.ERROR_INVALID_PARAMETER
:eparml.

@@WtkCalcMemMD5@REMARKS
When WtkCalcMemMD5 is called to calculate a 32-bit CRC for one
memory buffer, the variable pointed to by pulCRC32 has to be set to -1.
:p.
If the CRC is to be calculated for multiple memory buffers by calling
WtkCalcMemMD5 several times, the variable pointed to by pulCRC32 has
to be set to -1 for the first call only. On consecutive calls, leave
the value as returned by the previous call to WtkCalcMemMD5.

@@
*/

APIRET APIENTRY WtkCalcMemMD5( PVOID pvData, ULONG ulDatalen, PMD5DIGEST pmd5d)
{
         APIRET         rc = NO_ERROR;
         MD5_CTX        mdContext;
do
   {
   // check parm
   if ((!pvData) ||
       (!pmd5d))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // initialize
   MD5Init( &mdContext);

   // update context with provided data
   MD5Update( &mdContext, pvData, ulDatalen);

   // finalize context
   MD5Final( &mdContext);

   // copy over digest
   memcpy( pmd5d, mdContext.digest, sizeof( mdContext.digest));

   } while (FALSE);

return rc;
}

// ---------------------------------------------------------------------------

/*
@@WtkCalcFileMD5@SYNTAX
This function calculates a 32-bit CRC for a given file.
It uses the :hp2.RSA Data Security, Inc. MD5 Message-Digest Algorithm:ehp2.

@@WtkCalcFileMD5@PARM@pszFilename@in
Address of the ASCIIZ filename, for which the 32-bit CRC is to be calculated.
:p.
The path name may contain :hp2.?&colon.:ehp2. for the drive in order to
read the file from the boot drive.

@@WtkCalcFileMD5@PARM@pmd5d@out
The address of a buffer, into which the MD5 digest for the file
is returned.

@@WtkCalcFileMD5@RETURN
Return Code.
:p.
WtkCalcFileMD5 returns one of the following return codes&colon.
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

@@WtkCalcFileMD5@REMARKS
This function supports boot drive recognition.
Specify :hp2.?&colon.:ehp2. for the drive in order to
read the file from the boot drive.

@@
*/

APIRET APIENTRY WtkCalcFileMD5( PSZ pszFilename, PMD5DIGEST pmd5d)
{
         APIRET         rc = NO_ERROR;
         ULONG          i;
         MD5_CTX        mdContext;

         PVOID          pvData = NULL;
         ULONG          ulDatalen = 0x1000;

         CHAR           szFilename[ _MAX_PATH];
         HFILE          hfile = -1;
         ULONG          ulAction;
         ULONG          ulBytesRead;

do
   {
   // check parm
   if ((!pszFilename) ||
       (!pmd5d))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // initialize
   MD5Init( &mdContext);

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
         // update context with provided data
         MD5Update( &mdContext, pvData, ulBytesRead);
      else
         break;
      }

   // finalize context
   MD5Final( &mdContext);

   // copy over digest
   memcpy( pmd5d, mdContext.digest, sizeof( mdContext.digest));

   } while (FALSE);

// cleanup
DosClose( hfile);
if (pvData) free( pvData);
return rc;
}

// ---------------------------------------------------------------------------

/*
@@WtkMD5DigestToStr@SYNTAX
This function converts a MD5 digest to a string.

@@WtkMD5DigestToStr@PARM@pszFilename@in
Address of the ASCIIZ filename, for which the 32-bit CRC is to be calculated.
:p.
The path name may contain :hp2.?&colon.:ehp2. for the drive in order to
read the file from the boot drive.

@@WtkMD5DigestToStr@PARM@pmd5d@in
The address of a buffer, containing the MD5 digest.

@@WtkMD5DigestToStr@PARM@pszBuffer@out
The address of a buffer, receiving the string.

@@WtkMD5DigestToStr@PARM@ulBuflen@in
The length, in bytes, of the buffer described by :hp1.pszBuffer:ehp1..

@@WtkMD5DigestToStr@RETURN
Return Code.
:p.
WtkMD5DigestToStr returns one of the following return codes&colon.
:parml compact break=none.
:pt.0
:pd.NO_ERROR
:pt.87
:pd.ERROR_INVALID_PARAMETER
:pt.111
:pd.ERROR_BUFFER_OVERFLOW
:eparml.

@@WtkMD5DigestToStr@REMARKS
This function supports boot drive recognition.
Specify :hp2.?&colon.:ehp2. for the drive in order to
read the file from the boot drive.

@@
*/

APIRET APIENTRY WtkMD5DigestToStr( PMD5DIGEST pmd5d, PSZ pszBuffer, ULONG ulBuflen)
{
         APIRET         rc = NO_ERROR;
         ULONG          i;
         PBYTE          p;
static   CHAR           szDigest[ 33];

do
   {
   // check parm
   if ((!pmd5d) ||
       (!pszBuffer))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // convert
   memset( szDigest, 0, sizeof( szDigest));
   for (i = 0, p = (PBYTE)pmd5d; i < sizeof( MD5DIGEST); i++, p++)
      {
      sprintf( &szDigest[ strlen( szDigest)], "%x", *p);
      }

   // check buflen
   if (strlen( szDigest) + 1 > ulBuflen)
      {
      rc = ERROR_BUFFER_OVERFLOW;
      break;
      }

   // hand over result
   strcpy( pszBuffer, szDigest);

   } while (FALSE);

return rc;
}

