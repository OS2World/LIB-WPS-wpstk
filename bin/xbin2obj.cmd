/*
 *      XBIN2OBJ.CMD - WPS Toolkit - Christian Langanke 1997-2008
 *
 *    This is a bugfree clone of Peter Kobaks BIN2OBJ, which also can write
 *    16-bit object files.
 *
 *    Usage: XBIN2OBJ [options] binfile symbolname [objfile]
 *
 *       binfile    = name of the existing binary file.
 *       symbolname = symbolic name used in the object file.
 *                    For 16-bit modules an underscore is prepended internally !
 *       objfile    = name of the new object file; by default the object file name
 *                    will be same as the binfile with an .obj extension.
 *       options    = any of the following:
 *         -32      = write 32-bit object module (default)
 *         -16      = write 16-bit object module,
 *                    NOTE: data for 16-bit modules cannot be greater than 64Kb !
 *         -n       = size of the binfile, a 16/32-bit value, will be put in front of
 *                    the binary block, so that its size can be queried.
 *         -sName   = name of the segment; by default it is
 *                    - '_DATA' for 16-bit object modules
 *                    - 'DATA32' for 32-bit object modules
 *                    If you get linker error "DGROUP : group larger than 64K bytes"
 *                    for 16-bit modules, specify -cCODE or -cCONST to avoid placing
 *                    the data ino the data segment or reduce the stack size, as the
 *                    stack segment is also placed into the data DGROUP.
 *         -cClass  = class of the segment; by default it is 'DATA'.
 *                      The recommended classes are 'DATA', 'CONST', or 'CODE'.
 *         -lEnvvar = locate file within directories specified in environment
 *                    variable (like DPATH, PATH etc.)
 */
