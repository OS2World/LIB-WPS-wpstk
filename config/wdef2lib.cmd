/*
 *      WDEF2LIB.CMD - rules.in script for Open Watcom - Christian Langanke 2006
 *
 *      Syntax: wdef2lib LibFile DefFile
 *
 *      This program uses the Watcom Library Manager to create an import
 *      library from the export statements of a module definition file.
 *      Alias definitions for an export are not supported.
 */

 '@ECHO OFF';
 rc          = 0;
 env         = 'OS2ENVIRONMENT';
 Redirection = '> NUL 2>&1';

 fFileOpened        = 0;
 fExportSectionOpen = 0;
 fStatementFound    = 0;
 DllName            = '';
 StatementList      = 'BASE CODE DATA DESCRIPTION EXETYPE EXPORTS HEAPSIZE IMPORTS',
                      'LIBRARY NAME OLD PHYSICAL PROTMODE SEGMENTS STACKSIZE STUB VIRTUAL';


 DO 1
    /* get command line parms */
    /* ignore errors */
    PARSE ARG LibFile DefFile;
    LibFile = STRIP( LibFile);
    DefFile = STRIP( DefFile);
    IF (DefFile = '') THEN
       LEAVE;

    /* open def file */
    IF (STREAM( DefFile, 'C', 'OPEN READ') \= 'READY:') THEN
    DO
       SAY 'error: cannot open' DefFile;
       rc = 110;
       LEAVE;
    END;
    fFileOpened = 1;

    ExportList = '';
    DO WHILE (LINES( DefFile) > 0)
       ThisLine = LINEIN( DefFile);
       IF (LEFT( STRIP( ThisLine), 1) = ';') THEN ITERATE;

       /* ckeck for export statement */
       ThisStatement = TRANSLATE( WORD( ThisLine, 1));
       fStatementFound = (WORDPOS( ThisStatement, StatementList) > 0);

       IF (fStatementFound) THEN
       DO
          IF (ThisStatement = 'LIBRARY') THEN
          DO
             PARSE VALUE TRANSLATE( ThisLine) WITH . DllName .;
             ITERATE;
          END;

          fExportSectionOpen = (ThisStatement = 'EXPORTS');
          ITERATE;
       END;

       IF (fExportSectionOpen) THEN
          ExportList = ExportList WORD( ThisLine, 1);
    END;

    /* loop through all exports */
    TmpFile = VALUE( 'TMP',,env)'\wdef2ilib.tmp';
    'DEL' TmpFile Redirection;
    DO WHILE (ExportList \= '')
       PARSE VAR ExportList ThisExport ExportList;
       rc = LINEOUT( TmpFile, '++'ThisExport'.'DllName);
    END;
    'wlib -q -b' LibFile '@'TmpFile;
    IF (rc \= 0) THEN
       SAY '- error creating symbol' ThisExport'.'DllName 'in' FILESPEC( 'N', LibFile);
    'DEL' TmpFile Redirection;
    rc = 0;

 END;

 /* cleanup */
 IF (fFileOpened) THEN
    rcx = STREAM( Defile, 'C', 'CLOSE');

 RETURN( rc);

