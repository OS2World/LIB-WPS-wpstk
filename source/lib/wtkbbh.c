/****************************** Module Header ******************************\
*
* Module Name: wtkbbh.c
*
* Source for bubble help manager functions.
*
* Copyright (c) WPS Toolkit Project - Christian Langanke 2002
*
* $Id: wtkbbh.c,v 1.15 2004-11-08 19:33:55 cla Exp $
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

#define COMPILE_BUBBLEDPRINTF 0
#define COMPILE_CHECKFOCUS    0

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define INCL_DOS
#define INCL_WIN
#define INCL_GPI
#define INCL_ERRORS
#include <os2.h>

#include "wtkbbh.h"
#include "wpstk.ih"

#define MAX(a,b)        (a > b ? a : b)
#define MIN(a,b)        (a < b ? a : b)

typedef struct _BUBBLEHELPWINDOWDATA {
  USHORT         cbSize;
  HWND           hwnd;             // handle of window that should bring up the bubble help
  HWND           hwndShadowFrame;  // handle of shadow window
  BUBBLEHELPDATA bhd;              // copy of bubble help data
  PSZ            pszText;          // current buffer for help text
  ULONG          ulMaxTextLen;     // length of currently allocated buffer
  ULONG          ulWidth;          // width of last displayed help windo
  ULONG          ulHeight;         // height of last displayed help window
  POINTL         ptlPointer;       // pointer position when las help window was displayed
  BOOL           fTimerActive;     // current status of help window timers
} BUBBLEHELPWINDOWDATA, *PBUBBLEHELPWINDOWDATA;

// define style masks
#define BHELP_STYLE__HMASK (BHELP_STYLE_HLEFT | BHELP_STYLE_HRIGHT | BHELP_STYLE_HCENTER)
#define BHELP_STYLE__VMASK (BHELP_STYLE_VTOP | BHELP_STYLE_VBOTTOM | BHELP_STYLE_VCENTER)
#define BHSHADOW_STYLE__MASK (BHSHADOW_STYLE_RECTL)

// --------------------------------------------------------------------------

// define magic resource id
#define FID_BUBBLETEXT        0x4833

// private window message
#define TIMERID_START        35
#define TIMERID_STOP         36

/* message ids for the bubble help window */
#define BHM_FIRST               (WM_USER + 0x2000)
#define BHM_ACTIVATEBHELP       (BHM_FIRST)
#define BHM_DEACTIVATEBHELP     (BHM_FIRST + 1)
#define BHM_UPDATEBHELP          (BHM_FIRST + 2)

#if COMPILE_BUBBLEDPRINTF
#define BUBBLEDPRINTF(p) printf p
#else
#define BUBBLEDPRINTF(p)
#endif

// --------------------------------------------------------------------------

static BOOL _getTextDimensions( HWND hwndStatic, PULONG pulWidth, PULONG pulHeight, PSZ pszText)
{
         BOOL           fResult = FALSE;
         HPS            hps = NULLHANDLE;
         ULONG          ulChars;

do
   {
   // check parms
   if ((!pulWidth)    ||
       (!pulHeight)   ||
       (!pszText))
      break;

   // get cached presentation space
   hps = WinGetPS( hwndStatic);
   if (!hps)
      break;

   // calculate required height, loop for all lines
   *pulWidth = 0;
   *pulHeight = 0;
   while (*pszText)
       {
                RECTL          rcl = { 0, 0, 32767, 32767};

       // simulate drawing one line
       // - specifying DT_WORDBREAK will produce tighter width value
       ulChars = WinDrawText( hps, -1, pszText, &rcl, 0, 0,
                              DT_LEFT | DT_QUERYEXTENT | DT_WORDBREAK );
       *pulWidth    = MAX( *pulWidth, rcl.xRight - rcl.xLeft);
       *pulHeight  += rcl.yTop - rcl.yBottom;
       pszText     += ulChars;
       }

   // done
   fResult = TRUE;

   } while (FALSE);

// cleanup
if (hps) WinReleasePS( hps);
return fResult;
}

// --------------------------------------------------------------------------

