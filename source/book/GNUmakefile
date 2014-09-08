
# GNUmakefile for the WPS Toolkit documentation
#
# Copyright (c) WPS Toolkit Project - Christian Langanke 2000
#
# $Id: GNUmakefile,v 1.15 2006-08-13 14:41:00 cla Exp $
#
# ===========================================================================
#
# This file is part of the WPS Toolkit package and is free software.  You can
# redistribute it and/or modify it under the terms of the GNU Library General
# Public License as published by the Free Software Foundation, in version 2
# as it comes in the "COPYING.LIB" file of the WPS Toolkit main distribution.
# This library is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
# License for more details.


# include configuration (compiler options),
# also defines $(BINDIR), $(INCDIR) etc.
BASEDIR=../..
PRJINC=$(BASEDIR)/config/GNUproject.in
include $(BASEDIR)/config/GNUrules.in

DESCRIPTION=WPS Toolkit documentation makefile
$(shell cmd /c @echo $(STARTMSG)) 

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

# define what to build
TARGET= wtkref.inf
CLEANUP=$(BOOKDIR_BS)\$(TARGET)       \
	$(CMPDIR_BS)\functions.ipf    \
	$(CMPDIR_BS)\datatypes.ipf    \
	$(CMPDIR_BS)\epmkwds.c__      \
	$(CMPDIR_BS)\wpstk.ndx	\
	$(BOOKDIR_BS)\EPM\epmkwds.c__ \
	$(BOOKDIR_BS)\EPM\wpstk.ndx


# pseudotargets
all:  $(TARGET)
	@echo $(ENDMSG)


clean:
	@echo About to cleanup the target files ...
	-@for %a in ($(CLEANUP)) do @(echo - %a & del %a /N $(TO_NUL))
	@echo Done.
	@echo $(ENDMSG)


view: $(TARGET)
	start view $(BOOKDIR_BS)\$(TARGET) $(PANEL)
	@echo $(ENDMSG)

show: view

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

# build the book

wtkref.inf:  wtkref.txt functions.ipf $(wildcard *.inc)
	htext.cmd /N wtkref.txt $(CMPDIR)/wtkref.ipf $(BOOKDIR)/wtkref.inf

# build generated source

functions.ipf: $(INCDIR)/wtk.h $(wildcard $(SRCDIR)\*.c) $(wildcard $(INCDIR)\*.h) $(EXECDIR)\srcscan.cmd $(wildcard datatypes\*) GNUmakefile
	srcscan.cmd $(INCDIR)/wtk.h $(LIBSRCDIR) $(CMPDIR) $(INCDIR) warp4def.h /V /L
	@copy $(CMPDIR_BS)\*.ndx $(NDXDIR_BS) $(TO_NUL)
	@copy $(CMPDIR_BS)\*.c__ $(NDXDIR_BS) $(TO_NUL)

