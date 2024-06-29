# gstreamer-example

```bash
docker buildx build --file Dockerfile.Debian12 --tag hexaforce/deb-build .
docker buildx build --file Dockerfile.Fedora40 --tag hexaforce/rpm-build .
```

$ ldd webrtc-sendrecv
        linux-vdso.so.1 (0x00007fb023b9b000)
        libgstsdp-1.0.so.0 => /lib64/libgstsdp-1.0.so.0 (0x00007fb023b65000)
        libgstwebrtc-1.0.so.0 => /lib64/libgstwebrtc-1.0.so.0 (0x00007fb023b52000)
        libgstrtp-1.0.so.0 => /lib64/libgstrtp-1.0.so.0 (0x00007fb023b23000)
        libgstbase-1.0.so.0 => /lib64/libgstbase-1.0.so.0 (0x00007fb023a9d000)
        libgstreamer-1.0.so.0 => /lib64/libgstreamer-1.0.so.0 (0x00007fb023952000)
        libjson-glib-1.0.so.0 => /lib64/libjson-glib-1.0.so.0 (0x00007fb023926000)
        libsoup-2.4.so.1 => /lib64/libsoup-2.4.so.1 (0x00007fb023884000)
        libgmodule-2.0.so.0 => /lib64/libgmodule-2.0.so.0 (0x00007fb02387d000)
        libgio-2.0.so.0 => /lib64/libgio-2.0.so.0 (0x00007fb0236ae000)
        libgobject-2.0.so.0 => /lib64/libgobject-2.0.so.0 (0x00007fb02364f000)
        libglib-2.0.so.0 => /lib64/libglib-2.0.so.0 (0x00007fb023501000)
        libc.so.6 => /lib64/libc.so.6 (0x00007fb023314000)
        libgstpbutils-1.0.so.0 => /lib64/libgstpbutils-1.0.so.0 (0x00007fb0232d1000)
        libm.so.6 => /lib64/libm.so.6 (0x00007fb0231ee000)
        libunwind.so.8 => /lib64/libunwind.so.8 (0x00007fb0231d4000)
        libdw.so.1 => /lib64/libdw.so.1 (0x00007fb023143000)
        libxml2.so.2 => /lib64/libxml2.so.2 (0x00007fb022fe5000)
        libsqlite3.so.0 => /lib64/libsqlite3.so.0 (0x00007fb022e85000)
        libpsl.so.5 => /lib64/libpsl.so.5 (0x00007fb022e70000)
        libbrotlidec.so.1 => /lib64/libbrotlidec.so.1 (0x00007fb022e62000)
        libgssapi_krb5.so.2 => /lib64/libgssapi_krb5.so.2 (0x00007fb022e0e000)
        libz.so.1 => /lib64/libz.so.1 (0x00007fb022ded000)
        libmount.so.1 => /lib64/libmount.so.1 (0x00007fb022d9b000)
        libselinux.so.1 => /lib64/libselinux.so.1 (0x00007fb022d6e000)
        libffi.so.8 => /lib64/libffi.so.8 (0x00007fb022d5e000)
        libpcre2-8.so.0 => /lib64/libpcre2-8.so.0 (0x00007fb022cc2000)
        /lib64/ld-linux-x86-64.so.2 (0x00007fb023b9d000)
        libgstvideo-1.0.so.0 => /lib64/libgstvideo-1.0.so.0 (0x00007fb022bf5000)
        libgstaudio-1.0.so.0 => /lib64/libgstaudio-1.0.so.0 (0x00007fb022b72000)
        libgsttag-1.0.so.0 => /lib64/libgsttag-1.0.so.0 (0x00007fb022b32000)
        libelf.so.1 => /lib64/libelf.so.1 (0x00007fb022b16000)
        libzstd.so.1 => /lib64/libzstd.so.1 (0x00007fb022a57000)
        liblzma.so.5 => /lib64/liblzma.so.5 (0x00007fb022a24000)
        libbz2.so.1 => /lib64/libbz2.so.1 (0x00007fb022a10000)
        libunistring.so.5 => /lib64/libunistring.so.5 (0x00007fb022861000)
        libidn2.so.0 => /lib64/libidn2.so.0 (0x00007fb02283f000)
        libbrotlicommon.so.1 => /lib64/libbrotlicommon.so.1 (0x00007fb02281c000)
        libkrb5.so.3 => /lib64/libkrb5.so.3 (0x00007fb022754000)
        libk5crypto.so.3 => /lib64/libk5crypto.so.3 (0x00007fb02273d000)
        libcom_err.so.2 => /lib64/libcom_err.so.2 (0x00007fb022736000)
        libkrb5support.so.0 => /lib64/libkrb5support.so.0 (0x00007fb022724000)
        libkeyutils.so.1 => /lib64/libkeyutils.so.1 (0x00007fb02271d000)
        libcrypto.so.3 => /lib64/libcrypto.so.3 (0x00007fb022200000)
        libresolv.so.2 => /lib64/libresolv.so.2 (0x00007fb02270b000)
        libblkid.so.1 => /lib64/libblkid.so.1 (0x00007fb0226d1000)
        liborc-0.4.so.0 => /lib64/liborc-0.4.so.0 (0x00007fb022156000)

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
