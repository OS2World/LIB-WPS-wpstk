/*
 *      GENEXPLIB.CMD - WPS Toolkit - Christian Langanke 2000-2008
 *
 *      Syntax: genexplib libfile sourcefiles [sourcefiles[...]]
 *
 *      This program generates an import library from "#pragma import"
 *      statements of C source files. This is necessary for compilers like
 *      gcc not supporting this pragma statement.
 *
 *      NOTE: source pathnames may include wildcards, but no space characters!
 */

/* $Id: genexplib.cmd,v 1.5 2008-10-15 16:43:14 cla Exp $ */
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
 ERROR.NO_ERROR           =   0;
 ERROR.INVALID_FUNCTION   =   1;
 ERROR.FILE_NOT_FOUND     =   2;
 ERROR.PATH_NOT_FOUND     =   3;
 ERROR.ACCESS_DENIED      =   5;
 ERROR.NOT_ENOUGH_MEMORY  =   8;
 ERROR.INVALID_FORMAT     =  11;
 ERROR.INVALID_DATA       =  13;
 ERROR.NO_MORE_FILES      =  18;
 ERROR.WRITE_FAULT        =  29;
 ERROR.READ_FAULT         =  30;
 ERROR.GEN_FAILURE        =  31;
 ERROR.INVALID_PARAMETER  =  87;
 ERROR.ENVVAR_NOT_FOUND   = 203;

 GlobalVars = 'Title CmdName env TRUE FALSE Redirection ERROR.';

 /* show help */
 ARG Parm .
 IF ((Parm = '') | (POS('/?', Parm)) > 0) THEN
 DO
    rc = ShowHelp();
    EXIT( rc);
 END;

 /* load RexxUtil */
 CALL RxFuncAdd    'SysLoadFuncs', 'RexxUtil', 'SysLoadFuncs'
 CALL SysLoadFuncs

 /* default values */
 GlobalVars = GlobalVars 'Function. DataType. DocComment. MissingComment. Related. DefineList fVerbose fDebug fCommentError';
 rc         = ERROR.NO_ERROR;

 TmpDir = VALUE( 'TMP',,env);
 LibFile     = '';
 SourceFiles = '';
 fVerbose    = FALSE;

 Import.   = '';

 DO UNTIL (TRUE)

    /* get parms */
    PARSE ARG Parms;
    DO i = 1 TO WORDS(Parms);
       ThisParm = WORD(Parms, i);
       PARSE VAR ThisParm ThisTag':'ThisValue
       ThisTag = TRANSLATE(ThisTag);
       SELECT

          WHEN (POS(ThisTag, '/VERBOSE') = 1) THEN
             fVerbose = TRUE;


          OTHERWISE
          DO
             /* handle unix style path names */
             ThisParm = TRANSLATE( ThisParm, '\', '/');

             SELECT
                WHEN (LibFile = '') THEN
                   LibFile = ThisParm;
                OTHERWISE
                   SourceFiles = SourceFiles ThisParm;
             END;
          END;
       END;
    END;

    /* enough parms specified ? */
    IF (SourceFiles = '') THEN
    DO
       rc = ShowHelp();
      LEAVE;
    END;

    /* source file there ? */
    DO WHILE (SourceFiles \= '')
       PARSE VAR SourceFiles ThisMask SourceFiles;
       rc = SysFileTree( ThisMask, 'File.', 'FO');
       IF (rc \= 0) THEN
       DO
          SAY 'error in SysFilTree, rc='rc;
          EXIT( rc);
       END;

       IF (File.0 = 0) THEN
       DO
          SAY 'error: file(s)' ThisMask 'not found!';
          EXIT( ERROR.FILE_NOT_FOUND);
       END;
       ELSE
       DO i = 1 TO File.0

          ThisFile = File.i
          IF (fVerbose) THEN
             SAY '-' ThisFile

          rc = SysFileSearch( 'pragma', ThisFile, 'Line.');
          DO j = 1 TO Line.0
             ThisLine = STRIP( Line.j);

             /* check for compiler directive */
             IF (LEFT( ThisLine, 1) \= '#') THEN
                ITERATE;
             ThisLine = SUBSTR( ThisLine, 2);

             /* check for pragma import */
             PARSE VALUE ThisLine WITH Directive Command'('CommandValue')';
             IF (TRANSLATE( Directive) \= 'PRAGMA') THEN ITERATE;
             IF (TRANSLATE( Command) \= 'IMPORT') THEN ITERATE;
             IF (CommandValue = '') THEN ITERATE;

             /* tokenize value */
             PARSE VAR CommandValue Identifier','ExternalName','ModuleName','Ordinal;
             Identifier = STRIP( Identifier);
             PARSE VAR ExternalName '"'ExternalName'"';
             PARSE VAR ModuleName '"'ModuleName'"';
             Ordinal = STRIP( Ordinal);

             /* store import */
             m = WORDPOS( ModuleName, Import._ModuleList);
             IF (m = 0) THEN
             DO
                Import.ModuleName.0 = 0;
                Import._ModuleList                = Import._ModuleList ModuleName;
             END;

             m                                 = Import.ModuleName.0 + 1;
             Import.ModuleName.m._Identifier   = Identifier;
             Import.ModuleName.m._ExternalName = ExternalName;
             Import.ModuleName.m._Ordinal      = Ordinal;
             Import.ModuleName.0               = m;
          END

       END;
    END

    IF (fVerbose) THEN
       SAY '- converting import definitions';
    ModuleList = Import._ModuleList;
    LibList = '';
    DelFile.0 = 0;
    DO WHILE (ModuleList \= '')
       PARSE VAR ModuleList ModuleName ModuleList;

       TmpDefFile = SysTempFilename( TmpDir'\ge?????.def');
       TmpLibFile = SysTempFilename( TmpDir'\ge?????.lib');

       /* generate module definition file for that module */
       rc = LINEOUT( TmpDefFile, 'LIBRARY' ModuleName);
       rc = LINEOUT( TmpDefFile, 'EXPORTS');
       DO m = 1 TO Import.ModuleName.0

          IF (Import.ModuleName.m._Identifier = '') THEN
             InternalName = '';
          ELSE
             InternalName = ' =' Import.ModuleName.m._Identifier;

          DefLine = Import.ModuleName.m._ExternalName InternalName '@'Import.ModuleName.m._Ordinal

          rc = LINEOUT( TmpDefFile, '  'DefLine);
       END;
       rc = LINEOUT( TmpDefFile);

       /* generate import library for that module */
       'implib /nologo' TmpLibFile TmpDefFile;
       LibList = LibList '+' TmpLibFile;

       d = DelFile.0 + 1;
       DelFile.d = TmpLibFile TmpDefFile;
       DelFile.0 = d;
    END;

    /* copy all libs together */
    IF (fVerbose) THEN
       SAY '- copying library file';
    'COPY /B' SUBSTR( LibList, WORDINDEX( LibList, 2)) LibFile Redirection;
    rcSave = Rc;

    IF (fVerbose) THEN
       SAY '- cleaning up';
    DO d = 1 TO DelFile.0
       'DEL' DelFile.d Redirection;
    END;

    rc = rcSave;

 END;

 EXIT( rc);

/* ------------------------------------------------------------------------- */
HALT:
  SAY 'Interrupted by user.';
  EXIT(ERROR.GEN_FAILURE);

/* ------------------------------------------------------------------------- */
ShowHelp: PROCEDURE EXPOSE (GlobalVars)

 SAY;
 SAY Title
 SAY;

 /* read this file directly -> quicker than SOURCELINE() ! */
 PARSE SOURCE . . ThisFile;

 /* skip header */
 DO 3
    rc = LINEIN(ThisFile);
 END;

 /* show main part */
 ThisLine = LINEIN(Thisfile);
 DO WHILE (ThisLine \= ' */')
    SAY SUBSTR(ThisLine, 7);
    ThisLine = LINEIN(Thisfile);
 END;

 /* Datei wieder schlieáen */
 rc = LINEOUT(Thisfile);

 RETURN( ERROR.INVALID_PARAMETER);


