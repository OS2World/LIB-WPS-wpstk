/*
 *      SRCSCAN.CMD - WPS Toolkit - Christian Langanke 2000-2008
 *
 *      Syntax: srcscan header_list source_dir outputpath
 *                      limit_path exclude_list
 *                      [/V[erbose]] [/L[azy]] [/?|/??|/???]
 *
 *      /?    - shows syntax help
 *      /??   - shows source comment syntax help
 *      /???  - shows help about (external) datatypes
 */
/*
 *      header_list      - top level include file(s) (semicolon separated)
 *      base_header_list - headers of the toolkit to be included
 *      source_dir       - source base directory. .c files containing the
 *                         document comments must have the same name like the .h
 *                         files containing the prototypes
 *      outputpath       - outputpath for functions.ipf & datatypes.ipf
 *      limit_path       - path list to limit includes to
 *      exlude_list      - list of filenames (without path!) of files to ignore
 *      /V[erbose]       - verbose output, showing included files
 *      /L[azy]          - do not fail on missing comments
 */
/*
 *      The .c files must
 *      - have the same name as the header file
 *      - contain sections beginning with the following eyecatchers for each
 *        function (and paramater of function):
 *           @@<funcname>@SYNTAX
 *           @@<funcname>@PARM@<parm_name>@<parm_type>
 *           @@<funcname>@RETURN
 *           @@<funcname>@REMARKS
 *        where parm_type is one of "in out inout".
 *
 *        Each section is started with the keyword at column one in a line and
 *        ended by the next section or a simple @@ at column 1 in a following line.
 */
/*
 *      For all datatypes
 *      - <datatype>.ipf is being searched to include it as the definition page
 *      - <datatype>.im is being searched to include it as the head of the
 *        definition page
 */
