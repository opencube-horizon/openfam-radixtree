add_compile_options(-w)  # don't generate warnings from other projects

include(cityhash.cmake)

if(WITH_KVS_CLIENT AND NOT Libmemcached_FOUND)
  include(libmemcached.cmake)
endif()

if(METRICS)
  include(medida.cmake)
endif()
