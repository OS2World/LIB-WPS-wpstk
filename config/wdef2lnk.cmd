/*
 *      WDEF2LNK.CMD - rules.in script for Open Watcom - Christian Langanke 2006
 *
 *      Syntax: wdef2lnk LnkFile DefFile [DataSymbolNamePart [...]]
 *
 *      This program uses the Watcom Library Manager to append certain statements
 *      to a wlink definition file.
 *
 *      If it is required to export data symbols without a prepended underscore,
 *      the symbols (or parts of the names) can be specified as additional
 *      parameters (DataSymbolNamePart). This will let wdef2lnk link with a
 *      directive "export mysymbol=_mysymbol" instead of "export mysymbol".
 *
 *      This may be used as well, but is generally not required for 
 *      function symbols, as they can be defined with the keyword _System 
 *      to avoid the prepended underscore (which is not possible for data symbols).
 *
 *      To automatically export the data symbols of WPS classes automatically
 *      for Open Watcom V1.5 or older, all symbols including 'ClassName' in 
 *      their names are exported that way.
 *
 * ==============================================================================
 *
 *      The following wlink directives and options are provided by rules.in,
 *      either by writing it to link.tmp or specifying it on the wlink command line:
 *      directives: DEBUG FILE LIB NAME SYSTEM LIBPATH FORMAT
 *         options: ALIGNMENT DESCRIPTION OFFSET QUIET OSNAME
 *
 *      directive/option  unsupported atributes
 *      ----------------  ---------------------
 *      FILE              (obj_module)
 *
 *      The following wlink directives and options are not used by rules.in:
 *         options: RESOURCE
 *
 * ==============================================================================
 *
 *      The following wlink directives and options are supported by wdef2lnk:
 *
 *      def directive           wlink type  wlink statement or device name
 *      -------------           ----------  ------------------------------
 *      BASE
 *      CODE
 *      DATA
 *      DATA MULTIPLE           option      MANYAUTODATA
 *      DATA NONE               option      NOAUTODATA
 *      DATA SINGLE             option      ONEAUTODATA
 *      DESCRIPTION             option      DESCRIPTION
 *      EXETYPE
 *      EXPORTS                 directive   EXPORT
 *      HEAPSIZE
 *      IMPORTS 
 *      LIBRARY                 directive   FORMAT
 *      LIBRARY INITGLOBAL      option      ONEAUTODATA
 *      LIBRARY INITINSTANCE    option      MANYAUTODATA
 *      NAME                    directive   NAME
 *      OLD
 *      PHYSICAL DEVICE 
 *      PROTMODE
 *      SEGMENTS
 *      STACKSIZE
 *      STUB
 *      VIRTUAL DEVICE  
 *
 * ==============================================================================
 *
 *      The following wlink directives and options are not yet supported by
 *      wdef2lnk, which may lead to problems:
 *
 *       directives: OPTLIB ORDER OUTPUT SEGMENT SORT
 *          options: IMPFILE IMPLIB IMPORT LIBRARY
 *                    MODNAME MODFILE NEWFILES NEWSEGMENT
 *                    NODEFAULTLIBS NOEXTENSION OLDLIBRARY
 *                   PACKCODE PACKDATA PROTMODE REDEFSOK SHOWDEAD STACK ALL STUB 
 *                   SYMFILE 
 *
 *      For the following directive/option, the specified attributes are not supported:
 *
 *      directive/option  unsupported atributes
 *      ----------------  ---------------------
 *      EXPORT            ordinal, PRIVATE, RESIDENT, iopl_bytes, lbc_file
 *
 * ==============================================================================
 * 
 *      The following wlink directives and options do not match any directive
 *      in a module definition file:
 *      directives: ALIAS DISABLE INCREMENTAL INTERNALRELOCS MANGLEDNAMES MAXERRORS
 *                  MODTRACE NAMELEN SHIFT SYMTRACE
 *         options: ARTIFCIAL (NO)CACHE (NO)CASEEXACT CVPACK DOSSEG ELIMINATE
 *                  STARTLINK ENDLINK (NO)FARCALLS FILLCHAR START PATH SYMFILE
 *                  TOGGLERELOCS UNDEFSOK VERBOSE VERSION VFREMOVAL
 *
 *    TODO: 
 *        - check IMPLIB NEWFILES (for 16bit) STACK STUB SYMFILE HEAPSIZE STACK
 *        - check VERSION !
 */

 '@ECHO OFF';
 rc          = 0;
 CrLf        = '0d0a'x;
 env         = 'OS2ENVIRONMENT';
 TRUE         = (1 = 1);
 FALSE        = (0 = 1);
 Redirection = '> NUL 2>&1';

 GlobalVars = 'CrLf env TRUE FALSE Redirection';

 fDefFileOpened     = 0;
 fLnkFileOpened     = 0;
 fExportSectionOpen = 0;
 fStatementFound    = 0;
 ValidStatements    = 'BASE CODE DATA DESCRIPTION EXETYPE EXPORTS HEAPSIZE IMPORTS ',
                      'LIBRARY NAME OLD PHYSICAL PROTMODE SEGMENTS STACKSIZE STUB VIRTUAL';

 DO 1
    /* get command line parms */
    /* ignore errors */
    PARSE ARG LnkFile DefFile NamePartList;
    LnkFile = STRIP( LnkFile);
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
    fDefFileOpened = 1;

    /* -------------------------------------------------- */

    ExportList = '';
    StatementList = '';
    DllFormatParameters = '';
    DO WHILE (LINES( DefFile) > 0)
       ThisLine = LINEIN( DefFile);
       IF (LEFT( STRIP( ThisLine), 1) = ';') THEN ITERATE;

       /* ckeck for export statement */
       ThisStatement = TRANSLATE( WORD( ThisLine, 1));
       fStatementFound = (WORDPOS( ThisStatement, ValidStatements) > 0);

       NewStatement = '';
       IF (fStatementFound) THEN
       DO
          fExportSectionOpen = (ThisStatement = 'EXPORTS');

          SELECT

             WHEN (ThisStatement = 'DESCRIPTION') THEN
             DO
                PARSE VALUE ThisLine WITH . ThisValue;
                NewStatement = 'option description' ThisValue;
             END;

             /* ------------------ */

             WHEN (ThisStatement = 'LIBRARY') THEN
             DO
                PARSE VALUE ThisLine WITH . . ValueList;
                DllFormatParameters = TRANSLATE( ValueList);

             END;

             /* ------------------ */

             WHEN (ThisStatement = 'DATA') THEN
             DO
                PARSE VALUE ThisLine WITH . ValueList;
                ValueList = TRANSLATE( ValueList);

                SegmentKeywords = '';
                SegmentOption = '';
                DO WHILE (ValueList \= '')
                   PARSE VAR ValueList Keyword ValueList;
                   fIsSegmentKeyword = FALSE;
                   SELECT
                      WHEN (Keyword = 'PRELOAD')    THEN fIsSegmentKeyword = TRUE;
                      WHEN (Keyword = 'LOADONCALL') THEN fIsSegmentKeyword = TRUE;
                      WHEN (Keyword = 'READONLY')   THEN fIsSegmentKeyword = TRUE;
                      WHEN (Keyword = 'READWRITE')  THEN fIsSegmentKeyword = TRUE;
                      WHEN (Keyword = 'SHARED')     THEN fIsSegmentKeyword = TRUE;
                      WHEN (Keyword = 'NONSHARED')  THEN fIsSegmentKeyword = TRUE;
                      WHEN (Keyword = 'IOPL')       THEN fIsSegmentKeyword = TRUE;
                      WHEN (Keyword = 'NOIOPL')     THEN fIsSegmentKeyword = TRUE;
                      WHEN (Keyword = 'NONE')       THEN SegmentOption     = 'NOAUTODATA';
                      WHEN (Keyword = 'SINGLE')     THEN SegmentOption     = 'ONEAUTODATA';
                      WHEN (Keyword = 'MULTIPLE')   THEN SegmentOption     = 'MANYAUTODATA';
                   END;
                   IF (fIsSegmentKeyword) THEN
                      SegmentKeywords = SegmentKeywords Keyword;
                END;

                /* assemble wlink statements */
                NewStatement = 'segment data' SegmentKeywords;
                IF (SegmentOption \= '') THEN
                   NewStatement = NewStatement''CrLf'option' SegmentOption;

             END;

             /* ------------------ */

             OTHERWISE NOP;
          END;

          /* store new statement */
          IF (NewStatement \= '') THEN
          DO
             IF (StatementList = '') THEN
                StatementList = NewStatement;
             ELSE
                StatementList = StatementList''CrLf''NewStatement;
          END;

          ITERATE;
       END;

       IF (fExportSectionOpen) THEN
          ExportList = ExportList WORD( ThisLine, 1);
    END;

    /* open wlink linker def file */
    IF (STREAM( LnkFile, 'C', 'OPEN WRITE') \= 'READY:') THEN
    DO
       SAY 'error: cannot open' DefFile;
       rc = 110;
       LEAVE;
    END;
    fLnkFileOpened = 1;

    /* -------------------------------------------------- */

    /* if we need to write format parameters, read the file */
    IF (DllFormatParameters \= '') THEN
    DO

       IF (FileExist( LnkFile)) THEN
       DO
          /* open def file */
          IF (STREAM( LnkFile, 'C', 'OPEN READ') \= 'READY:') THEN
          DO
             SAY 'error: cannot open' LnkFile;
             rc = 110;
             LEAVE;
          END;
   
          /* discard old format directive                 */
          /* and prepend everything to the statement list */
          OldStatementList = '';
          DO WHILE (LINES( LnkFile) > 0)
             OldStatement = STRIP( LINEIN( LnkFile));
             IF (OldStatement = '') THEN ITERATE;
             IF (TRANSLATE( WORD( OldStatement, 1)) \= 'FORMAT') THEN
                OldStatementList = OldStatementList''CrLf''OldStatement;
          END;
   
          /* close and rewrite file */
          rcx = STREAM( LnkFile, 'C', 'CLOSE');
          'DEL' LnkFile Redirection;
          rc = LINEOUT( LnkFile, OldStatementList);

          /* add format statement */
          rc = LINEOUT( LnkFile, 'format os2 lx dll' DllFormatParameters);
          IF (SegmentOption = '') THEN
          DO
             IF (WORDPOS( 'INITINSTANCE', DllFormatParameters) > 0) THEN
                rcx = LINEOUT( LnkFile,  'option MANYAUTODATA');
             IF (WORDPOS( 'INITGLOBAL', DllFormatParameters) > 0) THEN
                rcx = LINEOUT( LnkFile, 'option ONEAUTODATA');
          END;

          rc = LINEOUT( LnkFile);
       END;

    END;

    /* -------------------------------------------------- */

    /* write additions to link file */
    IF (StatementList \= '') THEN
       rc = LINEOUT( LnkFile, StatementList);

    DO WHILE (ExportList \= '')
       PARSE VAR ExportList ThisExport ExportList;

       IF (IsSystemDataExport( ThisExport, NamePartList)) THEN
          ThisExport = ThisExport'=_'ThisExport;

       rc = LINEOUT( LnkFile, 'export' ThisExport);
    END;

 END;

 /* cleanup */
 IF (fDefFileOpened) THEN
    rcx = STREAM( DefFile, 'C', 'CLOSE');
 IF (fLnkFileOpened) THEN
    rcx = STREAM( LnkFile, 'C', 'CLOSE');

 RETURN( rc);

