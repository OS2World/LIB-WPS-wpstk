/*
 *      IPFCFILT.CMD - WPS Toolkit - Christian Langanke 2000-2008
 *
 *      This program serves a a filter to IPFC output and is especially
 *      useful, when you let it run within the make macros of EPM.
 *      This filter will change the error messages, so that they compiy
 *      to the normal error messages of the C compilers. This way you
 *      can use EPM to examine all the errors also within your IPF code.
 */
/* $Id: ipfcfilt.cmd,v 1.5 2008-10-15 16:43:15 cla Exp $ */
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

 /* empty system rexx queue */
 DO WHILE (QUEUED() > 0)
    PARSE PULL rc
 END;

 /* read everything from console */
 '@rxqueue'
 Trigger = 0;
 TriggerLine = 'Compiling document with following Country Code, Code Page';

 /* check all lines */
 DO WHILE (QUEUED() > 0)
    PARSE PULL ThisLine

    /* logo skipped ? */
    IF (POS( TriggerLine, ThisLine) = 1 ) THEN
       Trigger = 1;

    /* change all error lines so that EPM can handle them */
    IF (Trigger) THEN
    DO
       IF (LEFT(ThisLine, 1) = '<') THEN 
       DO
          PARSE VAR ThisLine '<'fileinfo'>' error;
          numpos = LASTPOS(':', fileinfo);
          file = LEFT( fileinfo, numpos - 1);
          line = SUBSTR( fileinfo, numpos + 1);
          ThisLine = file'('line')' error;
       END;
       SAY ThisLine
    END;

 END;

