/*
 *      MKREL.CMD - WPS Toolkit - Christian Langanke 2000-2008
 *
 *      Syntax: mkrel
 *
 *      This program builds a release zip file. All required files for
 *      all supported compiler versions must exist at that point.
 *
 *      Required programs (not checked !):
 *        - zip.exe / unzip.exe of Info-ZIP
 *        - gnu touch.exe
 */
/* $Id: mkrel.cmd,v 1.22 2008-10-15 16:43:16 cla Exp $ */
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
 env = 'OS2ENVIRONMENT';
 Redirection = '>NUL 2>&1';
 call RxFuncAdd    'SysLoadFuncs', 'RexxUtil', 'SysLoadFuncs'
 call SysLoadFuncs

 rc = 0;
 ZipStem     = 'wpstk';
 DirList     = 'book config samples include'
 RootFiles   = 'COPYING.LIB SUPPORTD ship\*.*';
 SubdirFiles = 'bin\*filt.* bin\wtkenv.cmd bin\xbin2obj.cmd';

 Excludes = '-x CVS/* */CVS/* */*.map */*.obj */*.lst */wpstki.def samples/excluded.c';

 RequiredLibs = 'cset2s cset2m emx vac3 gcc watcom';
 BinList  = 'wpstk.lib wpstki.lib wpstk.dll';

 RequiredFiles = 'book\wtkref.inf';

 TouchList = DirList 'release ship' RootFiles  SubdirFiles;

 DO 1

    SAY;

    /* check if the package is asked for one compiler only */
    PARSE ARG Libname;
    Libname = STRIP( Libname);
    IF (Libname \= '') THEN
    DO
       IF (WORDPOS( TRANSLATE( Libname), TRANSLATE( RequiredLibs)) > 0) THEN
          RequiredLibs = Libname;
       ELSE
       DO
          SAY 'Invalid compiler id specified:' Libname;
          LEAVE;
       END;
    END;

    /* determine call dir */
    PARSE SOURCE . . CallName;
    CallDir = LEFT( CallName, LASTPOS( '\', CallName) - 1);
    rcx = DIRECTORY( Calldir);

    /* determine version number */
    SAY '- determine version number';
    VersionFile = 'config\version.in';
    VersionTag  = 'CFG_VERSION=';
    rc = SysFileSearch( VersionTag, CallDir'\..\config\config.in', 'Line.', 'C');
    IF (rc \= 0) THEN
    DO
       SAY 'Cannot determine version number!'
       SAY 'error in SysFileSearch, rc='rc;
       LEAVE;
    END;
    IF ( Line.0 \= 1) THEN
    DO
       SAY 'Cannot find version number in file' VersionFile;
       rc = 13;
       LEAVE;
    END;
    PARSE VAR Line.1 (VersionTag)VersionNumber;
    IF ((DATATYPE( VersionNumber) \= 'NUM') | (LENGTH( VersionNumber) \= 3)) THEN
    DO
       SAY 'Invalid version number configured:' VersionNumber;
       rc = 13;
       LEAVE;
    END;

    /* determine zip name */
    IF (LibName = '') THEN
       ZipName = ZipStem''VersionNumber'.zip';
    ELSE
       ZipName = ZipStem''VersionNumber'_'LibName'.zip';

    SAY '- building:' ZipName;

    /* check for binary files */
    SAY '- check binary files for:' RequiredLibs;
    TodoList = RequiredLibs;
    IncludeList = '';
    DO WHILE (TodoList \= '')
       PARSE VAR TodoList Thisdir TodoList;

       FileList = BinList;
       DO WHILE (FileList \= '')
          PARSE VAR FileList ThisFile FileList;
          IF (\FileExist( '..\release\'Thisdir'\'ThisFile)) THEN
          DO
             SAY 'File' Thisdir'\'ThisFile 'is missing!';
             EXIT( 14);
          END;
       END;
       IncludeList = IncludeList 'lib\'Thisdir'\wpstk*';

    END;

    /* check for source files */
    SAY '- check source files and doc';
    TodoList = RequiredFiles;
    DO WHILE (TodoList \= '')
       PARSE VAR TodoList ThisFiles TodoList;

       IF (\FileExist( '..\'ThisFiles)) THEN
       DO
          SAY 'File' ThisFiles 'missing!';
          EXIT( 14);
       END;

    END;

    /* build zip */
    rc = DIRECTORY( CallDir'\..');
    SAY '- build zip';
    ZipFile = DIRECTORY()'\'ZipName;
    rcx = SysFileDelete( ZipFile);
    'CALL zip -r' ZipFile DirList     Excludes Redirection;
    'CALL zip -j' ZipFile RootFiles   Excludes Redirection;
    'CALL zip'    ZipFile SubdirFiles Excludes Redirection;

    'REN release lib'
    'CALL zip' ZipFile IncludeList Excludes Redirection;
    'REN lib release';

    /* unzip file to tmp dir */
    SAY '- unpack files'
    TmpDir = SysTempFilename( VALUE('TMP',,env)'\wpstk.???');
    'unzip' ZipFile '-d' TmpDir Redirection;;

    /* touch files */
    'CALL' CallDir'\touchrel' TmpDir;

    /* rezip files */
    SAY '- rebuild zip';
    rcx = SysFileDelete( ZipFile);
    CurDir = DIRECTORY();
    rcx = DIRECTORY( TmpDir);
    'zip -rmo' ZipFile '*' Redirection;
    rcx = DIRECTORY( '..');
    rcx = DIRECTORY( CurDir);

    /* cleanup directory tree */
    rc = SysFileTree( TmpDir'\*', 'Dir.', 'DOS')
    IF (rc = 0) THEN
    DO d = Dir.0 TO 1 BY -1
       rcx = SysRmDir( Dir.d);
    END;
    rcx = SysRmDir( TmpDir);

 END;

 RETURN( rc);

/* ------------------------------------------------------------------------- */
FileExist: PROCEDURE
 PARSE ARG FileName

 RETURN(STREAM(Filename, 'C', 'QUERY EXISTS') > '');