static VOID  _showHelp( HWND hwnd, PBUBBLEHELPWINDOWDATA pbhwd)
{
         BOOL           fResult   = FALSE;

         HAB            hab = WinQueryAnchorBlock( HWND_DESKTOP);
         HPS            hps;
         RECTL          rclWin;

         HWND           hwndFrame = WinQueryWindow( hwnd, QW_PARENT);
         HWND           hwndText  = WinWindowFromID( hwnd, FID_BUBBLETEXT);

         PSZ            pszText   = "*** internal error ***";
         POINTL         ptl;

         SWP            swpFrame;
         SWP            swpText;
         SWP            swpShadow;
         HWND           hwndShadowClient;

         LONG           lScreenX, lScreenY;
         ULONG          ulWidth;
         ULONG          ulHeight;

do
   {
   if (!pbhwd)
      break;

   // display text if available
   if (pbhwd->pszText)
      pszText = pbhwd->pszText;

   // calculate required width and height
   if (!_getTextDimensions( hwndText, &ulWidth, &ulHeight, pszText))
      break;

   // get screen dimentsions
   lScreenX = WinQuerySysValue( HWND_DESKTOP, SV_CXSCREEN);
   lScreenY = WinQuerySysValue( HWND_DESKTOP, SV_CYSCREEN);

   // has window size changed ?
   if ((ulWidth  != pbhwd->ulWidth) ||
       (ulHeight != pbhwd->ulHeight) ||
       (!WinIsWindowVisible( hwndFrame)))
      {
      // hide help and shadow window
      WinShowWindow( hwndFrame, FALSE);
      if (pbhwd->hwndShadowFrame)
         WinShowWindow( pbhwd->hwndShadowFrame, FALSE);

      // give PM a timeslice to cleanup
      // previous bubble help windows
      DosSleep( 0);

      // check pointer pos
      WinQueryPointerPos( HWND_DESKTOP, &ptl);

      // calculate window size
      WinQueryWindowPos( hwndFrame, &swpFrame);
      swpFrame.fl = SWP_SIZE | SWP_MOVE | SWP_SHOW;
      swpFrame.hwndInsertBehind = HWND_TOP;
      swpFrame.cx = ulWidth + (2 * pbhwd->bhd.ulXMargin);
      swpFrame.cy = ulHeight + (2 * pbhwd->bhd.ulYMargin);

      // keep help on top
      // drawback - this may also bring the requesting window to top
      if (pbhwd->bhd.flStyle & BHELP_STYLE_ALWAYSTOP)
         swpFrame.fl |= SWP_ZORDER;

      // ----------------------------------------------

      // now calculate horizintal window pos depending
      switch (pbhwd->bhd.flStyle & BHELP_STYLE__HMASK)
         {
         default:
         case BHELP_STYLE_HLEFT:
            swpFrame.x  = ptl.x - swpFrame.cx - pbhwd->bhd.ptlOffset.x;
            break;

         case BHELP_STYLE_HRIGHT:
            swpFrame.x  = ptl.x + pbhwd->bhd.ptlOffset.x;
            break;

         case BHELP_STYLE_HCENTER:
            swpFrame.x  = ptl.x - (swpFrame.cx / 2);
            break;
         }

      // if window does not fit onto screen, make it fit
      if ((swpFrame.x + swpFrame.cx) > lScreenX)
         swpFrame.x = lScreenX - swpFrame.cx;

      if (swpFrame.x < 0)
         swpFrame.x = 0;

      // ----------------------------------------------

      // now calculate vertical window pos depending
      switch (pbhwd->bhd.flStyle & BHELP_STYLE__VMASK)
         {
         case BHELP_STYLE_VTOP:
            swpFrame.y  = ptl.y + pbhwd->bhd.ptlOffset.y;

            // if window does not fit onto screen, flip to style BHELP_STYLE_VBOTTOM
            if ((swpFrame.y + swpFrame.cy) > lScreenY)
               swpFrame.y  = ptl.y - swpFrame.cy - pbhwd->bhd.ptlOffset.y;
            break;

         default:
         case BHELP_STYLE_VBOTTOM:
            swpFrame.y  = ptl.y - swpFrame.cy - pbhwd->bhd.ptlOffset.y;

            // if window does not fit onto screen, flip to style BHELP_STYLE_VTOP
            if (swpFrame.y < 0)
               swpFrame.y  = ptl.y + pbhwd->bhd.ptlOffset.y;
            break;

         case BHELP_STYLE_VCENTER:
            swpFrame.y  = ptl.y - (swpFrame.cy / 2);

            // if window does not fit onto screen, make it fit
            if ((swpFrame.y + swpFrame.cy) > lScreenY)
               swpFrame.y = lScreenY - swpFrame.cy;

            if (swpFrame.y < 0)
               swpFrame.y = 0;
            break;
         }

      // ----------------------------------------------

      // calculate size of text window
      WinQueryWindowPos( hwndText, &swpText);
      swpText.fl = SWP_SIZE | SWP_MOVE | SWP_SHOW;
      swpText.x  = pbhwd->bhd.ulXMargin;
      swpText.y  = pbhwd->bhd.ulYMargin;
      swpText.cx = ulWidth;
      swpText.cy = ulHeight;

      // ----------------------------------------------

      if (pbhwd->hwndShadowFrame)
         {
         // determine new shadow position
         WinQueryWindowPos( pbhwd->hwndShadowFrame, &swpShadow);
         swpShadow.hwndInsertBehind = hwndFrame;
         swpShadow.fl = SWP_SIZE | SWP_MOVE | SWP_SHOW | SWP_ZORDER;
         swpShadow.x  = swpFrame.x + pbhwd->bhd.ptlShadowOffset.x;
         swpShadow.y  = swpFrame.y + pbhwd->bhd.ptlShadowOffset.y;
         swpShadow.cx = swpFrame.cx;
         swpShadow.cy = swpFrame.cy;
         }

      // ----------------------------------------------

      // set window positions
      fResult = WinSetMultWindowPos( hab, &swpFrame, 1);
      fResult = WinSetMultWindowPos( hab, &swpText, 1);
      if (pbhwd->hwndShadowFrame)
         fResult = WinSetMultWindowPos( hab, &swpShadow, 1);

      // save new window dimensions
      pbhwd->ulWidth  = ulWidth;
      pbhwd->ulHeight = ulHeight;
      }

   // set string
   DosEnterCritSec();
   fResult = WinSetWindowText( hwndText, pszText);
   DosExitCritSec();

   // ----------------------------------------------

   // paint shadow and help window
   hps = WinBeginPaint( hwnd, NULLHANDLE, NULL);
   fResult = WinQueryWindowRect( hwnd, &rclWin);

   // siwtch to RGB if requested
   if (pbhwd->bhd.flStyle & BHELP_STYLE_RGBCOLOR)
      GpiCreateLogColorTable( hps, 0, LCOLF_RGB, 0, 0, NULL);

   // paint background
   WinFillRect( hps, &rclWin, pbhwd->bhd.ulBgColor);

   // paint frame border
   ptl.x = rclWin.xLeft;
   ptl.y = rclWin.yBottom;
   GpiMove( hps, &ptl);
   ptl.x = rclWin.xRight - 1;
   ptl.y = rclWin.yTop - 1;
   GpiSetColor( hps, pbhwd->bhd.ulBorderColor);
   GpiBox( hps, DRO_OUTLINE, &ptl, 0, 0);

   WinEndPaint( hps);

   } while (FALSE);

return;
}

