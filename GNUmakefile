
# top level GNU makefile for WPS Toolkit library
#
# Copyright (c) WPS Toolkit Project - Christian Langanke 2000
#
# $Id: GNUmakefile,v 1.32 2006-12-04 21:45:07 cla Exp $
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

# --- sample module list
#     module names are the subdirectory below samples directory
VIO_SAMPLES= \
crc32 dynamic eas file ioctl mmf nls process regexp sys time tmf xbin2obj \
faccess\cfgsys faccess\init faccess\syslvl setting\vio
PM_SAMPLES=ctl pm
WPS_SAMPLES=setting\wps

# --- source module lists
#     module names are the subdirectory below source directory
LIB_SOURCES=lib
DOC_SOURCES=book


# include configuration
CFG_MAIN=1
BASEDIR=.
PRJINC=$(BASEDIR)/config/GNUproject.in
include $(BASEDIR)/config/GNUrules.in

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

# --- list all phony targets

.PHONY: MODULE help targets symbols rel all lib doc view samples vio wps run docs libs sam clean

# --- default targets

# - generic default target for building a module

ifdef SOURCE
MODULE:
	@$(MAKE) -C source\$(SOURCE) $(ARG)
	@cmd /C echo.
endif

ifdef SAMPLE
MODULE:
	@$(MAKE) -C samples\$(SAMPLE) $(ARG)
	@cmd /C echo.
endif

# --- these targets to give help
help:
	@type $(CFG_HELPMAIN)

targets:
	@type $(CFG_HELPTARGETS)

symbols:
	@type $(CFG_HELPSYMBOL1)
	@type $(CFG_HELPSYMBOL2)

# --- compile release version
rel:
	@$(MAKE) all NDEBUG=1

# --- compile the world
all: lib doc samples

# --- compile developer version, no samples
dev: lib doc

# --- compile sources
lib:
	@for %a in ($(LIB_SOURCES)) do @($(MAKE) $(ARG) SOURCE=%a ARG=all)

doc:
	@for %a in ($(DOC_SOURCES)) do @($(MAKE) $(ARG) SOURCE=%a ARG=all)

view:
	@for %a in ($(DOC_SOURCES)) do @($(MAKE) $(ARG) SOURCE=%a ARG=view)


# --- compile samples
samples: lib vio pm wps

vio: lib
	@for %a in ($(VIO_SAMPLES)) do @($(MAKE) $(ARG) SAMPLE=%a ARG=all)

pm: lib
	@for %a in ($(PM_SAMPLES)) do @($(MAKE) $(ARG) SAMPLE=%a ARG=all)

wps: lib
	@for %a in ($(WPS_SAMPLES)) do @($(MAKE) $(ARG) SAMPLE=%a ARG=all)

# --- run all samples
run:
	@for %a in ($(VIO_SAMPLES)) do @($(MAKE) $(ARG) SAMPLE=%a ARG=run)


# --- some redefinition of targets
docs: doc

libs: lib

sam: samples


# --- this target to cleanup
clean:
	@echo About to cleanup the target directories ...
	-@for %a in ($(DIRSTOCLEAN)) do @(echo - %a & del %a\* /N $(TO_NUL) & rd %a $(TO_NUL))
	@echo Done.

