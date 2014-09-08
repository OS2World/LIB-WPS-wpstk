/*
 *      SEND.CMD - WPS Toolkit - Christian Langanke 2000
 *
 *      Syntax: send <setup string>
 *
 *      Use this batch to easily send settings strings to the test
 *      object created with on.cmd. This will invoke the method
 *      _wpSetup of the test object.
 *
 *      Example: send LOCALIP=1.2.3.4;
 */
/* $Id: send.cmd,v 1.2 2003-10-22 19:21:14 cla Exp $ */
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

 Object = "<SETTCLASS_OBJECT>";

 PARSE ARG Parms;
 rc = SysSetObjectData( Object, Parms);

