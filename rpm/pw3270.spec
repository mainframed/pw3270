#
# spec file for package @PACKAGE@
#
# Copyright (c) 2015 SUSE LINUX GmbH, Nuernberg, Germany.
# Copyright (C) <2008> <Banco do Brasil S.A.>
#
# All modifications and additions to the file contributed by third parties
# remain the property of their copyright owners, unless otherwise agreed
# upon. The license for this file, and modifications and additions to the
# file, is the same license as for the pristine package itself (unless the
# license for the pristine package is not an Open Source License, in which
# case the license is the MIT License). An "Open Source License" is a
# license that conforms to the Open Source Definition (Version 1.9)
# published by the Open Source Initiative.

# Please submit bugfixes or comments via http://bugs.opensuse.org/
#

%define LIBVERSION @MAJOR_VERSION@_@MINOR_VERSION@

# 
# References:
#
# https://en.opensuse.org/openSUSE:Build_Service_cross_distribution_howto#Detect_a_distribution_flavor_for_special_code
#

%define _dbus     	1
%define _office	  	0
%define _php		0
%define _ooRexx		1
%define _java		0
%define _python		1

%define _distro		linux

#---[ Packaging ]-----------------------------------------------------------------------------------------------------

Name:           @PACKAGE@
Version:        @PACKAGE_VERSION@
Release:        @PACKAGE_LEVEL@
Summary:        IBM 3270 Terminal emulator for GTK
License:        GPL-2.0
Group:          System/X11/Terminals
Url:            https://portal.softwarepublico.gov.br/social/pw3270/

Source:         %{name}-%{version}.tar.bz2

BuildRoot:      %{_tmppath}/%{name}-%{version}-build

Requires:       lib3270 = %{version}
Requires:       shared-mime-info

Provides:       lib@PACKAGE@ = %{version}
Provides:       lib@PACKAGE@.so = %{version}

#--[ Red HAT ]--------------------------------------------------------------------------------------------------------

%if 0%{?rhel_version}

%define _distro rhel%{rhel_version}

BuildRequires:  dbus-devel
BuildRequires:  dbus-glib-devel
BuildRequires:  openssl-devel

%define _office 0
%define _php	0

%if 0%{?rhel_version} >= 0700

# RHEL 7.0

BuildRequires:  gtk3-devel

%else

# RHEL 6.0
%define _python	0

BuildRequires:  gtk2-devel
BuildRequires:  java-1.6.0-devel

%endif

%endif

#--[ CentOS ]---------------------------------------------------------------------------------------------------------

%if 0%{?centos_version}

%define _distro centos%{centos_version}

BuildRequires:  pkgconfig(dbus-1)
BuildRequires:  pkgconfig(dbus-glib-1)
BuildRequires:  pkgconfig(openssl)

%if 0%{?centos_version} >= 0700

# CentOS 7

%define _php	5
BuildRequires:  php-devel

%define _office	  	0

BuildRequires:  pkgconfig(gtk+-3.0)

%else

# CentOS 6

%define _office 0
%define _python	0

BuildRequires:  gtk2-devel
BuildRequires:  java-1.8.0-devel

%endif

%endif

#--[ Fedora ]---------------------------------------------------------------------------------------------------------

%if 0%{?fedora}

%define _distro fedora%{fedora}

BuildRequires:  pkgconfig(dbus-1)
BuildRequires:  pkgconfig(dbus-glib-1)
BuildRequires:  pkgconfig(gtk+-3.0)
BuildRequires:  pkgconfig(openssl)

%define _php	  5
BuildRequires:  php-devel

%if 0%{?fedora} >= 21

# Fedora 21 fails on libreoffice sdk
%define _office	  	0

%else

#%define _office	  	0
#BuildRequires:  libreoffice-sdk
#BuildRequires:  libreoffice-ure
#BuildRequires:  librsvg2-tools

%endif

%endif

#--[ SuSE ]-----------------------------------------------------------------------------------------------------------

%if 0%{?suse_version}

# https://en.opensuse.org/openSUSE:Packaging_Conventions_RPM_Macros#.25sles_version
%if 0%{?sles_version}
	%define _distro sles%{sles_version}
	%define _office	  0
%else
%if 0%{?sled_version}
	%define _distro sled%{sled_version}
	%define _office	  0
%else
	%define _distro suse%{suse_version}
%endif

%endif

BuildRequires:  libopenssl-devel
BuildRequires:  rsvg-view

