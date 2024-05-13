#
# spec file for package openfam-radixtree
#
# Copyright (c) 2024 Hewlett Packard Enterprise Development, LP.
#
# All modifications and additions to the file contributed by third parties
# remain the property of their copyright owners, unless otherwise agreed
# upon. The license for this file, and modifications and additions to the
# file, is the same license as for the pristine package itself (unless the
# license for the pristine package is not an Open Source License, in which
# case the license is the MIT License). An "Open Source License" is a
# license that conforms to the Open Source Definition (Version 1.9)
# published by the Open Source Initiative.

# Please submit bugfixes or comments via https://bugs.opensuse.org/
#

%define git_sha e6493cf8c9e5e26c14e25f3d09f1584b2f62c2c1

Name:           openfam-radixtree
Version:        0.2_pre20240513
Release:        0
Summary:        User-space library implementing a radix tree atop of NVMM
License:        MIT
Group:          Development/Libraries/C and C++
URL:            https://github.com/opencube-horizon/openfam-radixtree
Source0:        https://github.com/opencube-horizon/openfam-radixtree/archive/%{git_sha}.tar.gz
BuildRequires:  cmake
BuildRequires:  pkg-config
BuildRequires:  gcc-c++
%if 0%{?suse_version}
BuildRequires:  libboost_log-devel
BuildRequires:  libboost_thread-devel
BuildRequires:  libboost_system-devel
BuildRequires:  libboost_filesystem-devel
BuildRequires:  libboost_atomic-devel
BuildRequires:  libboost_regex-devel
%else
BuildRequires:  boost-devel
BuildRequires:  libatomic
%endif
BuildRequires:  libpmem-devel
BuildRequires:  yaml-cpp-devel
BuildRequires:  libmemcached-devel
BuildRequires:  openfam-nvmm-devel

%description
A user-space library that implements a radix tree that allocates memory using the fabric-attached memory manager (Project Gull)
and relies on fabric-attached memory atomic operations for transactional updates.

%package -n libradixtree
Summary:        Radix Tree libraries
Group:          System/Libraries

%package        devel
Summary:        Development files for %{name}
Group:          Development/Libraries/C and C++
Requires:       %{name} = %{version}

# TODO: add a package for the content in bin/*

%description -n libradixtree
libradixtree provides the Radix Tree libraries.

%description    devel
The %{name}-devel package contains libraries and header files for
developing applications that use %{name}.

%prep
%setup -q -n %{name}-%{git_sha}

%build
%cmake -DWITH_TESTS=OFF -DWITH_EXAMPLES=OFF
%cmake_build

%install
%cmake_install

%post -n libradixtree -p /sbin/ldconfig
%postun -n libradixtree -p /sbin/ldconfig

%files -n libradixtree
%{_libdir}/*.so

%files devel
%{_includedir}/*
%{_libdir}/cmake/radixtree
%license COPYING
%doc README.md

%changelog
