# -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([netcdf_handler], [3.5.0], [dods-tech@unidata.ucar.edu])
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
AC_CHECK_LIBDAP([3.5.3],
 [
  LIBS="$LIBS $DAP_LIBS"
  CPPFLAGS="$CPPFLAGS $DAP_CFLAGS"
 ],
 [ AC_MSG_ERROR([Cannot find libdap])
])
  
AC_CHECK_NETCDF(
   [
       LIBS="$LIBS $NC_LIBS"
       LDFLAGS="$LDFLAGS $NC_LDFLAGS"
       CPPFLAGS="$CPPFLAGS $NC_CPPFLAGS"
   ],
   [
       echo "*** Use --with-netcdf for the root netcdf directory."
       echo "*** Otherwise use --with-netcdf-include switch for includes directory"
       echo "*** and --with-netcdf-libdir switch for libraries directory."
       AC_MSG_ERROR([netcdf library and netcdf headers are required.])
   ],[3]
)

dnl if test "$NC_PATH"
dnl then
dnl     AC_MSG_RESULT([$NC_PATH])
dnl     LDFLAGS="$LDFLAGS -L${NC_PATH}/lib"
dnl     LIBS="$LIBS -lnetcdf"
dnl     CPPFLAGS="$CPPFLAGS -I${NC_PATH}/include"
dnl fi

AC_ARG_WITH([bes],
            [AS_HELP_STRING([--with-bes=PATH],[bes install root directory])],
            [BES_PATH=${withval}], 
            [BES_PATH=""])

dnl Simple test for BES. A better test would look something like the libdap
dnl macro written by Patrice. This will get us going... If --with-bes is not
dnl given, then don't build the bes code. jhrg 10/3/05
if test -x "${BES_PATH}/bin/bes-config"
then
    AM_CONDITIONAL([OPENDAPSERVER], [true])
    BES_CPPFLAGS="`${BES_PATH}/bin/bes-config --cflags`"
    AC_SUBST([BES_CPPFLAGS])
    BES_LDFLAGS="`${BES_PATH}/bin/bes-config --libs`"
    AC_SUBST([BES_LDFLAGS])
    BES_MODULE_DIR="`${BES_PATH}/bin/bes-config --prefix`/lib/bes"
    AC_SUBST([BES_MODULE_DIR])
else
    AM_CONDITIONAL([OPENDAPSERVER], [false])
fi

OPENDAP_DEBUG_OPTION

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
