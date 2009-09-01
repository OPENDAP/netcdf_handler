
# Automake file for netcdf-handler
#
# 5/17/2005 jhrg
#
# $Id$

AUTOMAKE_OPTIONS = foreign check-news

ACLOCAL_AMFLAGS = -I conf

SUBDIRS = bes-testsuite

lib_besdir=$(libdir)/bes
lib_bes_LTLIBRARIES = libnc_module.la
dist_bin_SCRIPTS = bes-nc-data.sh

libnc_module_la_SOURCES = $(NCTYPE_SRC) $(NCTYPE_HDR) $(SERVER_SRC) $(SERVER_HDR)
libnc_module_la_CPPFLAGS = $(BES_CPPFLAGS)
libnc_module_la_LDFLAGS = -avoid-version -module 
libnc_module_la_LIBADD = $(BES_DAP_LIBS)

NCTYPE_SRC = ncdas.cc ncdds.cc \
	NCArray.cc NCByte.cc NCFloat64.cc NCGrid.cc NCUInt32.cc		\
	NCInt32.cc NCSequence.cc NCStr.cc NCStructure.cc NCUrl.cc	\
	NCUInt16.cc NCInt16.cc NCFloat32.cc

NCTYPE_HDR = NCFloat64.h NCArray.h NCGrid.h NCSequence.h NCUInt16.h 	\
	NCByte.h NCInt16.h NCStr.h NCUInt32.h NCFloat32.h NCInt32.h	\
	NCStructure.h NCUrl.h config_nc.h

SERVER_SRC = NCRequestHandler.cc NCModule.cc

SERVER_HDR = NCRequestHandler.h NCModule.h

EXTRA_DIST = data COPYRIGHT COPYING netcdf_handler.spec \
    bes-nc-data.sh.in OSX_Resources

CLEANFILES = *~ bes-nc-data.sh

sample_datadir = $(datadir)/hyrax/data/nc
sample_data_DATA = data/bears.nc data/coads_climatology.nc data/fnoc1.nc data/fnoc1.das data/fnoc1.nc.html

do_subst = sed -e 's,[@]sysconfdir[@],$(sysconfdir),g' \
		   	   -e 's,[@]libdir[@],$(libdir),g' \
		   	   -e 's,[@]datadir[@],$(datadir),g'

bes-nc-data.sh: bes-nc-data.sh.in config.status
	$(do_subst) < $(srcdir)/bes-nc-data.sh.in > bes-nc-data.sh
	chmod +x bes-nc-data.sh

# This makes sure that the handler modifies the bes which configure found
# even when distcheck is run.
bes-conf: bes-nc-data.sh
	(bes_prefix=`bes-config --prefix` ; \
	if test "$$bes_prefix" = "/usr" ; \
	 then \
	    ./bes-nc-data.sh /etc/bes/bes.conf /usr/lib/bes ;\
	else \
	    ./bes-nc-data.sh $$bes_prefix/etc/bes/bes.conf $$bes_prefix/lib/bes ; \
	fi)

###########################################################################

# Fortify targets.

.PHONY: fortify
fortify:
	sourceanalyzer -b @PACKAGE@ $(MAKE)
	sourceanalyzer -b @PACKAGE@ -scan -f @PACKAGE@-@PACKAGE_VERSION@.fpr

# Use this to clean the fortify project.
.PHONY: fortifyclean
fortifyclean:
	sourceanalyzer -b @PACKAGE@ -clean

###########################################################################
#
# Build linux RPMs

srpm: dist
	rpmbuild -ts netcdf_handler-@PACKAGE_VERSION@.tar.gz

rpm: dist
	rpmbuild -tb netcdf_handler-@PACKAGE_VERSION@.tar.gz

###########################################################################

# Build OS/X Packages.

PACKAGEMAKER=/Developer/Applications/Utilities/PackageMaker.app/Contents/MacOS/PackageMaker*

UNIV_ARCH=-arch ppc -arch i386
MAC_SYSROOT=Developer/SDKs/MacOSX10.4u.sdk

PKG_CONF_FLAGS=

clean-pkg:
	-rm -rf mac_osx @PACKAGE@-@PACKAGE_VERSION@.pkg @PACKAGE@-@PACKAGE_VERSION@

pkg-build:
	./configure --prefix=/usr --disable-dependency-tracking $(PKG_CONF_FLAGS)
	make clean all
	DESTDIR=${PWD}/mac_osx make install

pkg-main:
	./OSX_Resources/update_mac_package_contents.pl README "configure.ac" "mac_osx/usr"
	${PACKAGEMAKER} -build -p @PACKAGE@-@PACKAGE_VERSION@.pkg \
		-f mac_osx/usr \
		-ds -r OSX_Resources \
		-i OSX_Resources/Info.plist \
		-d OSX_Resources/Description.plist
	-rm -rf mac_osx

pkg-dmg:
	-rm -rf @PACKAGE@-@PACKAGE_VERSION@
	-rm -rf @PACKAGE@-@PACKAGE_VERSION@.dmg
	mkdir @PACKAGE@-@PACKAGE_VERSION@
	mv @PACKAGE@-@PACKAGE_VERSION@.pkg @PACKAGE@-@PACKAGE_VERSION@
	cp README NEWS @PACKAGE@-@PACKAGE_VERSION@
	dropdmg -i --sanitize-for-servers --format zlib @PACKAGE@-@PACKAGE_VERSION@
	-rm -rf @PACKAGE@-@PACKAGE_VERSION@

pkg: clean-pkg pkg-build pkg-main pkg-dmg