# OpenSUSE >= 12.2 enable GTK3 & D-Bus
%if 0%{?suse_version} >= 1220

BuildRequires:  pkgconfig(dbus-1)
BuildRequires:  pkgconfig(dbus-glib-1)
BuildRequires:  pkgconfig(gtk+-3.0)

%else

%define _dbus	0
%define _python	0
BuildRequires:  gtk2-devel

%endif

# OpenSUSE >= 13.1 uses PHP5
%if 0%{?suse_version} >= 1310
%define _php		5
BuildRequires:  php%{_php}-devel
%endif

%endif

#---------------------------------------------------------------------------------------------------------------------

BuildRequires:  autoconf >= 2.61
BuildRequires:  automake
BuildRequires:  binutils
BuildRequires:  coreutils

%if 0%{?_ooRexx}
BuildRequires:  ooRexx-devel >= 4.2.0
%endif

%if 0%{?_python}
BuildRequires:	python
BuildRequires:  python-devel

%define pythonextpath %(python -c "from distutils.sysconfig import get_python_lib; print get_python_lib()")
%endif

%if 0%{?_php}
%define _phpextdir %(php-config --extension-dir)
%define _phpconf %(php-config | sed 's@ @\\n@g' | grep "^--with-config-file-scan-dir=" | cut -d= -f2)
%endif

%if 0%{?_java}
BuildRequires:  java-devel
BuildRequires:  javapackages-tools
%endif

BuildRequires:  desktop-file-utils
BuildRequires:  findutils
BuildRequires:  gcc-c++
BuildRequires:  gettext-devel
BuildRequires:  m4
BuildRequires:  pkgconfig
BuildRequires:  sed


%description
Open-source GTK-based IBM 3270 terminal emulator with many advanced features. It can be used to communicate with any IBM host that supports 3270-style connections over TELNET.
Based on the original x3270 code, pw3270 was originally created for Banco do Brasil, and is now used worldwide. 

#--[ lib3270 ]--------------------------------------------------------------------------------------------------------

%package -n lib3270-%LIBVERSION
Summary:        3270 Communication library for %{name}
Group:          Development/Libraries/C and C++
Requires:       openssl
Provides:       lib3270

%description -n lib3270-%LIBVERSION
Open-source GTK-based IBM 3270 terminal emulator with many advanced features. It can be used to communicate with any IBM host that supports 3270-style connections over TELNET.

This package contains the tn3270 protocol library for %{name}

#--[ Devel ]----------------------------------------------------------------------------------------------------------

%package -n lib3270-devel
Summary:        Devel for 3270 Communication library for %{name}
Group:          Development/Libraries/C and C++
Requires:       lib3270-%LIBVERSION = %{version}

%description -n lib3270-devel
Open-source GTK-based IBM 3270 terminal emulator with many advanced features. It can be used to communicate with any IBM host that supports 3270-style connections over TELNET.
This package contains the development files for tn3270 protocol library for %{name}

%package -n @PACKAGE@-devel
Summary:        Files required for development of %{name} plugins
Group:          Development/Libraries/C and C++
Requires:       lib3270-devel = %{version}
Requires:       @PACKAGE@ = %{version}

%description -n @PACKAGE@-devel
Open-source GTK-based IBM 3270 terminal emulator with many advanced features. It can be used to communicate with any IBM host that supports 3270-style connections over TELNET.

This package contains the development files for %{name}

#--[ Plugins ]--------------------------------------------------------------------------------------------------------

%if 0%{?_dbus}
%package plugin-dbus
Summary:        DBUS object for %{name}
Group:          System/X11/Terminals
Requires:       %{name} = %{version}
Requires:       dbus-1

%description plugin-dbus
Plugin exporting a DBUS object from every %{name} open session.
%endif

%if 0%{?_ooRexx}
%package -n %{name}-plugin-rexx
Summary:        Rexx class for 3270 access
Group:          Development/Languages/Other
Requires:       lib3270 = %{version}
Requires:       ooRexx >= 4.1

%description -n %{name}-plugin-rexx
This package provides Rexx class and associated libraries
allowing rexx scripts to access tn3270e hosts.
%endif

#--[ Extensions ]-----------------------------------------------------------------------------------------------------

%if 0%{?_office}
%package -n %{name}-libreoffice
Summary:        3270 host access extension for libreoffice
Group:          Productivity/Office/Suite
Requires:       lib3270 = %{version}
Requires:       libreoffice

