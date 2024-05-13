cmake_minimum_required(VERSION 3.20)
project(radixtree
  LANGUAGES CXX)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

#
# build type: Release (default) or Debug
#
message(STATUS "====================")
if(CMAKE_BUILD_TYPE MATCHES Debug)
  message(STATUS "Build type: Debug")
elseif(CMAKE_BUILD_TYPE MATCHES Release)
  message(S$TATUS "Build type: Release")
elseif(CMAKE_BUILD_TYPE MATCHES RelWithDebInfo)
  message(STATUS "Build type: RelWithDebInfo")
elseif(CMAKE_BUILD_TYPE MATCHES Coverage)
  message(STATUS "Build type: Coverage")
else()
  message(STATUS "Invalid build type: ${CMAKE_BUILD_TYPE}")
  message(STATUS "Default to : Release")
  set(CMAKE_BUILD_TYPE "Release")
endif()

option(FAME "Build with FAME support" OFF)
option(METRICS "Enable metrics gathering via medida" ON)
option(LOG "Enable logging" ON)
option(WITH_KVS_CLIENT "Build the KVS client library (requires libmemcached)" ON)

#
# determine endianness at compile time
#
add_compile_definitions($<IF:$<STREQUAL:${CMAKE_CXX_BYTE_ORDER},BIG_ENDIAN>,SYS_BIG_ENDIAN,SYS_LITTLE_ENDIAN>)
message(STATUS "Current system is: ${CMAKE_CXX_BYTE_ORDER}")

#
# determine tmpfs/lfs location
#
if(FAME) # FAME
  add_compile_definitions(FAME)
  set(TMPFS_PATH "/lfs")
  message(STATUS "LFS enabled")
  message(STATUS "Lfs location: ${TMPFS_PATH}")
else() # NUMA
  set(TMPFS_PATH "/dev/shm")
  message(STATUS "tmpfs location: ${TMPFS_PATH}")
endif()

#
# determine shelf base dir. Used to store files backing the shelves
#
if(FAME)
  set(SHELF_BASE_DIR "${TMPFS_PATH}")
else()
  set(SHELF_BASE_DIR "${TMPFS_PATH}/$ENV{USER}")
endif()
add_compile_definitions(SHELF_USER="$ENV{USER}")
add_compile_definitions(SHELF_BASE_DIR="${SHELF_BASE_DIR}")
message(STATUS "Shelf base dir: ${SHELF_BASE_DIR}")

#
# enable logging
#
if(LOG)
  message(STATUS "logging: on")
  add_compile_definitions(PRINT_LOG)
else()
  message(STATUS "logging: off")
endif()

#
# which implementation to use? PMEM=ON: pmem_invalidate + load; PMEM=OFF: fam_atomics
#
if(PMEM)
  add_compile_definitions(PMEM)
  message(STATUS "implementation: use pmem")
else()
  message(STATUS "implementation: use fam_atomics")
endif()

#
# enable metrics collection
#
if(METRICS)
  add_compile_definitions(METRICS)
  message(STATUS "Metrics enabled")
else() # METRICS
  message(STATUS "Metrics disabled")
endif()

message(STATUS "====================")


#
# common gcc flags
#
# while radixtree requires only C++11, gtest needs C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(cxx_flags
  -Wall
  -W
  -Wextra
  -Wredundant-decls
  -Wunused
  -Wunused-macros
  -Wno-unused-parameter
  -Wcast-qual
  -Wcast-align
  -Wwrite-strings
  -Wmissing-field-initializers
  -Wendif-labels
  -Winit-self
  -Wlogical-op
  -Wpacked
  -Wstack-protector
  -Wformat=2
  -Wswitch-enum
  -Wstrict-overflow=5
  -Wpointer-arith
  -Wnormalized=nfc
  -Wno-long-long
  -Wconversion
  -Wunreachable-code
)
set(debug_flags -g -Og -fsanitize=address -fsanitize=leak)
# --coverage enables required flags on both GCC and LLVM and works with both compilation and linking
set(coverage_flags --coverage)
add_compile_options(
  "$<$<COMPILE_LANGUAGE:CXX>:${cxx_flags}>"
  "$<$<CONFIG:Debug>:${debug_flags}>"
  "$<$<CONFIG:Coverage>:${debug_flags}>"
  "$<$<CONFIG:Coverage>:${coverage_flags}>"
)
add_link_options(
  "$<$<CONFIG:Debug>:${debug_flags}>"
  "$<$<CONFIG:Coverage>:${coverage_flags}>"
)
add_compile_definitions(
  "$<$<OR:$<CONFIG:Debug>,$<CONFIG:Coverage>>:DEBUG>"
  "$<$<CONFIG:Coverage>:COVERAGE>"
  "$<$<AND:$<CONFIG:Coverage>,$<CXX_COMPILER_ID:GNU>>:GCOV>"
)

# Flags that are currently not used: -Wshadow -Wpadded
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-invalid-offsetof ") # to suppress warnings about offsetof

#
# add boost
#
find_package(Boost 1.65 COMPONENTS log log_setup thread system filesystem REQUIRED)


# for boost::log
add_compile_definitions(BOOST_LOG_DYN_LINK)

#
# header-files directory and library for nvmm
#
find_package(nvmm REQUIRED)

#
# add libpmem  
#
find_package(Pmem REQUIRED)

#
# add yaml-cpp
#
find_package(yaml-cpp REQUIRED)


if(WITH_KVS_CLIENT)
#
# add libmemcached if available, otherwise build bundled one
#
  find_package(Libmemcached)
  if (NOT Libmemcached_FOUND)
    message(STATUS "Libmemcached was not found, building a bundled one")
  endif()
endif()

#
# add source code
#
add_subdirectory(third-party)
add_subdirectory(src)


# create base shelf dir
if(FAME) # FAME
  add_custom_target(
    radixtree_shelf_base_dir
    COMMAND ${CMAKE_COMMAND} -E echo "${SHELF_BASE_DIR}"
    COMMENT show shelf base dir
  )
else() # NUMA
  add_custom_target(
    radixtree_shelf_base_dir
    COMMAND ${CMAKE_COMMAND} -E make_directory "${SHELF_BASE_DIR}"
    COMMENT create shelf base dir
  )
endif()

option(WITH_TESTS "Build tests" ON)
if(WITH_TESTS)
  enable_testing()
  add_subdirectory(test)
endif()

option(WITH_EXAMPLES "Build examples" ON)
if(WITH_EXAMPLES)
  add_subdirectory(bin)
endif()


#
# Install targets
#

# the following gives an radixtreeTargets.cmake to help other projects directly use the build/ dir
export(TARGETS radixtree cluster kvs_client
	NAMESPACE radixtree::
	FILE radixtreeTargets.cmake
)
export(PACKAGE radixtree)

include(GNUInstallDirs)
set(INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_INCLUDEDIR}
    CACHE PATH "Location of header files")

install(TARGETS radixtree cluster kvs_client
        EXPORT radixtreeTargets
)

# CMake 3.23+ would have FILE_SET instead
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/"
        DESTINATION "include"
        FILES_MATCHING
        PATTERN "*.h*"
)

include(CMakePackageConfigHelpers)
install(EXPORT radixtreeTargets
  FILE radixtreeTargets.cmake
  NAMESPACE radixtree::
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/radixtree"
)

configure_package_config_file(cmake/radixtreeConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/radixtreeConfig.cmake
  INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/radixtree"
  NO_SET_AND_CHECK_MACRO
  NO_CHECK_REQUIRED_COMPONENTS_MACRO
)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/radixtreeConfig.cmake"
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/radixtree"
)
