/*
 *      GCCFILT.CMD - WPS Toolkit - Christian Langanke 2000-2008
 *
 *      This program serves a a filter to GCC output and is especially
 *      useful, when you let it run within the make macros of EPM.
 *      This filter will change the error messages, so that they compiy
 *      to the error messages of the IBM C compilers. This way you
 *      can use EPM to examine all the errors also within your GCC output.
 */
/* $Id: gccfilt.cmd,v 1.6 2008-10-15 16:43:14 cla Exp $ */
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

 /* check all lines */
 DO WHILE (QUEUED() > 0)
    PARSE PULL ThisLine

    /* change all error lines so that EPM can handle them */
    PARSE VAR ThisLine file':'line':' Error;
    IF ((DATATYPE( line) = 'NUM') & (Error \= '')) THEN
    DO
       ThisLine = file'('line')' error;
       SAY ThisLine
    END;
 END;

