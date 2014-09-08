/****************************** Module Header ******************************\
*
* Module Name: _tmf.c
*
* text message file (TMF) sample
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2002
*
* $Id: _tmf.c,v 1.3 2008-12-14 22:21:28 cla Exp $
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

#define INCL_ERRORS
#define INCL_DOSMISC
#include <os2.h>

#define INCL_WTKTMF
#define INCL_WTKUTLFILE
#include <wtk.h>

// -----------------------------------------------------------------------------

#define PRINTSEPARATOR printf("\n------------------------------------------\n")

int main ( int argc, char *argv[])
{

         APIRET         rc = NO_ERROR;
         CHAR           szBuffer[1024];
         ULONG          ulMessageLen;

static   PSZ            apszParms[] = { "#1", "#2", "#3", "#4", "#5",
                                        "#6", "#7", "#8", "#9"};
static   PSZ            pszFilename = "test.tmf";

#define GETMESSAGE(m,t,c) \
           memset( szBuffer, 0, sizeof( szBuffer)); \
           rc = WtkGetTextMessage( (PCHAR*)t, c, szBuffer, sizeof( szBuffer), m, pszFilename, &ulMessageLen); \
           printf( "%u - %s: ***>%s<***\n\n", rc, m, szBuffer);

do
   {
   // --------------------------------------------------------------------------------

   PRINTSEPARATOR;

   printf( "test full parameter list\n");
   GETMESSAGE( "TEST_PARAMETER_LIST", apszParms, 9);

   printf( "test parameter list with missing parms\n");
   GETMESSAGE( "TEST_PARAMETER_LIST", apszParms, 4);

   printf( "test parameter list with no space\n");
   GETMESSAGE( "TEST_PARAMETER_NOSPACE", apszParms, 9);

   printf( "test parameter list with multiple lines\n");
   GETMESSAGE( "TEST_PARAMETER_MULTILINELIST", apszParms, 9);


   // --------------------------------------------------------------------------------

   PRINTSEPARATOR;


   } while (FALSE);

// cleanup

return rc;
}

