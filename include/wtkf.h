/****************************** Module Header ******************************\
*
* Module Name: wtkf.h
*
* Top level include file for file data functions of the WPS Toolkit
*
* This does not include the file utility functions,
* they are included by wtku.h
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: wtkf.h,v 1.7 2005-11-06 13:46:06 cla Exp $
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
*   INCL_WTKFILESYSLEVEL  access functions for syslevel files
*   INCL_WTKFILEINIT      access functions for text initialization files
*   INCL_WTKFILECFGSYS    access functions for CONFIG.SYS
*   INCL_WTKFILEBMP       access functions for bitmap files
*   INCL_WTKFILEBLDL      access functions for buildlevel information
*
\***************************************************************************/

#ifndef WTKFILE_INCLUDED
#define WTKFILE_INCLUDED

#ifdef __cplusplus
      extern "C" {
#endif

/*** include subsections ***************************************************/
#ifdef INCL_WTKFILE
   #define INCL_WTKFILESYSLEVEL
   #define INCL_WTKFILEINIT
   #define INCL_WTKFILECFGSYS
   #define INCL_WTKFILEBMP
   #define INCL_WTKFILEBLDL
#endif


#ifdef INCL_WTKFILESYSLEVEL
   #include <wtkfslvl.h>   /* access functions for syslevel files */
#endif

#ifdef INCL_WTKFILEINIT
   #include <wtkfinit.h>   /* access functions for text initialization files */
#endif

#ifdef INCL_WTKFILECFGSYS
   #include <wtkfcfgs.h>   /* access functions for CONFIG.SYS */
#endif

#ifdef INCL_WTKFILEBMP
   #include <wtkfbmp.h>   /* access functions for bitmap files */
#endif

#ifdef INCL_WTKFILEBLDL
   #include <wtkfbldl.h>   /* access functions for buildlevel information */
#endif

#ifdef __cplusplus
        }
#endif

#endif /* WTKFILE_INCLUDED */

