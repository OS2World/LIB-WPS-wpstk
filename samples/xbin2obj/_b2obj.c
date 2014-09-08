/****************************** Module Header ******************************\
*
* Module Name: _b2obj.c
*
* XBINOBJ sample
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: _b2obj.c,v 1.4 2004-04-19 15:02:50 cla Exp $
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
#include <io.h>

#define INCL_ERRORS
#include <os2.h>

#define INLC_WTKUTLXBIN2OBJ
#include <wtk.h>



// -----------------------------------------------------------------------------

#define PRINTSEPARATOR printf("\n------------------------------------------\n")


int main ( int argc, char *argv[])
{

         APIRET         rc = NO_ERROR;

extern   BINDATA        bdText;

do
   {

   // ----------------------------------------------------------------------

   PRINTSEPARATOR;


do
   {
   // set end of text mark
   bdText.bData[ bdText.cbSize] = 0;

   printf( "\n"
           "data len: %u\n"
           "data:\n"
           "--------------\n",
           bdText.cbSize);
   fflush( stdout);
   write( 1, bdText.bData, bdText.cbSize);
   printf( "--------------\n");
   fflush( stdout);

   } while (FALSE);

   PRINTSEPARATOR;

   } while (FALSE);

return rc;
}


