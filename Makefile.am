
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
dist_bin_SCRIPTS = configure-nc-data.sh

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

EXTRA_DIST = data COPYRIGHT COPYING netcdf_handler.spec

CLEANFILES = *~

if OPENDAPSERVER

EXTRA_DIST += configure-nc-data.sh.in
CLEANFILES += configure-nc-data.sh

sample_datadir = $(datadir)/hyrax/data/nc
sample_data_DATA = data/coads_climatology.nc data/fnoc1.nc data/fnoc1.das

do_subst = sed -e 's,[@]sysconfdir[@],$(sysconfdir),g' \
		   	   -e 's,[@]libdir[@],$(libdir),g' \
		   	   -e 's,[@]datadir[@],$(datadir),g'

configure-nc-data.sh: configure-nc-data.sh.in Makefile
	$(do_subst) < $(srcdir)/configure-nc-data.sh.in > configure-nc-data.sh
	chmod +x configure-nc-data.sh

# This makes sure that the handler modifies the bes  which confiure found
# even with distcheck is run.
install-data-hook: configure-nc-data.sh
	(bes_prefix=`bes-config --prefix` ; \
	if test "$$bes_prefix" = "/usr" ; \
	 then \
	    ./configure-nc-data.sh /etc/bes/bes.conf /usr/lib/bes ;\
	else \
	    ./configure-nc-data.sh $$bes_prefix/etc/bes/bes.conf $$bes_prefix/lib/bes ; \
	fi)

endif

###########################################################################
#
# Build linux RPMs

srpm: dist
	rpmbuild -ts netcdf_handler-@PACKAGE_VERSION@.tar.gz

rpm: dist
	rpmbuild -tb netcdf_handler-@PACKAGE_VERSION@.tar.gz

###########################################################################
#
# Build OS/X Packages.

PACKAGEMAKER=/Developer/Applications/Utilities/PackageMaker.app/Contents/MacOS/PackageMaker*

pkg:
	rm -rf mac_osx @PACKAGE@-@PACKAGE_VERSION@.pkg
	./configure --prefix=/usr $(PKG_CONFIGURE_FLAGS)
	make configure-nc-data.sh
	cp configure-nc-data.sh configure-nc-data.sh.macosx
	./configure --prefix=`pwd`/mac_osx $(PKG_CONFIGURE_FLAGS)
	make clean install
	mv configure-nc-data.sh.macosx mac_osx/bin/configure-nc-data.sh
	${PACKAGEMAKER} -build -p @PACKAGE@-@PACKAGE_VERSION@.pkg -f mac_osx -ds -r OSX_Resources -i OSX_Resources/Info.plist -d OSX_Resources/Description.plist

