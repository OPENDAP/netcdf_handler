
# Setup directories for intermediate files and built components
OBJDIR		= .\objects
INSTALLLIBDIR	= ..\..\..\lib
INSTALLEXEDIR	= ..\..\..\bin
INCLUDEDIR	= ..\..\..\include

# Setup basic compiler flags for all .obj's
CC		= cl
CFLAGS		= /nologo /MT /W0 /GR /GX /c
#CFLAGS		= /nologo /MTd /Zi /W3 /GR /GX /c	# flags for debugger

# Setup linker flags for libraries and .exe's
LINK		= link
LIBPATH		= /libpath:$(INSTALLLIBDIR)
#LEXEFLAGS	= /DEBUG /subsystem:console $(LIBPATH)	# flags for debugger
#LLIBFLAGS	= -lib					# flags for debugger
LEXEFLAGS	= /nologo /subsystem:console $(LIBPATH)
LLIBFLAGS	= -lib /nologo

DODSLIBS	= libdap++.lib pthreadVCE.lib libxml2.lib libcurl.lib librx.lib libz.lib libxdr.lib
SYSLIBS		= ws2_32.lib advapi32.lib
LINKLIBS	= $(DODSLIBS) $(SYSLIBS)

# Setup compiler includes and defines for libraries and .exe's
INCS		= /I . /I ..\lnetcdf /I ..\fortran /I .. /I ..\GNU \
		  /I $(INCLUDEDIR) /I $(INCLUDEDIR)\pthreads
DEFSLIB		= /D WIN32 /D LOCAL /D _LIB /D _WINDOWS
DEFSEXE		= /D WIN32 /D LOCAL /D _CONSOLE
CPPLIBFLAGS	= $(DEFSLIB) $(INCS)
CPPEXEFLAGS	= $(DEFSEXE) $(INCS)

# Combine compiler flags
LIBOBJFLAGS	= $(CFLAGS) $(CPPLIBFLAGS)
EXEOBJFLAGS	= $(CFLAGS) $(CPPEXEFLAGS)

# Define the targets
LIBRARY		= $(OBJDIR)\libnc-dods.lib
EXECUTABLE	= $(OBJDIR)\dncdump.exe

TARGETS		= $(LIBRARY) $(EXECUTABLE)

# Clear built-in rules and set new ones
.SUFFIXES :
.SUFFIXES :	.obj .c .cc

# Rules for library objects
{..\lnetcdf}.c{$(OBJDIR)}.obj:
	@$(CC) $(LIBOBJFLAGS) /Fo$@ /Tc $<

{..\fortran}.c{$(OBJDIR)}.obj:
	@$(CC) /D PowerStationFortran $(LIBOBJFLAGS) /Fo$@ /Tc $<

{..\ncdump}.cc{$(OBJDIR)}.obj:
	@$(CC) $(LIBOBJFLAGS) /Fo$@ /Tp $<

{..}.cc{$(OBJDIR)}.obj:
	@$(CC) $(LIBOBJFLAGS) /Fo$@ /Tp $<

NCOBJ = $(OBJDIR)\NCArray.obj \
	$(OBJDIR)\NCByte.obj \
	$(OBJDIR)\NCFloat64.obj \
	$(OBJDIR)\NCGrid.obj \
	$(OBJDIR)\NCInt32.obj \
	$(OBJDIR)\NCList.obj \
	$(OBJDIR)\NCSequence.obj \
	$(OBJDIR)\NCStr.obj \
	$(OBJDIR)\NCStructure.obj \
	$(OBJDIR)\NCUrl.obj \
	$(OBJDIR)\NCUInt32.obj \
	$(OBJDIR)\NCUInt16.obj \
	$(OBJDIR)\NCInt16.obj \
	$(OBJDIR)\NCFloat32.obj

NETOBJ = $(OBJDIR)\NCConnect.obj \
	$(OBJDIR)\inst.obj \
	$(NCOBJ)

LIBOBJS = $(NETOBJ) \
	$(OBJDIR)\Ddim.obj \
	$(OBJDIR)\Dattr.obj \
	$(OBJDIR)\Dvar.obj \
	$(OBJDIR)\Dnc.obj \
	$(OBJDIR)\Dputget.obj \
	$(OBJDIR)\Dncx.obj

NCOBJS = $(OBJDIR)\lattr.obj \
	$(OBJDIR)\ldim.obj \
	$(OBJDIR)\lerror.obj \
	$(OBJDIR)\llibvers.obj \
	$(OBJDIR)\lnc.obj \
	$(OBJDIR)\lncio.obj \
	$(OBJDIR)\lncx.obj \
	$(OBJDIR)\lputget.obj \
	$(OBJDIR)\lstring.obj \
	$(OBJDIR)\lv1hpg.obj \
	$(OBJDIR)\lv2i.obj \
	$(OBJDIR)\lvar.obj

FOBJS = $(OBJDIR)\fort-attio.obj \
	$(OBJDIR)\fort-genvar.obj \
	$(OBJDIR)\fort-varaio.obj \
	$(OBJDIR)\fort-control.obj \
	$(OBJDIR)\fort-lib.obj \
	$(OBJDIR)\fort-vario.obj \
	$(OBJDIR)\fort-dim.obj \
	$(OBJDIR)\fort-misc.obj \
	$(OBJDIR)\fort-varmio.obj \
	$(OBJDIR)\fort-genatt.obj \
	$(OBJDIR)\fort-v2compat.obj \
	$(OBJDIR)\fort-varsio.obj \
	$(OBJDIR)\fort-geninq.obj \
	$(OBJDIR)\fort-var1io.obj

EXEOBJS = $(OBJDIR)\ncdump.obj \
	$(OBJDIR)\vardata.obj \
	$(OBJDIR)\dumplib.obj

