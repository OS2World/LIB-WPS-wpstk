/*
 *      README.CMD - WPS Toolkit - Christian Langanke 2005-2008
 */
/* $Id: readme.cmd,v 1.6 2008-10-15 16:43:17 cla Exp $ */
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

 PARSE SOURCE . . CallName;
 SlashPos = LASTPOS( '\', CallName);
 InfName = LEFT( CallName, SlashPos)'book\wtkref.inf';

 PARSE ARG Section;
 IF (STRIP(Section) = '') THEN
    Section = 'How to use'

 '@START VIEW' InfName '"'Section'"';
