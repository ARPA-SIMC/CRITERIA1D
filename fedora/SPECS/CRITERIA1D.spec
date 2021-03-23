# Note: define srcarchivename in Travis build only.
%{!?srcarchivename: %global srcarchivename CRITERIA1D-%{version}}

Name:           CRITERIA1D
Version:        1.0.5
Release:        1%{?dist}
Summary:        One-dimensional soil water balance

URL:            https://github.com/ARPA-SIMC/CRITERIA1D
Source0:        https://github.com/ARPA-SIMC/CRITERIA1D/archive/v%{version}.tar.gz#/%{srcarchivename}.tar.gz
License:        GPL

BuildRequires:  qt5-qtbase
BuildRequires:  qt5-devel
BuildRequires:  qt5-qtcharts
BuildRequires:  qt5-qtcharts-devel
BuildRequires:  gdal-libs
BuildRequires:  gdal-devel
BuildRequires:  geos
BuildRequires:  geos-devel

Requires:       qt5-qtbase-mysql

%description
CRITERIA-1D is a soil water balance model simulating one-dimensional water
fluxes, crop development and crop water needs. The soil and crop parameters can
be defined at different level of detail. It requires as input daily
agro-meteorological data: minimum and maximum air temperature, total
precipitation and, if available, data of hypodermic watertable depth to
estimate the capillary rise.

%prep
%autosetup -n %{srcarchivename}

%build
pushd mapGraphics
qmake-qt5 MapGraphics.pro -spec linux-g++-64 CONFIG+=release CONFIG+=force_debug_info CONFIG+=qml_debug CONFIG+=c++11 CONFIG+=qtquickcompiler
make
popd

pushd bin/Makeall_CRITERIA1D
qmake-qt5 Makeall_CRITERIA1D.pro -spec linux-g++-64 CONFIG+=release CONFIG+=force_debug_info CONFIG+=c++11 CONFIG+=qtquickcompiler
make
popd

pushd bin/Makeall_CROP_EDITOR
qmake-qt5 Makeall_CROP_EDITOR.pro -spec linux-g++-64 CONFIG+=release CONFIG+=force_debug_info CONFIG+=c++11 CONFIG+=qtquickcompiler
make
popd

pushd bin/Makeall_SOIL_EDITOR
qmake-qt5 Makeall_SOIL_EDITOR.pro -spec linux-g++-64 CONFIG+=release CONFIG+=force_debug_info CONFIG+=c++11 CONFIG+=qtquickcompiler
make
popd

pushd bin/Makeall_CRITERIAGEO
qmake-qt5 Makeall_CRITERIAGEO.pro -spec linux-g++-64 CONFIG+=release CONFIG+=force_debug_info CONFIG+=c++11 CONFIG+=qtquickcompiler
make
popd

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p %{buildroot}/%{_bindir}/
cp -a bin/CRITERIA1D/release/CRITERIA1D %{buildroot}/%{_bindir}/
cp -a bin/CROP_EDITOR/release/CROP_EDITOR %{buildroot}/%{_bindir}/
cp -a bin/SOIL_EDITOR/release/SOIL_EDITOR %{buildroot}/%{_bindir}/
cp -a bin/CRITERIAGEO/release/CRITERIA_GEO %{buildroot}/%{_bindir}/
mkdir -p %{buildroot}/%{_datadir}/
cp -a deploy/appimage/usr/share/CRITERIA1D %{buildroot}/%{_datadir}/

%files
%{_bindir}/CRITERIA1D
%{_bindir}/CROP_EDITOR
%{_bindir}/SOIL_EDITOR
%{_bindir}/CRITERIA_GEO
%{_datadir}/CRITERIA1D/*

%changelog
* Fri Feb 12 2021 Emanuele Di Giacomo <edigiacomo@arpae.it> - 1.0.5-1
- Release 1.0.5

* Tue Feb  2 2021 Emanuele Di Giacomo <edigiacomo@arpae.it> - 1.0.4-1
- Release 1.0.4

* Mon Nov 02 2020 Emanuele Di Giacomo <edigiacomo@arpae.it> - 1.0.3-1
- Release 1.0.3

* Wed Oct 22 2020 Emanuele Di Giacomo <edigiacomo@arpae.it> - 1.0.1-3
- Application images (#6)

* Wed Oct 21 2020 Emanuele Di Giacomo <edigiacomo@arpae.it> - 1.0.1-2
- MySQL driver (#5)

* Wed Oct 21 2020 Emanuele Di Giacomo <edigiacomo@arpae.it> - 1.0.1-1
- Release 1.0.1

* Thu Oct 15 2020 Emanuele Di Giacomo <edigiacomo@arpae.it> - 1.0.0-1
- Release 1.0.0
