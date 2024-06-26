# gstreamer-example

```bash
docker buildx build --file Dockerfile.Debian12 --tag hexaforce/deb-build .
docker buildx build --file Dockerfile.Fedora40 --tag hexaforce/rpm-build .
```

cat /etc/os-release

docker run --rm -it -v ./:/gstreamer-example 37909784a722 bash
docker run --rm -it -v ./:/gstreamer-example 3364c3d7d36e bash



rm -f debian/changelog

export DEBEMAIL="your.email@example.com"
rm -f debian/changelog
dch --create --package gstreamer-example --newversion 1.0-1 --distribution unstable -b ""
dch warning: ignoring -a/-i/-e/-r/-b/--allow-lower-version/-n/--bin-nmu/-q/--qa/-R/-s/--lts/--team/--bpo/--stable,-l options with --create


rm -rf ~/rpmbuild
rpmdev-setuptree

cp -r /gstreamer-example ~/rpmbuild/BUILD/



rpmbuild -ba rpm/gstreamer-example.spec







tar czvf src.tar.gz /gstreamer-example
mv src.tar.gz ~/rpmbuild/SOURCES/


cp /gstreamer-example/rpm.spec ~/rpmbuild/SPECS/


rpmbuild -ba ~/rpmbuild/SPECS/rpm.spec
rpmbuild -ba rpm.spec


# RPM Package Specification File for gstreamer-example

%define _topdir %(echo $HOME)/rpmbuild
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
