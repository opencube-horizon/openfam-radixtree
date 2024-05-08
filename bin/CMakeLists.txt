function(add_bin file_name)
  add_executable(${file_name} ${file_name}.cc)
  add_dependencies(${file_name} radixtree_shelf_base_dir)
  target_link_libraries(${file_name} radixtree cluster)
endfunction()

function(add_client_bin file_name)
  add_executable(${file_name} ${file_name}.cc)
  target_link_libraries(${file_name} kvs_client cluster)
endfunction()

add_bin(kvs)

if(WITH_KVS_CLIENT)
  add_client_bin(client)
endif()

configure_file(single_node.yaml single_node.yaml COPYONLY)
configure_file(sp.yaml sp.yaml COPYONLY)
configure_file(ms.yaml ms.yaml COPYONLY)
configure_file(dynamo.yaml dynamo.yaml COPYONLY)
configure_file(modc.yaml modc.yaml COPYONLY)