/* ------------------------------------------------------------------------- */
FileExist: PROCEDURE
 PARSE ARG FileName

 RETURN(STREAM(Filename, 'C', 'QUERY EXISTS') > '');
    
/* ========================================================================= */
IsSystemDataExport: PROCEDURE EXPOSE (GlobalVars)
 PARSE ARG ExportName, SymbolNamePartList;

 fIsClassDataExport  = FALSE;
 fUnderScoreTranslationRequired = TRUE;

 DO 1

   /* check version of Watcom compiler */
   QueueName = RXQUEUE('CREATE');
   rcx       = RXQUEUE('SET', QueueName);
   '@wcc386 | rxqueue' QueueName;
   PARSE PULL VerLine;
   rc = RXQUEUE('DELETE', QueueName);
   rc = RXQUEUE('SET', 'SESSION');
   WatComVersion = WORD( VerLine, WORDS( VerLine));
   fUnderScoreTranslationRequired = (WatComVersion < 1.5);

   /* for watcom V1.5 or older, provide underscore translation */
   NamePartList = SymbolNamePartList;
   IF (fUnderScoreTranslationRequired) THEN
      SymbolNamePartList = SymbolNamePartList  'ClassData';

   /* check all name parts */
   DO WHILE (NamePartList \= '')

      PARSE VAR NamePartList ClassDataNamePart NamePartList;

      IF (LENGTH( ExportName) <= LENGTH( ClassDataNamePart)) THEN
         LEAVE;
   
      IF (POS( ClassDataNamePart, ExportName) > 0) THEN
      DO
         fIsClassDataExport = TRUE;
         LEAVE
      END;
   END;
 END;

 RETURN( fIsClassDataExport);

