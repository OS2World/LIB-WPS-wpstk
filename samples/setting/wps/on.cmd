/*
 *      ON.CMD - WPS Toolkit - Christian Langanke 2000
 *
 *      Syntax: ON
 *
 *      This batch file will prepare the test of the test WPS class.
 *      The program PM Printf is being used for displaying debug informations.
 *      For this purpose you will have to compile the WPS toolkit (or at least
 *      wtkset.c) with DEBUG being turned on.
 *
 */
/* First comment is being used as online helptext */
/* $Id: on.cmd,v 1.3 2005-03-26 16:38:56 cla Exp $ */
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

 /* get compile directory */
 PARSE ARG CompileDir .;
 CompileDir = STRIP( CompileDir);
 IF (CompileDir = '') THEN
 DO
    'call make install';
    RETURN( rc);
 END;

 /* is pmprintf available ? */
 ExecPmPrintf = SysSearchPath( 'PATH', 'pmprintf.exe');
 IF (ExecPmPrintf \= '') THEN
 DO
    /* is pmprintf already active ? Else start it.            */
    /* NOTE:                                                  */
    /*  this will not work within EPM, because within the EPM */
    /*  window neither redirection nor piping is possible.    */
    /*  So pmprintf will always start, causing an error !     */

    'call pstat | find "PMPRINTF.EXE" > NUL'
    IF (rc = 1) THEN
    DO
       'start pmprintf'
       rc = SysSleep(1)
    END;
 END;


 /* setup some strings */
 ClassDll    = STREAM( CompileDir'\settcls.dll', 'C', 'QUERY EXISTS');
 ClassName   = 'SettingsClass';

 FolderId    = '<SETTCLASS_FOLDER>';
 FolderName  = 'Details';
 FolderSetup = 'CCVIEW=NO;DEFAULTVIEW=DETAILS;DETAILSCLASS='ClassName';';

 ObjectId    = '<SETTCLASS_OBJECT>';
 ObjectName  = 'Test Connection';
 ObjectSetup = 'USER=cla;Password=toto;Phonenumber=0211444691;IPCONFIG=STATIC;NETMASK=255.255.255.0;MTU=256;NAMESERVER=149.221.247.30;OPEN=DEFAULT;TITLE=abc;';

 IF (ClassDLL = '') THEN
 DO
    SAY 'class dll not fould. Invoke make to build this file.';
    EXIT(2);
 END;

 /* now prepare the test */
 fSuccess = FALSE;
 DO UNTIL (TRUE)
    /* register class */
    CALL CHAROUT, 'Registering' ClassName 'of' ClassDll '...';
    IF (\SysRegisterObjectClass( ClassName, ClassDll)) THEN
       LEAVE;
    SAY ' Ok.';

    /* recreate folder */
    CALL CHAROUT, 'Creating folder' FolderName '...';
    IF (\SysCreateObject( 'WPFolder', 'Details', '<WP_DESKTOP>', FolderSetup';OBJECTID='FolderId';', 'U')) THEN
       LEAVE;
    SAY ' Ok.';

    /* create object of our class */
    CALL CHAROUT, 'Creating test object' ObjectName '...';
    IF (\SysCreateObject( ClassName,  ObjectName,  FolderId, 'OBJECTID='ObjectId';')) THEN
       LEAVE;
    SAY ' Ok.';

    /* modify setup in a separate call (to test _wpSetup) */
    CALL CHAROUT, 'Modifying setup of test object ...';
    IF (\SysSetObjectData( ObjectId, ObjectSetup)) THEN
       LEAVE;
    SAY ' Ok.';

    /* open the folder, send string twice */
    /* to bring folder to foreground      */
    /* Note see FolderSetup               */
    /*   - details view is default        */
    /*   - CCVIEW must be set to NO       */

    CALL CHAROUT, 'Open folder' FolderName '...';
    fSuccess = SysSetObjectData( ObjectId, 'OPEN=DEFAULT;');
    fSuccess = SysSetObjectData( ObjectId, 'OPEN=DEFAULT;');
    IF (fSuccess) THEN
       SAY ' Ok.';

 END;

 IF (\fSuccess) THEN
    SAY 'ERROR !';

 /* return zero for no error */
 EXIT(\fSuccess);

