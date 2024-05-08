
add_library(radixtree SHARED
  failinj.cc
  kvs_metrics_config.cc
  kvs_radix_tree.cc
  kvs_radix_tree_tiny.cc
  kvs_split_ordered.cc
  kvs.cc
  radix_tree.cc
  split_ordered.cc
)

target_link_libraries(radixtree
  PUBLIC nvmm::nvmm
  PRIVATE cityhash $<$<BOOL:${METRICS}>:Medida::medida>
)

target_include_directories(radixtree PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include/>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/>
  $<INSTALL_INTERFACE:include>
)

add_subdirectory(cluster)

if(WITH_KVS_CLIENT)
  add_subdirectory(kvs_client)
endif()

if(USE_QUARTZ)
  find_package(Quartz)
  target_link_libraries(radixtree ${QUARTZ_LIBRARY})
endif()
