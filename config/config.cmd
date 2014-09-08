/*
 *      CONFIG.CMD - WPS Toolkit - Christian Langanke 2000-20006
 *
 *      Syntax: CONFIG maketype outfile programlist
 *
 *      /?               - display this help
 *      maketype         - either NMAKE or GMAKE
 *      outfile          - file for to write configuration macros
 *                         The pathname may not contain blanks !
 *      programlist      - list of executables to be searched
 *
 *      This program writes the following variables to the output file:
 *
 *        CFG_COMPILER
 *
 *          for maketype NMAKE only:
 *              cset2  - IBM C/C++ Tools Version 2.01
 *              vac3   - IBM VisualAge C++ for OS/2, Version 3
 *              watcom - Open Watcom C/C++ Version 1.5
 *
 *          for maketype GMAKE only:
 *              emx  - EMX GCC
 *              gcc  - Innotek GCC
 *
 *        CFG_WARPTK
 *              V3   - IBM Developer's Toolkit for OS/2 Version 3
 *              V4   - IBM Developer's Toolkit for OS/2 Warp Version 4 / Version 4.5
 *
 *        CFG_TREE
 *              DEV  - running in a development tree (BINDIR == LIBDIR)
 *              REL  - running in a binary release   (separate LIBIDR)
 *
 *        CFG_RC
 *              0    - script ran successful
 *              >0   - OS/2 error code
 */