/* first comment is used as help text */
/* $Id: xbin2obj.cmd,v 1.5 2008-10-15 16:43:16 cla Exp $ */
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

 SIGNAL ON HALT

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
 ERROR.INVALID_FUNCTION   =  1;
 ERROR.FILE_NOT_FOUND     =  2;
 ERROR.PATH_NOT_FOUND     =  3;
 ERROR.ACCESS_DENIED      =  5;
 ERROR.NOT_ENOUGH_MEMORY  =  8;
 ERROR.INVALID_FORMAT     = 11;
 ERROR.INVALID_DATA       = 13;
 ERROR.NO_MORE_FILES      = 18;
 ERROR.WRITE_FAULT        = 29;
 ERROR.READ_FAULT         = 30;
 ERROR.GEN_FAILURE        = 31;
 ERROR.INVALID_PARAMETER  = 87;

 GlobalVars = 'Title CmdName env TRUE FALSE Redirection ERROR.';
 SAY;

 /* show help */
 ARG Parm .
 IF ((Parm = '') | (POS('?', Parm) > 0)) THEN
 DO
    rc = ShowHelp();
    EXIT(ERROR.INVALID_PARAMETER);
 END;

 /* Defaults */
 GlobalVars = GlobalVars 'Seg.';

 rc              = ERROR.NO_ERROR;
 Seg.ClassName   = 'DATA';
 Seg.SegmentName = '';
 Seg.TypeName    = 'FLAT';
 Seg.Envname     = '';
 Seg.f16Bit      = FALSE;
 Seg.WriteSize   = FALSE;
 BinFile         = '';
 SymbolName      = '';
 ObjFile         = '';


 DO UNTIL (TRUE)

    /* get parms */
    PARSE ARG Parms
    DO i = 1 TO WORDS( Parms)
       ThisParm = WORD( Parms, i);
       PARSE VAR ThisParm ThisTag +2 ThisValue;
       ThisTag   = TRANSLATE( ThisTag);
       SELECT
          WHEN (ThisTag  = '-N')  THEN Seg.WriteSize   = TRUE;
          WHEN (ThisTag  = '-S')  THEN Seg.SegmentName = ThisValue;
          WHEN (ThisTag  = '-C')  THEN Seg.ClassName   = ThisValue;
          WHEN (ThisTag  = '-L')  THEN Seg.Envname     = ThisValue;
          WHEN (ThisParm = '-16') THEN Seg.f16Bit      = TRUE;
          WHEN (ThisParm = '-32') THEN Seg.f16Bit      = FALSE;
          OTHERWISE
          DO
             /**/ IF (BinFile = '') THEN
                BinFile = ThisParm;
             ELSE IF (SymbolName = '') THEN
                SymbolName = ThisParm;
             ELSE IF (ObjFile = '') THEN
                ObjFile = ThisParm;
             ELSE
             DO
                SAY CmdName': error: Invalid parameter ®'ThisParm'¯ specified.';
                rc = ERROR.INVALID_PARAMETER;
             END;
          END;
       END;
    END;
    IF (rc \= ERROR.NO_ERROR) THEN
       LEAVE;

    IF (SymbolName = '') THEN
    DO
       rc = ShowHelp();
       rc = ERROR.INVALID_PARAMETER;
       LEAVE;
    END;

    /* check source file, search it in environment path ?  */
    IF (Seg.Envname \= '') THEN
    DO
       FullName = SysSearchPath( Seg.Envname, BinFile);
       IF (FullName \= '') THEN
          BinFile = FullName;
    END;

    IF (\FileExist( BinFile)) THEN
    DO
       SAY CmdName': Error: File ®'BinFile'¯ not found.';
       EXIT( ERROR.FILE_NO_FOUND);
    END;


    /* check 64 kb limit for 16-bit memory model */
    BinSize = STREAM( BinFile, 'C', 'QUERY SIZE');
    IF ((Seg.f16Bit) & (Binsize > 65535)) THEN
    DO
       SAY CmdName': error: binary file exceeds 64Kb boundary for 16-bit modules.';
       rc = ERROR.INVALID_PARAMETER;
       LEAVE;
    END;

    /* determine default segment name */
    IF (Seg.SegmentName = '') THEN
    DO
       IF (Seg.f16Bit) THEN
          Seg.SegmentName = '_DATA';
       ELSE
          Seg.SegmentName = 'DATA32';
    END;

    /* prepend an underscore for 16-bit compiler */
    IF (Seg.f16Bit) THEN
       SymbolName = '_'SymbolName;

    /* determine object file name */
    IF (ObjFile = '') THEN
    DO
       ObjFile = FILESPEC('N', BinFile);
       ExtPos = LASTPOS( '.', ObjFile);
       IF (ExtPos \= 0) THEN
          ObjFile = LEFT( ObjFile, ExtPos - 1);
       ELSE
          ObjFile = BinFile;
       ObjFile = ObjFile'.obj';
    END;

    /* delete target file */
    'IF EXIST' ObjFile 'DEL' ObjFile Redirection;
    IF (FileExist( ObjFile)) THEN
    DO
       SAY CmdName': Error: File ®'ObjFile'¯ cannot be rewritten.';
       EXIT( ERROR.WRITE_FAULT);
    END;

    /* generate object code */
    /* SAY 'writing seg name' Seg.SegmentName', class' Seg.ClassName', type' Seg.TypeName; */
    rc = GenerateObj( BinFile, SymbolName, ObjFile, Seg.f16Bit);
 END;

 EXIT( rc);

/* ------------------------------------------------------------------------- */
HALT:
 SAY 'Interrupted by User.';
 EXIT(ERROR.GEN_FAILURE);

/* ------------------------------------------------------------------------- */
ShowHelp: PROCEDURE EXPOSE (GlobalVars)

 SAY Title;
 SAY;

 PARSE SOURCE . . ThisFile

 DO i = 1 TO 3
    rc = LINEIN(ThisFile);
 END;

 ThisLine = LINEIN(Thisfile);
 DO WHILE (ThisLine \= ' */')
    SAY SUBSTR(ThisLine, 7);
    ThisLine = LINEIN(Thisfile);
 END;

 /* Datei wieder schlieáen */
 rc = LINEOUT(Thisfile);

 RETURN('');

/* ------------------------------------------------------------------------- */
FileExist: PROCEDURE
 PARSE ARG FileName

 RETURN(STREAM(Filename, 'C', 'QUERY EXISTS') > '');

