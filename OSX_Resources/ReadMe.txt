
Notes for version 3.7.6 (24 Nov 2006)

This is the OPeNDAP netCDF Data Handler. It is used
along with the OPeNDAP DAP Server3 and Hyrax 1.2.1
beta.

For information about building the OPeNDAP netCDF
Data Handler, see the INSTALL file.

This data handler is a component of the OPeNDAP DAP
Server; the server base software is designed to allow
any number of handlers to be configured easily.  See
the DAP Server README file for information about
configuration, including how to use this handler.

Test data are also installed, so after installing
this handler Hyrax will have data to serve providing
an easy way to test your new install and to see how a
working bes.conf should look. To use this, make sure
that you first install the bes, and that dap-server
gets installed too.  Finally, every time you install
or reinstall handlers, make sure to restart the BES
and OLFS.

REQUIREMENTS

  o You need the libdap library version 3.7.5 to
  build and install this software. If you're using
  Linux, this means either building from source or
  using the libdap and libdap-devel RPM packages.

  o If you're planning on using this with the OPeNDAP
  4 Data Server (Hyrax), you'll need version 3.5.1 of
  the BES software. Make sure to install that first.

  o To use this software with our cgi-based data
  server, you will need the OPeNDAP dap-server
  software package, version 3.7.3 (although for this
  release, operation with Server3 has not been
  tested).

  o You also need a recent copy of netCDF. We've
  tested this server with netcdf version 3.5,
  although any recent version should work fine. If
  the configure script cannot find your copy of
  libnetcdf, use the --with-netcdf options to tell it
  where to look (see configure --help).

