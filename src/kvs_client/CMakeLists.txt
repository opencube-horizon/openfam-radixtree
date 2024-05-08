add_library(kvs_client SHARED 
  kvs_client.cc
)

target_link_libraries(kvs_client
  cluster
  Libmemcached::memcached
)