%description -n %{name}-libreoffice
This package provides 3270 access object to StarBasic.
%endif

%if 0%{?_php}
%package -n php%{_php}-tn3270
Summary:        PHP%{_php} Extension Module implementing tn3270 protocol
Group:          Development/Libraries/PHP
Requires:       lib3270 = %{version}

%description -n php%{_php}-tn3270
This is an extension for acessing 3270 hosts directly
from PHP apps.
%endif

%if 0%{?_python}
%package -n python-tn3270
Summary:        Python Extension Module implementing tn3270 protocol
Group:          Development/Libraries/Python
Requires:       lib3270 = %{version}

%description -n python-tn3270
This is an extension for acessing 3270 hosts directly
from python apps.
%endif

%if 0%{?_java}
%package 		java
Summary:        Java class to interact with @PACKAGE@
Group:          Development/Libraries/Java
Requires:       javapackages-tools
Requires:       lib3270 = %{version}

%description java
Java class for lib3270/@PACKAGE@ interaction.

%package		plugin-java
Summary:        Java plugin for %{name}
Group:          Development/Libraries/Java
Requires:       %{name} = %{version}
Requires:       %{name}-java = %{version}

%description plugin-java
This package provides a plugin allowing calling of java classes
from @PACKAGE@ application.

%package javadoc 
Summary:        Javadoc for %{name} 
Group:          Documentation 
Requires:       jpackage-utils

%description javadoc 
API documentation for %{name}. 
%endif

#---[ Build & Install ]-----------------------------------------------------------------------------------------------

%prep

echo "Distribution: %{_distro}"

%if 0%{?_php}
	echo "	* PHP%{_php} is enabled"
%endif
%if 0%{?_dbus}
	echo "	* DBUS is enabled"
%endif

%setup -q -n %{name}-%{version}
find . -exec touch {} \;
aclocal
autoconf

export CFLAGS="$RPM_OPT_FLAGS"
export CXXFLAGS="$RPM_OPT_FLAGS"
export FFLAGS="$RPM_OPT_FLAGS"

%if 0%{?_java}
export JAVA_HOME=%{java_home}
echo "JAVA_HOME=$JAVA_HOME"
echo "javadocdir=%{_javadocdir}"
%endif

%configure	--disable-strip \
			--enable-pic \
			--with-jnidir="%{_jnidir}" \
			--with-jvmjardir="%{_jvmjardir}" \
			--with-javadocdir="%{_javadocdir}"

%build
make clean
make all

%install
export NO_BRP_CHECK_BYTECODE_VERSION=true

make DESTDIR=$RPM_BUILD_ROOT install

find $RPM_BUILD_ROOT

%find_lang %{name} langfiles

cat > @PACKAGE@.desktop << EOF
[Desktop Entry]
X-SuSE-translate=true
GenericName=@PACKAGE@
Name=3270 Terminal
Comment=IBM 3270 Terminal emulator
Exec=@PACKAGE@
Icon=%{_datadir}/@PACKAGE@/@PACKAGE@.png
Terminal=false
Type=Application
StartupNotify=true
EOF
chmod 644 @PACKAGE@.desktop

desktop-file-install	--mode 644 \
			--dir $RPM_BUILD_ROOT/%{_datadir}/applications \
			--add-category System \
			--add-category TerminalEmulator \
			@PACKAGE@.desktop

%if 0%{?_ooRexx} == 0
# remove rexx-files from builddir if it's disabled
rm -f ${RPM_BUILD_ROOT}/%{_datadir}/@PACKAGE@/ui/80rexx.xml
%endif 

%if 0%{?_java} == 0
# remove java-files from builddir if it's disabled
rm ${RPM_BUILD_ROOT}/%{_datadir}/@PACKAGE@/ui/*java*.xml
%endif 

find ${RPM_BUILD_ROOT}

%clean
rm -rf %{buildroot}

#---[ Files ]---------------------------------------------------------------------------------------------------------

%files -f langfiles
%defattr(-,root,root)
%doc AUTHORS LICENSE 
%{_mandir}/*/*

# Main application
%dir %{_datadir}/@PACKAGE@
%dir %{_datadir}/@PACKAGE@/ui
%{_bindir}/@PACKAGE@
%{_libdir}/lib@PACKAGE@.so.5
%{_libdir}/lib@PACKAGE@.so.%version

%{_datadir}/applications/@PACKAGE@.desktop