// --------------------------------------------------------------------------

static VOID  _stopHelp( HWND hwnd, PBUBBLEHELPWINDOWDATA pbhwd, BOOL fRestartTimer)
{
         HWND           hwndFrame = WinQueryWindow( hwnd, QW_PARENT);
         HAB            hab = WinQueryAnchorBlock( HWND_DESKTOP);

WinStopTimer( hab, hwnd, TIMERID_START);
WinStopTimer( hab, hwnd, TIMERID_STOP);
pbhwd->fTimerActive = FALSE;

if (WinIsWindowVisible( hwndFrame))
   WinShowWindow( hwndFrame, FALSE);

if ((pbhwd->hwndShadowFrame) &&
    ((WinIsWindowVisible( pbhwd->hwndShadowFrame))))
   WinShowWindow( pbhwd->hwndShadowFrame, FALSE);

// start START timer again
if (fRestartTimer)
   {
   WinStartTimer( hab, hwnd, TIMERID_START, pbhwd->bhd.ulStartTimeout);
   pbhwd->fTimerActive = TRUE;
   }

return;
}

// --------------------------------------------------------------------------

static BOOL _hasPointerPosChanged( PPOINTL pptlPointer)
{

         BOOL           fResult = FALSE;
         POINTL         ptlPointerCurrent;
do
   {
   // check if mouse ptr is still over window
   if (!pptlPointer)
      break;

   // query current pointer position
   if (!WinQueryPointerPos( HWND_DESKTOP, &ptlPointerCurrent))
      break;

   // determine result flag
   fResult = (memcmp( pptlPointer, &ptlPointerCurrent, sizeof( POINTL)) != 0);

   // if changed, store new position
   if (fResult)
      memcpy( pptlPointer, &ptlPointerCurrent, sizeof( POINTL));

   } while (FALSE);

return fResult;
}

// --------------------------------------------------------------------------

static BOOL _isMouseOverWindow( HWND hwnd)
{
         BOOL           fResult = FALSE;
         SWP            swp;
         POINTL         aptlWindow[ 2];
         POINTL         ptl;
do
   {
   // check if mouse ptr is still over window
   if (!WinQueryWindowPos( hwnd, &swp))
      break;

   // check mouse position
   aptlWindow[ 0].x = swp.x;
   aptlWindow[ 0].y = swp.y;
   aptlWindow[ 1].x = swp.x + swp.cx;
   aptlWindow[ 1].y = swp.y + swp.cy;
   WinMapWindowPoints( hwnd, HWND_DESKTOP, aptlWindow, 2);
   WinQueryPointerPos( HWND_DESKTOP, &ptl);

   // mouse still over window
   fResult = ((ptl.x >= aptlWindow[ 0].x) &&
              (ptl.x <= aptlWindow[ 1].x) &&
              (ptl.y >= aptlWindow[ 0].y) &&
              (ptl.y <= aptlWindow[ 1].y));

#if COMPILE_CHECKFOCUS
   // check also if this window or a client of it has the focus
   if (fResult)
      {
               HWND           hwndFocus;
               HWND           hwndParent;

      // show bubble help only if window or a child of it has focus
      hwndParent = WinQueryFocus( HWND_DESKTOP);

      while (hwndParent)
         {
         // check this window
         fResult = (hwnd == hwndParent);
         if (fResult)
            break;

         // get parent of it
         hwndParent = WinQueryWindow( hwndParent, QW_PARENT);

         } // while (hwndParent)

      } // if (fResult)
#endif

   } while (FALSE);

return fResult;
}

