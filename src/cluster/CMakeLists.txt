add_library(cluster SHARED
  config.cc
  cluster.cc
  resource_manager.cc
  partition_manager.cc
  node_manager.cc
  server_manager.cc
)

target_link_libraries(cluster
  ${YAML_CPP_LIBRARIES}
  $<$<BOOL:${METRICS}>:${MEDIDA_LIBRARY}>
)
target_include_directories(cluster PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../include/>
  $<INSTALL_INTERFACE:include>
)
