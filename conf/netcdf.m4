dnl example of use
dnl AC_CHECK_NETCDF(
dnl   [
dnl       LIBS="$LIBS $NC_LIBS"
dnl       LDFLAGS="$LDFLAGS $NC_LDFLAGS"
dnl       CPPFLAGS="$CPPFLAGS $NC_CPPFLAGS"
dnl   ],
dnl   [
dnl       echo "*** Use --with-netcdf for the root netcdf directory."
dnl       echo "*** Otherwise use --with-netcdf-include switch for includes directory"
dnl       echo "*** and --with-netcdf-libdir switch for libraries directory."
dnl       AC_MSG_ERROR([netcdf library and netcdf headers are required.])
dnl   ]
dnl )

# Check for the netcdf library.
# AC_CHECK_NETCDF([ACTION-IF-FOUND],[ACTION-IF-NOT-FOUND],[INTERFACE-NR])
# if interface number is given, check for a specific interface
# sets NC_LDFLAGS, NC_LIBS, and, by calling other macros
# NC_CPPFLAGS and maybe NC_NETCDF_3_CPPFLAG
AC_DEFUN([AC_CHECK_NETCDF],
[
  AC_ARG_WITH([netcdf],
            [AS_HELP_STRING([--with-netcdf=ARG],[netcdf directory])],
            [NC_PATH=$withval], 
            [NC_PATH=""])

  AC_ARG_WITH([netcdf_include],
            [AS_HELP_STRING([--with-netcdf-include=ARG],[netcdf include directory])],
            [NC_PATH_INC=$withval], 
            [NC_PATH_INC=""])

  AC_ARG_WITH([netcdf_libdir],
            [AS_HELP_STRING([--with-netcdf-libdir=ARG],[netcdf library directory])],
            [NC_PATH_LIBDIR=$withval], 
            [NC_PATH_LIBDIR=""])

  AS_IF([test "z$NC_PATH_LIBDIR" = "z"],
    [AS_IF([test "z$NC_PATH" != "z"],
      [NC_PATH_LIBDIR="$NC_PATH/lib"])])

  AS_IF([test "z$NC_PATH_INC" = "z"],
    [AS_IF([test "z$NC_PATH" != "z"],
      [NC_PATH_INC="$NC_PATH/include"])])

  ac_netcdf_ok='no'

  NC_LIBS=
  NC_LDFLAGS=
  ac_nc_save_LDFLAGS=$LDFLAGS
  ac_nc_save_LIBS=$LIBS
  AS_IF([test "z$NC_PATH_LIBDIR" != "z"],[LDFLAGS="$LDFLAGS -L$NC_PATH_LIBDIR"])
  LDFLAGS="$LDFLAGS -L${NC_PATH_LIBDIR}"
  ac_check_func_checked='ncopen'
  ac_check_interface=
dnl the interface number isn't quoted with "" otherwise a newline 
dnl following the number isn't stripped.
  m4_if([$3],[],[:],[ac_check_interface=$3])
  AS_IF([test "z$ac_check_interface" = 'z3'],
    [ac_check_func_checked='nc_open'])
dnl the autoconf internal cache isn't avoided because we really check for
dnl libnetcdf, other libraries that implement the same api have other names
dnl  AC_LINK_IFELSE([AC_LANG_CALL([],[$ac_check_func_checked])],
  AC_CHECK_LIB([netcdf],[$ac_check_func_checked],
    [
      ac_netcdf_ok='yes'
      NC_LIBS="-lnetcdf"
      NC_LDFLAGS="-L$NC_PATH_LIBDIR" 
    ],
    [
      ac_netcdf_ok='no'
    ])
  LDFLAGS=$ac_nc_save_LDFLAGS

  AC_SUBST([NC_LDFLAGS])
  AC_SUBST([NC_LIBS])

  AC_CHECK_NETCDF_HEADER([$NC_PATH_INC],
    [ac_netcdf_header='yes'],
    [ac_netcdf_header='no'],
    [$3])

  AS_IF([test "$ac_netcdf_ok" = 'no' -o "$ac_netcdf_header" = 'no'],
    [m4_if([$2], [], [:], [$2])],
    [m4_if([$1], [], [:], [$1])])
])

