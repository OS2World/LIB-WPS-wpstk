/****************************** Module Header ******************************\
*
* Module Name: wtkioc.c
*
* Source for device IO control helper functions
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: wtkuioc.c,v 1.9 2005-12-01 20:53:36 cla Exp $
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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INCL_DOS
#define INCL_ERRORS
#include <os2.h>

#include "wtkuioc.h"
#include "wpstk.ih"

// bit masks for open flags

#define WTK_OPENDEVICE_MASK  (WTK_OPENDEVICE_EXCLUSIVE | WTK_OPENDEVICE_NOCACHE | WTK_OPENDEVICE_BLOCKDEVICE)

// ---------------------------------------------------------------------------

/*
@@WtkOpenDevice@SYNTAX
This function opens a device.

@@WtkOpenDevice@PARM@pszName@in
Address of the ASCIIZ name of the device.
:p.
If the last character is a colon, the device is opened
in block mode.

@@WtkOpenDevice@PARM@phdevice@out
The address of a variable where the handle of the
opened device is returned.

@@WtkOpenDevice@PARM@ulOpenMode@in
The mode, in which the device has to be opened.

:parml.
:pt.WTK_OPENDEVICE_SHARED
:pd.opens the device in readonly and thus in shared mode
:pt.WTK_OPENDEVICE_EXCLUSIVE
:pd.opens the device in readwrite and thus in exlusive mode
.br
(overrides WTK_OPENDEVICE_SHARED)
:pt.WTK_OPENDEVICE_NOCACHE
:pd.bypasses the cache, if block devices are opened
:pt.WTK_OPENDEVICE_BLOCKDEVICE
:pd.explicitely states, that the specified device is a block device.
:eparml.
:note.
If the last character is a colon, the device is opened
in block mode, even if WTK_OPENDEVICE_BLOCKDEVICE is not specified.

@@WtkOpenDevice@RETURN
Return Code.
:p.
WtkOpenDevice returns one of the following return codes&colon.

:parml compact break=none.
:pt.0
:pd.NO_ERROR
:pt.87
:pd.ERROR_INVALID_PARAMETER
:eparml.
:p.
or return codes of the following functions
:ul compact.
:li.DosOpen
:eul.

@@WtkOpenDevice@REMARKS
A device opened with WtkOpenDevice must be closed by a call to DosClose.
:p.
If the last character specified with pszName is a colon, the device is opened
in block mode, even when WTK_OPENDEVICE_BLOCKDEVICE is not specified within
ulOpenMode.

@@
*/

APIRET APIENTRY WtkOpenDevice( PSZ pszName, PHFILE phdevice, ULONG ulOpenMode)
{
         APIRET         rc = NO_ERROR;
         ULONG          ulAction;
         ULONG          fsOpenMode = 0;
         HFILE          hdevice;

do
   {
   // check parms
   if (!pszName)
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }

   // check for correct flags
   if (fsOpenMode & ~WTK_OPENDEVICE_MASK)
      {
      rc = ERROR_INVALID_PARAMETER;
      break;
      }
   if (ulOpenMode & WTK_OPENDEVICE_EXCLUSIVE)
      fsOpenMode |= OPEN_SHARE_DENYREADWRITE | OPEN_ACCESS_READWRITE;
   else
      fsOpenMode |= OPEN_SHARE_DENYNONE      | OPEN_ACCESS_READONLY;

   if (ulOpenMode & WTK_OPENDEVICE_NOCACHE)
      fsOpenMode |= OPEN_FLAGS_WRITE_THROUGH | OPEN_FLAGS_NO_CACHE;

   if ((ulOpenMode & WTK_OPENDEVICE_BLOCKDEVICE) ||
       (*(pszName + strlen( pszName) - 1) == ':'))
      fsOpenMode |= OPEN_FLAGS_DASD;

   // open device
   rc = DosOpen( pszName,
                 &hdevice,
                 &ulAction,
                 0, 0,
                 OPEN_ACTION_OPEN_IF_EXISTS,
                 fsOpenMode,
                 NULL);
   if (rc != NO_ERROR)
      break;

   // hand over result
   *phdevice = hdevice;

   } while (FALSE);

return rc;
}

// ---------------------------------------------------------------------------

