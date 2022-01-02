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
%define __requires_exclude ^libboost_filesystem|libboost_regex|libboost_system|libboost_iostreams|libboost_chrono|libboost_thread|libboost_date_time|libpostal|libprotobuf|liblz4|libfreetype|libharfbuzz|libjpeg|libmapnik|libproj|libtiff.*$
%endif

Summary:    Offline maps, routing, and search
Version:    2.1.2
Release:    1
Group:      Qt/Qt
License:    GPLv3
URL:        https://rinigus.github.io/osmscout-server
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
BuildRequires:  pkgconfig(date)
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
OSM Scout server can be used as a drop-in replacement for online map
services providing map tiles, search, and routing. As a result, an
offline operation is possible if the device has a server and map
client programs installed and running.

Among other services, the server can be used to provide: vector or
raster tiles for other applications; search for locations and free
text search; search for POIs next to a reference point or route;
calculating routes between given sequence of points;

PackageName: OSM Scout Server
Type: desktop-application
Categories:
  - Maps
  - Science
Custom:
  Repo: https://github.com/rinigus/osmscout-server
Icon: https://raw.githubusercontent.com/rinigus/osmscout-server/master/gui/icons/osmscout-server.svg
Screenshots:
  - https://raw.githubusercontent.com/rinigus/osmscout-server/master/screenshots/main.png
  - https://rinigus.github.io/osmscout-server/en/manager/manager_9.jpg
  - https://rinigus.github.io/osmscout-server/en/manager/manager_5.jpg
  - https://rinigus.github.io/osmscout-server/en/storage/storage_1.jpg
Url:
  Donation: https://rinigus.github.io/donate

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

desktop-file-install --delete-original       \
  --dir %{buildroot}%{_datadir}/applications             \
   %{buildroot}%{_datadir}/applications/*.desktop

%if 0%{?sailfishos}

# ship all shared libraries not allowed in Harbour with the app
mkdir -p %{buildroot}%{_datadir}/%{name}/lib

cp %{_libdir}/libmapnik.so.3.0 %{buildroot}%{_datadir}/%{name}/lib
cp %{_libdir}/libproj.so.12 %{buildroot}%{_datadir}/%{name}/lib
cp %{_libdir}/libtiff.so.5 %{buildroot}%{_datadir}/%{name}/lib
cp %{_libdir}/libharfbuzz.so.0 %{buildroot}%{_datadir}/%{name}/lib
cp %{_libdir}/libjpeg.so.62 %{buildroot}%{_datadir}/%{name}/lib
cp %{_libdir}/libfreetype.so.6 %{buildroot}%{_datadir}/%{name}/lib
#cp %{_libdir}/libicui18n.so.52 %{buildroot}%{_datadir}/%{name}/lib
#cp %{_libdir}/libicudata.so.52 %{buildroot}%{_datadir}/%{name}/lib
#cp %{_libdir}/libicuuc.so.52 %{buildroot}%{_datadir}/%{name}/lib

cp %{_libdir}/libboost_filesystem-mt.so.* %{buildroot}%{_datadir}/%{name}/lib
cp %{_libdir}/libboost_filesystem.so.* %{buildroot}%{_datadir}/%{name}/lib
cp %{_libdir}/libboost_regex-mt.so.* %{buildroot}%{_datadir}/%{name}/lib
cp %{_libdir}/libboost_regex.so.* %{buildroot}%{_datadir}/%{name}/lib
cp %{_libdir}/libboost_system-mt.so.* %{buildroot}%{_datadir}/%{name}/lib
cp %{_libdir}/libboost_system.so.* %{buildroot}%{_datadir}/%{name}/lib
cp %{_libdir}/libboost_iostreams-mt.so.* %{buildroot}%{_datadir}/%{name}/lib
cp %{_libdir}/libboost_date_time-mt.so.* %{buildroot}%{_datadir}/%{name}/lib
cp %{_libdir}/libboost_chrono-mt.so.* %{buildroot}%{_datadir}/%{name}/lib
cp %{_libdir}/libboost_thread-mt.so.* %{buildroot}%{_datadir}/%{name}/lib

cp %{_libdir}/libpostal.so*  %{buildroot}%{_datadir}/%{name}/lib

cp %{_libdir}/libprotobuf.so*  %{buildroot}%{_datadir}/%{name}/lib

cp %{_libdir}/liblz4.so.1* %{buildroot}%{_datadir}/%{name}/lib

# mapnik fonts and input plugins
# not needed anymore since input plugins are linked
# into main mapnik library and fonts are distributed with
# the styles
#cp -r %{_libdir}/mapnik %{buildroot}%{_datadir}/%{name}/lib

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
%{_datadir}/applications/%{name}-gui.desktop
%if 0%{?sailfishos}
%{_datadir}/%{name}-gui
%else
%{_datadir}/metainfo/%{name}.appdata.xml
%endif
