
# Automake file for netcdf-handler
#
# 5/17/2005 jhrg
#
# $Id$

AUTOMAKE_OPTIONS = foreign check-news

ACLOCAL_AMFLAGS = -I conf

if OPENDAPSERVER
    bin_PROGRAMS = dap_nc_handler opendapserver
else
    bin_PROGRAMS = dap_nc_handler
endif

NCTYPE_SRC = NCArray.cc NCByte.cc NCFloat64.cc NCGrid.cc NCUInt32.cc	\
	NCInt32.cc NCSequence.cc NCStr.cc NCStructure.cc NCUrl.cc	\
	NCUInt16.cc NCInt16.cc NCFloat32.cc NCTypeFactory.cc

NCTYPE_HDR = NCFloat64.h NCRequestHandler.h NCTypeFactory.h		\
    NCArray.h NCGrid.h NCSequence.h NCUInt16.h NCByte.h NCInt16.h	\
    NCStr.h NCUInt32.h NCFloat32.h NCInt32.h NCStructure.h NCUrl.h

HANDLER_SRC = ncdas.cc ncdds.cc nc_handler.cc 

dap_nc_handler_SOURCES = $(NCTYPE_SRC) $(HANDLER_SRC) $(NCTYPE_HDR)

noinst_HEADERS = config_nc.h

SERVER_SRC = ncdas.cc ncdds.cc NCRequestHandler.cc nc_module.cc

SERVER_HDR = NCRequestHandler.h

MODULE_DIR = $(BES_MODULE_DIR)
MODULES = $(MODULE_DIR)/OPeNDAPServerHandler.o $(MODULE_DIR)/ServerApp.o \
	$(MODULE_DIR)/opendap_commands.o $(MODULE_DIR)/dods_module.o \
	$(MODULE_DIR)/test_authenticator.o

#SERVER_LIBS = -L../bes/command -lopendap_command -L../bes/dispatch -ldispatch -L../bes/ppt -lppt

opendapserver_CPPFLAGS = $(BES_CPPFLAGS)
opendapserver_SOURCES = $(NCTYPE_SRC) $(SERVER_SRC) $(NCTYPE_HDR) $(SERVER_HDR)
opendapserver_LDFLAGS = $(BES_LDFLAGS) 
opendapserver_LDADD = $(MODULES) 

# $(AUTH_MOD)  $(SERVER_LIBS)

EXTRA_PROGRAMS = opendapserver

EXTRA_DIST = COPYRIGHT

DISTCLEANFILES = *~

