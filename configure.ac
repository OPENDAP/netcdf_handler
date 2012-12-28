# -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.63)
AC_INIT(netcdf_handler, 3.10.2, opendap-tech@opendap.org)
AC_CONFIG_AUX_DIR([conf])
AC_CONFIG_MACRO_DIR([conf])

AM_INIT_AUTOMAKE(1.10)

# The handler is a shared object library, so we don't normally need
# the static lib. Override this default by specifying `--enable-static',
# which can make running unit tests with valgrind far more pleasant. 
AM_DISABLE_STATIC

LT_INIT
AC_CONFIG_TESTDIR(bes-testsuite, [.])

PACKAGE_MAJOR_VERSION=`echo $PACKAGE_VERSION | sed 's@^\([[0-9]]\)*\.\([[0-9]]*\)\.\([[0-9]]*\)$@\1@'`
PACKAGE_MINOR_VERSION=`echo $PACKAGE_VERSION | sed 's@^\([[0-9]]\)*\.\([[0-9]]*\)\.\([[0-9]]*\)$@\2@'`
PACKAGE_SUBMINOR_VERSION=`echo $PACKAGE_VERSION | sed 's@^\([[0-9]]\)*\.\([[0-9]]*\)\.\([[0-9]]*\)$@\3@'`
AC_MSG_NOTICE(Package Major Version:     $PACKAGE_MAJOR_VERSION)
AC_MSG_NOTICE(Package Minor Version:     $PACKAGE_MINOR_VERSION)
AC_MSG_NOTICE(Package SubMinor Version:  $PACKAGE_SUBMINOR_VERSION)
AC_SUBST(PACKAGE_MAJOR_VERSION)
AC_SUBST(PACKAGE_MINOR_VERSION)
AC_SUBST(PACKAGE_SUBMINOR_VERSION)

AC_CONFIG_SRCDIR([NCStr.h])
AC_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AM_PROG_LIBTOOL

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
AC_CHECK_LIBDAP([3.11.0],
 [
  LIBS="$LIBS $DAP_SERVER_LIBS"
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

AC_MSG_CHECKING([Checking the netCDF library version number])

AC_EGREP_HEADER(nc_inq_user_type, netcdf.h, 
        NETCDF_MAJOR_VERSION=4, NETCDF_MAJOR_VERSION=3)
AC_DEFINE_UNQUOTED(NETCDF_VERSION, $NETCDF_MAJOR_VERSION, [What version of netcdf were we built with?])

AC_MSG_RESULT($NETCDF_MAJOR_VERSION)

AM_CONDITIONAL([NETCDF4_TESTS], [test "$NETCDF_MAJOR_VERSION" = "4"])

AC_CHECK_BES([3.9.0],
[
],
[ AC_MSG_ERROR([Cannot find bes])
])

dnl The AC_CHECK_LIBDAP sets LIBS correctly
echo "LIBS is $LIBS"
AC_LANG_PUSH([C++])
AC_CHECK_LIB(dap, function_ugrid_restrict, gf_found="yes", gf_found="no")
AC_LANG_POP

AC_LANG_PUSH([C++])
SAVED_LDFLAGS=$LDFLAGS
LDFLAGS="$LDFLAGS -ldap"

AC_LINK_IFELSE(
  [AC_LANG_PROGRAM([], [libdap::function_ugrid_restrict(0, 0, 0)])],
  [HAVE_UGRID="yes"],
  [HAVE_UGRID="no"])
  
LDFLAGS=$SAVED_LDFLAGS
AC_LANG_POP

echo "HAVE_UGRID is $HAVE_UGRID"

OPENDAP_DEBUG_OPTION

AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([bes-testsuite/Makefile bes-testsuite/atlocal bes-testsuite/package.m4])
AC_CONFIG_FILES([bes-testsuite/generate_metadata_baseline.sh], [chmod +x bes-testsuite/generate_metadata_baseline.sh])
AC_CONFIG_FILES([bes-testsuite/generate_data_baseline.sh], [chmod +x bes-testsuite/generate_data_baseline.sh])

dnl AC_CONFIG_FILES([ugrid_project/bes.conf])

AC_OUTPUT
