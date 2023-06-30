# Note: define srcarchivename in Travis build only.
%{!?srcarchivename: %global srcarchivename CRITERIA1D-%{version}}

Name:           CRITERIA1D
Version:        1.7.1-1
Release:        1%{?dist}
Summary:        One-dimensional agro-hydrological model

URL:            https://github.com/ARPA-SIMC/CRITERIA1D
Source0:        https://github.com/ARPA-SIMC/CRITERIA1D/archive/v%{version}.tar.gz#/%{srcarchivename}.tar.gz
License:        GPL

BuildRequires:  qt5-qtbase
BuildRequires:  qt5-qtbase-devel
BuildRequires:  qt5-qtcharts
BuildRequires:  qt5-qtcharts-devel
BuildRequires:  gdal-libs
BuildRequires:  gdal-devel
BuildRequires:  geos
BuildRequires:  geos-devel
BuildRequires:  netcdf
BuildRequires:  netcdf-devel

Requires:       qt5-qtbase-mysql

%description
CRITERIA-1D is an agro-hydrological model that simulates one-dimensional water flow 
in variable saturation soils, crop development, root water extraction and irrigation water needs. 
Soil water flow can be simulated with two different approaches depending on the user's choice: 
a physically based numerical model or a layer-based conceptual model. 
Soil and crop parameters can be defined at different levels of detail. 
It requires daily agro-meteorological data as input: minimum and maximum air temperature, 
total precipitation and, if available, water table depth data to estimate capillary rise.

%prep
%autosetup -n %{srcarchivename}

%build
pushd mapGraphics
qmake-qt5 MapGraphics.pro -spec linux-g++-64 CONFIG+=release CONFIG+=force_debug_info CONFIG+=c++11 CONFIG+=qtquickcompiler
make
popd

pushd bin/Makeall_CRITERIA1D
qmake-qt5 Makeall_CRITERIA1D.pro -spec linux-g++-64 CONFIG+=release CONFIG+=force_debug_info CONFIG+=c++11 CONFIG+=qtquickcompiler
make
popd

pushd bin/Makeall_SOIL_EDITOR
qmake-qt5 Makeall_SOIL_EDITOR.pro -spec linux-g++-64 CONFIG+=release CONFIG+=force_debug_info CONFIG+=c++11 CONFIG+=qtquickcompiler
make
popd

pushd bin/Makeall_CRITERIA1D_PRO
qmake-qt5 Makeall_CRITERIA1D_PRO.pro -spec linux-g++-64 CONFIG+=release CONFIG+=force_debug_info CONFIG+=c++11 CONFIG+=qtquickcompiler
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
cp -a bin/SOIL_EDITOR/release/SOIL_EDITOR %{buildroot}/%{_bindir}/
cp -a bin/CRITERIA1D_PRO/release/CRITERIA1D_PRO %{buildroot}/%{_bindir}/
cp -a bin/CRITERIAGEO/release/CRITERIA_GEO %{buildroot}/%{_bindir}/
mkdir -p %{buildroot}/%{_datadir}/
cp -a deploy/appimage/usr/share/CRITERIA1D %{buildroot}/%{_datadir}/

%files
%{_bindir}/CRITERIA1D
%{_bindir}/SOIL_EDITOR
%{_bindir}/CRITERIA1D_PRO
%{_bindir}/CRITERIA_GEO
%{_datadir}/CRITERIA1D/*

%changelog
* fri Jun 30 2023 Fausto Tomei <ftomei@arpae.it> - 1.7.1-1
- Release 1.7.1

* fri Jun 30 2023 Fausto Tomei <ftomei@arpae.it> - 1.7.0-1
- Release 1.7.0

* Mon Jun 19 2023 Fausto Tomei <ftomei@arpae.it> - 1.6.4-1
- Release 1.6.4

* Mon May 08 2023 Fausto Tomei <ftomei@arpae.it> - 1.6.3-1
- Release 1.6.3

* Mon Apr 24 2023 Fausto Tomei <ftomei@arpae.it> - 1.6.2-1
- Release 1.6.2

* Thu Feb 09 2023 Fausto Tomei <ftomei@arpae.it> - 1.6.0-1
- Release 1.6.0

* Thu Jan 05 2023 Fausto Tomei <ftomei@arpae.it> - 1.5.7-1
- Release 1.5.7

* Wed Nov 04 2022 Fausto Tomei <ftomei@arpae.it> - 1.5.6-1
- Release 1.5.6

* Mon Jul 25 2022 Fausto Tomei <ftomei@arpae.it> - 1.5.5-1
- Release 1.5.5

* Mon Jun 06 2022 Fausto Tomei <ftomei@arpae.it> - 1.5.4-1
- Release 1.5.4

* Tue May 17 2022 Fausto Tomei <ftomei@arpae.it> - 1.5.2-1
- Release 1.5.2

* Thu Oct 14 2021 Fausto Tomei <ftomei@arpae.it> - 1.4.0-1
- Release 1.4.0

* Tue Aug 10 2021 Fausto Tomei <ftomei@arpae.it> - 1.3.0-1
- Release 1.3.0

* Tue May 25 2021 Fausto Tomei <ftomei@arpae.it> - 1.2.5-1
- Release 1.2.5

* Thu Mar 25 2021 Emanuele Di Giacomo <edigiacomo@arpae.it> - 1.1.5-1
- Release 1.1.5

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
