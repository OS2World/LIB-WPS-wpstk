/****************************** Module Header ******************************\
*
* Module Name: wtkfbmp.c
*
* Source for access functions for bitmap files
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: wtkfbmp.c,v 1.5 2005-01-25 19:19:13 cla Exp $
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
#include <ctype.h>

#define INCL_DOS
#define INCL_GPI
#define INCL_ERRORS
#include <os2.h>

#include "wtkfbmp.h"
#include "wtkufil.h"
#include "wpstk.ih"

// file macros
#define READVAR(h,o,v)   _readFilePart( h, o, &v, sizeof( v))
#define READBUF(h,o,v,s) _readFilePart( h, o, &v, s)
#define READPART(h,o,pb,s)  _readFilePartDyn( h, o, pb,s)

// --------------------------------------------------------------------------

static APIRET _readFilePart( HFILE hfile, ULONG ulOffset, PVOID pvBuffer, ULONG ulReadLen)
{

         APIRET         rc = NO_ERROR;
         ULONG          ulFilePtr;
         ULONG          ulBytesRead;

do
   {
   // check parms
   if ((!pvBuffer) ||
       (!ulReadLen))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // goto offset
   rc = DosSetFilePtr( hfile, ulOffset, FILE_BEGIN, &ulFilePtr);
   if (rc != NO_ERROR)
      break;


   // read data
   // read file
   rc = DosRead( hfile, pvBuffer, ulReadLen, &ulBytesRead);
   if ((rc != NO_ERROR) || (ulReadLen != ulBytesRead))
      break;

   } while (FALSE);

return rc;
}

// --------------------------------------------------------------------------

static APIRET _readFilePartDyn( HFILE hfile, ULONG ulOffset, PVOID *ppvData, ULONG ulReadLen)
{
         PVOID          pvData = NULL;
         APIRET         rc = NO_ERROR;

do
   {
   if ((!ulReadLen) ||
       (!ppvData))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   pvData = malloc( ulReadLen);
   if (!pvData)
      {
      rc = ERROR_NOT_ENOUGH_MEMORY;
      break;
      }

   rc = _readFilePart( hfile, ulOffset, pvData, ulReadLen);
   if (rc != NO_ERROR)
      break;

   *ppvData = pvData;

   } while (FALSE);

// cleanup
if ((rc != NO_ERROR) && (pvData))
   {
   free( pvData);
   pvData = NULL;
   }

return rc;
}

// =============================================================================

/*
@@WtkLoadBitmapFromFile@SYNTAX
This function loads a bitmap from a bitmap file. Unlike with GpiLoadBitmap,
a specified width and height can be forced, that is if a bitmap with the
specified dimension does not exist in the file, no bitmap is loaded.

@@WtkLoadBitmapFromFile@PARM@hps@in
Handle to presentation space.

@@WtkLoadBitmapFromFile@PARM@pszFile@in
Address of the ASCII pathname of the bitmap file

@@WtkLoadBitmapFromFile@PARM@phbmp@in
Address of a vaiable receiving the handle of the created bitmap.

@@WtkLoadBitmapFromFile@PARM@ulWidth@in
Desired width of the loaded bitmap or zero.
:p.
Also ulHeight must be specified, otherwise this parameter is ignored.

@@WtkLoadBitmapFromFile@PARM@ulHeight@in
Desired height of the loaded bitmap or zero.
:p.
Also ulWidth must be specified, otherwise this parameter is ignored.

@@WtkLoadBitmapFromFile@PARM@fForceSize@in
Flag indicating wether the specified sizes are enforced.
:p.
If this parameter is set to true, no bitmap is loaded if none is found
with the specified dimensions (returns ERROR_INVALID_DATA). If it is
set to false, also any other bitmap (of any dimension) may be loaded.
:p.
When enforcing the size, both parameters ulWidth and ulHeight must be specified,
else ERROR_INVALID_PARAMETER is returned.

@@WtkLoadBitmapFromFile@RETURN
Return Code.
:p.
WtkLoadBitmapFromFile returns one of the following return codes:
:parml compact break=none.
:pt.0
:pd.NO_ERROR
:pt.8
:pd.ERROR_NOT_ENOUGH_MEMORY
:pt.13
:pd.ERROR_INVALID_DATA
:pt.85
:pd.ERROR_INVALID_PARAMETER
:eparml.
:p.
or return codes of the following functions
:ul compact.
:li.DosOpen
:li.WinGetLastError, when GpiCreateBitmap fails
:li.WtkReadFile
:eul.
:p.
The return code ERROR_INVALID_DATA specifies that no bitmap could
be loaded, because none matched the specified size, while the parameter
fForceSize was set.

@@WtkLoadBitmapFromFile@REMARKS
A bitmap loaded by WtkLoadBitmapFromFile can be destroyed
by GpiDeleteBitmap.

@@
*/

