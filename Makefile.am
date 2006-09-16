
# Automake file for netcdf-handler
#
# 5/17/2005 jhrg
#
# $Id$

AUTOMAKE_OPTIONS = foreign check-news

ACLOCAL_AMFLAGS = -I conf

bin_PROGRAMS = dap_nc_handler

lib_LTLIBRARIES = libnc_handler.la
if OPENDAPSERVER
    lib_besdir=$(libdir)/bes
    lib_bes_LTLIBRARIES = libnc_module.la
endif

libnc_handler_la_SOURCES = $(NCTYPE_SRC) $(NCTYPE_HDR)

libnc_module_la_SOURCES = $(NCTYPE_SRC) $(NCTYPE_HDR) $(SERVER_SRC) $(SERVER_HDR)
libnc_module_la_CPPFLAGS = $(BES_CPPFLAGS)
libnc_module_la_LDFLAGS = -avoid-version -module 
libnc_module_la_LIBADD = $(BES_LIBS)

dap_nc_handler_SOURCES = nc_handler.cc
dap_nc_handler_LDADD = libnc_handler.la

NCTYPE_SRC = ncdas.cc ncdds.cc \
	NCArray.cc NCByte.cc NCFloat64.cc NCGrid.cc NCUInt32.cc	\
	NCInt32.cc NCSequence.cc NCStr.cc NCStructure.cc NCUrl.cc	\
	NCUInt16.cc NCInt16.cc NCFloat32.cc NCTypeFactory.cc

NCTYPE_HDR = NCFloat64.h NCRequestHandler.h NCTypeFactory.h		\
    NCArray.h NCGrid.h NCSequence.h NCUInt16.h NCByte.h NCInt16.h	\
    NCStr.h NCUInt32.h NCFloat32.h NCInt32.h NCStructure.h NCUrl.h	\
    config_nc.h

SERVER_SRC = NCRequestHandler.cc NCModule.cc

SERVER_HDR = NCRequestHandler.h NCModule.h

EXTRA_DIST = COPYRIGHT COPYING netcdf_handler.spec

CLEANFILES = *~

###########################################################################

# Build linux RPMs

srpm:
	rpmbuild -ts netcdf_handler-@PACKAGE_VERSION@.tar.gz

rpm:
	rpmbuild -tb netcdf_handler-@PACKAGE_VERSION@.tar.gz

