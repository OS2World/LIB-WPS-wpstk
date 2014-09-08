/****************************** Module Header ******************************\
*
* Module Name: _ioctl.c
*
* device I/O control related functions sample
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: _ioctl.c,v 1.2 2002-11-20 10:57:31 cla Exp $
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define INCL_ERRORS
#define INCL_DOSDEVIOCTL
#define INCL_DOSDEVICES
#include <os2.h>

#define INCL_WTKUTLIOCTRL
#include <wtk.h>

// -----------------------------------------------------------------------------

#define PRINTSEPARATOR printf("\n------------------------------------------\n")


int main ( int argc, char *argv[])
{

         APIRET         rc = NO_ERROR;
         ULONG          i;
         PSZ            pszDrive = "?:";

         USHORT         usParm;
         ULONG          ulParm;
         BYTE           bData;
         USHORT         usData;
         ULONG          ulParmLen;
         ULONG          ulDataLen;
         HFILE          hdevice = NULLHANDLE;
         PSZ            pszStatus;


do
   {

   // ----------------------------------------------------------------------

   PRINTSEPARATOR;

   printf( "check all drives from C:\n");
   // query all drives from c:
   for (i = 'C'; i <= 'Z'; i++)
      {
      *pszDrive = i;

      // perform IOCTL transaction call
      ulParmLen = sizeof( usParm);
      ulDataLen = sizeof( bData);
      usParm = (USHORT) (*pszDrive - 'A');
      rc = WtkDevIOCtl( pszDrive, 0, IOCTL_DISK, DSK_BLOCKREMOVABLE,
                        &usParm, &ulParmLen, &bData, &ulDataLen);


      printf( "  %s", pszDrive);
      switch (rc)
         {
         case ERROR_FILE_NOT_FOUND:
            printf( " - invalid drive\n");
            break;

         case ERROR_NOT_READY:
            printf( " - drive not ready\n");
            break;

         case ERROR_INVALID_DRIVE:
         case ERROR_NOT_SUPPORTED:
         case ERROR_NETWORK_ACCESS_DENIED:
            printf( " - network drive\n");
            break;

         case NO_ERROR:
            if (!bData)
               {
               printf( " - removeable\n");

               do
                  {
                  // check lock status using a file handle
                  // (normally not neccessary fot DSK_GETLOCKSTATUS!)
                  printf( "        checking lock status\n");
                  rc = WtkOpenDevice( pszDrive, &hdevice, 0);
                  if (rc == NO_ERROR)
                     printf( "        - WtkOpenDevice: opened device %s\n", pszDrive);
                  else
                     {
                     printf( "        - error: WtkOpenDevice: cannot open device %s. rc=%u\n", pszDrive, rc);
                     break;
                     }

                  // do the native ioctl call
                  ulParmLen = sizeof( ulParm);
                  ulDataLen = sizeof( usData);
                  ulParm = 0;
                  rc = DosDevIOCtl( hdevice, IOCTL_DISK, DSK_GETLOCKSTATUS,
                                   &ulParm, ulParmLen, &ulParmLen,
                                   &usData, ulDataLen, &ulDataLen);
                  if (rc == NO_ERROR)
                     {
                     switch (usData & 3)
                        {
                        case 0: pszStatus = "Lock/Unlock/Eject/Status functions not supported.";                 break;
                        case 1: pszStatus = "Drive locked.  Lock/Unlock/Eject functions supported.";             break;
                        case 2: pszStatus = "Drive unlocked.  Lock/Unlock/Eject functions supported.";           break;
                        case 3: pszStatus = "Lock Status not supported.  Lock/Unlock/Eject functions supported"; break;
                        }
                     printf( "        - DosDevIOCtl: lock status of device %s is:\n"
                             "             > %s\n",
                             pszDrive, pszStatus);
                     }
                  else
                     {
                     printf( "     - error: DosDevIOCtl: cannot determine lock status. rc=%u\n", rc);
                     break;
                     }


                  } while (FALSE);

               // cleanup
               if (hdevice) DosClose( hdevice);

               }
            else
               printf( " - fixed\n");
            break;

         default:
            printf( "- WtkDevIOCtl: cannot execute IOCTL_DISK:DSK_BLOCKREMOVABLE on drive %s. rc=%u\n", pszDrive, rc);
            break;

         } // switch

      // reset error status
      rc = NO_ERROR;

      } // for


   // ----------------------------------------------------------------------

   PRINTSEPARATOR;


   } while (FALSE);

return rc;
}


