/****************************** Module Header ******************************\
*
* Module Name: wtkmmf.h
*
* include file for bubble help manager functions.
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
*
* $Id: wtkbbh.h,v 1.5 2005-06-12 18:13:06 cla Exp $
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

#ifndef WTKBBH_INCLUDED
#define WTKBBH_INCLUDED Bubble Help manager functions

#ifdef __cplusplus
      extern "C" {
#endif

#pragma pack(1)

/* data structs */
typedef struct _BUBBLEHELPDATA {
                                  /* defaults are shown in round brackets, see     */
                                  /* remarks of WtkInitializeBubbleHelp for flags  */
  ULONG          cbSize;          /* size of structure                             */
  ULONG          flModify;        /* modify flags BHELP_MODIFY_* (BHELP_MODIFY_ALL)*/
  ULONG          flStyle;         /* help style flags BHELP_STYLE_*                */
                                  /*  (BHELP_STYLE_HCENTER | BHELP_STYLE_VBOTTOM)  */
  POINTL         ptlOffset;       /* offset of help window to mouse pointer (0,28) */
  ULONG          ulFgColor;       /* help window foreground color (CLR_BLACK)      */
  ULONG          ulBgColor;       /* help window background color (CLR_WHITE)      */
  ULONG          ulBorderColor;   /* help window border color     (CLR_BLACK)      */
  CHAR           szFont[32];      /* help window font             (9.WarpSans)     */
  ULONG          ulXMargin;       /* horizontal margin for help window text (6)    */
  ULONG          ulYMargin;       /* vertical margin for help window text (3)      */
  ULONG          ulStartTimeout;  /* time before showing help window in ms (1000)  */
  ULONG          ulStopTimeout;   /* time before checking pointer pos in ms (100)  */
  ULONG          flShadowStyle;   /* help shadow style flags BHSHADOW_STYLE_*      */
                                  /*                       (BHSHADOW_STYLE_RECTL)  */
  ULONG          ulShadowColor;   /* help shadow color (CLR_BLACK)                 */
  ULONG          ulShadowPattern; /* help shadow pattern (PATSYM_HALFTONE)         */
  POINTL         ptlShadowOffset; /* offset of help shadow (10, -13)               */
} BUBBLEHELPDATA, *PBUBBLEHELPDATA;

/* modification flags for BUBBLEHELPDATA.flModify */
#define BHELP_MODIFY_ALL            0xFFFFFFFF
#define BHELP_MODIFY_STYLE          0x00000001
#define BHELP_MODIFY_PTLOFFSET      0x00000002
#define BHELP_MODIFY_COLOR          0x00000004
#define BHELP_MODIFY_FONT           0x00000008
#define BHELP_MODIFY_MARGIN         0x00000010
#define BHELP_MODIFY_STARTTIMEOUT   0x00000020
#define BHELP_MODIFY_STOPTIMEOUT    0x00000040
#define BHELP_MODIFY_SHADOWSTYLE    0x00000080
#define BHELP_MODIFY_SHADOWCOLOR    0x00000100
#define BHELP_MODIFY_SHADOWPATTERN  0x00000200
#define BHELP_MODIFY_SHADOWOFFSET   0x00000400

/* styles for BUBBLEHELPDATA.flStyle */
#define BHELP_STYLE_HLEFT         0x0001
#define BHELP_STYLE_HRIGHT        0x0002
#define BHELP_STYLE_HCENTER       0x0003

#define BHELP_STYLE_VTOP          0x0010
#define BHELP_STYLE_VBOTTOM       0x0020
#define BHELP_STYLE_VCENTER       0x0030

#define BHELP_STYLE_ALWAYSTOP     0x4000
#define BHELP_STYLE_RGBCOLOR      0x8000

/* styles for BUBBLEHELPDATA.flShadowStyle */
#define BHSHADOW_STYLE_NONE       0x0000
#define BHSHADOW_STYLE_RECTL      0x0001

#define BHSHADOW_STYLE_RGBCOLOR   0x8000

/* control message to pass back a PSZ */
/* pointing to the bubble help text   */
/* NOT YET USED */
#define BHN_QUERYHELPTEXT         0x0001

/* prototypes */
BOOL APIENTRY WtkInitializeBubbleHelp( HWND hwnd, PBUBBLEHELPDATA pbhd, PHWND phwndBubbleHelp);
BOOL APIENTRY WtkTerminateBubbleHelp( HWND hwndBubbleHelp);
BOOL APIENTRY WtkActivateBubbleHelp( HWND hwndBubbleHelp);
BOOL APIENTRY WtkDeactivateBubbleHelp( HWND hwndBubbleHelp);
BOOL APIENTRY WtkUpdateBubbleHelp( HWND hwndBubbleHelp, PSZ pszText);

#pragma pack()

#ifdef __cplusplus
        }
#endif

#endif /* WTKBBH_INCLUDED */