/* ========================================================================= */
GenerateObj: PROCEDURE EXPOSE (GlobalVars)
 PARSE ARG BinFile, SymbolName, ObjFile, f16Bit;

 rc = ERROR.NO_ERROR;
 DataOffset      = 0;
 SegSizeFieldLen = 0;

 /* segment attribute                             */
 /* 0110 100?                                     */
 /* 011          relocateable, paragraph aligned  */
 /*    0 10      public                           */
 /*        0     high order bit of lenth field    */
 /*         x    0: 16-bit, 1: 32-bit             */
 IF (f16Bit) THEN
    SegmentAttr = X2D( 68);
 ELSE
    SegmentAttr = X2D( 69);

 /* determine module name */
 ModuleName = FILESPEC( 'N', BinFile);

 /* query filesize of file */
 BinSize = STREAM( BinFile, 'C', 'QUERY SIZE');

 /* determine seg size */
 SegSize = BinSize;
 IF (Seg.WriteSize) THEN
 DO
    IF (f16Bit) THEN
       SegSizeFieldLen = 2;
    ELSE
       SegSizeFieldLen = 4;
 END;
 SegSize = SegSize + SegSizeFieldLen;

 /* 80 RECTYPE_THEADR */
 StringLen = LENGTH( ModuleName);
 HeaderLen = StringLen + 2;
 rc = CHAROUT( ObjFile, D2C( X2D( 80)));
 rc = CHAROUT( ObjFile, REVERSE( D2C( HeaderLen, 2)));

 rc = CHAROUT( ObjFile, D2C( StringLen));
 rc = CHAROUT( ObjFile, ModuleName);

 rc = CHAROUT( ObjFile, D2C(0));

 /* 96 RECTYPE_NAMELIST */
 GroupName = 'DGROUP';
 StringLen = LENGTH( Seg.SegmentName''Seg.ClassName''Seg.TypeName''GroupName);
 HeaderLen = StringLen + 5;
 rc = CHAROUT( ObjFile, D2C( X2D( 96)));
 rc = CHAROUT( ObjFile, REVERSE( D2C( HeaderLen, 2)));

 rc = CHAROUT( ObjFile, D2C( LENGTH( Seg.SegmentName)));
 rc = CHAROUT( ObjFile, Seg.SegmentName);

 rc = CHAROUT( ObjFile, D2C( LENGTH( Seg.ClassName)));
 rc = CHAROUT( ObjFile, Seg.ClassName);

 rc = CHAROUT( ObjFile, D2C( LENGTH( Seg.TypeName)));
 rc = CHAROUT( ObjFile, Seg.TypeName);

 rc = CHAROUT( ObjFile, D2C( LENGTH( GroupName)));
 rc = CHAROUT( ObjFile, GroupName);

 rc = CHAROUT( ObjFile, D2C(0));

 /* 98 RECTYPE_SEGDEF 16 bit */
 /* 99 RECTYPE_SEGDEF 32 bit */
 IF (f16Bit) THEN
 DO
    HeaderLen = 7;
    rc = CHAROUT( ObjFile, D2C( X2D(98)));
 END
 ELSE
 DO
    HeaderLen = 9;
    rc = CHAROUT( ObjFile, D2C( X2D(99)));
 END
 rc = CHAROUT( ObjFile, REVERSE( D2C(HeaderLen, 2)));
 rc = CHAROUT( ObjFile, D2C( SegmentAttr));

 rc = CHAROUT( ObjFile, REVERSE( D2C(SegSize, SegSizeFieldLen))); /* size of all data */

 rc = CHAROUT( ObjFile, REVERSE( D2C( 1)));    /* DB: Segm Name Index    */
 rc = CHAROUT( ObjFile, D2C( 2));              /* DB: Class Name Index   */
 rc = CHAROUT( ObjFile, REVERSE( D2C( 0)));    /* DB: Overlay name index */

 rc = CHAROUT( ObjFile, D2C(0));

 /* 9a RECTYPE_GRPDEF for pseudo FLAT group */
 HeaderLen = 2;
 rc = CHAROUT( ObjFile, D2C( X2D( 9A)));
 rc = CHAROUT( ObjFile, REVERSE( D2C( HeaderLen, 2)));

 rc = CHAROUT( ObjFile, D2C(3));       /* DB: group name index */
 rc = CHAROUT( ObjFile, D2C(0));

 /* 9a RECTYPE_GRPDEF for DGROUP  group */
 HeaderLen = 4;
 rc = CHAROUT( ObjFile, D2C( X2D( 9A)));
 rc = CHAROUT( ObjFile, REVERSE( D2C( HeaderLen, 2)));

 rc = CHAROUT( ObjFile, D2C( 4));        /* DB: group name index */
 rc = CHAROUT( ObjFile, X2C( FF));       /* DB: delimiter        */
 rc = CHAROUT( ObjFile, D2C( 1));        /* DB: segment index    */
 rc = CHAROUT( ObjFile, D2C( 0));

 /* 91 RECTYPE_PUBDEF */
 HeaderLen = 2;
 HeaderLen = LENGTH( SymbolName) + 9;
 rc = CHAROUT( ObjFile, D2C( X2D( 91)));
 rc = CHAROUT( ObjFile, REVERSE( D2C( HeaderLen, 2)));

 rc = CHAROUT( ObjFile, D2C( 2));       /* DB: base group index */
 rc = CHAROUT( ObjFile, D2C( 1));       /* DB: base segment index */

 rc = CHAROUT( ObjFile, D2C( LENGTH( SymbolName)));
 rc = CHAROUT( ObjFile, SymbolName);

 rc = CHAROUT( ObjFile, REVERSE( D2C( 0, 4))); /* data offset */
 rc = CHAROUT( ObjFile, D2C( 0));             /* type index */

 rc = CHAROUT( ObjFile, D2C( 0));

