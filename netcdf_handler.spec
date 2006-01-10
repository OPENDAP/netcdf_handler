Summary:         NetCDF 3 data handler for the OPeNDAP Data server
Name:            netcdf_handler
Version:         3.5.2
Release:         1
License:         GPL
Group:           System Environment/Daemons 
Source0:         http://www.opendap.org/pub/3.5/source/%{name}-%{version}.tar.gz
URL:             http://www.opendap.org/

BuildRoot:       %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
BuildRequires:   libdap-devel >= 3.5.2
Requires:        dap-server >= 3.5.0

%description
This is the netcdf data handler for our data server. It reads netcdf 3
files and returns DAP responses that are compatible with DAP2 and the
dap-server 3.5 software.

%prep 
%setup -q

%build
%configure

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
* Wed Nov 16 2005 James Gallagher <jgallagher@opendap.org> 3.5.1-1
- Removed netcdf-devel from BuildRequires. it does, unless you install 
- netcdf some other way.
* Thu Sep 21 2005 James Gallagher <jgallagher@opendap.org> 3.5.0-1
- initial release
