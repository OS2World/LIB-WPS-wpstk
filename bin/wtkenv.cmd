/*
 *      WTKENV.CMD - WPS Toolkit - Christian Langanke 2000-2008
 *
 *      This script sets un the environment variables
 *      INCLUDE, LIB, BOOKSHELF and HELPNDX for usage of the
 *      Workplace Shell Toolkit for development projects.
 *
 *      Syntax: wtkenv [library_type]
 *
 *      library_type is one of:
 *        cset2s - selects IBM C Set ++  Version 2.1 - single-threaded
 *        cset2m - selects IBM C Set ++  Version 2.1 - multi-threaded
 *        vac3   - IBM VisualAge C++ Version 3.0
 *        gcc    - gcc of emx V0.9d or Innotek 3.3.5
 *        watcom - Open Watcom
 *
 *      If library_type is not specified, it is determined by the
 *      following steps:
 *        - either cset2? or vac3, if icc.exe can be located
 *          - cset2?, if envvar CPPLOCAL is not defined
 *            - cset2s, if envvar WPSTK_THREADMODEL=S
 *            - cset2m
 *          - vac3
 *        - emx, if gcc.exe V2.x can be located
 *        - gcc, if gcc.exe V3.x can be located
 *        - watcom, if wcc386.exe  can be located
 *        - no compiler found, error
 */
/* $Id: wtkenv.cmd,v 1.9 2008-10-15 16:43:16 cla Exp $ */
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

 '@ECHO OFF';
 env = 'OS2ENVIRONMENT';
 CrLf = '0d0a'x;

 CALL RxFuncAdd    'SysLoadFuncs', 'RexxUtil', 'SysLoadFuncs'
 CALL SysLoadFuncs

 /* determine base directory */
 PARSE SOURCE . . CallName;
 CallDir = LEFT( CallName, LASTPOS( '\', CallName) - 1);
 BaseDir = LEFT( CallDir, LASTPOS( '\', CallDir) - 1);
 CmdName = FILESPEC( 'N', CallName);
 rc      = 0;

 /* sepcial directory handling when using development version */
 LibSubDir = VALUE( 'WTKENV_LIBDIR',,env);
 IF (LibSubDir = '') THEN
    LibSubDir = 'lib';

 ValidTypes = 'CSET2S CSET2M VAC3 EMX GCC WATCOM';

 DO 1
    /* get parms */
    PARSE ARG LibType;
    CheckType = TRANSLATE( STRIP( LibType));

    SELECT
       WHEN ( WORDPOS( CheckType, ValidTypes) > 0) THEN NOP;

       WHEN (SysSearchPath( 'PATH', 'ICC.EXE') \= '') THEN
       DO
          /* determine library type */
          IF (VALUE('CPPLOCAL',,env) = '') THEN
          DO
             /* for C Set/2 determine thread model */
             Model = VALUE( 'WPSTK_THREADMODEL',,env);
             Model = LEFT( STRIP( Model), 1);
             IF (Model = '') THEN
                Model = 'M';
             SELECT
                WHEN (Model = 'S') THEN Model = 's';
                WHEN (Model = 'M') THEN Model = 'm';
                OTHERWISE
                DO
                   SAY CmdName': error: invalid thread model' Model 'specified.';
                   'PAUSE';
                   rc = 87;
                   LEAVE;
                END;
             END;

             LibType = 'cset2'Model;
          END;
          ELSE
             LibType = 'vac3';
       END;

       WHEN (SysSearchPath( 'PATH', 'GCC.EXE') \= '') THEN
          LibType = CheckGccType();

       WHEN (SysSearchPath( 'PATH', 'WCC386.EXE') \= '') THEN
          LibType = 'watcom';

       OTHERWISE
       DO
          SAY CmdName': error:'
          SAY 'Could not find any supported compiler!';
          'PAUSE';
          rc = 3;
          LEAVE;
       END;
    END;

    /* determine path- and filenames */
    IncludeDir = BaseDir'\include';
    LibDir     = BaseDir'\'LibSubDir'\'LibType;
    BookDir    = BaseDir'\book';
    IndexFile  = BaseDir'\book\epm\wpstk.ndx';

    CheckFiles = IncludeDir'\wtk.h',
                 LibDir'\wpstk.dll',
                 LibDir'\wpstk.lib',
                 LibDir'\wpstki.lib',
                 BookDir'\wtkref.inf',
                 IndexFile;

    /* check if files are available */
    MissingFiles = '';
    DO WHILE (CheckFiles \= '')
       PARSE VAR CheckFiles ThisFile CheckFiles;
       IF (\FileExist( ThisFile)) THEN
          MissingFiles = MissingFiles SUBSTR( ThisFile, LENGTH( BaseDir) + 2);
    END;
    IF (MissingFiles \= '') THEN
    DO
       SAY CmdName': error:'
       SAY 'Could not setup Workplace Shell Toolkit environment for:' LibType;
       SAY 'The following file(s) could not be found:';
       SAY MissingFiles;
       'PAUSE';
       rc = 2;
       LEAVE;
    END;

    /* extend environment */
    SAY 'setup Workplace Shell Toolkit environment for:' LibType;
    SELECT
       WHEN (LibType = 'gcc') THEN
       DO
          IncludeDir = TRANSLATE( IncludeDir, '/', '\');
          LibDir = TRANSLATE( LibDir, '/', '\');
          'SET C_INCLUDE_PATH='IncludeDir';%C_INCLUDE_PATH%';
          'SET CPLUS_INCLUDE_PATH='IncludeDir';%CPLUS_INCLUDE_PATH%';
          'SET LIBRARY_PATH='LibDir';%LIBRARY_PATH%;';
       END;
       OTHERWISE
       DO
          'SET INCLUDE='IncludeDir';%INCLUDE%';
          'SET LIB='LibDir';%LIB%';
       END
    END
    'SET BOOKSHELF='BookDir';%BOOKSHELF%';
    'SET HELPNDX='IndexFile'+%HELPNDX%';
 END;

 RETURN( rc);

/* ------------------------------------------------------------------------- */
FileExist: PROCEDURE
 PARSE ARG FileName

 RETURN(STREAM(Filename, 'C', 'QUERY EXISTS') > '');

/* ========================================================================= */
CheckGccType: PROCEDURE EXPOSE (GlobalVars)

 Compiler = '';
 DO 1

    /* create private queue */
    QueueName = RXQUEUE('CREATE');
    rc        = RXQUEUE('SET', QueueName);

    /* get version output from compiler */
    'gcc --version 2>&1 | rxqueue' QueueName;

    /* read first line of input */
    IF (QUEUED() > 0) THEN
    DO
       PARSE PULL ThisLine
       IF (WORDS( ThisLine) = 1) THEN
          Version = ThisLine;
       ELSE
          PARSE VAR ThisLine . '('.')' Version .;
       PARSE VAR Version MajorVersion'.'.;
       SELECT
          WHEN (MajorVersion <= 2) THEN Compiler = 'emx';
          OTHERWISE                     Compiler = 'gcc';
       END;
    END;

    rc = RXQUEUE('DELETE', QueueName);
    rc = RXQUEUE('SET', 'SESSION');

 END;

 RETURN( Compiler);

