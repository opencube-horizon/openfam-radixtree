# enable GTest
#
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/58d77fa8070e8cec2dc1ed015d66b454c8d78850.zip  # 1.12.1, the last release supporting C++11
  #  FIND_PACKAGE_ARGS NAMES GTest  # this would tell CMake to first look for an installed GTest, but requires CMake 3.28+
  #  EXCLUDE_FROM_ALL  # for CMake 3.28+ this can be used instead of the FetchContent_Properties below
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# FetchContent_MakeAvailable(googletest)  # for CMake 3.28+ use this instead of the block below
FetchContent_GetProperties(googletest)
if(NOT googletest_POPULATED)
  FetchContent_Populate(googletest)
  add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR} EXCLUDE_FROM_ALL)
  target_compile_options(gtest PRIVATE "-w")
  target_compile_options(gtest_main PRIVATE "-w")
endif()

include(GoogleTest)

add_library(test_common OBJECT common.cc)
target_link_libraries(test_common
  nvmm::nvmm
  Boost::log
  gtest
)

function(add_radixtree_test file_name)
  add_executable(${file_name} ${file_name}.cc)
  add_dependencies(${file_name} radixtree_shelf_base_dir)

  target_link_libraries(${file_name}
    test_common
    radixtree
    cluster
    gtest
    nvmm::nvmm
    Boost::log
    $<$<BOOL:${METRICS}>:Medida::medida>
  )
  gtest_discover_tests(${file_name})
endfunction()

add_radixtree_test(test_radix_tree)
add_radixtree_test(test_split_ordered)
add_radixtree_test(test_kvs)
add_radixtree_test(test_cluster)

configure_file(test_cluster.yaml test_cluster.yaml COPYONLY)
