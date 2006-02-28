Summary:         NetCDF 3 data handler for the OPeNDAP Data server
Name:            netcdf_handler
Version:         3.6.0
Release:         1
License:         LGPL
Group:           System Environment/Daemons 
Source0:         ftp://ftp.unidata.ucar.edu/pub/opendap/source/%{name}-%{version}.tar.gz
URL:             http://www.opendap.org/

BuildRoot:       %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
BuildRequires:   libdap-devel >= 3.6.0 netcdf-devel >= 3.5
# Don't require dap-server as dap_nc_handler works without dap-server,
# however dap_nc_handler should be of use only with dap-server in most cases.
#Requires:        dap-server

%description
This is the netcdf data handler for our data server. It reads netcdf 3
files and returns DAP responses that are compatible with DAP2 and the
dap-server 3.6 software.

%prep 
%setup -q

%build
%configure
make %{?_smp_mflags}

%install
rm -rf $RPM_BUILD_ROOT
make DESTDIR=$RPM_BUILD_ROOT install

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root,-)
%{_bindir}/dap_nc_handler
%doc COPYING COPYRIGHT NEWS
%doc README

%changelog
* Wed Feb  1 2006 Patrice Dumas <pertusus at free.fr> - 3.5.2-1
- re-add netcdf-devel

* Wed Nov 16 2005 James Gallagher <jgallagher@opendap.org> 3.5.1-1
- Removed netcdf-devel from BuildRequires. it does, unless you install 
- netcdf some other way.

* Thu Sep 21 2005 James Gallagher <jgallagher@opendap.org> 3.5.0-1
- initial release