APIRET APIENTRY WtkLoadBitmapFromFile( HPS hps, PSZ pszFile, PHBITMAP phbmp, ULONG ulWidth, ULONG ulHeight, BOOL fForceSize)

{
         APIRET         rc = NO_ERROR;
         ULONG          i;

         ULONG          ulFileSize;
         HFILE          hfile = -1;
         ULONG          ulAction;

         ULONG          ulOffset;
         ULONG          ulHeaderLen;

         USHORT         usSignature;

         BITMAPARRAYFILEHEADER2 bafh2;
         BITMAPFILEHEADER2 bfh2;

         PBITMAPARRAYFILEHEADER pbafh = (PBITMAPARRAYFILEHEADER) &bafh2;
         PBITMAPARRAYFILEHEADER2 pbafh2 = &bafh2;

         PBITMAPFILEHEADER2 pbfh2;
         PBITMAPFILEHEADER  pbfh;

         BOOL           fOldData;
         ULONG          ulBmpBitCount;
         ULONG          ulBmpPlanes;
         ULONG          ulBmpWidth;
         ULONG          ulBmpHeight;
         ULONG          ulBmpInfoOffset;
         ULONG          ulBmpColors;
         ULONG          ulBmpImgOffset;
         ULONG          ulBmpImgLen;

do
   {
   // check parms
   if ((!pszFile)     ||
       (!*pszFile)    ||
       (!phbmp))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }
   *phbmp = NULLHANDLE;

   // if size is enforced, both size values must be specified
   if ((fForceSize) &&
       ((!ulWidth) ||
        (!ulHeight)))
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }
   // open the file
   rc = DosOpen( pszFile,
                 &hfile,
                 &ulAction,
                 0,
                 FILE_NORMAL,
                 OPEN_ACTION_FAIL_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS,
                 OPEN_ACCESS_READONLY | OPEN_SHARE_DENYWRITE,
                 (PEAOP2)NULL);
   if (rc != NO_ERROR)
      break;

   // get some values
   ulFileSize =  WtkQueryFileSize( pszFile);

   // check number of bitmaps
   READVAR( hfile, 0, usSignature);

   // check for single bitmap or array
   switch (usSignature)
      {
      case BFT_BMAP:
         {

         // read complete file to memory
         ulOffset = 0;
         rc = READPART( hfile, ulOffset, (PVOID*)&pbfh2, ulFileSize);
         if (rc != NO_ERROR)
            break;

         // get bitmap attributes
         pbfh = (PBITMAPFILEHEADER) pbfh2;
         fOldData        = (pbfh2->bmp2.cbFix == sizeof( BITMAPINFOHEADER));
         ulBmpWidth      = (fOldData) ? pbfh->bmp.cx        : pbfh2->bmp2.cx;
         ulBmpHeight     = (fOldData) ? pbfh->bmp.cy        : pbfh2->bmp2.cy;
         ulBmpBitCount   = (fOldData) ? pbfh->bmp.cBitCount : pbfh2->bmp2.cBitCount;
         ulBmpPlanes     = (fOldData) ? pbfh->bmp.cPlanes   : pbfh2->bmp2.cPlanes;
         ulBmpColors     = 1 << (ulBmpPlanes * ulBmpBitCount);
         ulBmpImgOffset  = (fOldData) ? pbfh->offBits       : pbfh2->offBits;

         // check dimensions, if requested
         if ((fForceSize) && (ulWidth) && (ulHeight))
            {
            if ((ulBmpWidth  != ulWidth) ||
                (ulBmpHeight != ulHeight))
               {
               rc = ERROR_INVALID_DATA;
               break;
               }
            }

         // create bitmap
         *phbmp = GpiCreateBitmap( hps,
                                   &pbfh2->bmp2,
                                   CBM_INIT,
                                   (PBYTE)pbfh2 + pbfh2->offBits,
                                   (PBITMAPINFO2)&pbfh2->bmp2);
         if (*phbmp == GPI_ERROR)
            rc = ERRORIDERROR( WinGetLastError( WinQueryAnchorBlock( HWND_DESKTOP)));

         // free file data
         free( pbfh2);

         }
         break; // case BFT_BMAP:

      case BFT_BITMAPARRAY:
         {
                  ULONG          ulOffsetMatch                = 0;
                  ULONG          ulOffsetDependent            = 0;
                  ULONG          ulOffsetDependentLowColor    = 0;
                  ULONG          ulOffsetIndependent          = 0;
                  ULONG          ulOffsetIndependentLowColor  = 0;
                  ULONG          ulOffsetFirstBitmap          = 0;

                  ULONG          ulCapsWidth;
                  ULONG          ulCapsHeight;
                  ULONG          ulCapsColors;

         // get certain caps values
         DevQueryCaps( GpiQueryDevice( hps), CAPS_COLORS,         1, (PLONG)&ulCapsColors);
         DevQueryCaps( GpiQueryDevice( hps), CAPS_WIDTH,          1, (PLONG)&ulCapsWidth);
         DevQueryCaps( GpiQueryDevice( hps), CAPS_HEIGHT,         1, (PLONG)&ulCapsHeight);

         // adjust pointers for easy access
         pbafh = (PBITMAPARRAYFILEHEADER) &bafh2;
         pbafh2 = &bafh2;

         // loop all array file headers
         ulOffset = 0;
         memset( &bafh2, 0, sizeof( bafh2));
         READVAR( hfile, ulOffset + 2, ulHeaderLen);
         READBUF( hfile, ulOffset, bafh2, ulHeaderLen);

         while (TRUE)
            {
            do
               {
               // skip data with invalid signature
               if (bafh2.usType != BFT_BITMAPARRAY)
                  break;

               // get bitmap attributes
               pbfh  = (PBITMAPFILEHEADER)  &pbafh2->bfh2;
               pbfh2 = (PBITMAPFILEHEADER2) &pbafh2->bfh2;
               fOldData        = (pbfh2->bmp2.cbFix == sizeof( BITMAPINFOHEADER));
               ulBmpWidth      = (fOldData) ? pbfh->bmp.cx        : pbfh2->bmp2.cx;
               ulBmpHeight     = (fOldData) ? pbfh->bmp.cy        : pbfh2->bmp2.cy;
               ulBmpBitCount   = (fOldData) ? pbfh->bmp.cBitCount : pbfh2->bmp2.cBitCount;
               ulBmpPlanes     = (fOldData) ? pbfh->bmp.cPlanes   : pbfh2->bmp2.cPlanes;
               ulBmpColors     = 1 << (ulBmpPlanes * ulBmpBitCount);
               ulBmpImgOffset  = (fOldData) ? pbfh->offBits       : pbfh2->offBits;

               // calculate offset of info header
               ulBmpInfoOffset = ulOffset + FIELDOFFSET( BITMAPARRAYFILEHEADER, bfh);

               // check for exact dimension match
               if ((ulWidth) && (ulHeight))
                  if ((ulBmpWidth  == ulWidth) &&
                      (ulBmpHeight == ulHeight))
                     ulOffsetMatch = ulBmpInfoOffset;

               // exit for no-matching dimension, if requested
               if ((fForceSize) && (ulWidth) && (ulHeight))
                  {
                  if ((ulBmpWidth  != ulWidth) ||
                      (ulBmpHeight != ulHeight))
                     break;
                  }

               // keep first bitmap
               if (!ulOffsetFirstBitmap)
                  ulOffsetFirstBitmap = ulBmpInfoOffset;

               // check for device independant bitmap
               if ((!bafh2.cxDisplay) && (!bafh2.cyDisplay))
                  {
                  if (ulBmpColors > ulCapsColors)
                     ulOffsetIndependentLowColor = ulBmpInfoOffset;
                  else
                     ulOffsetIndependent = ulBmpInfoOffset;
                  }
               // check for device independant bitmap
               else if ((ulCapsWidth  >= bafh2.cxDisplay) ||
                        (ulCapsHeight >= bafh2.cxDisplay))
                  {
                  if (ulBmpColors > ulCapsColors)
                     ulOffsetDependentLowColor = ulBmpInfoOffset;
                  else
                     ulOffsetDependent = ulBmpInfoOffset;
                  }

               } while (FALSE);

            // bail out on last header
            if (!bafh2.offNext)
               break;

            // next source and target
            ulOffset = bafh2.offNext;
            memset( &bafh2, 0, sizeof( bafh2));
            READVAR( hfile, ulOffset + 2, ulHeaderLen);
            READBUF( hfile, ulOffset, bafh2, ulHeaderLen);
            }

         // select between what has been found
         // best case first, dependent over independent
         if ((ulBmpInfoOffset = ulOffsetMatch)               ||
             (ulBmpInfoOffset = ulOffsetDependent)           ||
             (ulBmpInfoOffset = ulOffsetDependentLowColor)   ||
             (ulBmpInfoOffset = ulOffsetIndependent)         ||
             (ulBmpInfoOffset = ulOffsetIndependentLowColor) ||
             (ulBmpInfoOffset = ulOffsetFirstBitmap))
            {

                     PBITMAPFILEHEADER2 pbfh2 = NULL;
                     PBITMAPFILEHEADER  pbfh  = NULL;
                     PVOID          pvImage = NULL;

                     ULONG          ulColorTableLen;
                     ULONG          ulPlaneLen;
                     ULONG          ulImageOffset;
                     ULONG          ulBytesPerLine;
                     ULONG          ulAlignedLineSize;
                     ULONG          ulImageLen;
                     ULONG          ulFullHeaderLen;

            // read header
            rc = READPART( hfile, ulBmpInfoOffset, (PVOID*)&pbfh2, sizeof( BITMAPFILEHEADER2));
            if (rc != NO_ERROR)
               break;

            // skip data with invalid signature
            if (pbfh2->usType != BFT_BMAP)
               break;

            // get bitmap attributes
            pbfh = (PBITMAPFILEHEADER) pbfh2;
            fOldData        = (pbfh2->bmp2.cbFix == sizeof( BITMAPINFOHEADER));
            ulBmpWidth      = (fOldData) ? pbfh->bmp.cx        : pbfh2->bmp2.cx;
            ulBmpHeight     = (fOldData) ? pbfh->bmp.cy        : pbfh2->bmp2.cy;
            ulBmpBitCount   = (fOldData) ? pbfh->bmp.cBitCount : pbfh2->bmp2.cBitCount;
            ulBmpPlanes     = (fOldData) ? pbfh->bmp.cPlanes   : pbfh2->bmp2.cPlanes;
            ulBmpColors     = 1 << (ulBmpPlanes * ulBmpBitCount);
            ulBmpImgOffset  = (fOldData) ? pbfh->offBits       : pbfh2->offBits;

            // calculate length of color table and image
            ulColorTableLen   = ulBmpColors * ((fOldData) ? sizeof( RGB) : sizeof( RGB2));
            ulBytesPerLine    = ulBmpWidth * ulBmpBitCount / 8;
            ulAlignedLineSize = ((ulBytesPerLine / 4) + 1) * 4;
            ulImageLen        = ulAlignedLineSize * ulBmpHeight * ulBmpPlanes;

            ulImageOffset     = (fOldData) ?  pbfh->offBits : pbfh2->offBits;
            ulHeaderLen       = (fOldData) ?  sizeof( BITMAPFILEHEADER) :  sizeof( BITMAPFILEHEADER2);
            ulFullHeaderLen   = ulHeaderLen + ulColorTableLen;

            free( pbfh2);
            pbfh2 = NULL;

            // reread bitmap file header including color table
            rc = READPART( hfile, ulBmpInfoOffset, (PVOID*)&pbfh2, ulFullHeaderLen);
            if (rc != NO_ERROR)
               break;

            // read image data
            rc = READPART( hfile, ulImageOffset, &pvImage, ulImageLen);
            if (rc != NO_ERROR)
               break;

            // create bitmap
            *phbmp = GpiCreateBitmap( hps,
                                      &pbfh2->bmp2,
                                      CBM_INIT,
                                      pvImage,
                                      (PBITMAPINFO2)&pbfh2->bmp2);

            if (*phbmp == GPI_ERROR)
               rc = ERRORIDERROR( WinGetLastError( WinQueryAnchorBlock( HWND_DESKTOP)));

            // free file data
            if (pbfh2) free( pbfh2);
            if (pvImage) free( pvImage);
            }
         else
            // no bitmap found - this can happen only when no
            // bitmap was found for the forced size
            rc = ERROR_INVALID_DATA;

         }
         break; // case BFT_BITMAPARRAY:
      }

   } while (FALSE);

// cleanup
DosClose( hfile);
return rc;
}

