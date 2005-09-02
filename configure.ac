# -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(FULL-PACKAGE-NAME, VERSION, BUG-REPORT-ADDRESS)
AC_CONFIG_AUX_DIR([conf])

AM_INIT_AUTOMAKE

AC_CONFIG_SRCDIR([NCStr.h])
AC_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

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

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
