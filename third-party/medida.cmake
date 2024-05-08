include(ExternalProject)

set(bundled_medida medida-1a488e50dd22140bc7d365d2cb2a5c47b69b6044)

ExternalProject_Add(bundled_medida
  URL ${CMAKE_CURRENT_LIST_DIR}/${bundled_medida}.tar.gz
  URL_HASH SHA256=12ead4ed9205e897c9a08ec50e3be7f5b5e06b35bf739c9774e4e5b48105819c
  SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/${bundled_medida}-src
  INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/${bundled_medida}
  PATCH_COMMAND sed -i -e s|SHARED|STATIC| CMakeLists.txt
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_BINARY_DIR}/${bundled_medida} -DCMAKE_POSITION_INDEPENDENT_CODE=ON
  BUILD_IN_SOURCE 1
  BUILD_BYPRODUCTS
    ${CMAKE_CURRENT_BINARY_DIR}/${bundled_medida}/lib/libmedida.a
  )

add_library(Medida::medida STATIC IMPORTED GLOBAL)
add_dependencies(Medida::medida bundled_medida)
# This is a workaround for the fact that included directories of an imported
# target should exist in the filesystem already at the configuration time.
# ref: https://gitlab.kitware.com/cmake/cmake/-/issues/15052
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${bundled_medida}/include/)
set_target_properties(Medida::medida PROPERTIES
  IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/${bundled_medida}/lib/libmedida.a
  INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_BINARY_DIR}/${bundled_medida}/include/
)
