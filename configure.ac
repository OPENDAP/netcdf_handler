# -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(dap_netcdf_handler, 3.5.0, dods-tech@unidata.ucar.edu)
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
AC_DEFUN([AC_CHECK_NETCDF],
[
AC_ARG_WITH([netcdf],
            [AS_HELP_STRING([--with-netcdf=ARG],[netcdf directory])],
            [NC_PATH=${withval}], 
            [NC_PATH="/usr/local/netcdf"])

AC_ARG_WITH([netcdf_inc],
            [AS_HELP_STRING([--with-netcdf-include=ARG],[netcdf include directory])],
            [NC_PATH_INC=${withval}], 
            [NC_PATH_INC=""])

AC_ARG_WITH([netcdf_libdir],
            [AS_HELP_STRING([--with-netcdf-libdir=ARG],[netcdf library directory])],
            [NC_PATH_LIBDIR=${withval}], 
            [NC_PATH_LIBDIR=""])



dnl if test ! -d "$NC_PATH" -o ! -f "${NC_PATH}/lib/libnetcdf.a"
dnl then
dnl    AC_MSG_ERROR([not found; use configure --with-netcdf=...])
dnl fi

AS_IF([test "z$NC_PATH_LIBDIR" = "z"],[NC_PATH_LIBDIR="$NC_PATH/lib"])

AS_IF([test "z$NC_PATH_INC" = "z"],[NC_PATH_INC="$NC_PATH/include"])

ac_netcdf_ok='no'

NC_LIBS=
NC_LDFLAGS=
ac_nc_save_LDFLAGS="$LDFLAGS"
ac_nc_save_LIBS="$LIBS"
LIBS="$LIBS -lnetcdf"
LDFLAGS="$LDFLAGS -L${NC_PATH_LIBDIR}"
AC_CHECK_LIB([netcdf],[nc_open],
   [
     ac_netcdf_ok='yes'
     NC_LIBS="-lnetcdf"
     NC_LDFLAGS="-L${NC_PATH_LIBDIR}" 
   ],
   [ac_netcdf_ok='no']
)
LIBS="$ac_nc_save_LIBS"
LDFLAGS="$ac_nc_save_LDFLAGS"

AC_SUBST([NC_LDFLAGS])
AC_SUBST([NC_LIBS])

NC_CPPFLAGS=
ac_netcdf_h='no'
ac_nc_save_CPPFLAGS="$CPPFLAGS"
CPPFLAGS="$CPPFLAGS -I$NC_PATH_INC"
AC_CHECK_HEADERS([netcdf.h],
  [ac_netcdf_h='yes'],
  [ac_netcdf_h='no']
)
CPPFLAGS=$ac_nc_save_CPPFLAGS

AS_IF([test "$ac_netcdf_h" = 'yes' ],
  [NC_CPPFLAGS="-I$NC_PATH_INC"]
)
AC_SUBST([NC_CPPFLAGS])

AS_IF([test "$ac_netcdf_ok" = 'no' -o "$ac_netcdf_h" = 'no'],
  [m4_if([$2], [], [:], [$2])],
  [m4_if([$1], [], [:], [$1])]
)

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
   ]
)

dnl if test "$NC_PATH"
dnl then
dnl     AC_MSG_RESULT([$NC_PATH])
dnl     LDFLAGS="$LDFLAGS -L${NC_PATH}/lib"
dnl     LIBS="$LIBS -lnetcdf"
dnl     CPPFLAGS="$CPPFLAGS -I${NC_PATH}/include"
dnl fi


OPENDAP_DEBUG_OPTION

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
