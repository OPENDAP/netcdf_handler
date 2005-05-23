
# Automake file for netcdf-handler
#
# 5/17/2005 jhrg
#
# $Id: Makefile.am,v 1.1 2005/05/23 22:51:47 jimg Exp $

AUTOMAKE_OPTIONS = foreign check-news

sbin_PROGRAMS = nc_dap

NCTYPE_SRC = NCArray.cc NCByte.cc NCFloat64.cc NCGrid.cc NCUInt32.cc \
	NCInt32.cc NCSequence.cc NCStr.cc NCStructure.cc NCUrl.cc \
	NCUInt16.cc NCInt16.cc NCFloat32.cc NCTypeFactory.cc

NCTYPE_HDR = NCFloat64.h NCRequestHandler.h NCTypeFactory.h \
    NCArray.h NCGrid.h NCSequence.h NCUInt16.h NCByte.h NCInt16.h \
    NCStr.h NCUInt32.h NCFloat32.h NCInt32.h NCStructure.h NCUrl.h

NCTYPE_OBJ = NCArray.o NCByte.o NCFloat64.o NCGrid.o NCInt32.o \
	NCSequence.o NCStr.o NCStructure.o NCUrl.o NCUInt32.o \
	NCUInt16.o NCInt16.o NCFloat32.o NCTypeFactory.o

HANDLER_SRC = ncdas.cc ncdds.cc nc_handler.cc 
HANDLER_OBJ = ncdas.o ncdds.o nc_handler.o 

nc_dap_SOURCES = $(NCTYPE_SRC) $(HANDLER_SRC) $(NCTYPE_HDR)

nc_dap_DEPENDENCIES = $(NCTYPE_OBJ) $(HANDLER_OBJ)

EXTRA_DIST = SECURITY

DISTCLEANFILES = *~
