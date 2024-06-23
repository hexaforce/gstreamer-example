# RPM Package Specification File for gstreamer-example

%define _topdir /root/rpmbuild
%define name gstreamer-example
%define version 1.0
%define release 1%{?dist}
%define source_dir %{_topdir}/SOURCES
%define buildroot %{_topdir}/BUILDROOT/%{name}-%{version}-%{release}

Name:           %{name}
Version:        %{version}
Release:        %{release}
Summary:        A simple example using GStreamer

License:        GPL
URL:            https://example.com/gstreamer-example
Source0:        %{name}-%{version}.tar.gz

BuildRequires:  cmake
BuildRequires:  gcc-c++
BuildRequires:  gstreamer1-devel
BuildRequires:  gstreamer1-plugins-base-devel
BuildRequires:  gstreamer1-plugins-good
BuildRequires:  gstreamer1-plugins-bad-free
BuildRequires:  gstreamer1-libav

%description
This package provides a simple example application using GStreamer.

%prep
%autosetup -p1 -n %{name}-%{version}

%build
make %{?_smp_mflags}

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/%{_bindir}
cp -p src/build/gstreamer-example %{buildroot}/%{_bindir}/

%files
%license LICENSE
%{_bindir}/gstreamer-example

%changelog
* Fri Jun 24 2024 Your Name <your.email@example.com> - 1.0-1
- Initial RPM package release