// -----------------------------------------------------------------------------

static PFNWP _getWindowProcPtr( HWND hwnd)
{
         HAB            hab = WinQueryAnchorBlock( HWND_DESKTOP);
         CHAR           szClassName[ 20];
         CLASSINFO      ci;

WinQueryClassName( hwnd, sizeof( szClassName), szClassName);
WinQueryClassInfo( hab, szClassName, &ci);
return ci.pfnWindowProc;
}

// --------------------------------------------------------------------------

static MRESULT EXPENTRY _shadowWindowProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{

          PBUBBLEHELPWINDOWDATA pbhwd = (PBUBBLEHELPWINDOWDATA)WinQueryWindowULong( hwnd, QWL_USER);

switch (msg)
   {

   // -----------------------------------------

   case WM_CREATE:

      // save window data ptr
      pbhwd = (PBUBBLEHELPWINDOWDATA) mp1;
      WinSetWindowPtr( hwnd, QWL_USER, pbhwd);

      break; // case WM_CREATE:

   // -----------------------------------------

   case WM_PAINT:
      {

               HPS            hps;
               POINTL         ptl;
               RECTL          rclWin;

      hps = WinBeginPaint( hwnd, NULLHANDLE, NULL);
      WinQueryWindowRect( hwnd, &rclWin);

      if (pbhwd->bhd.flShadowStyle & BHSHADOW_STYLE_RGBCOLOR)
         GpiCreateLogColorTable( hps, 0, LCOLF_RGB, 0, 0, NULL);

      GpiSetColor( hps, pbhwd->bhd.ulShadowColor);
      GpiSetPattern( hps, pbhwd->bhd.ulShadowPattern);
      ptl.x = rclWin.xLeft;
      ptl.y = rclWin.yBottom;
      GpiMove( hps, &ptl);
      ptl.x = rclWin.xRight;
      ptl.y = rclWin.yTop;
      GpiBox( hps, DRO_FILL, &ptl, 0, 0);
      GpiSetPattern( hps, PATSYM_DEFAULT);

      WinEndPaint( hps);
      return 0;
      }
      break;
   }

return WinDefWindowProc( hwnd, msg, mp1, mp2);
}

// --------------------------------------------------------------------------

static MRESULT EXPENTRY _bubbbleHelpWindowProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
          PBUBBLEHELPWINDOWDATA pbhwd = (PBUBBLEHELPWINDOWDATA)WinQueryWindowULong( hwnd, QWL_USER);

