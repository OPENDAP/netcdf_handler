Summary:         NetCDF 3 data handler for the OPeNDAP Data server
Name:            netcdf_handler
Version:         3.7.6
Release:         1
License:         LGPL
Group:           System Environment/Daemons 
Source0:         ftp://ftp.unidata.ucar.edu/pub/opendap/source/%{name}-%{version}.tar.gz
URL:             http://www.opendap.org/

BuildRoot:       %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
BuildRequires:   libdap-devel >= 3.7.5 netcdf-devel
# BuildRequires:   bes-devel

%description
This is the netcdf data handler for our data server. It reads netcdf 3
files and returns DAP responses that are compatible with DAP2 and the
dap-server software.

%prep 
%setup -q

%build
%configure --disable-static --disable-dependency-tracking
make %{?_smp_mflags}

%install
rm -rf $RPM_BUILD_ROOT
make DESTDIR=$RPM_BUILD_ROOT install

rm -f $RPM_BUILD_ROOT%{_libdir}/*.la
rm -f $RPM_BUILD_ROOT%{_libdir}/*.so
rm -f $RPM_BUILD_ROOT%{_libdir}/bes/*.la

%clean
rm -rf $RPM_BUILD_ROOT

%post -p /sbin/ldconfig

# Only try to configure the bes.conf file if the bes can be found.
if bes-config --version >/dev/null 2>&1
then
	bes_prefix=`bes-config --prefix`
	configure-ff-data.sh $bes_prefix/etc/bes/bes.conf $bes_prefix/lib/bes
fi

%postun -p /sbin/ldconfig

%files
%defattr(-,root,root,-)
%{_bindir}/dap_nc_handler
%{_bindir}/configure-nc-data.sh
%{_libdir}/
%{_libdir}/bes/
%{_datadir}/hyrax/data/nc
%doc COPYING COPYRIGHT NEWS README

%changelog
* Thu Sep  7 2006 Patrice Dumas <pertusus at free.fr> - 3.7.2-1
- Update to 3.7.2

* Fri Mar  3 2006 Patrice Dumas <pertusus at free.fr> - 3.6.0-1
- Update to 3.6.0

* Wed Feb  1 2006 Patrice Dumas <pertusus at free.fr> - 3.5.2-1
- re-add netcdf-devel

* Wed Nov 16 2005 James Gallagher <jgallagher@opendap.org> 3.5.1-1
- Removed netcdf-devel from BuildRequires. it does, unless you install 
- netcdf some other way.

* Thu Sep 21 2005 James Gallagher <jgallagher@opendap.org> 3.5.0-1
- initial release