%{_datadir}/@PACKAGE@/ui/00default.xml
%{_datadir}/@PACKAGE@/ui/10functions.xml
%{_datadir}/@PACKAGE@/ui/10keypad.xml
%{_datadir}/@PACKAGE@/colors.conf
%{_datadir}/@PACKAGE@/@PACKAGE@.png
%{_datadir}/@PACKAGE@/@PACKAGE@-logo.png
%{_datadir}/locale/pt_BR/LC_MESSAGES/@PACKAGE@.mo
%dir %{_libdir}/@PACKAGE@-plugins

%files -n lib3270-%LIBVERSION
%defattr(-,root,root)
%{_libdir}/lib3270.so.%version
%{_libdir}/lib3270.so.5

%files -n lib3270-devel
%defattr(-,root,root)
%{_includedir}/lib3270
%{_includedir}/lib3270.h
%{_libdir}/pkgconfig/lib3270.pc
%{_libdir}/lib3270.so

%files -n @PACKAGE@-devel
%defattr(-,root,root)
%{_includedir}/@PACKAGE@
%{_includedir}/@PACKAGE@.h
%{_datadir}/@PACKAGE@/ui/98trace.xml
%{_datadir}/@PACKAGE@/ui/99debug.xml
%{_libdir}/lib@PACKAGE@.so
%{_libdir}/pkgconfig/@PACKAGE@.pc

%{_libdir}/lib@PACKAGE@cpp.a
%{_includedir}/@PACKAGE@cpp.h

%dir %{_datadir}/@PACKAGE@/sample
%{_datadir}/@PACKAGE@/sample/*

%if 0%{?_dbus}
%files plugin-dbus
%defattr(-,root,root)
%{_libdir}/@PACKAGE@-plugins/dbus3270.so
%endif

%if 0%{?_office}
%files -n %{name}-libreoffice
%defattr(-,root,root)
%dir %{_libdir}/libreoffice/share/extensions/%{name}
%dir %{_libdir}/libreoffice/share/extensions/%{name}/META-INF
%{_libdir}/libreoffice/share/extensions/%{name}/META-INF/manifest.xml
%{_libdir}/libreoffice/share/extensions/%{name}/description.txt
%{_libdir}/libreoffice/share/extensions/%{name}/description.xml
%{_libdir}/libreoffice/share/extensions/%{name}/@PACKAGE@.png
%{_libdir}/libreoffice/share/extensions/%{name}/@PACKAGE@.rdb
%{_libdir}/libreoffice/share/extensions/%{name}/@PACKAGE@.uno.so
%endif

%if 0%{?_ooRexx}
%files -n %{name}-plugin-rexx
%defattr(-,root,root)
%{_rexxlibdir}/librx3270.so.%version
%{_rexxlibdir}/librx3270.so
%{_rexxclassdir}/rx3270.cls
%{_libdir}/@PACKAGE@-plugins/rx3270.so
%{_datadir}/@PACKAGE@/ui/80rexx.xml
%endif

%if 0%{?_php}
%files -n php%{_php}-tn3270
%defattr(-,root,root)
%config %{_phpconf}/tn3270.ini
%{_phpextdir}/*.so
%endif

%if 0%{?_python}
%files -n python-tn3270
%defattr(-,root,root)
%{pythonextpath}/py3270.so
%endif

%if 0%{?_java}
%files java
%defattr(-,root,root)
%dir %{_jnidir}
%dir %{_jvmjardir}

%{_jnidir}/libjni3270.so
%{_jvmjardir}/@PACKAGE@.jar

%files plugin-java
%defattr(-,root,root)
%{_libdir}/@PACKAGE@-plugins/j3270.so

%files javadoc 
%defattr(-,root,root)
%dir %{_javadocdir}
%{_javadocdir}/%{name} 
%{_datadir}/@PACKAGE@/ui/*java*.xml
%endif

#---[ Scripts ]-------------------------------------------------------------------------------------------------------

%post
/sbin/ldconfig
exit 0

%postun
/sbin/ldconfig
exit 0

%post -n lib3270-%LIBVERSION
/sbin/ldconfig
exit 0

%postun -n lib3270-%LIBVERSION
/sbin/ldconfig
exit 0

%if 0%{?_ooRexx}
%post -n %{name}-plugin-rexx
/sbin/ldconfig
exit 0

%postun -n %{name}-plugin-rexx
/sbin/ldconfig
exit 0
%endif

%changelog