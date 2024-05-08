# Output variables:
#  Pmem_INCLUDE_DIR : e.g., /usr/include/.
#  Pmem_LIBRARY     : Library path of PMEM library
#  Pmem_FOUND       : True if found.

find_package(PkgConfig)
pkg_check_modules(PC_Pmem QUIET libpmem)

find_path(Pmem_INCLUDE_DIR
  NAMES libpmem.h
  PATHS ${PC_Pmem_INCLUDE_DIRS}
)

find_library(Pmem_LIBRARY
  NAMES pmem
  PATHS ${PC_Pmem_LIBRARY_DIRS}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Pmem
  REQUIRED_VARS
    Pmem_LIBRARY
    Pmem_INCLUDE_DIR
  VERSION_VAR Pmem_VERSION
)

if(Pmem_FOUND AND NOT TARGET Pmem::pmem)
  add_library(Pmem::pmem UNKNOWN IMPORTED)
  set_target_properties(Pmem::pmem PROPERTIES
    IMPORTED_LOCATION "${Pmem_LIBRARY}"
    INTERFACE_COMPILE_OPTIONS "${PC_Pmem_CFLAGS_OTHER}"
    INTERFACE_INCLUDE_DIRECTORIES "${Pmem_INCLUDE_DIR}"
  )
endif()