if 0 then
do
 /* 88 RECTYPE_COMMENT Type 40 Subtype A2 */
 HeaderLen = 4;
 rc = CHAROUT( ObjFile, D2C( X2D( 88)));
 rc = CHAROUT( ObjFile, REVERSE( D2C( HeaderLen, 2)));

 rc = CHAROUT( ObjFile, D2C( X2D( 40)));       /* comment type              */
 rc = CHAROUT( ObjFile, D2C( X2D( A2)));       /* comment class : link pass */
 rc = CHAROUT( ObjFile, D2C( 1));              /* link pass 1               */

 rc = CHAROUT( ObjFile, D2C( 0));
end

 IF (Seg.WriteSize) THEN
 DO
    /* a1 RECTYPE_LEDATA for each 1024 bytes */
    DataSize   = SegSizeFieldLen;
    HeaderLen  = DataSize + 6;
    rc = CHAROUT( ObjFile, D2C( X2D( A1)));
    rc = CHAROUT( ObjFile, REVERSE( D2C( HeaderLen, 2)));
    rc = CHAROUT( ObjFile, D2C( 1));                                     /* segment index    */
    rc = CHAROUT( ObjFile, REVERSE( D2C( DataOffset, 4)));               /* data offset      */
    rc = CHAROUT( ObjFile, REVERSE( D2C( BinSize, SegSizeFieldLen)));    /* this is the data */
    rc = CHAROUT( ObjFile, D2C( 0));
    DataOffset = DataOffset + DataSize;
 END;

 /* make data records for each 1024 bytes ! */
 DataToWrite = BinSize;
 DO WHILE (DataToWrite  \= 0)
    /* write data item */
    DataSize   = MIN( 1024, DataToWrite);
    Data = CHARIN(BinFile,, DataSize);
    HeaderLen  = DataSize + 6;
    rc = CHAROUT( ObjFile, D2C( X2D( A1)));
    rc = CHAROUT( ObjFile, REVERSE( D2C( HeaderLen, 2)));
    rc = CHAROUT( ObjFile, D2C( 1));                       /* segment index */
    rc = CHAROUT( ObjFile, REVERSE( D2C( DataOffset, 4))); /* data offset */
    rc = CHAROUT( ObjFile, data);
    rc = CHAROUT( ObjFile, D2C( 0));
    DataOffset  = DataOffset + DataSize;
    DataToWrite = DataToWrite - DataSize;
 END;

 /* write end-of-module */
 HeaderLen  = 2
 rc = CHAROUT( ObjFile, D2C( X2D( 8B)));
 rc = CHAROUT( ObjFile, REVERSE( D2C( HeaderLen, 2)));
 rc = CHAROUT( ObjFile, D2C( 1));
 rc = CHAROUT( ObjFile, D2C( 0));

 /* close file */
 rc = STREAM( BinFile, 'C', 'CLOSE');
 rc = LINEOUT( ObjFile);

 RETURN(0);