switch (msg)
   {

   // -----------------------------------------

   case WM_CREATE:
      // save window data ptr
      pbhwd = (PBUBBLEHELPWINDOWDATA) mp1;
      WinSetWindowPtr( hwnd, QWL_USER, pbhwd);
      break; // case WM_CREATE:

   // -----------------------------------------

   case BHM_ACTIVATEBHELP:
      {
               HAB            hab = WinQueryAnchorBlock( HWND_DESKTOP);

      BUBBLEDPRINTF(( "BHM_ACTIVATEBHELP\n"));

      if (!_isMouseOverWindow( pbhwd->hwnd))
         {
         BUBBLEDPRINTF(( "BHM_ACTIVATEBHELP: stop help, not over window\n"));
         _stopHelp( hwnd, pbhwd, TRUE);
         return (MRESULT) FALSE;
         }

      // if timer is already active, check if mouse is still over window
      if (pbhwd->fTimerActive)
         {
         // check mouse position
         if (_hasPointerPosChanged(  &pbhwd->ptlPointer))
            {
            // pointerpos changed, stop help
            BUBBLEDPRINTF(( "BHM_ACTIVATEBHELP: stop help, ptr pos changed\n"));
            _stopHelp( hwnd, pbhwd, TRUE);

            // start START timer again
            WinStartTimer( hab, hwnd, TIMERID_START, pbhwd->bhd.ulStartTimeout);
            }
         break;
         }

      // save current mouse pointer position and start timer
      WinQueryPointerPos( HWND_DESKTOP, &pbhwd->ptlPointer);
      WinStartTimer( hab, hwnd, TIMERID_START, pbhwd->bhd.ulStartTimeout);
      pbhwd->fTimerActive = TRUE;
      }
      return (MRESULT) TRUE;
      break; // case WM_USER_ACTIVATEBHELP:

   // -----------------------------------------

   case BHM_DEACTIVATEBHELP:

      BUBBLEDPRINTF(( "BHM_DEACTIVATEBHELP\n"));

      // deactivate help
      _stopHelp( hwnd, pbhwd, FALSE);

      return (MRESULT) TRUE;
      break;

   // -----------------------------------------

   case BHM_UPDATEBHELP:
      {
               PSZ            pszNewText = PVOIDFROMMP( mp1);
               ULONG          ulNewBufferLen;
               PSZ            pszNewBuffer;


      if (!pszNewText)
         return (MRESULT) FALSE;

      BUBBLEDPRINTF(( "BHM_UPDATEBHELP\n"));

      // reallocate memory, if new text is too large
      ulNewBufferLen = strlen( pszNewText) + 1;
      if (ulNewBufferLen > pbhwd->ulMaxTextLen)
         {

         if (pbhwd->pszText)
            pszNewBuffer = realloc( pbhwd->pszText, ulNewBufferLen);
         else
            pszNewBuffer = malloc( ulNewBufferLen);

         if (!pszNewBuffer)
            {
            pbhwd->pszText      = NULL;
            pbhwd->ulMaxTextLen = 0;
            return (MRESULT) FALSE;
            }

         // store new pointer and maxsize
         pbhwd->pszText      = pszNewBuffer;
         pbhwd->ulMaxTextLen = ulNewBufferLen;
         }

      // copy text
      DosEnterCritSec();
      strcpy( pbhwd->pszText, pszNewText);
      DosExitCritSec();

//    // make sure that stored pointer pos is ignored
//    memset( &pbhwd->ptlPointer, 0xFF, sizeof( pbhwd->ptlPointer));

      // update window
      WinInvalidateRect( hwnd, NULL, TRUE);

      }
      return (MRESULT) TRUE;
      break;

   // -----------------------------------------

   case WM_TIMER:
      switch (LONGFROMMP( mp1))
         {
         case TIMERID_START:
            {
                     HAB            hab = WinQueryAnchorBlock( HWND_DESKTOP);

            // mouse pointer may already have left the window
            if (!_isMouseOverWindow( pbhwd->hwnd))
               {
               BUBBLEDPRINTF(( "TIMERID_START: stop help, not over window\n"));
               _stopHelp( hwnd, pbhwd, TRUE);
               break;
               }

            // start help window here
            WinStopTimer( hab, hwnd, TIMERID_START);
            _showHelp( hwnd, pbhwd);
            WinStartTimer( hab, hwnd, TIMERID_STOP, pbhwd->bhd.ulStopTimeout);

            // save mouse pointer pos
            WinQueryPointerPos( HWND_DESKTOP, &pbhwd->ptlPointer);
            }
            break;

         case TIMERID_STOP:
            // check if window has to be stopped
            WinPostMsg( hwnd, BHM_ACTIVATEBHELP, 0, 0);
            break;

         }
      break; // case WM_TIMER:

   // -----------------------------------------

   case WM_PAINT:
      // set current text
      _showHelp( hwnd, pbhwd);
      break; // case WM_PAINT:

   // -----------------------------------------

   case WM_DESTROY:

      if (pbhwd)
         {
         free( pbhwd);
         WinSetWindowPtr( hwnd, QWL_USER, NULL);
         }
      break; // case WM_DESTROY

   }

return WinDefWindowProc( hwnd, msg, mp1, mp2);
}



// #############################################################################

// ===========================================================================

/*
@@WtkInitializeBubbleHelp@SYNTAX
This function initializes bubble help support for the specified window.

@@WtkInitializeBubbleHelp@PARM@hwnd@in
The handle of the window requesting bubble help support.

@@WtkInitializeBubbleHelp@PARM@pbhd@in
The address of a buffer containing initialization data
:p.
This parameter is optional. Specify NULL to use default bubble help options.
When using this parameter, specify the appropriate flags in
:hp2.BUBBLEHELPDATA.flModify:ehp2. to modify the defaults.

@@WtkInitializeBubbleHelp@PARM@phwndBubbleHelp@out
The address of a variable containing the window handle of the bubble help.

@@WtkInitializeBubbleHelp@RETURN
Return Code.
:p.
Success flag:
:parml compact break=none.
:pt.TRUE
:pd.Successful initialization
:pt.FALSE
:pd.Error occurred
:eparml.

@@WtkInitializeBubbleHelp@REMARKS
The Bubble Help must be terminated by a call to
:link reftype=hd refid=WtkTerminateBubbleHelp.WtkTerminateBubbleHelp:elink..

The following flags can be specified when using the parameter
:link reftype=hd viewport dependent refid=pbhd_WtkInitializeBubbleHelp.pbhd:elink.&colon.
:p.
:hp2.BUBBLEHELPDATA.flModify:ehp2.
:parml.
:pt.BHELP_MODIFY_ALL
:pd.Use all fields in the strcuture
:pt.BHELP_MODIFY_STYLE
:pd.modify field flStyle
:pt.BHELP_MODIFY_PTLOFFSET
:pd.modify field ptlOffset
:pt.BHELP_MODIFY_COLOR
:pd.modify fields ulFgColor, ulBgColor and ulBorderColor
:pt.BHELP_MODIFY_FONT
:pd.modify field szFont
:pt.BHELP_MODIFY_MARGIN
:pd.modify fields ulXMargin and ulYMargin
:pt.BHELP_MODIFY_STARTTIMEOUT
:pd.modify field ulStartTimeout
:pt.BHELP_MODIFY_STOPTIMEOUT
:pd.modify field ulStopTimeout
:pt.BHELP_MODIFY_SHADOWSTYLE
:pd.modify field flShadowStyle
:pt.BHELP_MODIFY_SHADOWCOLOR
:pd.modify field ulShadowColor
:pt.BHELP_MODIFY_SHADOWPATTERN
:pd.modify field ulShadowPattern
:pt.BHELP_MODIFY_SHADOWOFFSET
:pd.modify field ptlShadowOffset
:eparml.
:p.
:hp2.BUBBLEHELPDATA.flStyle:ehp2.
:parml.
:pt.BHELP_STYLE_HLEFT
:pd.Display help window left to the mouse pointer
:pt.BHELP_STYLE_HRIGHT
:pd.Display help window right to the mouse pointer
:pt.BHELP_STYLE_HCENTER
:pd.Center the help text horizontally under the mouse pointer
:pt.BHELP_STYLE_VTOP
:pd.Display help window above the mouse pointer
:pt.BHELP_STYLE_VBOTTOM
:pd.Display help window below the mouse pointer
:pt.BHELP_STYLE_VCENTER
:pd.Center the help text vertically under the mouse pointer
:pt.BHELP_STYLE_ALWAYSTOP
:pd.Let the bubble help windows always be on top, even when requesting window is in background.
This may be usable for larger bubble help windows.
Beware: as a drawback this may also bring the requesting window to top.
:pt.BHELP_STYLE_RGBCOLOR
:pd.Use the values in ulFgColor, ulBgColor and ulBorderColor as RGB colors
:eparml.
:p.
:hp2.BUBBLEHELPDATA.flShadowStyle:ehp2.
:parml.
:pt.BHSHADOW_STYLE_NONE
:pd.Display no shadow
:pt.BHSHADOW_STYLE_RECTL
:pd.Display a rectangular shadow
:pt.BHSHADOW_STYLE_RGBCOLOR
:pd.Use the value in ulShadowColor as RGB color
:eparml.

@@
*/