/* First comment is being used as online helptext */
/* $Id: config.cmd,v 1.15 2008-12-14 22:46:00 cla Exp $ */
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

 SIGNAL ON HALT;

 TitleLine = STRIP(SUBSTR(SourceLine(2), 3));
 PARSE VAR TitleLine CmdName'.CMD 'Info
 Title     = CmdName Info

 env          = 'OS2ENVIRONMENT';
 TRUE         = (1 = 1);
 FALSE        = (0 = 1);
 Redirection  = '> NUL 2>&1';
 '@ECHO OFF'

 /* OS/2 errorcodes */
 ERROR.NO_ERROR           =  0;
 ERROR.FILE_NOT_FOUND     =  2;
 ERROR.ACCESS_DENIED      =  5;
 ERROR.GEN_FAILURE        = 31;
 ERROR.INVALID_PARAMETER  = 87;

 GlobalVars = 'Title CmdName env TRUE FALSE Redirection ERROR.';

 /* show help */
 ARG Parm .
 IF (POS('/?', Parm) > 0) THEN
 DO
    rc = ShowHelp();
    EXIT( rc);
 END;

 /* load REXXUTIL */
 call RxFuncAdd    'SysLoadFuncs', 'RexxUtil', 'SysLoadFuncs'
 call SysLoadFuncs

 /* default values */
 GlobalVars = GlobalVars 'fVerbose CrLf';
 rc         = ERROR.NO_ERROR;
 fVerbose   = TRUE;
 CrLf       = '0d0a'x;

 MakeType       = '';
 OutFile        = '';

 DO UNTIL (TRUE)


    /* check parms */
    PARSE ARG MakeType Outfile ProgramList;
    MakeType = TRANSLATE( STRIP( MakeType));
    fIsNmake = (MakeType = 'NMAKE');

    IF (OutFile = '') THEN
    DO
       rc = ERROR.INVALID_PARAMETER;
       LEAVE;
    END;

    OutFile = TRANSLATE( OutFile, '\', '/');
    OutDir = GetDrivePath( OutFile);
    rcx = SysMkDir( OutDir);

    /* check programs */
    IF (ProgramList \= '') THEN
    DO

       /* query (BEGIN)(END)LIBPATH, if necessary */
       IF (POS('.DLL', TRANSLATE(ProgramList)) > 0) THEN
       DO
          /* Note: VALUE function of REXX will not return BEGIN|ENDLIBPATH */
          /*       as these are not true environment variables !           */
          rc = SETLOCAL();
          'SET CFG_LIBPATH=%BEGINLIBPATH%;'GetLibpath()';%ENDLIBPATH%'
       END;

       /* search the programs */
       MissingProgs = '';
       InvalidProgs = '';
       DO WHILE (ProgramList \= '')

          PARSE VAR ProgramList ThisExecutable ProgramList;

          /* no backslash allowed ! */
          IF (POS('\', ThisExecutable) > 0) THEN
          DO
             InvalidProgs = InvalidProgs ThisExecutable;
             ITERATE;
          END;

          /* extension required */
          IF (POS('.', ThisExecutable) = 0) THEN
          DO
             InvalidProgs = InvalidProgs ThisExecutable;
             ITERATE;
          END;

          /* no backslash allowed ! */
          ThisExtension = TRANSLATE(SUBSTR( ThisExecutable, LASTPOS( '.', ThisExecutable)));
          SELECT
             WHEN (ThisExtension = '.DLL') THEN EnvVar = 'CFG_LIBPATH';
             OTHERWISE                          EnvVar = 'PATH';
          END;

          IF (SysSearchPath( EnvVar, ThisExecutable) = '') THEN
          DO
             MissingProgs = MissingProgs ThisExecutable;
             ITERATE;
          END;
       END;

       /* handle errors */
       IF (InvalidProgs \= '') THEN
       DO
          SAY CmdName': error: invalid programname:'InvalidProgs;
          rc = ERROR.INVALID_PARAMETER;
       END;

       IF (MissingProgs \= '') THEN
       DO
          SAY CmdName': error: missing executable(s):'MissingProgs;
          rc = ERROR.FILE_NOT_FOUND;
       END;
    END;

    /* check compiler */
    IF (fIsNMake) THEN
    DO
       SELECT
          WHEN (CheckOS2Component( 'ICC.EXE', '..\SYSLEVEL.CT3',          '562201703', '2.0')) THEN Compiler = 'cset2'
          WHEN (CheckOS2Component( 'ICC.EXE', '..\SYSLEVEL\SYSLEVEL.CT3', '562201703', '3.0')) THEN Compiler = 'vac3';
          WHEN (SysSearchPath( 'PATH', 'WCC386.EXE') \= '')                                    THEN Compiler = 'watcom';
          OTHERWISE
          DO
             Compiler = '';
             rc = ERROR.FILE_NOT_FOUND;
          END;
       END;
    END;
    ELSE
    DO
       SELECT
          WHEN (SysSearchPath( 'PATH', 'GCC.EXE') \= '') THEN Compiler = CheckGccType();
          OTHERWISE
          DO
             Compiler = '';
             rc = ERROR.FILE_NOT_FOUND;
          END;
       END;
    END;

    /* check toolkit */
    SELECT
       WHEN (CheckOS2Component( 'DLGEDIT.EXE', 'SYSLEVEL.TLK', '562268600', '3.0')) THEN Toolkit = 'V3';
       WHEN (CheckOS2Component( 'DLGEDIT.EXE', 'SYSLEVEL.TLK', '562268600', '4.0')) THEN Toolkit = 'V4';
       WHEN (CheckOS2Component( 'DLGEDIT.EXE', 'SYSLEVEL.TLK', '5639F9300', '4.5')) THEN Toolkit = 'V4';
       OTHERWISE
       DO
          Toolkit = '';
          rc = ERROR.FILE_NOT_FOUND;
       END;
    END;

    /* check if make is running on a binary release install */
    PARSE SOURCE . . CallName;
    CallDir = LEFT( CallName, LASTPOS( '\', CallName) - 1);
    IF (DirExist( CallDir'\..\lib')) THEN
       Tree = 'REL';
    ELSE
       Tree = 'DEV';

    /* make some GCC special configuration tasks */
    rc = GccConfig( MakeType, OutDir, Compiler, Toolkit);

    /* write output file */
    cp = '';
    if (fIsNmake) THEN cp='!'
    rcx = SysFileDelete( OutFile);
    rcx = LINEOUT( OutFile, cp'ifndef CFG_COMPILER');
    rcx = LINEOUT( OutFile, 'CFG_COMPILER='Compiler);
    rcx = LINEOUT( OutFile, cp'endif');
    rcx = LINEOUT( OutFile, cp'ifndef CFG_WARPTK');
    rcx = LINEOUT( OutFile, 'CFG_WARPTK='Toolkit);
    rcx = LINEOUT( OutFile, cp'endif');
    rcx = LINEOUT( OutFile, cp'ifndef CFG_TREE');
    rcx = LINEOUT( OutFile, 'CFG_TREE='Tree);
    rcx = LINEOUT( OutFile, cp'endif');
    rcx = LINEOUT( OutFile, 'CFG_RC='rc);
    rcx = LINEOUT( OutFile);

 END;

 EXIT( rc);

/* ------------------------------------------------------------------------- */
HALT:
  SAY 'Interrupted by user.';
  EXIT(ERROR.GEN_FAILURE);

/* ------------------------------------------------------------------------- */
ShowHelp: PROCEDURE  EXPOSE (GlobalVars)

 SAY;
 SAY Title
 SAY;

 DO i = 4 TO SOURCELINE()
    ThisLine = SOURCELINE(i);
    IF (ThisLine = ' */') THEN LEAVE;
    SAY SUBSTR(ThisLine, 7);
 END;

 RETURN( ERROR.INVALID_PARAMETER);

/* ------------------------------------------------------------------------- */
FileExist: PROCEDURE
 PARSE ARG FileName

 RETURN(STREAM(Filename, 'C', 'QUERY EXISTS') > '');

/* ------------------------------------------------------------------------- */
GetInstDrive: PROCEDURE EXPOSE env
 ARG DirName, EnvVarName

 IF (DirName = '') THEN DirName = '\OS2';
 IF (EnvVarName = '') THEN EnvVarName = 'PATH';
 PathValue = TRANSLATE(VALUE(EnvVarName,,env));

 DirName = TRANSLATE(':'DirName';');
 EntryPos = POS(DirName, PathValue) - 1;
 IF (EntryPos = -1) THEN
    RETURN('');
 InstDrive = SUBSTR(PathValue, EntryPos, 2);
 RETURN(InstDrive);

/* ------------------------------------------------------------------------- */
GetDrivePath: PROCEDURE  EXPOSE (GlobalVars)
 PARSE ARG FileName

 FullPath = FILESPEC('D', FileName)||FILESPEC('P', FileName);
 IF (FullPath \= '') THEN
    RETURN(LEFT(FullPath, LENGTH(FullPath) - 1));
 ELSE
    RETURN('');

/* ------------------------------------------------------------------------- */
GetLibpath: PROCEDURE EXPOSE (GlobalVars)

 BootDrive = GetInstDrive();
 rc = SysFileSearch('LIBPATH=', BootDrive'\config.sys', 'line.');
 IF ((rc \= ERROR.NO_ERROR) | (Line.0 = 0)) THEN
    RETURN('');
 PARSE VAR Line.1 'LIBPATH='LibPath;
 RETURN( STRIP( LibPath));

/* ------------------------------------------------------------------------- */
DirExist: PROCEDURE
 PARSE ARG Dirname;

 rc = SysFileTree( DirName, 'Dir.', 'DO');
 RETURN( (rc = 0) & (Dir.0 = 1));

/* ========================================================================= */
CheckOS2Component: PROCEDURE EXPOSE (GlobalVars)
 PARSE ARG Executable, RelPath, ComponentId, ComponentVersion;

 fResult        = FALSE;
 ComponentTitle = '';
 SyslevelFile   = '';

 DO UNTIL (TRUE)
    /* check parms */
    IF ((Executable = '')  |,
        (RelPath = '')     |,
        (ComponentId = '') |,
        (ComponentVersion = '')) THEN
       LEAVE;

    /* search executable */
    FullName = SysSearchPath( 'PATH', Executable);
    IF (FullName = '') THEN
       LEAVE;

    /* determine syslevel file */
    SyslevelFile = LEFT( FullName, LASTPOS( '\', FullName))''RelPath;
    SysLevelFile = STREAM( SyslevelFile, 'C', 'QUERY EXISTS');
    IF (SysLevelFile = '') THEN
       LEAVE;

    /* read header */
    header        = C2D(CHARIN(SyslevelFile,,2));
    sig           =     CHARIN(SyslevelFile,,8);
    DateJulian    = C2D(CHARIN(SyslevelFile,,5));
    sVersion      = C2D(CHARIN(SyslevelFile,,2));
    reserved      =     CHARIN(SyslevelFile,,16);
    offset        = C2D(REVERSE(CHARIN(SyslevelFile,,4))) + 1;

    /* is signature valid */
    IF (sig \= 'SYSLEVEL') THEN
       LEAVE;

    /* read table */
    sysid          = C2D(REVERSE(CHARIN(SyslevelFile,offset, 2)));
    edition        = C2D(CHARIN(SyslevelFile,,1));
    version        = D2X(C2D(CHARIN(SyslevelFile,,1)));
    modify         = C2D(CHARIN(SyslevelFile,,1));
    DateValue      = CHARIN(SyslevelFile,,2);

    currCsd        = CHARIN(SyslevelFile,,8);
    prevCsd        = CHARIN(SyslevelFile,,8);
    sysName        = CHARIN(SyslevelFile,,80);
    compId         = CHARIN(SyslevelFile,,9); /* ignore the rest */

    /* transform version */
    version = LEFT(Version, 1)'.'SUBSTR(version, 2);

    /* check compid */
    IF (compId  \= ComponentId)      THEN LEAVE;
    IF (version \= ComponentVersion) THEN LEAVE

    /* component is valid */
    fResult = TRUE;

 END;

 IF (SyslevelFile \= '') THEN
    rcx = STREAM( SyslevelFile, 'C', 'CLOSE');

 RETURN( fResult);

/* ========================================================================= */
GccConfig: PROCEDURE EXPOSE (GlobalVars)
 PARSE ARG MakeType, OutDir, Compiler, Toolkit

 fHeaderDisplayed = FALSE;
 BadWords         = '_Far16 _Seg16 _System _Pascal';
 CompilerDir      = OutDir'\'Compiler;
 IncludeDir       = CompilerDir'\'Toolkit;

 /* search names of headers to be patched in toolkit?\h with: grep "\\ $" *.h */
 BadWPHeader       = 'wpobject.h wpfolder.h';

 /* only for GCC ! */
 IF (WORDPOS( TRANSLATE( Compiler), 'EMX GCC') = 0) THEN
    RETURN(0);

 /* not if target clean is specified */
 IF (MakeType = 'CLEAN') THEN
    RETURN(0);

 /* create directories, if not exist */
 rcx = SysMkDir( CompilerDir);
 rcx = SysMkDir( IncludeDir);

 /* ---- EMX only -------------------------------- */

 IF (Compiler = 'emx') THEN
 DO

    /* patch out thunking keywords of os2def.h  */
    IF (Toolkit = 'V4') THEN
    DO
       /* get file names */
       CheckFile = 'os2def.h';
       SourceFile = SysSearchPath( 'INCLUDE', CheckFile);
       TargetFile = IncludeDir'\'FILESPEC('N', CheckFile);
       IF (SourceFile = '') THEN
       DO
          SAY CmdName': error: cannot find:' CheckFile;
          EXIT( ERROR.FILE_NOT_FOUND);
       END;
       IF (\FileExist( TargetFile)) THEN
       DO
          IF (fVerbose) THEN
             GccConfigMessage( '- prepare' TargetFile 'of Toolkit for OS/2 WARP 4 for emx usage');

          /* scan thru all lines */
          DO WHILE (LINES( SourceFile) > 0)
             ThisLine = LINEIN( SourceFile);

             NewLine = '';
             DO WHILE (ThisLine \= '')
                PARSE Var ThisLine ThisWord ThisLine;
                IF (WORDPOS( ThisWord, BadWords) = 0) THEN
                   NewLine = NewLine ThisWord;
             END;
             ThisLine = STRIP( NewLine);

             rc = LINEOUT( TargetFile, ThisLine);
          END;

          rc = STREAM( SourceFile, 'C', 'CLOSE');
          rc = STREAM( TargetFile, 'C', 'CLOSE');
       END;
    END;

    /* patch out false blanks of toolkit headers */
    DO WHILE (BadWPHeader \= '')
       PARSE VAR BadWPHeader CheckFile BadWPHeader;
       SourceFile = SysSearchPath( 'INCLUDE', CheckFile);
       TargetFile = IncludeDir'\'FILESPEC('N', CheckFile);
       IF (SourceFile = '') THEN
       DO
          SAY CmdName': error: cannot find:' CheckFile;
          EXIT( ERROR.FILE_NOT_FOUND);
       END;

       IF (\FileExist( TargetFile)) THEN
       DO
          IF (fVerbose) THEN
             GccConfigMessage( '- prepare' TargetFile 'for emx usage');
          /* scan thru all lines */
          DO WHILE (LINES( SourceFile) > 0)
             ThisLine = LINEIN( SourceFile);

             /* check for errant blanks after backslash at end of line */
             IF (RIGHT( STRIP( ThisLine), 1) = '\') THEN
             DO
                DO WHILE (RIGHT( ThisLine, 1) \= '\')
                   ThisLine = LEFT( ThisLine, LENGTH( ThisLine) - 1);
                END;
             END;

             rc = LINEOUT( TargetFile, ThisLine);
          END;

          rc = STREAM( SourceFile, 'C', 'CLOSE');
          rc = STREAM( TargetFile, 'C', 'CLOSE');
       END;
    END;

    /* create omf version of emx libs */
    ExludeList = 'check.lib';
    IF (SysSearchPath( 'LIBRARY_PATH', 'os2.lib') = '') THEN
    DO
       SourceFile = SysSearchPath( 'LIBRARY_PATH', 'os2.a');
       IF (SourceFile \= '') THEN
       DO
          ExcludeList = TRANSLATE( ExludeList);
          SourcePath = GetDrivePath( SourceFile);
          IF (fVerbose) THEN
             GccConfigMessage( '- convert emx libraries in' SourcePath 'for OMF usage');

          /* search emx libraries */
          rc = SysFileTree( SourcePath'\*.a', 'Lib.', 'FOS');
          IF (rc = 0) THEN
          DO i = 1 TO Lib.0
             /* does library exist ? */
             SourceFile     = Lib.i;
             TargetFile     = LEFT( SourceFile, LASTPOS( '.', SourceFile))'lib';
             TargetFileName = TRANSLATE( FILESPEC( 'N', TargetFile));
             IF ((WORDPOS( TargetFileName , ExcludeList) = 0) & (\FileExist( TargetFile))) THEN
                'call emxomf -o' TargetFile SourceFile;
          END;
       END;
    END;
 END;

 /* ---------------------------------------------- */

 IF (fHeaderDisplayed) THEN
    SAY;

 RETURN 0;

/* ========================================================================= */
GccConfigMessage: PROCEDURE EXPOSE (GlobalVars) fHeaderDisplayed
 PARSE ARG Message;

 IF ((fVerbose) & (\fHeaderDisplayed)) THEN
 DO
    SAY;
    SAY 'Setup configuration for EMX for OS/2';
    SAY '------------------------------------';
    fHeaderDisplayed = TRUE;
 END;
 SAY Message;

 RETURN('');

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

