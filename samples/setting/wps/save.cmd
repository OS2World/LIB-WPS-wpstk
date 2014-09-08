/*
 *      SAVE.CMD - WPS Toolkit - Christian Langanke 2000
 *
 *      Syntax: SAVE
 *
 *      Use this batch to easily save the test object created with on.cmd.
 *      This will invoke the moethod _wpSaveState of the test object.
 */
/* $Id: save.cmd,v 1.2 2003-10-22 19:21:14 cla Exp $ */
/*
 * This file is part of the WPS Toolkit package and is free software.  You can
 * redistribute it and/or modify it under the terms of the GNU Library General
 * Public License as published by the Free Software Foundation, in version 2
 * as it comes in the "COPYING.LIB" file of the WPS Toolkit main distribution.
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
 * License for more details.
 */

 call RxFuncAdd    'SysLoadFuncs', 'RexxUtil', 'SysLoadFuncs'
 call SysLoadFuncs

 Object = '<SETTCLASS_OBJECT>';

 CALL CHAROUT, "Saving" Object "...";
 rc = SysSaveObject( Object, 0);
 IF (rc = 1) then
    SAY " Ok.";
 ELSE
    SAY " ERROR!";