BOOL APIENTRY WtkInitializeBubbleHelp( HWND hwnd, PBUBBLEHELPDATA pbhd, PHWND phwndBubbleHelp)
{
         BOOL           fResult = FALSE;
         APIRET         rc = NO_ERROR;
         HAB            hab = WinQueryAnchorBlock( HWND_DESKTOP);

         FRAMECDATA     fcdata;

         HWND           hwndBubbleHelp = NULLHANDLE;
         HWND           hwndClient = NULLHANDLE;
         HWND           hwndText = NULLHANDLE;
         HWND           hwndShadowClient;

         PBUBBLEHELPWINDOWDATA pbhwd = NULL;

         ULONG          flModify;
         ULONG          ulFgColorType;
         ULONG          ulBgColorType;
         PSZ            pszFont;

static   PSZ            pszClientWindowClass = "BubbleHelpClientClass";
static   PSZ            pszShadowClientWindowClass = "BubbleHelpShadowClientClass";

do
   {
   // check parms
   if ((!WinIsWindow( hab, hwnd)) ||
       (!phwndBubbleHelp))
      break;

   // init targets
   *phwndBubbleHelp = NULLHANDLE;

   // get window memory
   pbhwd = malloc( sizeof( BUBBLEHELPWINDOWDATA));
   if (!pbhwd)
      break;
   memset( pbhwd, 0, sizeof( BUBBLEHELPWINDOWDATA));
   pbhwd->cbSize = sizeof( BUBBLEHELPWINDOWDATA);
   pbhwd->hwnd   = hwnd;

   // get memory for help text
   pbhwd->ulMaxTextLen   = 256;
   pbhwd->pszText = malloc( pbhwd->ulMaxTextLen);
   if (!pbhwd->pszText)
      break;
   memset( pbhwd->pszText, 0, pbhwd->ulMaxTextLen);


   // --------------------------------------------------

   // set details data of request or use default values
   if (pbhd)
      {
      memcpy( &pbhwd->bhd, pbhd, sizeof( BUBBLEHELPDATA));
      // if no flags to mask, assume all data should be taken
      if (!pbhwd->bhd.flModify)
         pbhwd->bhd.flModify = BHELP_MODIFY_ALL;
      }
   else
      pbhwd->bhd.cbSize         = sizeof( BUBBLEHELPDATA);

   // use defaults where modification flag is not set
   flModify = pbhwd->bhd.flModify;

   if (!(flModify & BHELP_MODIFY_STYLE))
      pbhwd->bhd.flStyle = BHELP_STYLE_HCENTER | BHELP_STYLE_VBOTTOM;


   if (!(flModify & BHELP_MODIFY_PTLOFFSET))
      {
      pbhwd->bhd.ptlOffset.x = 0;
      pbhwd->bhd.ptlOffset.y = 28;
      }

   if (!(flModify & BHELP_MODIFY_COLOR))
      {
      pbhwd->bhd.ulFgColor     = CLR_BLACK;
      pbhwd->bhd.ulBgColor     = CLR_WHITE;
      pbhwd->bhd.ulBorderColor = CLR_BLACK;
      }

   if (!(flModify & BHELP_MODIFY_FONT))
      strcpy( pbhwd->bhd.szFont, "9.WarpSans");

   if (!(flModify & BHELP_MODIFY_MARGIN))
      {
      pbhwd->bhd.ulXMargin = 6;
      pbhwd->bhd.ulYMargin = 3;
      }

   if (!(flModify & BHELP_MODIFY_STARTTIMEOUT))
      pbhwd->bhd.ulStartTimeout = 1000;

   if (!(flModify & BHELP_MODIFY_STOPTIMEOUT))
      pbhwd->bhd.ulStopTimeout = 100;

   if (!(flModify & BHELP_MODIFY_SHADOWSTYLE))
      pbhwd->bhd.flShadowStyle = BHSHADOW_STYLE_RECTL;

   if (!(flModify & BHELP_MODIFY_SHADOWCOLOR))
      pbhwd->bhd.ulShadowColor = CLR_BLACK;

   if (!(flModify & BHELP_MODIFY_SHADOWPATTERN))
      pbhwd->bhd.ulShadowPattern = PATSYM_HALFTONE;

   if (!(flModify & BHELP_MODIFY_SHADOWOFFSET))
      {
      pbhwd->bhd.ptlShadowOffset.x = 10;
      pbhwd->bhd.ptlShadowOffset.y = -13;
      }

   // --------------------------------------------------

   // create class for client
   if (!WinRegisterClass( hab, pszClientWindowClass, _bubbbleHelpWindowProc, 0, sizeof( PVOID)))
      break;

   // create windows
   memset( &fcdata, 0, sizeof( FRAMECDATA));
   fcdata.cb            = sizeof( FRAMECDATA);
   fcdata.flCreateFlags = FCF_NOBYTEALIGN;
   fcdata.idResources   = 0;

   hwndBubbleHelp = WinCreateWindow( HWND_DESKTOP, WC_FRAME, "", WS_VISIBLE,
                                     0, 0, 0, 0, hwnd, HWND_TOP, -1,
                                     &fcdata, NULL);
   if (!hwndBubbleHelp)
      break;

   hwndClient = WinCreateWindow( hwndBubbleHelp, pszClientWindowClass, "", WS_VISIBLE,
                                 0, 0, 0, 0, hwndBubbleHelp, HWND_TOP, FID_CLIENT, pbhwd, NULL);
   if (!hwndClient)
      break;

   // create text window
   hwndText = WinCreateWindow( hwndClient, WC_STATIC, "", SS_TEXT | DT_LEFT | DT_TOP | DT_WORDBREAK,
                               0, 0, 0, 0, hwndClient, HWND_TOP, FID_BUBBLETEXT, NULL, NULL);
   if (!hwndText)
      break;

   // set fonts of all windows
   pszFont = pbhwd->bhd.szFont;
   WinSetPresParam( hwndBubbleHelp, PP_FONTNAMESIZE, strlen( pszFont) + 1, pszFont);
   WinSetPresParam( hwndClient,     PP_FONTNAMESIZE, strlen( pszFont) + 1, pszFont);
   WinSetPresParam( hwndText,       PP_FONTNAMESIZE, strlen( pszFont) + 1, pszFont);

   // set window color attributes
   if (pbhwd->bhd.flStyle & BHELP_STYLE_RGBCOLOR)
      {
      ulBgColorType = PP_BACKGROUNDCOLOR;
      ulFgColorType = PP_FOREGROUNDCOLOR;
      }
   else
      {
      ulBgColorType = PP_BACKGROUNDCOLORINDEX;
      ulFgColorType = PP_FOREGROUNDCOLORINDEX;
      }

   WinSetPresParam( hwndClient, ulBgColorType, sizeof( ULONG), &pbhwd->bhd.ulBgColor);
   WinSetPresParam( hwndText,   ulBgColorType, sizeof( ULONG), &pbhwd->bhd.ulBgColor);
   WinSetPresParam( hwndText,   ulFgColorType, sizeof( ULONG), &pbhwd->bhd.ulFgColor);

   // --------------------------------------------------

   // create shadow frame
   if (pbhwd->bhd.flShadowStyle & BHSHADOW_STYLE__MASK)
      {

      if (!WinRegisterClass( hab, pszShadowClientWindowClass, _shadowWindowProc, 0, sizeof( PVOID)))
         break;

      // create windows
      memset( &fcdata, 0, sizeof( FRAMECDATA));
      fcdata.cb            = sizeof( FRAMECDATA);
      fcdata.flCreateFlags = FCF_NOBYTEALIGN;
      fcdata.idResources   = 0;

      pbhwd->hwndShadowFrame = WinCreateWindow( HWND_DESKTOP, WC_FRAME, "", WS_VISIBLE,
                                                0, 0, 0, 0, hwnd, HWND_TOP, -1,
                                                &fcdata, NULL);
      if (!pbhwd->hwndShadowFrame)
         break;

      hwndShadowClient = WinCreateWindow( pbhwd->hwndShadowFrame, pszShadowClientWindowClass,
                                          "", WS_VISIBLE, 0, 0, 0, 0,
                                          pbhwd->hwndShadowFrame, HWND_TOP, FID_CLIENT,
                                          pbhwd, NULL);
      if (!hwndShadowClient)
         break;
      }

   // --------------------------------------------------


   // successfully done
   fResult = TRUE;
   *phwndBubbleHelp = hwndBubbleHelp;

   } while (FALSE);

// cleanup
if (!fResult)
   {
   if (pbhwd)
      {
      if (pbhwd->pszText) free( pbhwd->pszText);
      free( pbhwd);
      }
   if (hwndText)       WinDestroyWindow( hwndText);
   if (hwndClient)     WinDestroyWindow( hwndClient);
   if (hwndBubbleHelp) WinDestroyWindow( hwndBubbleHelp);
   }

return fResult;
}

