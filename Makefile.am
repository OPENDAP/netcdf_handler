
# Automake file for netcdf-handler
#
# 5/17/2005 jhrg
#
# $Id$

AUTOMAKE_OPTIONS = foreign check-news

sbin_PROGRAMS = dap_nc_handler

NCTYPE_SRC = NCArray.cc NCByte.cc NCFloat64.cc NCGrid.cc NCUInt32.cc	\
	NCInt32.cc NCSequence.cc NCStr.cc NCStructure.cc NCUrl.cc	\
	NCUInt16.cc NCInt16.cc NCFloat32.cc NCTypeFactory.cc

NCTYPE_HDR = NCFloat64.h NCRequestHandler.h NCTypeFactory.h		\
    NCArray.h NCGrid.h NCSequence.h NCUInt16.h NCByte.h NCInt16.h	\
    NCStr.h NCUInt32.h NCFloat32.h NCInt32.h NCStructure.h NCUrl.h

HANDLER_SRC = ncdas.cc ncdds.cc nc_handler.cc 

dap_nc_handler_SOURCES = $(NCTYPE_SRC) $(HANDLER_SRC) $(NCTYPE_HDR)

EXTRA_DIST = SECURITY COPYRIGHT

DISTCLEANFILES = *~

