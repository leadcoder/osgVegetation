
add_subdirectory(sample-billboard-layers)
add_subdirectory(sample-mesh-layers)

add_subdirectory(sample1)
add_subdirectory(sample2)
add_subdirectory(sample3)
add_subdirectory(sample4)
add_subdirectory(sample7)
add_subdirectory(sample8)
add_subdirectory(sample9)

add_subdirectory(test)
add_subdirectory(test2)

#install data
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/sample-data/ DESTINATION sample-data)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/sample-data  DESTINATION  ${CMAKE_BINARY_DIR}/out) 



