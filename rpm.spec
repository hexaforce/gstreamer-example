%define _topdir /gstreamer-example

Name: gstreamer-example
Version: 1.0
Release: 1
Summary: A simple example using GStreamer
License: MIT

%description
This package provides a simple example application using GStreamer.

%changelog
* Tue Jun 25 2024 Your Name <your.email@example.com> - 1.0-1
- Initial RPM package release

%build
cp -r ../src .
cp ../Makefile .
cp ../CMakeLists.txt .
make %{?_smp_mflags}

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/usr/bin
cp -a build/GStreamerExample %{buildroot}/usr/bin/

%files
%defattr(-,root,root,-)
/usr/bin/GStreamerExample
