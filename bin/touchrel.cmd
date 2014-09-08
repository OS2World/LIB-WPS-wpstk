/*
 *      TOUCHREL.CMD - WPS Toolkit - C.Langanke 2002-2008
 *
 *      Syntax: touchrel dirname|filename
 *
 *    This program calls GNU touch to set the timestamp of the files specified,
 *    or all files  below  the specified directory (tree).
 *    The timestamp of the last full hour or last half hour is used.
 */
/* $Id: touchrel.cmd,v 1.10 2009-07-10 21:36:11 cla Exp $ */
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

 TRUE         = (1 = 1);
 FALSE        = (0 = 1);
 '@ECHO OFF'

 /* get command parms */
 PARSE ARG Parms;
 IF (Parms = '') THEN
 DO
    SAY 'no parms given.';
    EXIT( 87);
 END;

 TimeStamp = GetTimeStamp();

 DO WHILE (Parms \= '')
    PARSE VAR Parms ThisParm Parms;

    /* SAY '- touching' ThisParm; */

    IF (FileExist( ThisParm)) THEN
       rcx = TouchFiles( ThisParm, TimeStamp, FALSE);
    ELSE
       rcx = TouchFiles( ThisParm'\*', TimeStamp, TRUE);
 END;

 EXIT( rc);

/* ------------------------------------------------------------------------- */
FileExist: PROCEDURE
 PARSE ARG FileName

 RETURN(STREAM(Filename, 'C', 'QUERY EXISTS') > '');

/* ------------------------------------------------------------------------- */
TouchFiles: PROCEDURE
 PARSE ARG Parm, TimeStamp, fRecursive;

 IF (fRecursive) THEN
 DO
    /* process subdirectories */
    rc = SysFileTree( Parm, 'Dir.', 'DOS');
    IF (rc = 0) THEN
    DO
       /* process all directories */
       DO d = 1 TO Dir.0
          rc = TouchFiles( Dir.d'\*', TimeStamp, fRecursive);
       END;
    END;
 END;

 /* touch files */
 rc = SysFileTree( Parm, 'Files.', 'FO');
 IF (rc = 0) THEN
 DO
    /* process all files */
    /* - as newer versions of touch don't support  */
    /*   wildcards we have to expand them here     */
    /* - to reduce overhead, we collect some files */
    /*   for each call to touch.exe                */
    FileList = '';
    DO f = 1 TO Files.0
       FileList = FileList Files.f;
       IF ((LENGTH( FileList) > 768) | (f = Files.0)) THEN
       DO
          /* explicitely call touch.exe to avoid 4os2 internal command */
          Command = 'call touch.exe -t' TimeStamp FileList;
          '' Command
          FileList = '';
       END
    END;
 END;

 RETURN( 0);

/* ------------------------------------------------------------------------- */
GetTimeStamp: PROCEDURE

 /* check timestamp */
 TimeStamp = '';
 PARSE VALUE DATE('S')  WITH Year +4 MonthDay;
 PARSE VALUE TIME( 'N') WITH Hours':'Mins':'Secs;
 Hours = RIGHT( Hours, 2, '0');
 IF (Mins > 30) THEN
    Mins = '30';
 ELSE
    Mins = '00';

 /* determine timestamp parameter from help text of touch */
 QueueName = RXQUEUE('CREATE');
 rc        = RXQUEUE('SET', QueueName);

 'touch --help | rxqueue' QueueName;

 DO WHILE (QUEUED() > 0)
    PARSE PULL Line;
    SELECT
       WHEN (POS( 'MMDDhhmm[[CC]YY][.ss]', Line) > 0) THEN
          TimeStamp = MonthDay''Hours''Mins''Year'.00';

       WHEN (POS( '[[CC]YY]MMDDhhmm[.ss]', Line) > 0) THEN
          TimeStamp = Year''MonthDay''Hours''Mins'.00';

       WHEN (POS( '[[HH]JJ]MMTTSSmm[.ss]', Line) > 0) THEN
          TimeStamp = Year''MonthDay''Hours''Mins'.00';

       OTHERWISE NOP;
    END;
 END;

 rc = RXQUEUE('DELETE', QueueName);
 rc = RXQUEUE('SET', 'SESSION');

 IF (TimeStamp = '') THEN
 DO
    SAY 'error: touchrel does not know this version of touch.exe'
    SAY '       please modify procedure GetTimeStamp accordingly';
    EXIT(87);
 END;

 RETURN( TimeStamp);

