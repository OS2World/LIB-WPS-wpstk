/*
 *      OFF.CMD - WPS Toolkit - Christian Langanke 2000
 *
 *      Syntax: OFF
 *
 *      This batchfile destroys the test object created with on.cmd and
 *      deregisters the WPS class, thus making the DLL available for recompile.
 *
 */
/* First comment is being used as online helptext */
/* $Id: off.cmd,v 1.3 2003-10-22 19:21:14 cla Exp $ */
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


 '@ECHO OFF'
 TRUE         = (1 = 1);
 FALSE        = (0 = 1);
 call RxFuncAdd    'SysLoadFuncs', 'RexxUtil', 'SysLoadFuncs'
 call SysLoadFuncs

 /* setup some strings (nned to be the same as in ON.CMD */
 FolderId    = '<SETTCLASS_FOLDER>';
 ObjectId    = '<SETTCLASS_OBJECT>';
 ClassName   = 'SettingsClass';

 /* destroy test object, if it exists */
 IF (SysSetObjectData( ObjectId, ';')) THEN
 DO
    CALL CHAROUT, 'Destroying test object ...';
    IF (SysDestroyObject( ObjectId)) THEN
       SAY ' Ok.';
    ELSE
       SAY ' Error.';
 END;
 ELSE
    SAY 'warning: test object does not exist.';

 /* destroy folder if it exists */
 IF (SysSetObjectData( FolderId, ';')) THEN
 DO
    CALL CHAROUT, 'Destroying test folder ...';
    IF (SysDestroyObject( FolderId)) THEN
       SAY ' Ok.';
    ELSE
       SAY ' Error.';
 END;
 ELSE
    SAY 'warning: test folder does not exist.';

 /* deregister test WPS class, if there */
 fDeregister = FALSE;
 List.0 = 0;
 rc = SysQueryClassList( 'List.');
 IF (List.0 = 0) THEN
    fDeregister = TRUE;
 ELSE
 DO i = List.0 TO 1 BY -1
    PARSE VAR List.i ThisClass .;
    IF (ThisClass = ClassName) THEN
    DO
       fDeregister = TRUE;
       LEAVE;
    END;
 END;

 IF (fDeregister) THEN
 DO
    CALL CHAROUT, 'Deregistering test WPS class ...';
    IF (SysDeregisterObjectClass( ClassName)) THEN
       SAY ' Ok.';
    ELSE
       SAY ' Error.';
 END;
 ELSE
    SAY 'test WPS class is not registered.';

 /* always return zero for no erorr */
 EXIT(0);

