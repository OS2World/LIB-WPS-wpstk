/****************************** Module Header ******************************\
*
* Module Name: wtku.h
*
* Top level include file for the utility functions of the WPS Toolkit
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: wtku.h,v 1.12 2009-11-18 21:33:07 cla Exp $
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
* The following symbols are used in this file for conditional sections.
*
*   #define:                To include:
*
*   INCL_WTKUTLMODULE     helper functions for module functions (EXE and DLL)
*   INCL_WTKUTLNLSMODULE  helper functions for module NLS functions
*   INCL_WTKUTLPM         helper functions for PM
*   INCL_WTKUTLCONTROL    helper functions for PM controls
*   INCL_WTKUTLSYSTEM     helper functions for system
*   INCL_WTKUTLIOCTRL     helper functions for device I/O control
*   INCL_WTKUTLERROR      helper functions for handling PM errors
*   INCL_WTKUTLFILE       file and directory helper functions
*   INCL_WTKUTLTIME       time related helper functions
*   INCL_WTKUTLCRC        CRC related helper functions
*   INCL_WTKUTLMD5        MD5 related helper functions
*   INCL_WTKUTLREGEXP     Regular expression support functions
*   INCL_WTKUTLXBIN2OBJ   XBIN2OBJ access to binary data
*   INCL_WTKUTLLOCALE     helper functions for locale
*
\***************************************************************************/

#ifndef WTKUTL_INCLUDED
#define WTKUTL_INCLUDED

#ifdef __cplusplus
      extern "C" {
#endif

/*** include subsections ***************************************************/
#ifdef INCL_WTKUTL
   #define INCL_WTKUTLMODULE
   #define INCL_WTKUTLNLSMODULE
   #define INCL_WTKUTLPM
   #define INCL_WTKUTLCONTROL
   #define INCL_WTKUTLSYSTEM
   #define INCL_WTKUTLIOCTRL
   #define INCL_WTKUTLERROR
   #define INCL_WTKUTLFILE
   #define INCL_WTKUTLTIME
   #define INCL_WTKUTLCRC
   #define INCL_WTKUTLMD5
   #define INCL_WTKUTLREGEXP
   #define INCL_WTKUTLXBIN2OBJ
   #define INCL_WTKUTLLOCALE
#endif


#ifdef INCL_WTKUTLMODULE
   #include <wtkumod.h>   /* helper functions for modules (EXE and DLL) */
#endif

#ifdef INCL_WTKUTLNLSMODULE
   #include <wtkulmd.h>   /* NLS helper functions for modules */
#endif

#ifdef INCL_WTKUTLPM
   #include <wtkupm.h>   /* helper functions for PM */
#endif

#ifdef INCL_WTKUTLCONTROL
   #include <wtkuctl.h>   /* helper functions for PM controls */
#endif

#ifdef INCL_WTKUTLSYSTEM
   #include <wtkusys.h>   /* helper functions for system */
#endif

#ifdef INCL_WTKUTLIOCTRL
   #include <wtkuioc.h>   /* helper functions for device I/O control */
#endif

#ifdef INCL_WTKUTLERROR
   #include <wtkuerr.h>   /* helper functions for handling PM errors */
#endif

#ifdef INCL_WTKUTLFILE
   #include <wtkufil.h>   /* file and directory helper functions */
#endif


#ifdef INCL_WTKUTLTIME
   #include <wtkutim.h>   /* time related helper functions */
#endif

#ifdef INCL_WTKUTLCRC
   #include <wtkucrc.h>   /* CRC related helper functions */
#endif

#ifdef INCL_WTKUTLMD5
   #include <wtkumd5.h>   /* MD5 related helper functions */
#endif

#ifdef INCL_WTKUTLREGEXP
   #include <wtkurgx.h>   /* Regular expression support functions */
#endif

#ifdef INCL_WTKUTLXBIN2OBJ
   #include <wtkubdat.h>   /* Regular expression support functions */
#endif

#ifdef INCL_WTKUTLLOCALE
   #include <wtkuloc.h>   /* helper functions for locale */
#endif

#ifdef __cplusplus
        }
#endif

#endif /* WTKUTL_INCLUDED */

