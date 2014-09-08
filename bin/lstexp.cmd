/*
 *      LSTEXP.CMD - WPS Toolkit - Christian Langanke 2000-2008
 *
 *      Syntax: lstexp lib_listfile [func_prefix [func_prefix [...]]]
 *
 *      This program dunps all public symbols from a library
 *      list file to standard output. This is used to generate EXPORTS
 *      entries for a DEF file being used to generate an import library.
 *
 *      An alternative to this scheme is using the _Exports keyword
 *      within the source, but this is not supported by the IBM compilers.
 *      LSTEXP.CMD implements somewhat the GNU emxexp utility, exporting
 *      the public definitions from all object files. Because input
 *      redirection does not work from emxexp to the find command under
 *      eCS V1.1, we use this script also for gcc, so for all compilers.
 */
/* $Id: lstexp.cmd,v 1.9 2008-10-15 16:43:16 cla Exp $ */
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

 rc = 0;
 env = 'OS2ENVIRONMENT';
 '@ECHO OFF';

 call RxFuncAdd    'SysLoadFuncs', 'RexxUtil', 'SysLoadFuncs'
 call SysLoadFuncs

 DO 1

    /* get filename */
    PARSE ARG Filename PrefixList;
    Filename   = STRIP( Filename);
    PrefixList = STRIP( PrefixList);
    IF (Filename = '') THEN
    DO
       SAY 'error: no library listfile specified!';
       rc = 87;
       LEAVE;
    END;
    IF (PrefixList = '') THEN
    DO
       SAY 'error: no prefix specified!';
       rc = 87;
       LEAVE;
    END;

    rcx = STREAM( Filename, 'C', 'OPEN READ');
    IF (rcx \= 'READY:') THEN
    IF (Filename \= '') THEN
    DO
       SAY 'error: cannot open file' Filename'!';
       rc = 5;
       LEAVE;
    END;

    /* open sort file */
    SortFile = SysTempFileName( VALUE('TMP',,env)'\lstexp.???');
   
    /* check all lines */
    ApiList = '';
    DO WHILE (LINES( Filename) > 0)
       ThisLine = LINEIN( Filename);

       /* dump remaining lines */
       DO WHILE (ThisLine \= '')

          PARSE VAR ThisLine ThisWord ThisLine;

          /* skip word if already found */
          IF (WORDPOS( ThisWord, ApiList) > 0) THEN ITERATE;
          IF (POS( '.', ThisWord) > 0) THEN ITERATE;

          /* check for matching function name */
          CheckList = PrefixList;
          DO WHILE (CheckList \= '')
             PARSE VAR CheckList ThisPrefix CheckList;
             fMatch = (LEFT( ThisWord, LENGTH( ThisPrefix)) = ThisPrefix);
             IF (fMatch) THEN LEAVE;
          END;

          IF (fMatch) THEN
          DO
             rcx = LINEOUT( SortFile, '  'ThisWord);
             ApiList = ApiList ThisWord;
          END;
       END;

    END;
    rcx = STREAM( Filename, 'C', 'CLOSE');
    rcx = STREAM( SortFile, 'C', 'CLOSE');

    /* display sorted file and delete it */
    'sort <' SortFile;
    rcx = SysFileDelete( SortFile);


 END;

 RETURN( rc);

