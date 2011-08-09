# -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.63)
AC_INIT(netcdf_handler, 3.9.3, opendap-tech@opendap.org)
AC_CONFIG_AUX_DIR([conf])
AC_CONFIG_MACRO_DIR([conf])

AM_INIT_AUTOMAKE(1.10)
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

AC_MSG_NOTICE([Checking the netCDF library version number])

# Look for nc-config and ncgen; if nc-config exists, use it to get the netcdf
# version. Otherwise, if ncgen exists, use it to get the netcdf version.
AC_CHECK_PROG(nc_config_found, nc-config, "yes", "no")
AC_CHECK_PROG(ncgen_found, ncgen, "yes", "no")

if test "$nc_config_found" = "yes"
then
	NETCDF_VERSION=`nc-config --version | sed 's@.* \([[0-9]]*\.[[0-9]]*\.[[0-9]]*\)$@\1@'`
elif test "$ncgen_found" = "yes"
then
	ncgen_info=`ncgen -? 2>&1 | grep 'netcdf library version'`
	NETCDF_VERSION=`echo $ncgen_info | sed 's@.*"\([[0-9]]*\.[[0-9]]*\.[[0-9]]*\)".*$@\1@'`
else
	AC_MSG_WARN([Could not find nc-config nor ncgen.])
	NETCDF_VERSION=3.0.0
fi

NETCDF_MAJOR_VERSION=`echo $NETCDF_VERSION | sed 's@^\([[0-9]]\)*\.\([[0-9]]*\)\.\([[0-9]]*\)$@\1@'`
NETCDF_MINOR_VERSION=`echo $NETCDF_VERSION | sed 's@^\([[0-9]]\)*\.\([[0-9]]*\)\.\([[0-9]]*\)$@\2@'`
NETCDF_SUBMINOR_VERSION=`echo $NETCDF_VERSION | sed 's@^\([[0-9]]\)*\.\([[0-9]]*\)\.\([[0-9]]*\)$@\3@'`
#AC_MSG_NOTICE(NETCDF Major Version:     $NETCDF_MAJOR_VERSION)
#AC_MSG_NOTICE(NETCDF Minor Version:     $NETCDF_MINOR_VERSION)
#AC_MSG_NOTICE(NETCDF SubMinor Version:  $NETCDF_SUBMINOR_VERSION)
# AC_SUBST(NETCDF_MAJOR_VERSION)
# AC_SUBST(NETCDF_MINOR_VERSION)
# AC_SUBST(NETCDF_SUBMINOR_VERSION)

AC_MSG_NOTICE(Setting netCDF library version number to $NETCDF_MAJOR_VERSION)
AC_DEFINE_UNQUOTED(NETCDF_VERSION, $NETCDF_MAJOR_VERSION, [What version of netcdf were we built with?])

AC_CHECK_BES([3.9.0],
[
],
[ AC_MSG_ERROR([Cannot find bes])
])

OPENDAP_DEBUG_OPTION

AC_CONFIG_FILES([Makefile bes-testsuite/Makefile bes-testsuite/atlocal data/nc4_test_files/Makefile])
AC_OUTPUT
