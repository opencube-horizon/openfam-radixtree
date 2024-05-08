# Output variables:
#  Libmemcached_INCLUDE_DIR : e.g., /usr/include/.
#  Libmemcached_LIBRARY     : Library path of libmemcached library
#  Libmemcached_FOUND       : True if found.

find_package(PkgConfig)
pkg_check_modules(PC_Libmemcached QUIET libmemcached)

find_path(Libmemcached_INCLUDE_DIR
  NAMES libmemcached/memcached.h
  PATHS ${PC_Libmemcached_INCLUDE_DIRS}
)

find_library(Libmemcached_LIBRARY
  NAMES memcached
  PATHS ${PC_Libmemcached_LIBRARY_DIRS}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Libmemcached
  REQUIRED_VARS
    Libmemcached_LIBRARY
    Libmemcached_INCLUDE_DIR
  VERSION_VAR Libmemcached_VERSION
)

if(Libmemcached_FOUND AND NOT TARGET Libmemcached::memcached)
  add_library(Libmemcached::memcached UNKNOWN IMPORTED)
  set_target_properties(Libmemcached::memcached PROPERTIES
    IMPORTED_LOCATION "${Libmemcached_LIBRARY}"
    INTERFACE_COMPILE_OPTIONS "${PC_Libmemcached_CFLAGS_OTHER}"
    INTERFACE_INCLUDE_DIRECTORIES "${Libmemcached_INCLUDE_DIR}"
  )
endif()
