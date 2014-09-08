/*
 *      CHECKTEST.CMD - WPS Toolkit - Christian Langanke 2002-2008
 *
 *      This program checks the testcase programs for occurrences
 *      of calls to Wtk* APIs and report APIs that are not yet called
 *      within any testcase.
 *
 *      NOTE:
 *      The source files of test programs are not really checked for
 *      wether an API is actually called or not. Except all
 *      occurrences of a symbol starting with 'Wtk' are counted as a
 *      call, even from within comments. This allows to name APIS
 *      within comments that shoudl or cannot be tested, but also are
 *      not to be enlisted here.
 */
/* $Id: checktest.cmd,v 1.9 2008-10-15 16:43:13 cla Exp $ */
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
 IF (POS('/?', Parm) > 0) THEN
 DO
    rc = ShowHelp( );
    EXIT( rc);
 END;

 /* load RexxUtil */
 CALL RxFuncAdd    'SysLoadFuncs', 'RexxUtil', 'SysLoadFuncs'
 CALL SysLoadFuncs

 /* default values */
 GlobalVars = GlobalVars 'fVerbose ';
 rc         = ERROR.NO_ERROR;
 fVerbose   = FALSE;

 CallDir = GetCallDir( );

 DO UNTIL (TRUE)

    /* examine files */
    SAY;
    SAY 'searching toolkit source files';
    ImplementedList = GetApiList( 'include', CallDir'\..\include\*.h', FALSE);
    TestedList = GetApiList( 'test source', CallDir'\..\samples\*.c', TRUE);
    SAY;

    /* remove APIs not truly implemented, but somwhoe stated in sample source */
    CheckList  = TestedList;
    TestedList = '';
    DO WHILE (CheckList \= '')
       PARSE VAR CheckList ThisApi CheckList;
       IF (WORDPOS( ThisApi, ImplementedList) > 0) THEN
          TestedList = TestedList ThisApi;
    END;

    SAY WORDS( ImplementedList) 'APIs found implemented';
    SAY WORDS( TestedList) 'APIs found tested/skipped';

    /* check APIs not tested */
    CheckList    = ImplementedList;
    UntestedList = '';
    DO WHILE (CheckList \= '')
       PARSE VAR CheckList ThisApi CheckList;
       IF (WORDPOS( ThisApi, TestedList) = 0) THEN
          UntestedList = UntestedList ThisApi;
    END;

    /* display final result */
    SAY;
    Untested = WORDS( UntestedList);
    IF (Untested = 0) THEN
       SAY 'All APIs are tested.';
    ELSE
    DO
       Msg = Untested 'untested APIs:';
       SAY Msg;
       SAY COPIES( '-', LENGTH( Msg));
       DO WHILE (UntestedList \= '')
          PARSE VAR UntestedList ThisApi UntestedList
          SAY '-' ThisApi;
       END;
    END;

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

 PARSE SOURCE . . ThisFile;

 DO 3
    rc = LINEIN(ThisFile);
 END;

 ThisLine = LINEIN(Thisfile);
 DO WHILE (ThisLine \= ' */')
    SAY SUBSTR(ThisLine, 7);
    ThisLine = LINEIN(Thisfile);
 END;

 rc = LINEOUT(Thisfile);

 RETURN( ERROR.INVALID_PARAMETER);

/* ------------------------------------------------------------------------- */
FileExist: PROCEDURE
 PARSE ARG FileName

 RETURN( STREAM( Filename, 'C', 'QUERY EXISTS') > '');

/* ------------------------------------------------------------------------- */
GetDrivePath: PROCEDURE
 PARSE ARG FileName

 FullPath = FILESPEC('D', FileName)||FILESPEC('P', FileName);
 IF (FullPath \= '') THEN
    RETURN( LEFT( FullPath, LENGTH(FullPath) - 1));
 ELSE
    RETURN( '');

/* ------------------------------------------------------------------------- */
LOWER: PROCEDURE

 Lower = 'abcdefghijklmnopqrstuvwxyz„”';
 Upper = 'ABCDEFGHIJKLMNOPQRSTUVWXYZŽ™š';

 PARSE ARG String
 RETURN( TRANSLATE( String, Lower, Upper));

/* -------------------------------------------------------------------------- */
GetDirName: PROCEDURE
 PARSE ARG Name

 /* save environment */
 CurrentDrive = FILESPEC('D', DIRECTORY());
 CurrentDir   = DIRECTORY(FILESPEC('D', Name));

 /* try directory */
 DirFound  = DIRECTORY(Name);

 /* reset environment */
 rc = DIRECTORY(CurrentDir);
 rc = DIRECTORY(CurrentDrive);

 RETURN( DirFound);

/* ------------------------------------------------------------------------- */
GetCalldir: PROCEDURE
PARSE SOURCE . . CallName
 CallDir = FILESPEC('Drive', CallName)||FILESPEC('Path', CallName);
 RETURN(LEFT(CallDir, LENGTH(CallDir) - 1));


/* ========================================================================= */
GetApiList: PROCEDURE EXPOSE (GlobalVars)
 PARSE ARG ApiType, SearchMask, fRecursive

 ApiList = '';
 StripChars   = ',:.(\/)*"'
 ReplaceChars = COPIES( ' ', LENGTH( StripChars));

 DO UNTIL( TRUE)
    /* determine options */
    SearchOptions = 'FO';
    IF (fRecursive) THEN
       SearchOptions = SearchOptions'S';

    CALL CHAROUT, '- searching' ApiType 'files ...';
    /* search all include and source files */
    rc = SysFileTree( SearchMask, 'File.', SearchOptions);
    IF (rc \= ERROR.NO_ERROR) THEN
    DO
       SAY;
       SAY 'error in SysFileTree, rc='rc;
       LEAVE;
    END;
    SAY ' Ok.';

    IF (File.0 = 0) THEN
    DO
       SAY CmdName': error: no' ApiType 'files found!'
       rc = ERROR.FILE_NOT_FOUND;
       LEAVE;
    END;

    DO i = 1 TO File.0
       ThisFile = File.i;
       IF (fVerbose) THEN
       DO
          SAY;
          SAY '-------------------'
          SAY ThisFile
          SAY;
       END;
       rcx = STREAM( ThisFile, 'C', 'OPEN READ');
       DO WHILE (LINES( ThisFile) > 0)
          ThisLine = TRANSLATE( STRIP( LINEIN( ThisFile)), ReplaceChars, StripChars);
          wPos = POS( 'Wtk', ThisLine);
          IF (wPos = 0) THEN ITERATE;
          ThisApi = WORD( SUBSTR( ThisLine, wPos), 1);
          IF (LENGTH( ThisApi) = 3) THEN ITERATE;
          IF (WORDPOS( ThisApi, ApiList) = 0) THEN
          DO
             IF (fVerbose) THEN
                SAY '->' ThisApi;
             ApiList = ApiList ThisApi;
          END;
       END;
       rcx = STREAM( ThisFile, 'C', 'CLOSE');
    END;

 END;



 RETURN( ApiList)

