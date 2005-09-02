#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT([netcdf-handler], [3.5.0], [support@unidata.ucar.edu])
AC_CONFIG_AUX_DIR([conf])

AM_INIT_AUTOMAKE

AC_CONFIG_SRCDIR([NCArray.cc])
AM_CONFIG_HEADER([config_nc.h])

dnl flags for the compilers and linkers - set these before locating the
dnl actual tools since some of the AC_PROG macros set these `flag variables'
dnl to default values otherwise.

AC_CANONICAL_HOST
AC_SUBST(host)

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lib':
# AC_CHECK_LIB([ib], [main])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_TYPE_SIZE_T
AC_CHECK_TYPES([ptrdiff_t])

# Checks for library functions.
AC_CHECK_FUNCS([strchr])

dnl Checks for specific libraries
AC_CHECK_LIBDAP([3.5.2],
 [
  LIBS="$LIBS $DAP_LIBS"
  CPPFLAGS="$CPPFLAGS $DAP_CFLAGS"
 ],
 [ AC_MSG_ERROR([Cannot find libdap])
])

dnl Check for the netcdf library.
AC_ARG_WITH([netcdf],
            [  --with-netcdf=ARG          Where is the netcdf library (directory)],
            NC_PATH=${withval}, 
            NC_PATH="/usr/local/netcdf")

AC_MSG_CHECKING([for the netcdf library base directory])
if test ! -d "$NC_PATH" -o ! -f "${NC_PATH}/lib/libnetcdf.a"
then
    AC_MSG_ERROR([not found; use configure --with-netcdf=...])
fi
if test "$NC_PATH"
then
    AC_MSG_RESULT([$NC_PATH])
    LDFLAGS="$LDFLAGS -L${NC_PATH}/lib"
    LIBS="$LIBS -lnetcdf"
    CPPFLAGS="$CPPFLAGS -I${NC_PATH}/include"
fi

OPENDAP_DEBUG_OPTION

dnl autoheader macros; tack some text at the top and bottom of config_dap.h.in
AH_TOP([#ifndef _config_nc_h
#define _config_nc_h])

AH_BOTTOM([/* Shorthand for gcc's unused attribute feature */
#if defined(__GNUG__) || defined(__GNUC__)
#define not_used __attribute__ ((unused))
#else
#define not_used 
#endif /* __GNUG__ || __GNUC__ */

#endif /* _config_nc_h */])

dnl INCS is non-standard...
dnl AC_SUBST(INCS)

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