/*
@@WtkDevIOCtl@SYNTAX
This function performs an I/O control call transaction
using DosDevIOCtl.
The devices is opened, the I/O control call is made and
the device is immediately closed again.

@@WtkDevIOCtl@PARM@pszName@in
Address of the ASCIIZ name of the device.
:p.
If the last character is a colon, the device is opened
in block mode.
:p.
The device name may be :hp2.?&colon.:ehp2. in order to perform actions
on the boot drive.

@@WtkDevIOCtl@PARM@ulOpenMode@in
The mode, in which the device has to be opened.

:parml.
:pt.WTK_OPENDEVICE_SHARED
:pd.opens the device in readonly and thus in shared mode
:pt.WTK_OPENDEVICE_EXCLUSIVE
:pd.opens the device in readwrite and thus in exlusive mode
:pt.WTK_OPENDEVICE_NOCACHE
:pd.bypasses the cache, if block devices are opened
:pt.WTK_OPENDEVICE_BLOCKDEVICE
:pd.explicitely states, that the specified device is a block device.
:eparml.
:note.
If the last character is a colon, the device is opened
in block mode, even when WTK_OPENDEVICE_BLOCKDEVICE is not specified.

@@WtkDevIOCtl@PARM@ulCategory@in
Device category.
:p.
The valid range is 0 to 255.

@@WtkDevIOCtl@PARM@ulFunction@in
Device-specific function code.
:p.
The valid range is 0 to 255.

@@WtkDevIOCtl@PARM@pvParams@in
Address of the command-specific argument list.


@@WtkDevIOCtl@PARM@pcbParmLen@inout
Pointer to the length of parameters.

:parml break=none.
:pt.Input
:pd.Pointer to the length, in bytes, of the parameters passed in pParams by the application.
:pt.Output
:pd.Pointer to the length, in bytes, of the parameters returned.
:p.
If this function returns ERROR_BUFFER_OVERFLOW, then
pcbParmLen points to the size of the buffer required to hold
the parameters returned. No other data is returned in this
case.
:eparml.

:note.
Compared to DosDevIOCtl, this function is missing the parameter cbParmLenMax.
On calling DosDevIOCtl this function sets cbParmLenMax to the value pointed to by
pcbParmLen. If this is not applicable to your code, use DosDevIOCtl instead.


@@WtkDevIOCtl@PARM@pvData@in
Address of the data area.


@@WtkDevIOCtl@PARM@pcbDataLen@inout
Pointer to the length of data.

:parml break=none.
:pt.Input
:pd.Pointer to the length, in bytes, of the data passed by the application in pData.
:pt.Output
:pd.Pointer to the length, in bytes, of the data returned.
:p.
If this function returns ERROR_BUFFER_OVERFLOW, then
pcbDataLen points to the size of the buffer required to hold
the data returned.
:eparml.

:note.
Compared to DosDevIOCtl, this function is missing the parameter cbDataLenMax.
On calling DosDevIOCtl this function sets cbDataLenMax to the value pointed to by
pcbDataLen. If this is not applicable to your code, use DosDevIOCtl instead.

@@WtkDevIOCtl@RETURN
Return Code.
:p.
WtkDevIOCtl returns one of the following return codes&colon.

:parml compact break=none.
:pt.0
:pd.NO_ERROR
:pt.87
:pd.ERROR_INVALID_PARAMETER
:eparml.
:p.
or return codes of the following functions
:ul compact.
:li.DosDevIOCtl
:eul.

@@WtkDevIOCtl@REMARKS
In order to simplify coding a littlebit, this function does not provide
the parameters cbParmLenMax and cbDataLenMax, but instead implicitely sets them
to the values pointed by the parameters pcbParmLen and pcbDataLen.
This is done as in most cases this is sufficient.
If this is not applicable to your code, use DosDevIOCtl instead.
:p.
:p.
This function supports boot drive recognition.
Specify :hp2.?&colon.:ehp2. for the drive in order to
perform actions on the boot drive.

@@
*/

APIRET APIENTRY WtkDevIOCtl( PSZ pszName, ULONG ulOpenMode,
                             ULONG ulCategory, ULONG ulFunction,
                             PVOID pvParams, PULONG pcbParmLen,
                             PVOID pvData, PULONG pcbDataLen)

{
         APIRET         rc = NO_ERROR;
         HFILE          hdevice = -1;
         ULONG          ulParmLen = 0;
         ULONG          ulDataLen = 0;
         CHAR           szName[ _MAX_PATH];

do
   {


   // copy name and replace ?: with bootdrive
   strcpy( szName, pszName);
   __PatchBootDrive( szName);

   // open device
   rc = WtkOpenDevice( szName, &hdevice, ulOpenMode);
   if (rc != NO_ERROR)
      break;

   // check for length parameters (pointers can be NULL)
   if (pcbParmLen) ulParmLen = *pcbParmLen;
   if (pcbDataLen) ulDataLen = *pcbDataLen;

   // perform io control
   rc = DosDevIOCtl( hdevice,
                     ulCategory,
                     ulFunction,
                     pvParams,
                     ulParmLen,
                     pcbParmLen,
                     pvData,
                     ulDataLen,
                     pcbDataLen);



   } while (FALSE);

// cleanup
DosClose( hdevice);

return rc;

}

