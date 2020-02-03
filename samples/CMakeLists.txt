#Copy sample data to out folder
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/data  DESTINATION  ${CMAKE_BINARY_DIR}/out) 

add_subdirectory(sample1)
add_subdirectory(sample2)
add_subdirectory(sample3)
add_subdirectory(sample4)
add_subdirectory(sample7)
add_subdirectory(sample8)
add_subdirectory(sample9)

add_subdirectory(test)
add_subdirectory(test2)

#copy sample data to install folder
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data DESTINATION ./)



