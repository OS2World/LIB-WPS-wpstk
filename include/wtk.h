/****************************** Module Header ******************************\
*
* Module Name: wtk.h
*
* Top level include file for the WPS Toolkit
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: wtk.h,v 1.7 2004-08-15 19:57:34 cla Exp $
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
*   INCL_WTK                 ALL of WPSTK headers
*   INCL_WTKUTL              helper functions
*   INCL_WTKTMF              text message file functions
*   INCL_WTKSETTINGS         Settings and Details Manager for WPS classes
*   INCL_WTKEAS              Extended Ettributes Manager functions
*   INCL_WTKMMF              Memory Mapped Files Manager functions
*   INCL_WTKBBH              Bubble Help Manager functions
*   INCL_WTKPROCESS          Process related functions
*
\***************************************************************************/

#ifndef WTK_INCLUDED
#define WTK_INCLUDED

#ifdef __cplusplus
      extern "C" {
#endif


/*** at least we need basic OS/2 types *************************************/
#include <os2.h>

/*** include some WARP 4 stuff not being included in WARP 3 toolkit ********/
#include <warp4def.h>

/*** include version function **********************************************/
#include <wpstk.h>

/*** include subsections ***************************************************/
#ifdef INCL_WTK
   #define INCL_WTKUTL
   #define INCL_WTKFILE
   #define INCL_WTKTMF
   #define INCL_WTKSETTINGS
   #define INCL_WTKEAS
   #define INCL_WTKMMF
   #define INCL_WTKPROCESS
   #define INCL_WTKBBH
#endif

/*** include top level helper header file **********************************/
#include <wtku.h>            /* helper functions top level include */
#include <wtkf.h>            /* file access functions top level include */

#ifdef INCL_WTKTMF
   #include <wtktmf.h>       /* text message functions */
#endif

#ifdef INCL_WTKSETTINGS
   #include <wtkset.h>       /* settings manager for WPS classes */
#endif

#ifdef INCL_WTKEAS
   #include <wtkeas.h>       /* extended attributes manager functions */
#endif

#ifdef INCL_WTKMMF
   #include <wtkmmf.h>       /* memory mapped files manager functions */
#endif

#ifdef INCL_WTKBBH
   #include <wtkbbh.h>       /* bubble help manager functions */
#endif

#ifdef INCL_WTKPROCESS
   #include <wtkproc.h>      /* process related functions */
#endif

#ifdef __cplusplus
        }
#endif

#endif /* WTK_INCLUDED */

