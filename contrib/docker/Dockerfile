FROM opensuse/leap:15.5

RUN --mount=type=cache,target=/var/cache/zypp \
  set -ex ; \
  sed -i -e 's|http:|https:|g' /etc/zypp/repos.d/* ; \
  zypper --non-interactive addrepo \
    https://download.opensuse.org/repositories/home:/dev-zero/15.5/home:dev-zero.repo ; \
  zypper --gpg-auto-import-keys refresh ; \
  zypper --non-interactive install \
    cmake \
    gcc-c++ \
    libboost_log-devel \
    libboost_thread-devel \
    libboost_system-devel \
    libboost_filesystem-devel \
    libboost_atomic-devel \
    libboost_regex-devel \
    libpmem-devel \
    yaml-cpp-devel \
    rpmbuild \
    libmemcached-devel \
    openfam-nvmm-devel