// ---------------------------------------------------------------------------

/*
@@WtkTerminateBubbleHelp@SYNTAX
This function terminates bubble help support previously initialized by a
call to :link reftype=hd refid=WtkInitializeBubbleHelp.WtkInitializeBubbleHelp:elink..

@@WtkTerminateBubbleHelp@PARM@hwndBubbleHelp@in
The address of a variable containing the window handle of the bubble help.

@@WtkTerminateBubbleHelp@RETURN
Return Code.
:p.
Success flag:
:parml compact break=none.
:pt.TRUE
:pd.Successful Termination
:pt.FALSE
:pd.Error occurred
:eparml.

@@WtkTerminateBubbleHelp@REMARKS
-none-

@@
*/

BOOL APIENTRY WtkTerminateBubbleHelp( HWND hwndBubbleHelp)
{
         BOOL           fResult = FALSE;
         HAB            hab = WinQueryAnchorBlock( HWND_DESKTOP);

do
   {
   // check parm
   if (!WinIsWindow( hab, hwndBubbleHelp))
      break;

   WinDestroyWindow( hwndBubbleHelp);

   // successfully done
   fResult = TRUE;

   } while (FALSE);

return fResult;
}

// --------------------------------------------------------------------------

/*
@@WtkActivateBubbleHelp@SYNTAX
This function activates the bubble help
previously initialized by a
call to :link reftype=hd refid=WtkInitializeBubbleHelp.WtkInitializeBubbleHelp:elink..

@@WtkActivateBubbleHelp@PARM@hwndBubbleHelp@in
The address of a variable containing the window handle of the bubble help.

@@WtkActivateBubbleHelp@RETURN
Return Code.
:p.
Success flag:
:parml compact break=none.
:pt.TRUE
:pd.Successful deactivation
:pt.FALSE
:pd.Error occurred
:eparml.

@@WtkActivateBubbleHelp@REMARKS
-none-

@@
*/

