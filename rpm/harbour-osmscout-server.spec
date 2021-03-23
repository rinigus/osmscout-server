# Define Sailfish as it is absent
%if !0%{?fedora}
%define sailfishos 1
%endif


%if 0%{?sailfishos}
Name:       harbour-osmscout-server
%else
Name:       osmscout-server
%endif

%if 0%{?sailfishos}
%define __provides_exclude_from ^%{_datadir}/.*$
%define __requires_exclude ^libboost_filesystem|libboost_regex|libboost_system|libboost_iostreams|libboost_chrono|libboost_thread|libboost_date_time|libprotobuf|liblz4|libfreetype|libharfbuzz|libjpeg|libmapnik|libproj|libtiff.*$
%endif

Summary:    OSM Scout Server
Version:    1.17.1
Release:    1
Group:      Qt/Qt
License:    GPLv3
URL:        https://github.com/rinigus/osmscout-server
Source0:    %{name}-%{version}.tar.bz2
%if 0%{?sailfishos}
Source101:  harbour-osmscout-server-rpmlintrc
%endif

BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5Qml)
BuildRequires:  pkgconfig(Qt5Quick)
BuildRequires:  pkgconfig(Qt5Xml)
BuildRequires:  pkgconfig(Qt5Sql)
BuildRequires:  pkgconfig(Qt5Positioning)
BuildRequires:  pkgconfig(Qt5DBus)
BuildRequires:  pkgconfig(icu-uc)
BuildRequires:  pkgconfig(sqlite3)
BuildRequires:  pkgconfig(zlib)
BuildRequires:  pkgconfig(libcurl)
BuildRequires:  pkgconfig(marisa)
BuildRequires:  pkgconfig(libmicrohttpd)
BuildRequires:  pkgconfig(kyotocabinet)
BuildRequires:  pkgconfig(libpostal) >= 1.0.0
BuildRequires:  qt5-qttools-linguist
BuildRequires:  pkgconfig(libvalhalla)
BuildRequires:  protobuf-devel
BuildRequires:  boost-devel >= 1.51
BuildRequires:  boost-date-time >= 1.51
BuildRequires:  boost-chrono >= 1.51
BuildRequires:  boost-filesystem >= 1.51
BuildRequires:  boost-iostreams >= 1.51
BuildRequires:  boost-regex >= 1.51
BuildRequires:  boost-system >= 1.51
BuildRequires:  lz4-devel
BuildRequires:  desktop-file-utils

%if 0%{?sailfishos}
Requires:   sailfishsilica-qt5 >= 0.10.9
BuildRequires:  pkgconfig(sailfishapp) >= 1.0.2
BuildRequires:  mapnik-devel
#BuildRequires:  pkgconfig(libsystemd-daemon)
BuildRequires:  pkgconfig(libsystemd)
%else
BuildRequires:  pkgconfig(libsystemd)
BuildRequires:  cmake(KF5Kirigami2)
BuildRequires:  pkgconfig(Qt5QuickControls2)
BuildRequires:  pkgconfig(mapnik)
%endif


%description
Server providing map tiles, search, and routing


%prep
%setup -q -n %{name}-%{version}

%build

%if 0%{?sailfishos}
%qmake5 VERSION='%{version}-%{release}' SCOUT_FLAVOR='silica' CONFIG+=use_dbusactivation
%else
%qmake5 VERSION='%{version}-%{release}' SCOUT_FLAVOR='kirigami' CONFIG+=use_dbusactivation
%endif

make %{?_smp_mflags}

%install
rm -rf %{buildroot}
make install INSTALL_ROOT=%{buildroot}

ls -lR %{buildroot}

desktop-file-install --delete-original       \
  --dir %{buildroot}%{_datadir}/applications             \
   %{buildroot}%{_datadir}/applications/*.desktop

%if 0%{?sailfishos}

# ship all shared libraries not allowed in Harbour with the app
mkdir -p %{buildroot}%{_datadir}/%{name}/lib

cp /usr/lib/libmapnik.so.3.0 %{buildroot}%{_datadir}/%{name}/lib
cp /usr/lib/libproj.so.12 %{buildroot}%{_datadir}/%{name}/lib
cp /usr/lib/libtiff.so.5 %{buildroot}%{_datadir}/%{name}/lib
cp /usr/lib/libharfbuzz.so.0 %{buildroot}%{_datadir}/%{name}/lib
cp /usr/lib/libjpeg.so.62 %{buildroot}%{_datadir}/%{name}/lib
cp /usr/lib/libfreetype.so.6 %{buildroot}%{_datadir}/%{name}/lib
#cp /usr/lib/libicui18n.so.52 %{buildroot}%{_datadir}/%{name}/lib
#cp /usr/lib/libicudata.so.52 %{buildroot}%{_datadir}/%{name}/lib
#cp /usr/lib/libicuuc.so.52 %{buildroot}%{_datadir}/%{name}/lib

cp /usr/lib/libboost_filesystem-mt.so.* %{buildroot}%{_datadir}/%{name}/lib
cp /usr/lib/libboost_filesystem.so.* %{buildroot}%{_datadir}/%{name}/lib
cp /usr/lib/libboost_regex-mt.so.* %{buildroot}%{_datadir}/%{name}/lib
cp /usr/lib/libboost_regex.so.* %{buildroot}%{_datadir}/%{name}/lib
cp /usr/lib/libboost_system-mt.so.* %{buildroot}%{_datadir}/%{name}/lib
cp /usr/lib/libboost_system.so.* %{buildroot}%{_datadir}/%{name}/lib
cp /usr/lib/libboost_iostreams-mt.so.* %{buildroot}%{_datadir}/%{name}/lib
cp /usr/lib/libboost_date_time-mt.so.* %{buildroot}%{_datadir}/%{name}/lib
cp /usr/lib/libboost_chrono-mt.so.* %{buildroot}%{_datadir}/%{name}/lib
cp /usr/lib/libboost_thread-mt.so.* %{buildroot}%{_datadir}/%{name}/lib

cp /usr/lib/libprotobuf.so*  %{buildroot}%{_datadir}/%{name}/lib

cp /usr/lib/liblz4.so.1.8.1 %{buildroot}%{_datadir}/%{name}/lib
cp /usr/lib/liblz4.so.1 %{buildroot}%{_datadir}/%{name}/lib

# mapnik fonts and input plugins
# not needed anymore since input plugins are linked
# into main mapnik library and fonts are distributed with
# the styles
#cp -r /usr/lib/mapnik %{buildroot}%{_datadir}/%{name}/lib

strip %{buildroot}%{_datadir}/%{name}/lib/libmapnik.so.3.0
#strip %{buildroot}%{_datadir}/%{name}/lib/libicudata.so.52

# strip executable bit from all libraries
chmod -x %{buildroot}%{_datadir}/%{name}/lib/*.so*
#chmod -x %{buildroot}%{_datadir}/%{name}/lib/mapnik/*/*

%endif # sailfishos


%files
%defattr(-,root,root,-)
%{_bindir}/%{name}
%{_bindir}/%{name}-gui
%{_datadir}/%{name}
%{_datadir}/dbus-1/services/io.github.rinigus.OSMScoutServer.service
%{_datadir}/icons/hicolor/*/apps/%{name}.png
%{_datadir}/applications/%{name}.desktop
%if 0%{?sailfishos}
%{_datadir}/%{name}-gui
%else
%{_datadir}/metainfo/%{name}.appdata.xml
%endif