/* First three comments are being used as online helptext */
/* $Id: srcscan.cmd,v 1.14 2008-10-15 16:43:16 cla Exp $ */
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
/*
 * current limitations:
 *  - typedef struct definitions must end with semicolon at last char on a line
 *    (no comment after that)
 *
 *  - all other typedefs may not span across multiple lines
 *
 *  - function prototypes must end with a semicolon as last char on a line
 *    (except for comments)
 *
 *  - the first IPF line (which may span multiple lines within the comment)
 *    of parameter or return value description must not contain IPF tags.
 *  - first IPF lines of parameter or return value description must
 *    end either with
 *    - end-of-comment
 *    - an IPF tag at pos 1 of a line
 *    - an empty line
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
    SELECT
       WHEN (POS('/???', Parm) > 0) THEN Section = 2;
       WHEN (POS('/??', Parm) > 0)  THEN Section = 1;
       OTHERWISE                         Section = 0;
    END;
    rc = ShowHelp( Section);
    EXIT( rc);
 END;

 /* load RexxUtil */
 CALL RxFuncAdd    'SysLoadFuncs', 'RexxUtil', 'SysLoadFuncs'
 CALL SysLoadFuncs

 /* default values */
 GlobalVars = GlobalVars 'Function. DataType. DocComment. MissingComment. Related. DefineList fVerbose fDebug fCommentError';
 rc         = ERROR.NO_ERROR;

 fCommentError  = TRUE;
 fVerbose       = FALSE;
 fDebug         = FALSE;
 HeaderList     = '';
 SourceDir      = '';
 OutDir         = '';
 LimitPath      = '';
 ExcludeList    = '';

 /* init global stems */
 Function.0         = 0;
 Function._List     = '';
 DataType.0         = 0;
 DataType._List     = '';

 DocComment.        = '';
 DocComment._ValidKeys = 'SYNTAX PARM RETURN REMARKS';

 MissingComment.    = ''
 MissingComment.0   = 0;

 Related.           = '';
 Related._List      = '';

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

          WHEN (POS(ThisTag, '/DEBUG') = 1) THEN
             fDebug = TRUE;

          WHEN (POS(ThisTag, '/LAZY') = 1) THEN
             fCommentError = FALSE;

          OTHERWISE
          DO
             /* handle unix style path names */
             ThisParm = TRANSLATE( ThisParm, '\', '/');

             SELECT
                WHEN (HeaderList     = '') THEN HeaderList     = ThisParm;
                WHEN (SourceDir      = '') THEN SourceDir      = ThisParm;
                WHEN (OutDir         = '') THEN OutDir         = ThisParm;
                WHEN (LimitPath      = '') THEN LimitPath      = ThisParm;
                WHEN (ExcludeList    = '') THEN ExcludeList    = ThisParm;
                OTHERWISE

                DO
                   SAY CmdName': error: invalid parameters.'
                   rc = ERROR.INVALID_PARAMETER;
                   LEAVE;
                END;
             END;
          END;
       END;
    END;

    /* enough parms specified ? */
    IF (OutDir = '') THEN
    DO
       rc = ShowHelp();
      LEAVE;
    END;

    /* source file there ? */
    FileList = HeaderList;
    DO WHILE (FileList \= '')
       PARSE VAR FileList HeaderFile';'FileList;
       IF (\FileExist( HeaderFile)) THEN
       DO
          SAY CmdName': error: file' HeaderFile 'not found.';
          rc = ERROR.FILE_NOT_FOUND;
          LEAVE;
       END;
    END;

    /* get full name of all directories of limitpath */
    NewLimitPath = '';
    DO WHILE (LimitPath \= '')
       PARSE VAR LimitPath ThisPath';'LimitPath;
       IF (ThisPath \= '') THEN
       DO
          /* does directory exist ? */
          NewPath = GetDirName( ThisPath);
          IF (NewPath \= '') THEN
             NewLimitPath = NewLimitPath';'NewPath;
          ELSE
          DO
             SAY CmdName': error: directory' ThisPath 'not found.';
             rc = ERROR.PATH_NOT_FOUND;
             LEAVE;
          END;
       END;
    END;
    IF (rc \= ERROR.NO_ERROR) THEN
       LEAVE;
    LimitPath = NewLimitPath';'

    /* prepare excludelist */
    ExcludeList = TRANSLATE( TRANSLATE( ExcludeList, ' ', ';'));

    /* read header files */
    FileList = HeaderList;
    DO WHILE (FileList \= '')
       PARSE VAR FileList HeaderFile';'FileList;
       IF (fVerbose) THEN
          SAY 'Reading' HeaderFile;
       rc = ReadHeaderFile( HeaderFile, LimitPath, SourceDir, ExcludeList);
       if (rc \= ERROR.NO_ERROR) THEN
          LEAVE;
    END;
    IF (rc \= ERROR.NO_ERROR) THEN
       LEAVE;

    /* remove all pointer types, that we also have a "pointed-to" type of */
    AvailableTypes = DataType._List;
    DO i = 1 TO WORDS( DataType._List)
       ThisType = WORD( DataType._List, i);
       IF ((LEFT( ThisType, 1) = 'P') & (WORDPOS( SUBSTR( ThisType, 2), DataType._List) > 0)) THEN
       DO
          DataType._List = DELWORD( DataType._List, i, 1);
          i = i - 1;
       END;
    END;

    /* save list of own datatypes */
    DataType._OwnList = DataType._List

    /* add all datatypes of the toolkit, that we have */
    rc = SysFileTree( 'datatypes\*.ipf', 'Includes.', 'FO');
    IF ((rc = ERROR.NO_ERROR) & (Includes.0 > 0)) THEN
    DO i = 1 TO Includes.0
       ThisInclude = FILESPEC( 'N', Includes.i);
       PARSE VAR ThisInclude TypeName'.'.;
       DataType._List = DataType._List TypeName;
    END;

    /* write output file */
    rc = WriteIpfFiles( OutDir);
    if (rc \= ERROR.NO_ERROR) THEN
       LEAVE;

    /* write EPM index file */
    rc = WriteEPMFiles( OutDir, 'wtkref', 'WPS Toolkit for OS/2');
    if (rc \= ERROR.NO_ERROR) THEN
       LEAVE;


 END;

 EXIT( rc);

/* ------------------------------------------------------------------------- */
HALT:
  SAY 'Interrupted by user.';
  EXIT(ERROR.GEN_FAILURE);

/* ------------------------------------------------------------------------- */
ShowHelp: PROCEDURE EXPOSE (GlobalVars)
 ARG Section;

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

 /* now skip n blocks */
 IF (Section = '') THEN Section = 0;
 DO i = 1 TO Section
    ThisLine = LINEIN(Thisfile);
    DO WHILE (ThisLine \= ' */')
       ThisLine = LINEIN(Thisfile);
    END;
 END;

 /* show desired help block */
 ThisLine = LINEIN(Thisfile);
 DO WHILE (ThisLine \= ' */')
    SAY SUBSTR(ThisLine, 7);
    ThisLine = LINEIN(Thisfile);
 END;

 /* Datei wieder schlieáen */
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

/* ========================================================================= */
/* finds the qeuivalent .c file for a .h file */
FindSourceFileInTree: PROCEDURE EXPOSE (GlobalVars)
 PARSE ARG File;

 SourceFile = '';

 DO UNTIL (TRUE)

    /* determine extension */
    FileExtPos = LASTPOS( '.', File);
    IF (FileExtPos = 0) THEN
       LEAVE;

    /* replace extension */
    FileExt  = TRANSLATE( SUBSTR( File, FileExtPos));
    SELECT
       WHEN( FileExt = '.H') THEN FileExt = '.c';
       OTHERWISE                  LEAVE;
    END;
    File = LEFT( File, FileExtPos - 1)''FileExt;

    /* Search the file */
    rc = SysFileTree( File, 'File.', 'OFS');
    SELECT
       WHEN (rc \= ERROR.NO_ERROR) THEN LEAVE;
       WHEN (File.0 \= 1)          THEN LEAVE;
       OTHERWISE                        SourceFile = File.1;
    END;
 END;

 RETURN( SourceFile);

/* ========================================================================= */
IsInList: PROCEDURE
 PARSE ARG String, List;
 RETURN( WORDPOS( string, List) > 0);

/* ========================================================================= */
/* simple string sort */
SortString: PROCEDURE
 PARSE ARG SortString;

 NewString = '';
 DO WHILE (SortString \= '')
    PARSE VAR SortString ThisString SortString;

    NewWordPos = LENGTH( NewString);
    DO i = 1 TO WORDS( NewString)
       IF (ThisString < WORD( NewString, i)) THEN
       DO
          NewWordPos = WORDINDEX( NewString, i) - 1;
          LEAVE;
       END;
    END;

    NewString = INSERT( ThisString' ', NewString, NewWordPos);

 END;

 RETURN( NewString);

/* ========================================================================= */
/* read a line and increase line counter */
_READLINE: PROCEDURE EXPOSE LineCount;
 PARSE ARG File;
 LineCount = LineCount + 1;
 RETURN( LINEIN( File));

/* ========================================================================= */
/* read top level header file with recursive inclusions while  */
/* not including files not resigin in LimitPath or with a name */
/* of the exclude list                                         */

ReadHeaderFile: PROCEDURE EXPOSE (GlobalVars)
 PARSE ARG File, LimitPath, SourceDir, ExludeList;

 /* default values */
 rc             = ERROR.NO_ERROR;
 LineCount      = 0;
 fCommentActive = FALSE;
 IncludedTag    = '';
 CrLf           = "0d0a"x;

 /* determine source file */
 IF (SourceDir \= '') THEN
 DO
    SourceFile = FindSourceFileInTree( SourceDir'\'FILESPEC( 'N', File));
    IF (SourceFile \= '') THEN
       rcx = ReadSourceFile( SourceFile);
 END;

 /* read header file */
 DO WHILE (LINES( File) > 0)

    /* read line */
    ThisLine = _READLINE( File);

    /* is there an open comment ? */
    IF (fCommentActive) THEN
    DO
       CommentEnd = POS( '*/', ThisLine);
       IF (CommentEnd > 0) THEN
       DO
          /* close comment */
          ThisLine = SUBSTR( ThisLine, CommentEnd + 2);
          fCommentActive = FALSE;
       END;
       ELSE
          ITERATE;
    END;

    /* --------------------------------------------------------------------- */

    /* is it a double slash comment line ? */
    CommentPos = POS( '//', ThisLine);
    IF (CommentPos > 0) THEN
       ThisLine = LEFT( ThisLine, CommentPos - 1);

    /* --------------------------------------------------------------------- */

    /* is it a slash/star comment ? */
    CommentStart = POS( '/*', ThisLine);
    DO WHILE (CommentStart > 0)
       /* does the comment not end on this line ? */
       CommentEnd = POS( '*/', ThisLine, CommentStart + 2);
       IF (CommentEnd = 0) THEN
       DO
          fCommentActive = TRUE;
          ThisLine = LEFT( ThisLine, CommentStart - 1);
       END;
       ELSE
          ThisLine = DELSTR( ThisLine, CommentStart, CommentEnd - CommentStart + 2);

       /* search next comment */
       CommentStart = POS( '/*', ThisLine);

    END;

    /* skip empty lines */
    ThisLine = STRIP( ThisLine);
    IF (ThisLine = '') THEN
       ITERATE;

    /* --------------------------------------------------------------------- */

    /* check for precompiler commands */
    PARSE VAR ThisLine Tag +1 Command CommandOption;
    IF ( Tag = '#') THEN
    DO

       /* - - - - - - - - - - - - - - - - - - - - - - - - - - */

       IF (Command = 'include') THEN
       DO
          /* check for include file */
          CommandOption = STRIP( CommandOption);
          IncludeChar = LEFT( CommandOption, 1);
          SELECT
             WHEN (IncludeChar = '<') THEN
             DO
                PARSE VAR CommandOption '<'IncludeFile'>';
                IncludeFileName = SysSearchPath( 'INCLUDE', IncludeFile);
             END;
             WHEN (IncludeChar = '"') THEN
             DO
                PARSE VAR CommandOption '"'IncludeFile'"';
                IncludeFileName = STREAM( IncludeFile, 'C', 'QUERY EXISTS');
             END;
             OTHERWISE NOP;
          END;

          /* check filename and include path */
          IF (IncludeFileName = '') THEN ITERATE;
          IF (POS( ';'GetDrivePath( IncludeFileName)';', ';'LimitPath';') = 0) THEN
             ITERATE;

          /* check filename and exclude list */
          IF (ExludeList \= '') THEN
             IF (WORDPOS( TRANSLATE( FILESPEC( 'N', IncludeFileName)), ExludeList) > 0) THEN
             ITERATE;

          /* include the file */
          IF (fVerbose) THEN
             SAY '- Including' IncludeFileName 'within' FILESPEC( 'N', File);
          rc = ReadHeaderFile( IncludeFileName, LimitPath, SourceDir, ExcludeList);

       END;

       /* - - - - - - - - - - - - - - - - - - - - - - - - - - */

       ELSE IF (Command = 'define') THEN
       DO
          SELECT
             /* use _INCLUDED to determine include tags for functions */
             WHEN (POS( '_INCLUDED', CommandOption) > 0) THEN
             DO
                PARSE VAR CommandOption IncludeTag'_INCLUDED' IncludedDescription;
                IncludedTag = 'INCL_'IncludeTag;
             END;

             OTHERWISE
             DO
                /* save own defines */
                IF (SourceDir \= '') THEN
                DO
                   PARSE VAR CommandOption DefineName .;
                   DefineList = DefineList DefineName;
                END;
             END
          END;

          DO WHILE (RIGHT(ThisLine, 1) = '\')
             /* read line */
             ThisLine = _READLINE( File);
          END;

       END;

       /* - - - - - - - - - - - - - - - - - - - - - - - - - - */

       ELSE IF (Command = 'ifdef') THEN
       DO
          SELECT
             WHEN (CommandOption = '__cplusplus') THEN
             DO
                IF (fDebug) THEN SAY 'Parsing out __cplusplus';

                /* parse out C++ sections */
                DO WHILE (ThisLine \= '#endif')
                   /* read line */
                   ThisLine = _READLINE( File);
                END;
             END;

             OTHERWISE NOP;
          END;

       END;

       /* - - - - - - - - - - - - - - - - - - - - - - - - - - */

       /* don't go further with precompiler commands */
       ITERATE;
    END;

    /* --------------------------------------------------------------------- */

    /* check for typedefs */
    PARSE VAR ThisLine Keyword .;
    IF (Keyword = 'typedef') THEN
    DO

       /* init vars */
       ThisDataType   = '';
       ThisDefinition = '';
       fStoreTypedef = FALSE;

       SELECT

          /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


          WHEN (WORD( ThisLine, 2) = 'struct') THEN
          DO
             fStoreTypedef = TRUE;

             /* hande multiline typedefs */
             ThisDefinition = ThisLine;
             PARSE VAR ThisLine . . ThisDataType .;
             IF (LEFT( ThisDataType, 1) = '_') THEN
                PARSE VAR ThisDataType . +1 ThisDataType;

             /* search end of struct */
             DO WHILE (POS( '}', ThisLine) = 0)
                /* read line */
                ThisLine = _READLINE( File);
                ThisDefinition = ThisDefinition''CrLf''ThisLine;
             END;

             /* search end of definition */
             EndOfStruct = POS( '}', ThisLine);
             EndOfDef = POS( ';', ThisLine, EndOfStruct);
             DO WHILE ( EndOfDef = 0)
                /* read line */
                ThisLine = _READLINE( File);
                ThisDefinition = ThisDefinition''CrLf''ThisLine;
                EndOfDef = POS( ';', ThisLine);
             END;
          END;

          /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

          OTHERWISE
          DO
             /* read up to semicolon */
             DO WHILE (POS( ';', ThisLine) = 0)
                ThisLine = ThisLine  _READLINE( File);
             END;

             /* handle simple one line typedefs */
             ThisDefinition = ThisLine;
             fStoreTypedef = TRUE;

             SELECT
                /* handle "pointer to function" typedefs (like PFN) */
                WHEN (POS( ',', ThisLine) > 0) THEN
                DO
                   /* isolate the FN type */
                   CheckLine = TRANSLATE( ThisLine, '  ', '()');
                   PARSE VAR CheckLine . . ThisDataType .;
                   PARSE VAR ThisDataType ThisDataType',';

                   /* add pointer definition */
                   ThisDefinition = ThisDefinition''CrLf''CrLf'typedef' ThisDataType '*P'ThisDataType';';
                END;

                OTHERWISE
                DO
                   ThisDataType = WORD( ThisLine, WORDS(ThisLine));
                   PARSE VAR ThisDataType ThisDataType';'
                   IF (LEFT( ThisDataType, 1) = '*') THEN
                      ThisDataType = SUBSTR( ThisDataType, 2);
                END;
             END;

             /* ignore all "pointer to anything" definitions */
             IF (FALSE) THEN /* IF (POS(' *P', ThisLine) > 0) THEN*/
             DO
                IF (fDebug) THEN SAY 'Skipping typedef:' ThisLine
                ITERATE;
             END;
          END;

       END; /* SELECT */

       /* save datatype or struct */
       IF (fStoreTypedef) THEN
       DO

          d = WORDPOS( ThisDataType, DataType._List);
          IF (d > 0) THEN
          DO
             /* don't include twice, only add definition */
             DataType.d._Def       = DataType.d._Def''CrLf''ThisDefinition;
          END;
          ELSE
          DO
             d                     = DataType.0 + 1;
             DataType.d            = ThisDataType;
             DataType.d._Def       = ThisDefinition;
             DataType.0            = d;
             DataType._List        = DataType._List ThisDataType;
             DataType.ThisDataType = d;
          END;

          ITERATE;
       END;
    END;

    /* --------------------------------------------------------------------- */

    /* all others should be functions, read up to semicolon */
    DO WHILE (POS( ';', ThisLine) = 0)
       ThisLine = ThisLine  _READLINE( File);
    END;

    /* store function */
    PARSE VAR ThisLine ThisReturnType ThisName'('ThisParms');'.
    ThisName               = WORD( ThisName, WORDS( ThisName));
    fFunctionEnded         = (POS(');', ThisLine) > 0);
    ThisName               = STRIP( ThisName);
    f                      = Function.0 + 1;
    Function.0             = f;
    Function.f             = ThisName;
    Function.f._ReturnType = ThisReturnType
    Function.f._Parms      = ThisParms;
    Function.f._Included   = IncludedTag;
    Function._List         = Function._List ThisName
    Function.ThisName      = f;

    /* add this function to related list */
    Related.IncludedTag    = Related.IncludedTag ThisName;

    /* store list of incuded tags */
    IF (WORDPOS( IncludedTag, Related._List) = 0) THEN
    DO
       Related._List             = Related._List IncludedTag;
       Related._Desc.IncludedTag = IncludedDescription;
    END;

    ITERATE;

 END;

 /* close file */
 rcx = STREAM( File, 'C', 'CLOSE');
 RETURN( rc);


/* ========================================================================= */
/* if a comment is missing, store this information  */
StoreMissingComment: PROCEDURE EXPOSE (GlobalVars)
 PARSE ARG GivenComment, Function, CommentInfo;

 GivenComment = STRIP( GivenComment);
 CommentInfo  = STRIP( CommentInfo);

 IF (GivenComment = '') THEN
 DO
    c                = MissingComment.0 + 1;
    MissingComment.c = Function '-' CommentInfo;
    MissingComment.0 = c;
 END;

 RETURN( GivenComment);

/* ========================================================================= */
/* return the first line of a comment, where the line stops with */
/* - an epmty line                                               */
/* - a ':' at the forst pos of a line                            */
/* NOTE: the first IPF line of a comment must not include        */
/*       any IPF tags                                            */

FirstIPFLine: PROCEDURE
 PARSE ARG Comment;

 FirstLine    = Comment;
 CrLf         = "0d0a"x;
 ReplaceBytes = '  ';

 IF ((Comment \= '') & (LEFT( Comment, 2) \= CrLf)) THEN
 DO
    BreakPos = POS( CrLf':', Comment);
    IF (BreakPos = 0) THEN
       BreakPos = POS( CrLf''CrLf, Comment);

    IF (BreakPos > 0) THEN
    DO
       FirstLine = LEFT( Comment, BreakPos - 1);
       DO WHILE ( C2D(RIGHT( FirstLine, 1)) < 32)
          FirstLine = LEFT( FirstLine, LENGTH( FirstLine) - 1);
       END;
    END;
    FirstLine = SPACE( TRANSLATE( FirstLine, ReplaceBytes, CrLf));
 END;

 RETURN FirstLine;

/* ========================================================================= */
/* determines the lengh of a line exlcuding IPF tags */
IPFLength: PROCEDURE
 PARSE ARG Line;

 /* determine the length of an IPF string (without IPF tags) */
 Len      = LENGTH( Line);
 IpfLen   = 0;
 StartPos = POS( ':', Line);
 DO WHILE (StartPos > 0)
    EndPos = POS( '.', Line, StartPos);
    IF (EndPos = 0) THEN
       LEAVE;
    IpfLen = IpfLen + EndPos - StartPos + 1;
    StartPos = POS( ':', Line, EndPos);
 END;

 RETURN Len - IpfLen;

/* ========================================================================= */
/* replace datatypes in structs with the appropriate IPF link */
IPFDatatype: PROCEDURE EXPOSE (GlobalVars)
 PARSE ARG Line;

 CrLf = "0d0a"x;
 fStructOpen = FALSE;

 IF (POS( 'struct', Line) = 0) THEN
    RETURN Line;

 NewLine = '';
 DO WHILE (Line \= '')

    /* split at crlf */
    EOL = POS( CrLf, Line);
    IF (EOL = 0) THEN
    DO
       ThisLine = Line;
       Line = '';
    END
    ELSE
    DO
       ThisLine = LEFT( Line, EOL - 1);
       Line     = SUBSTR( Line, EOL + 2);
    END;

    /* check the line for datatypes */
    IF (\fStructOpen) THEN
    DO
       IF (POS( '{', ThisLine) > 0) THEN
          fStructOpen = TRUE;
    END;
    ELSE
    DO
       IF (POS( '}', ThisLine) > 0) THEN
          fStructOpen = FALSE;
       ELSE
       DO
          /* is it a complete line ? */
          IF (POS( ';', ThisLine) > 0) THEN
          DO
             /* replace type with link to type */
             TypePos = WORDINDEX( ThisLine, 1);
             VarPos  = WORDINDEX( ThisLine, 2);
             Type    = WORD( ThisLine, 1);
             TypeRef = Type;

             /* check for type without pointer prefix */
             IF ((LEFT( Type, 1) = 'P') & (WORDPOS( SUBSTR( Type, 2), DataType._List) > 0)) THEN
                TypeRef = SUBSTR( Type,  2);

             Gap     = VarPos - TypePos - LENGTH( Type);
             ThisLine = DELWORD( ThisLine, 1, 1);
             NewType =  ':link reftype=hd viewport dependent refid='TypeRef'.'Type':elink.'COPIES( ' ', Gap);
             ThisLine = INSERT( NewType, ThisLine, TypePos - 1);
          END;
       END;
    END;

    /* append */
    NewLine = NewLine''CrLf''ThisLine;
 END;
 NewLine = SUBSTR( NewLine, 3);

 RETURN NewLine;


/* ========================================================================= */
/* write target files */
WriteIpfFiles: PROCEDURE EXPOSE (GlobalVars)
 PARSE ARG OutDir;

 /* default values */
 rc = ERROR.NO_ERROR;
 CrLf = "0d0a"x;

 PageLeft            = 'x=LEFT y=TOP width=30% height=100%';
 PageRight           = 'x=RIGHT y=TOP width=70% height=100%';
 PageRightRight      = 'x=RIGHT y=TOP width=30% height=100%';
 PageRightbottom     = 'x=RIGHT y=BOTTOM width=70% height=50%';


 DatatypesFile = OutDir'\datatypes.ipf';
 FunctionsFile = OutDir'\functions.ipf';

 rcx = SysFileDelete( DatatypesFile);
 rcx = SysFileDelete( FunctionsFile);

 /* ********************** write function overview ********************** */

 WorkList = Related._List;
 rcx = LINEOUT( FunctionsFile, ':p.');
 DO WHILE (WorkList \= '')
    PARSE VAR WorkList ThisRelatedName WorkList;
    ThisRelatedDesription = Related._Desc.ThisRelatedName;
    rcx = LINEOUT( FunctionsFile, ':hp2.'ThisRelatedDesription':ehp2.');
    rcx = LINEOUT( FunctionsFile, ':ul compact.');
    RelatedList = SortString( Related.ThisRelatedName);
    DO WHILE (RelatedList \= '')
       PARSE VAR RelatedList ThisRelated RelatedList;
       rcx = LINEOUT( FunctionsFile, ':li.:link reftype=hd viewport refid='ThisRelated'.'ThisRelated':elink.');
    END;
    rcx = LINEOUT( FunctionsFile, ':eul.');
    rcx = LINEOUT( FunctionsFile, '.br');

 END;

 /* ********************** write function pages ************************* */

 WorkList = SortString( Function._List);
 DO WHILE (WorkList \= '')

    /* get index */
    PARSE VAR WorkList ThisFunction WorkList;
    f = Function.ThisFunction;
    PageIdBase = ThisFunction;

    /* =========== write main section for function =========== */
    rcx = LINEOUT( FunctionsFile, ':h2 id='PageIdBase PageLeft'.'ThisFunction);
    rcx = LINEOUT( FunctionsFile, ':link auto viewport dependent reftype=hd refid=S_'PageIdBase'.');
    rcx = LINEOUT( FunctionsFile, ':p.');
    rcx = LINEOUT( FunctionsFile, 'Select an item&colon.');
    rcx = LINEOUT( FunctionsFile, ':ul compact.');
    rcx = LINEOUT( FunctionsFile, ':li.:link reftype=hd viewport dependent refid=S_'PageIdBase'.Syntax:elink.');
    rcx = LINEOUT( FunctionsFile, ':li.:link reftype=hd viewport dependent refid=P_'PageIdBase'.Parameters:elink.');
    rcx = LINEOUT( FunctionsFile, ':li.:link reftype=hd viewport dependent refid=RT_'PageIdBase'.Returns:elink.');
    rcx = LINEOUT( FunctionsFile, ':li.:link reftype=hd viewport dependent refid=RM_'PageIdBase'.Remarks:elink.');
    rcx = LINEOUT( FunctionsFile, ':li.:link reftype=hd viewport dependent refid=RF_'PageIdBase'.Related Functions:elink.');
    rcx = LINEOUT( FunctionsFile, ':eul.');
    rcx = LINEOUT( FunctionsFile, '.br');

    /* =========== write syntax section for function =========== */

    ThisComment = StoreMissingComment( DocComment.ThisFunction.SYNTAX,,
                                       ThisFunction, 'Syntax');

    rcx = LINEOUT( FunctionsFile, ':h2 hide id=S_'PageIdBase PageRight'.'ThisFunction '- Syntax');
    rcx = LINEOUT( FunctionsFile, ':link auto viewport dependent reftype=hd refid='PageIdBase'.');
    rcx = LINEOUT( FunctionsFile, ':p.');
    rcx = LINEOUT( FunctionsFile, ThisComment);
    rcx = LINEOUT( FunctionsFile, ':xmp.');
    rcx = LINEOUT( FunctionsFile, '#define' Function.f._Included);
    rcx = LINEOUT( FunctionsFile, '#include <wtk.h>');
    rcx = LINEOUT( FunctionsFile, '');

    /* determine Return Variable */
    ReturnType = Function.f._ReturnType;
    SELECT
       WHEN (ReturnType = 'APIRET') THEN ReturnVar = 'ulrc';
       WHEN (ReturnType = 'BOOL')   THEN ReturnVar = 'fResult';
       WHEN (ReturnType = 'ULONG')  THEN ReturnVar = 'ulResult';
       WHEN (ReturnType = 'LONG')   THEN ReturnVar = 'lResult';
       WHEN (ReturnType = 'CHAR')   THEN ReturnVar = 'chResult';
       WHEN (ReturnType = 'PSZ')    THEN ReturnVar = 'pszResult';
       OTHERWISE                         ReturnVar = LOWER( ReturnType);
    END;

    /* syntax section: parameters */
    CallList      = '';
    MaxTypeLen    = 0;
    MaxNameLen    = 0;
    MaxCommentLen = 0;

    /* determine maxlen of typenames, var names and comment names */
    ParmList = Function.f._Parms;
    IF (TRANSLATE( ParmList) = 'VOID') THEN
       ParmList = '';
    DO WHILE (ParmList \= '')
       PARSE VAR ParmList ThisType ThisName',' ParmList;
       MaxTypeLen    = MAX( MaxTypeLen, LENGTH( ThisType));
       MaxNameLen    = MAX( MaxNameLen, LENGTH( ThisName));
       MaxCommentLen = MAX( MaxCommentLen, LENGTH( FirstIPFLine( DocComment.ThisFunction.PARM.ThisName)));
    END;
    MaxTypeLen    = MAX( MaxTypeLen, LENGTH( ReturnType)) + 3;
    MaxNameLen    = MAX( MaxNameLen, LENGTH( ReturnVar))  + 3;
    MaxCommentLen = MAX( MaxCommentLen, LENGTH( FirstIPFLine( DocComment.ThisFunction.RETURN)));

    /* scan through all parms and write them */
    ParmList = Function.f._Parms;
    IF (TRANSLATE( ParmList) = 'VOID') THEN
       ParmList = '';
    DO WHILE (ParmList \= '')
       PARSE VAR ParmList ThisType ThisParm',' ParmList;
       CallList = CallList',' ThisParm;

       /* is this type included in our datatype list ? */
       RefType = CheckDataType( ThisType, DataType._List);
       RefParm = ThisParm;

       /* take care for asterisks and make IPF Version of parm ntype and name */
       IpfNameType = GetIpfNameType( ThisType, ThisParm)
       PARSE VAR IpfNameType IpfType IpfParm;

       /* write it onto the syntax page */
       FirstComment    = FirstIPFLine( DocComment.ThisFunction.PARM.ThisParm);
       FirstCommentLen = IPFLength( FirstComment);
       rcx = CHAROUT( FunctionsFile, ':link reftype=hd viewport dependent refid='RefType'.'IpfType':elink.');
       rcx = CHAROUT( FunctionsFile, COPIES( ' ', MaxTypeLen - LENGTH( ThisType)));
       rcx = CHAROUT( FunctionsFile, ':link reftype=hd viewport dependent refid='ThisParm'_'PageIdBase'.'IpfParm':elink.;');
       rcx = CHAROUT( FunctionsFile, COPIES( ' ', MaxNameLen - LENGTH( ThisParm)));
       rcx = CHAROUT( FunctionsFile, '/*' FirstComment COPIES( ' ', MaxCommentLen - FirstCommentLen) '*/');
       rcx = LINEOUT( FunctionsFile, '');


       /* generate ipf source for this parameter for later use */
       ThisComment = StoreMissingComment( DocComment.ThisFunction.PARM.ThisParm,,
                                          ThisFunction, 'parameter' ThisParm);
       IF (ThisComment \= '') THEN
          ThisInOutTpye = StoreMissingComment( DocComment.ThisFunction.PARM.ThisParm,,
                                               ThisFunction, 'parameter' ThisParm '- input/output type definition');
       ELSE
          ThisInOutTpye = '';

       ThisInOutTpye = DocComment.ThisFunction.PARM.ThisParm._InOutType;
       Function.f._Parm.ThisParm = ':hp2.'ThisParm':ehp2.',
                                   '(:link reftype=hd viewport dependent refid='RefType'.'ThisType':elink.) -',
                                   ThisInOutTpye''CrLf'.br'CrLf||,
                                   ':lm margin=4.'ThisComment':lm margin=1.'
    END;

    /* take care for asterisks and make IPF Version of parm ntype and name */
    IpfNameType = GetIpfNameType( ReturnType, ReturnVar)
    PARSE VAR IpfNameType IpfType IpfVar;

    /* write variable for return value */
    RefType = CheckDataType( ReturnType, DataType._List);
    FirstComment = FirstIPFLine( DocComment.ThisFunction.RETURN);
    FirstCommentLen = IPFLength( FirstComment);
    rcx = CHAROUT( FunctionsFile, ':link reftype=hd viewport dependent refid='RefType'.'IpfType':elink.');
    rcx = CHAROUT( FunctionsFile, COPIES( ' ', MaxTypeLen - LENGTH( ReturnType)));
    rcx = CHAROUT( FunctionsFile, ':link reftype=hd viewport dependent refid=RT_'PageIdBase'.'IpfVar':elink.;');
    rcx = CHAROUT( FunctionsFile, COPIES( ' ', MaxNameLen - LENGTH( ReturnVar)));
    rcx = CHAROUT( FunctionsFile, '/*' FirstComment COPIES( ' ', MaxCommentLen - FirstCommentLen) '*/');
    rcx = LINEOUT( FunctionsFile, '');

    /* generate ipf source for this parameter for later use */
    ThisComment = StoreMissingComment( DocComment.ThisFunction.RETURN,,
                                       ThisFunction, 'return value ' ReturnVar);

    Function.f._Return  = ':hp2.'ReturnVar':ehp2.',
                          '(:link viewport dependent reftype=hd refid='RefType'.'ReturnType':elink.) -',
                          'returns'CrLf'.br'CrLf||,
                          ':lm margin=4.'ThisComment':lm margin=1.';

    /* write sample call of function */
    CallList = SUBSTR( CallList, 3);
    IF (CallList \= '') THEN
       CallList = ' 'CallList;
    rcx = LINEOUT( FunctionsFile, '');
    rcx = LINEOUT( FunctionsFile, ReturnVar '=' ThisFunction'('CallList');');

    /* - close syntax section */
    rcx = LINEOUT( FunctionsFile, '');
    rcx = LINEOUT( FunctionsFile, ':exmp.');
    rcx = LINEOUT( FunctionsFile, '.br');

    /* =========== write subsection for each parameter as a subpage =========== */

    /* scan through all parms and write them */
    ParmList = Function.f._Parms;
    IF (TRANSLATE( ParmList) = 'VOID') THEN
       ParmList = '';
    DO WHILE (ParmList \= '')
       PARSE VAR ParmList ThisType ThisParm',' ParmList;
       rcx = LINEOUT( FunctionsFile, ':h2 hide id='ThisParm'_'PageIdBase PageRightBottom' viewport.'ThisFunction '- Parameter' ThisParm);
       rcx = LINEOUT( FunctionsFile, ':p.');
       rcx = LINEOUT( FunctionsFile, Function.f._Parm.ThisParm);
       rcx = LINEOUT( FunctionsFile, '.br');
    END;

    /* =========== write parameters section for function =========== */

    rcx = LINEOUT( FunctionsFile, ':h2 hide id=P_'PageIdBase PageRight'.'ThisFunction '- Parameters');
    ParmList = Function.f._Parms;
    IF (TRANSLATE( ParmList) = 'VOID') THEN
       ParmList = '';
    DO WHILE (ParmList \= '')
       PARSE VAR ParmList ThisType ThisParm',' ParmList;
       rcx = LINEOUT( FunctionsFile, ':p.');
       rcx = LINEOUT( FunctionsFile, Function.f._Parm.ThisParm);
    END;
    rcx = LINEOUT( FunctionsFile, ':p.');
    rcx = LINEOUT( FunctionsFile, Function.f._Return);
    rcx = LINEOUT( FunctionsFile, '.br');

    /* =========== write returns section for function =========== */

    rcx = LINEOUT( FunctionsFile, ':h2 hide id=RT_'PageIdBase PageRightBottom'.'ThisFunction 'Return Value -' ReturnVar);
    rcx = LINEOUT( FunctionsFile, ':p.');
    rcx = LINEOUT( FunctionsFile, Function.f._Return);
    rcx = LINEOUT( FunctionsFile, '.br');

    /* =========== write remarks section for function =========== */

    ThisComment = StoreMissingComment( DocComment.ThisFunction.REMARKS,,
                                       ThisFunction, 'Remarks');

    rcx = LINEOUT( FunctionsFile, ':h2 hide id=RM_'PageIdBase PageRight'.'ThisFunction '- Remarks');
    rcx = LINEOUT( FunctionsFile, ':p.');
    rcx = LINEOUT( FunctionsFile, DocComment.ThisFunction.REMARKS);
    rcx = LINEOUT( FunctionsFile, '.br');

    /* =========== write related functions section for function =========== */

    rcx = LINEOUT( FunctionsFile, ':h2 hide id=RF_'PageIdBase PageRightRight'.'ThisFunction '- Related functions');
    rcx = LINEOUT( FunctionsFile, ':p.');
    rcx = LINEOUT( FunctionsFile, ':hp2.Related Functions:ehp2.');
    IncludedTag = Function.f._Included;
    RelatedList = SortString( Related.IncludedTag);
    wPos =  WORDPOS( ThisFunction, RelatedList);
    IF (wPos > 0) THEN
       RelatedList = DELWORD( RelatedList, wPos, 1);
    IF (STRIP( RelatedList) = '') THEN
    DO
       rcx = LINEOUT( FunctionsFile, ':p.');
       rcx = LINEOUT( FunctionsFile, 'none');
    END;
    ELSE
    DO
        rcx = LINEOUT( FunctionsFile, ':ul compact.');
        DO WHILE (RelatedList \= '')
           PARSE VAR RelatedList ThisRelated RelatedList;
           rcx = LINEOUT( FunctionsFile, ':li.:link reftype=hd refid='ThisRelated'.'ThisRelated':elink.');
        END;
        rcx = LINEOUT( FunctionsFile, ':eul.');
    END;
    rcx = LINEOUT( FunctionsFile, '.br');

 END;

 /* ********************** write datatype overview ********************** */

 /* own datatypes */
 rcx = LINEOUT( DatatypesFile, ':p.');
 rcx = LINEOUT( DatatypesFile, ':hp2.Data types of the WPS Toolkit:ehp2.');
 rcx = LINEOUT( DatatypesFile, ':ul compact.');
 WorkList = SortString( DataType._OwnList);
 DO WHILE (WorkList \= '')
    PARSE VAR WorkList ThisDataType WorkList;
    rcx = LINEOUT( DatatypesFile, ':li.:link reftype=hd viewport refid='ThisDataType'.'ThisDataType':elink.');
 END;
 rcx = LINEOUT( DatatypesFile, ':eul.');
 rcx = LINEOUT( DatatypesFile, ':p.');

 /* datatypes from elsewhere */
 rcx = LINEOUT( DatatypesFile, ':hp2.Data types from the Toolkit for OS/2 WARP or compiler runtime:ehp2.');
 rcx = LINEOUT( DatatypesFile, ':ul compact.');
 WorkList = SortString( DataType._List);
 DO WHILE (WorkList \= '')
    PARSE VAR WorkList ThisDataType WorkList;
    IF (WORDPOS( ThisDataType, DataType._OwnList) = 0) THEN
       rcx = LINEOUT( DatatypesFile, ':li.:link reftype=hd viewport refid='ThisDataType'.'ThisDataType':elink.');
 END;
 rcx = LINEOUT( DatatypesFile, ':eul.');
 rcx = LINEOUT( DatatypesFile, '.br');

 /* ********************** write datatype pages ************************* */

 WorkList = SortString(DataType._List);
 DO WHILE (WorkList \= '')

    /* get index */
    PARSE VAR WorkList ThisDataType WorkList;
    d = DataType.ThisDataType;

    /* is imbed file for the complete page present ? */
    PageFile = 'datatypes\'ThisDataType'.ipf';
    fUsePageFile = FileExist( PageFile);

    /* is imbed file for page head present ? */
    PageHeaderFile = 'datatypes\'ThisDataType'.im';
    fUsePageHeaderFile = FileExist( PageHeaderFile);

    /* write main section for function */

    rcx = LINEOUT( DatatypesFile, ':h2 id='ThisDataType' viewport.'ThisDataType);
    rcx = LINEOUT( DatatypesFile, ':p.');

    IF (fUsePageFile) THEN
       rcx = LINEOUT( DatatypesFile, '.im' PageFile);
    ELSE
    DO
       IF (fUsePageHeaderFile) THEN
          rcx = LINEOUT( DatatypesFile, '.im' PageHeaderFile);

       rcx = LINEOUT( DatatypesFile, ':xmp.');
       IF (POS( '(', Datatype.d) = 0) THEN
       DO
          rcx = LINEOUT( DatatypesFile, IPFDatatype( DataType.d._Def));
       END
       ELSE
          rcx = LINEOUT( DatatypesFile, DataType.d);
       rcx = LINEOUT( DatatypesFile, ':exmp.');
    END;
    rcx = LINEOUT( DatatypesFile, '.br');
 END;

 /* close file */
 rcx = STREAM( FunctionsFile, 'C', 'CLOSE');
 rcx = STREAM( DatatypesFile, 'C', 'CLOSE');

 /* ********************* check missing comments ************************ */

 /* tell specific missing comments first */
 MissingFuncs = '';
 IF (fCommentError) THEN
    MsgType = 'Error:';
 ELSE
    MsgType = 'Warning:';

 DO i = 1 TO MissingComment.0
    PARSE VAR MissingComment.i ThisFunction ThisInfo;

    IF (DocComment.ThisFunction._Found = '') THEN
    DO
       IF (WORDPOS( ThisFunction, MissingFuncs) = 0) THEN
          MissingFuncs = MissingFuncs ThisFunction;
    END;
    ELSE
       SAY MsgType 'missing comment for:' ThisFunction ThisInfo;
 END;

 /* now tell about functions without any comment */
 DO WHILE (MissingFuncs \= '')
    PARSE VAR  MissingFuncs ThisFunction MissingFuncs ;
    SAY MsgType  'missing any comment for:' ThisFunction;
 END;

 /* tell about error*/
 IF ((fCommentError) & (MissingComment.0 > 0)) THEN
 DO
    rcx = SysFileDelete( DatatypesFile);
    rcx = SysFileDelete( FunctionsFile);
    rc = ERROR.INVALID_DATA;
 END;


 RETURN( rc);

/* ========================================================================= */
/* return the datatype with regard to the "implicit pointer type "Pxxxxx" rule */
CheckDataType: PROCEDURE
 PARSE ARG Type, Typelist;

 Type = STRIP( TRANSLATE( Type, ' ', '*')); /* strip stars */
 NewType   = Type;

 IF ((WORDPOS( Type, TypeList) = 0) & (LEFT( Type, 1) = 'P')) THEN
 DO
    /* is it a Ptype of one of our datatypes ? Then reference to this one */
    IF (WORDPOS( SUBSTR( Type, 2), Typelist) > 0) THEN
       NewType = SUBSTR( Type, 2);
 END;

 RETURN( NewType);

/* ========================================================================= */
/* relayouts variable type and variable name:                                */
/* if type has a '*' at the end, remove it from type and append it as symbol */
/* to the name                                                               */
GetIpfNameType: PROCEDURE
 PARSE ARG Type, Name
 IF (RIGHT( Type, 1) = '*') THEN
 DO
 Type = LEFT( Type, LENGTH( Type) - 1);
 Name = '&asterisk.'Name;
 END;
 RETURN( Type Name);

/* ========================================================================= */
/* read .c file and catch all online doc comments */
/* read <name>*.c !                               */
ReadSourceFile: PROCEDURE EXPOSE (GlobalVars)
 PARSE ARG File;

 /* defaults */
 rc   = ERROR.NO_ERROR;
 CrLf = "0d0a"x;

 /* check for more than one source file */
 ExtPos = LASTPOS( '.', File);
 IF (ExtPos > 0) THEN
    FileMask = INSERT( '*', File, ExtPos - 1);
 ELSE
    FileMask = File;
 rc = SysFileTree( FileMask, 'File.', 'OF');

 DO i = 1 TO File.0

    rcx = STREAM( File.i, 'C', 'OPEN READ');

    /* read source file */
    ThisLine = LINEIN( File.i);
    DO WHILE (LINES( File.i) > 0)
   
       /* determine the tag */
       PARSE VAR ThisLine '@@'ThisFunction'@'ThisKey'@'ThisName'@'ThisType;
       IF ((LEFT( ThisLine, 2) = '@@') & (ThisFunction \= '') & (ThisKey \= '')) THEN
       DO
          ThisKey = TRANSLATE( ThisKey);
   
          /* check key */
          IF (WORDPOS( ThisKey, DocComment._ValidKeys) = 0) THEN ITERATE;
   
          /* store the info */
          NextLine = LINEIN( File.i);
          DO WHILE ( LEFT( NextLine, 2) \= '@@')
   
             /* handle key values */
             IF (ThisName = '') THEN
                DocComment.ThisFunction.ThisKey = DocComment.ThisFunction.ThisKey''CrLf''NextLine;
             ELSE
             /* handle key/name values values */
                DocComment.ThisFunction.ThisKey.ThisName = DocComment.ThisFunction.ThisKey.ThisName''CrLf''NextLine;
             NextLine = LINEIN( File.i);
          END;
   
          IF (ThisName = '') THEN
             DocComment.ThisFunction.ThisKey          = SUBSTR( DocComment.ThisFunction.ThisKey, 3);
          ELSE
             DocComment.ThisFunction.ThisKey.ThisName = SUBSTR( DocComment.ThisFunction.ThisKey.ThisName, 3);
   
          /* check type: input or output */
          ThisType = TRANSLATE( ThisType);
          SELECT
             WHEN (ThisType = 'IN')    THEN  ThisType = 'input';
             WHEN (ThisType = 'OUT')   THEN  ThisType = 'output';
             WHEN (ThisType = 'INOUT') THEN  ThisType = 'input/output';
             OTHERWISE                       ThisType = '';
          END;
          DocComment.ThisFunction.ThisKey.ThisName._InOutType = ThisType;
   
          /* state that something for this function has been found ! */
          DocComment.ThisFunction._Found = 1;
   
          /* go on with the line last read */
          ThisLine = NextLine;
          ITERATE;
   
       END;
   
       /* read next line */
       ThisLine = LINEIN( File.i);
   
    END;
    rcx = STREAM( File.i, 'C', 'CLOSE');

 END; /* DO i = 1 TO File.0 */

 RETURN( rc);

/* ========================================================================= */
/* write help index file and EPM keyword file */
WriteEPMFiles: PROCEDURE EXPOSE (GlobalVars)
 PARSE ARG OutDir, BookName, Description;

 /* default values */
 rc       = ERROR.NO_ERROR;
 CrLf     = "0d0a"x;
 Tab      = "09"x;
 StemList = '';

 BgColor  = '-1';
 FgColor  = '5';

 /* write index file */
 File = OutDir'\wpstk.ndx';
 rcx = SysFileDelete( File);

 rc = LINEOUT( File, 'EXTENSIONS: *');
 rc = LINEOUT( File, 'DESCRIPTION:' Description);

 /* check functions */
 WorkList = Function._List;
 DO WHILE (WorkList \= '')
    PARSE VAR WorkList ThisFunction WorkList;
    ThisStem = GetFuncStem( ThisFunction);
    IF (WORDPOS( ThisStem, StemList) = 0) THEN
       StemList = StemList ThisStem;
 END;
 StemList = SortString( StemList);
 DO WHILE (StemList \= '')
    PARSE VAR StemList ThisStem StemList;
 rc = LINEOUT( File, '('ThisStem'*, view' BookName '~)');
 END;

 /* take over all own dataypes */
 WorkList = SortString( DataType._OwnList);
 DO WHILE (WorkList \= '')
    PARSE VAR WorkList ThisType WorkList;
    rc = LINEOUT( File, '('ThisType', view' BookName '~)');
 END;
 rcx = STREAM( File, 'C', 'CLOSE');

 /* write hilighting file */
 File = OutDir'\epmkwds.c__';
 rcx = SysFileDelete( File);

 /* write lines for functions */
 rc = LINEOUT( File, '@ ------------ ' Description ' functions --------------');
 WorkList = SortString( Function._List);
 DO WHILE (WorkList \= '')
    PARSE VAR WorkList ThisFunction WorkList;
    rc = LINEOUT( File, ThisFunction''Tab''BgColor''Tab''FgColor);
 END;

 /* take over all own dataypes */
 rc = LINEOUT( File, '@ -------------------- ' Description ' types and defines --------------------');
 WorkList = SortString( DataType._OwnList) SortString(DefineList);
 DO WHILE (WorkList \= '')
    PARSE VAR WorkList ThisType WorkList;
    rc = LINEOUT( File, ThisType''Tab''BgColor''Tab''FgColor);
 END;
 rcx = STREAM( File, 'C', 'CLOSE');

 RETURN( rc);

/* ========================================================================= */
/* return the first part of a functions name */
GetFuncStem: PROCEDURE EXPOSE (GlobalVars)
 PARSE ARG Name;

 FromChars = XRANGE( 'a', 'z');
 ToChars   = COPIES( ' ', LENGTH( FromChars));
 CheckName = TRANSLATE( Name, ToChars, FromChars);
 RETURN( LEFT( Name, WORDINDEX( CheckName, 2) - 1));

