# Note: define srcarchivename in Travis build only.
%{!?srcarchivename: %global srcarchivename CRITERIA1D-%{version}}

Name:           CRITERIA1D
Version:        1.0.0
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
qmake-qt5 MapGraphics.pro -spec linux-g++-64 CONFIG+=debug CONFIG+=qml_debug CONFIG+=c++11 CONFIG+=qtquickcompiler
make
popd

pushd tools/csvToMeteoDb
qmake-qt5 csvToMeteoDb.pro -spec linux-g++-64 CONFIG+=release CONFIG+=c++11 CONFIG+=qtquickcompiler
make
popd

pushd tools/Makeall_CriteriaOutput
qmake-qt5 Makeall_CriteriaOutput.pro -spec linux-g++-64 CONFIG+=release CONFIG+=c++11 CONFIG+=qtquickcompiler
make
popd

pushd bin/Makeall_CRITERIA1D
qmake-qt5 Makeall_CRITERIA1D.pro -spec linux-g++-64 CONFIG+=release CONFIG+=c++11 CONFIG+=qtquickcompiler
make
popd

pushd bin/Makeall_CROP_EDITOR
qmake-qt5 Makeall_CROP_EDITOR.pro -spec linux-g++-64 CONFIG+=release CONFIG+=c++11 CONFIG+=qtquickcompiler
make
popd

pushd bin/Makeall_SOIL_EDITOR
qmake-qt5 Makeall_SOIL_EDITOR.pro -spec linux-g++-64 CONFIG+=release CONFIG+=c++11 CONFIG+=qtquickcompiler
make
popd

pushd bin/Makeall_CRITERIAGEO
qmake-qt5 Makeall_CRITERIAGEO.pro -spec linux-g++-64 CONFIG+=release CONFIG+=c++11 CONFIG+=qtquickcompiler
make
popd

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p %{buildroot}/%{_bindir}/
cp -a tools/csvToMeteoDb/release/CsvToMeteoDb %{buildroot}/%{_bindir}/
cp -a tools/criteriaOutputTools/release/CriteriaOutput %{buildroot}/%{_bindir}/
cp -a bin/CRITERIA1D/release/CRITERIA1D %{buildroot}/%{_bindir}/
cp -a bin/CROP_EDITOR/release/CROP_EDITOR %{buildroot}/%{_bindir}/
cp -a bin/SOIL_EDITOR/release/SOIL_EDITOR %{buildroot}/%{_bindir}/
cp -a bin/CRITERIAGEO/release/CRITERIA_GEO %{buildroot}/%{_bindir}/

%files
%{_bindir}/CsvToMeteoDb
%{_bindir}/CriteriaOutput
%{_bindir}/CRITERIA1D
%{_bindir}/CROP_EDITOR
%{_bindir}/SOIL_EDITOR
%{_bindir}/CRITERIA_GEO

%changelog
* Thu Oct 15 2020 Emanuele Di Giacomo <edigiacomo@arpae.it>
- Release 1.0.0