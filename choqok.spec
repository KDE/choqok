Name: choqok
Version: 0.1
Release: 1
Vendor: KDE Project
Summary: A KDE 4 Twitter Client
Group: Utility
Url: http://choqok.ospdev.net
License: GPL
Packager: Mehrdad Momeny <mehrdad.momeny@gmail.com>
BuildRoot:  %{_tmppath}/rpm/%{name}-root 
Source0: %{name}-%{version}.tar.gz

%description
choqoK is a 

%prep
%setup -q
cmake -DCMAKE_INSTALL_PREFIX=`kde4-config --prefix` CMAKE_BUILD_TYPE=relwithdebinfo .

%build
make

%install
make DESTDIR=%{?buildroot:%{buildroot}} install

%clean
rm -rf %{buildroot}


%files
%defattr(-, root, root)
/*


%changelog
* Sat Jun 21 2008 Mehrdad Momeny <mehrdad.momeny@gmail.com>
  - first spec file