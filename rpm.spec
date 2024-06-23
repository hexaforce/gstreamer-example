%define _topdir /gstreamer-example

Name: gstreamer-example
Version: 1.0
Release: 1
Summary: A simple example using GStreamer
License: MIT
Source0: gstreamer-example.tar.gz

%description
This package provides a simple example application using GStreamer.

%changelog
* Tue Jun 25 2024 Your Name <your.email@example.com> - 1.0-1
- Initial RPM package release

%build
make
