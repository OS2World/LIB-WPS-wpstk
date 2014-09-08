/*
 *      XEMXEXP.CMD - WPS Toolkit - Christian Langanke 2005-2008
 *
 *      This program is an enhancement to the exmexp command of the
 *      gcc compiler. It was required because under eCS the find command
 *      could not longer be used to filter certain symbols from the source
 *      library export list. When calling find, due to unknonw reasons the
 *      double quotes were always swallowed, so find.exe reported a syntax
 *      error. This script will alredy filter out the internal symbols 
 *      (starting with an underscore in the sambol name), so that the find
 *      command is not longer required.
 */
/* $Id: xemxexp.cmd,v 1.4 2008-10-15 16:43:17 cla Exp $ */
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

 '@ECHO OFF';
 rc = 0;
 SymbolFilter = '"_';

 DO 1

    /* set new queue */
    QueueName = RXQUEUE('CREATE');
    rc        = RXQUEUE('SET', QueueName);


    /* execute command */
    PARSE ARG Parms;
    'emxexp' Parms '| rxqueue' QueueName;
    IF (rc \= 0) THEN
       LEAVE;

    /* display only symbols that are not internal */
    DO WHILE (QUEUED() > 0)
       PARSE PULL Line;
       IF (POS( SymbolFilter, Line) = 0) THEN
          SAY Line;
    END;

    /* reset to default queue */
    rcx = RXQUEUE('DELETE', QueueName);
    rcx = RXQUEUE('SET', 'SESSION');

    /* done */
    rc = 0;
 END;

 RETURN( rc);

