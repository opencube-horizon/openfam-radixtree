include(ExternalProject)

set(bundled_libmemcached libmemcached-1.0.18)

ExternalProject_Add(bundled_libmemcached
  URL ${CMAKE_CURRENT_LIST_DIR}/${bundled_libmemcached}.tar.gz
  URL_HASH SHA256=e22c0bb032fde08f53de9ffbc5a128233041d9f33b5de022c0978a2149885f82
  SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/${bundled_libmemcached}-src
  PATCH_COMMAND patch -f -p0 < ${CMAKE_CURRENT_LIST_DIR}/${bundled_libmemcached}-client_memflush.patch
  CONFIGURE_COMMAND
    ./configure
      --enable-static
      --disable-shared
      --prefix=<INSTALL_DIR>
      --libdir=<INSTALL_DIR>/lib
      --with-pic
      --disable-sasl
  BUILD_COMMAND make -j
  INSTALL_COMMAND make install
  INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/${bundled_libmemcached}
  BUILD_IN_SOURCE 1
  BUILD_BYPRODUCTS
    ${CMAKE_CURRENT_BINARY_DIR}/${bundled_libmemcached}/lib/libmemcached.a
  )

add_library(Libmemcached::memcached STATIC IMPORTED GLOBAL)
add_dependencies(Libmemcached::memcached bundled_libmemcached)
# This is a workaround for the fact that included directories of an imported
# target should exist in the filesystem already at the configuration time.
# ref: https://gitlab.kitware.com/cmake/cmake/-/issues/15052
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${bundled_libmemcached}/include/)
set_target_properties(Libmemcached::memcached PROPERTIES
  IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/${bundled_libmemcached}/lib/libmemcached.a
  INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_BINARY_DIR}/${bundled_libmemcached}/include/
)