BOOL APIENTRY WtkActivateBubbleHelp( HWND hwndBubbleHelp)
{
return (BOOL) WinSendMsg( hwndBubbleHelp, BHM_ACTIVATEBHELP, 0, 0);
}

// --------------------------------------------------------------------------

/*
@@WtkDeactivateBubbleHelp@SYNTAX
This function deactivates the bubble help
previously initialized by a
call to :link reftype=hd refid=WtkInitializeBubbleHelp.WtkInitializeBubbleHelp:elink..

@@WtkDeactivateBubbleHelp@PARM@hwndBubbleHelp@in
The address of a variable containing the window handle of the bubble help.

@@WtkDeactivateBubbleHelp@RETURN
Return Code.
:p.
Success flag:
:parml compact break=none.
:pt.TRUE
:pd.Successful deactivation
:pt.FALSE
:pd.Error occurred
:eparml.

@@WtkDeactivateBubbleHelp@REMARKS
-none-

@@
*/

BOOL APIENTRY WtkDeactivateBubbleHelp( HWND hwndBubbleHelp)
{
return (BOOL) WinSendMsg( hwndBubbleHelp, BHM_DEACTIVATEBHELP, 0, 0);
}

// --------------------------------------------------------------------------

/*
@@WtkUpdateBubbleHelp@SYNTAX
This function updates the bubble help text
previously initialized by a
call to :link reftype=hd refid=WtkInitializeBubbleHelp.WtkInitializeBubbleHelp:elink..

@@WtkUpdateBubbleHelp@PARM@hwndBubbleHelp@in
The address of a variable containing the window handle of the bubble help.

@@WtkUpdateBubbleHelp@PARM@pszText@in
The address of the text to be used as the new bubble help text.

@@WtkUpdateBubbleHelp@RETURN
Return Code.
:p.
Success flag:
:parml compact break=none.
:pt.TRUE
:pd.Successful update
:pt.FALSE
:pd.Error occurred
:eparml.

@@WtkUpdateBubbleHelp@REMARKS
-none-

@@
*/

BOOL APIENTRY WtkUpdateBubbleHelp( HWND hwndBubbleHelp, PSZ pszText)
{
if (!pszText)
   return FALSE;
else
   return (BOOL) WinSendMsg( hwndBubbleHelp, BHM_UPDATEBHELP, MPFROMP( pszText), 0);
}

