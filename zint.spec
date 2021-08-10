Name:      zint
Version:   2.10.0
Release:   2%{?dist}
Summary:   A barcode generator and library
License:   GPLv3+
URL:       http://www.zint.org.uk
Source:    http://downloads.sourceforge.net/project/%{name}/%{name}/%{version}/%{name}-%{version}.src.tar.gz
Group:     Applications/Engineering
BuildRoot: %(mktemp -ud %{_tmppath}/%{name}-%{version}-%{release}-XXXXXX)

# Reset locales to "C" when exporting to EPS or SVG in order to force
# decimal points in all language environments. 
# This issue has been reported upstream:
# http://sourceforge.net/mailarchive/forum.php?thread_name=4BF78012.7090508%40uos.de&forum_name=zint-barcode
Patch0:    zint-locale.patch

BuildRequires: cmake
BuildRequires: libpng-devel
BuildRequires: zlib-devel

%description
Zint is a C library for encoding data in several barcode variants. The
bundled command-line utility provides a simple interface to the library.
Features of the library:
- Over 50 symbologies including all ISO/IEC standards, like QR codes.
- Unicode translation for symbologies which support Latin-1 and 
  Kanji character sets.
- Full GS1 support including data verification and automated insertion of 
  FNC1 characters.
- Support for encoding binary data including NULL (ASCII 0) characters.
- Health Industry Barcode (HIBC) encoding capabilities.
- Output in the following file formats: PNG, GIF, EPS, WMF, BMP, TIF, SVG.
- Verification stage for SBN, ISBN and ISBN-13 data.


%package -n zint-devel
Summary:       Library and header files for %{name}
Group:         Development/Libraries
Requires:      %{name} = %{version}-%{release}

%description -n zint-devel 
C library and header files needed to develop applications using %{name}.
The API documentation can be found ont the project website:
http://www.zint.org.uk/zintSite/Manual.aspx


%package -n zint-qt
Summary:       Zint Barcode Studio GUI and library
Group:         Applications/Engineering
Requires:      %{name} = %{version}-%{release}
BuildRequires: qt-devel >= 4.4
BuildRequires: desktop-file-utils

%description -n zint-qt
Zint Barcode Studio is a Qt-based GUI which allows desktop users to generate 
barcodes which can then be embedded in documents or HTML pages, and a library 
which can be used to incorporate barcode generation functionality into other 
software.


%package -n zint-qt-devel
Summary:       Library and header files for %{name}-qt
Group:         Development/Libraries
Requires:      %{name}-devel = %{version}-%{release}
Requires:      %{name}-qt = %{version}-%{release}

%description -n zint-qt-devel 
C library and header files needed to develop applications using %{name}-qt.


%prep
%setup -q
%patch0 -p1

# remove BSD-licensed file required for Windows only (just to ensure that this package is plain GPLv3+)
rm -f backend/ms_stdint.h

# remove bundled getopt sources (we use the corresponding Fedora package instead)
rm -f frontend/getopt*.*

%build
%cmake CMakeLists.txt
make VERBOSE=1 %{?_smp_mflags}

cat <<EOF >zint-qt.desktop
[Desktop Entry]
Name=Zint Barcode Studio
GenericName=Zint Barcode Studio
Exec=zint-qt
Icon=zint
Terminal=false
Type=Application
Categories=Utility;
EOF


%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT
rm -rf $RPM_BUILD_ROOT/%{_datadir}/cmake
install -D -p -m 644 frontend_qt4/images/%{name}_white.png %{buildroot}/usr/share/pixmaps/%{name}.png
install -D -p -m 644 %{name}-qt.desktop %{buildroot}%{_datadir}/applications/%{name}-qt.desktop
desktop-file-validate %{buildroot}%{_datadir}/applications/%{name}-qt.desktop

%clean
rm -rf $RPM_BUILD_ROOT


%post -p /sbin/ldconfig
%postun -p /sbin/ldconfig
%post -n %{name}-qt -p /sbin/ldconfig
%postun -n %{name}-qt -p /sbin/ldconfig


%files
%defattr(-,root,root,-)
%doc COPYING README
%{_bindir}/%{name}
%{_libdir}/libzint.so.*

%files -n %{name}-devel
%defattr(-,root,root,-)
%{_includedir}/%{name}.h
%{_libdir}/libzint.so

%files -n %{name}-qt
%defattr(-,root,root,-)
%{_bindir}/%{name}-qt
%{_libdir}/libQZint.so.*
%{_datadir}/applications/%{name}-qt.desktop
%{_datadir}/pixmaps/%{name}.png

%files -n %{name}-qt-devel
%defattr(-,root,root,-)
%{_includedir}/qzint.h
%{_libdir}/libQZint.so


%changelog

* Sat Feb 1 2020 Harald Oehlmann <oehhar@sourceforge.net> - 2.7.1
- Version -> 2.7.1

* Thu Dec 5 2019 Harald Oehlmann <oehhar@sourceforge.net> - 2.7.0
- Version -> 2.7.0

* Wed Sep 18 2019 Harald Oehlmann <oehhar@sourceforge.net> - 2.6.6
- Version -> 2.6.6

* Mon Sep 1 2019 Harald Oehlmann <oehhar@sourceforge.net> - 2.6.5
- Version -> 2.6.5

* Fri Aug 30 2019 Harald Oehlmann <oehhar@sourceforge.net> - 2.6.4
- Version -> 2.6.4

* Thu Feb 15 2018 Robin Stuart <rstuart114@gmail.com> - 2.6.3
- Version -> 2.6.3

* Sun Oct 22 2017 Robin Stuat <rstuart114@gmail.com> - 2.6.2
- Version -> 2.6.2

* Sun Aug 27 2017 Robin Stuart <rstuart114@gmail.com> - 2.6.1
- Version -> 2.6.1

* Thu May 11 2017 Robin Stuart <rstuart114@gmail.com> - 2.6.0
- Update version number

* Sat May 22 2010 Martin Gieseking <martin.gieseking@uos.de> - 2.3.1-2
- Added patch to fix export issue

* Fri May 21 2010 Martin Gieseking <martin.gieseking@uos.de> - 2.3.1-1
- initial package