all: setup $(TARGETS)

$(LIBRARY): $(LIBOBJS) $(NCOBJS) $(FOBJS)
	$(LINK) $(LLIBFLAGS) /out:$@ $(LIBOBJS) $(NCOBJS) $(FOBJS)

$(EXECUTABLE): $(EXEOBJS)
	$(LINK) $(LEXEFLAGS) /out:$(EXECUTABLE) $(EXEOBJS) $(LINKLIBS) $(LIBRARY)

install:
	copy $(LIBRARY) $(INSTALLLIBDIR)
	copy $(EXECUTABLE) $(INSTALLEXEDIR)
	copy ..\lnetcdf\netcdf.h $(INCLUDEDIR)
	copy ..\fortran\netcdf.inc $(INCLUDEDIR)

# Dods Netcdf Objects
$(OBJDIR)\NCArray.obj		: ..\NCArray.cc
$(OBJDIR)\NCByte.obj		: ..\NCByte.cc
$(OBJDIR)\NCFloat64.obj		: ..\NCFloat64.cc
$(OBJDIR)\NCGrid.obj		: ..\NCGrid.cc
$(OBJDIR)\NCInt32.obj		: ..\NCInt32.cc
$(OBJDIR)\NCList.obj		: ..\NCList.cc
$(OBJDIR)\NCSequence.obj	: ..\NCSequence.cc
$(OBJDIR)\NCStr.obj		: ..\NCStr.cc
$(OBJDIR)\NCStructure.obj	: ..\NCStructure.cc
$(OBJDIR)\NCUrl.obj		: ..\NCUrl.cc
$(OBJDIR)\NCUInt32.obj		: ..\NCUInt32.cc
$(OBJDIR)\NCUInt16.obj		: ..\NCUInt16.cc
$(OBJDIR)\NCInt16.obj		: ..\NCInt16.cc
$(OBJDIR)\NCFloat32.obj		: ..\NCFloat32.cc
$(OBJDIR)\NCConnect.obj		: ..\NCConnect.cc
$(OBJDIR)\inst.obj		: ..\inst.cc
$(OBJDIR)\Ddim.obj		: ..\Ddim.cc
$(OBJDIR)\Dattr.obj		: ..\Dattr.cc
$(OBJDIR)\Dvar.obj		: ..\Dvar.cc
$(OBJDIR)\Dnc.obj		: ..\Dnc.cc
$(OBJDIR)\Dputget.obj		: ..\Dputget.cc
$(OBJDIR)\Dncx.obj		: ..\Dncx.cc

# NetCDF code
$(OBJDIR)\lattr.obj		: ..\lnetcdf\lattr.c
$(OBJDIR)\ldim.obj		: ..\lnetcdf\ldim.c
$(OBJDIR)\lerror.obj		: ..\lnetcdf\lerror.c
$(OBJDIR)\llibvers.obj		: ..\lnetcdf\llibvers.c
$(OBJDIR)\lnc.obj		: ..\lnetcdf\lnc.c
$(OBJDIR)\lncio.obj		: ..\lnetcdf\lncio.c
$(OBJDIR)\lncx.obj		: ..\lnetcdf\lncx.c
$(OBJDIR)\lputget.obj		: ..\lnetcdf\lputget.c
$(OBJDIR)\lstring.obj		: ..\lnetcdf\lstring.c
$(OBJDIR)\lv1hpg.obj		: ..\lnetcdf\lv1hpg.c
$(OBJDIR)\lv2i.obj		: ..\lnetcdf\lv2i.c
$(OBJDIR)\lvar.obj		: ..\lnetcdf\lvar.c

# Fortran wrappers
$(OBJDIR)\fort-attio.obj	: ..\fortran\fort-attio.c
$(OBJDIR)\fort-genvar.obj	: ..\fortran\fort-genvar.c
$(OBJDIR)\fort-varaio.obj	: ..\fortran\fort-varaio.c
$(OBJDIR)\fort-control.obj	: ..\fortran\fort-control.c
$(OBJDIR)\fort-lib.obj		: ..\fortran\fort-lib.c
$(OBJDIR)\fort-vario.obj	: ..\fortran\fort-vario.c
$(OBJDIR)\fort-dim.obj		: ..\fortran\fort-dim.c
$(OBJDIR)\fort-misc.obj		: ..\fortran\fort-misc.c
$(OBJDIR)\fort-varmio.obj	: ..\fortran\fort-varmio.c
$(OBJDIR)\fort-genatt.obj	: ..\fortran\fort-genatt.c
$(OBJDIR)\fort-v2compat.obj	: ..\fortran\fort-v2compat.c
$(OBJDIR)\fort-varsio.obj	: ..\fortran\fort-varsio.c
$(OBJDIR)\fort-geninq.obj	: ..\fortran\fort-geninq.c
$(OBJDIR)\fort-var1io.obj	: ..\fortran\fort-var1io.c

# Objects for dncdump
$(OBJDIR)\ncdump.obj		: ..\ncdump\ncdump.cc
$(OBJDIR)\vardata.obj		: ..\ncdump\vardata.cc
$(OBJDIR)\dumplib.obj		: ..\ncdump\dumplib.cc

# Don't use the built-in "extended DOS" commands with all the
# fancy flags.  Win95/98 systems don't have those so we must
# keep it simple.  Sticking with no-flag versions of copy, mkdir,
# rmdir and del are a safe bet.
setup:
	@-mkdir $(OBJDIR)
	@-mkdir $(INSTALLEXEDIR)
	@-mkdir $(INSTALLLIBDIR)

clean:
	@-del $(OBJDIR)\*.obj
	@-del $(OBJDIR)\*.lib
	@-rmdir $(OBJDIR)

!INCLUDE "dependancies"



