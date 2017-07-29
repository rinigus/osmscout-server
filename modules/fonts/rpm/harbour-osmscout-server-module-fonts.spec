Name:       harbour-osmscout-server-module-fonts

%{!?qtc_qmake:%define qtc_qmake %qmake}
%{!?qtc_qmake5:%define qtc_qmake5 %qmake5}
%{!?qtc_make:%define qtc_make make}
%{?qtc_builddir:%define _builddir %qtc_builddir}
Summary:    OSM Scout Server Module: Fonts
Version:    0.10.0
Release:    1
Group:      Qt/Qt
License:    MIT
URL:        https://github.com/rinigus/osmscout-server
Source0:    %{name}-%{version}.tar.bz2
BuildArch:  noarch

BuildRequires: pkgconfig(sailfishapp)
Requires: sailfishsilica-qt5
Requires: libsailfishapp-launcher

%description
This a module for OSM Scout Server providing fonts for Mapnik
rendering backend

%prep
%setup -q -n %{name}-%{version}

%build

%qtc_qmake5 SPECVERSION=%{version}

%qtc_make %{?_smp_mflags}

%install
rm -rf %{buildroot}

%qmake5_install


%files
%defattr(644,root,root,755)
%{_datadir}/%{name}
%{_datadir}/applications/%{name}.desktop
%{_datadir}/icons/hicolor/*/apps/%{name}.png

