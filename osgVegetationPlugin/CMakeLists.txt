
set(LIB_NAME "osgdb_ovt")

set(CPP_FILES 
	ov_Serializer.cpp
	ReaderWriterVegTerrain.cpp
	tinystr.cpp
	tinyxml.cpp
	tinyxmlerror.cpp
	tinyxmlparser.cpp
)

set(H_FILES
	ov_Serializer.h
	ReaderWriterVegTerrain.h
)

add_library(${LIB_NAME} SHARED  ${CPP_FILES} ${H_FILES} ${SHADERS_FILES})
target_link_libraries(${LIB_NAME} osgVegetation)
set_target_properties(${LIB_NAME} PROPERTIES DEBUG_POSTFIX d)

install(TARGETS ${LIB_NAME}
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION bin
  ARCHIVE DESTINATION lib)

#if(WIN32)
#	file(COPY ${SHADERS_FILES}  DESTINATION  ${CMAKE_BINARY_DIR}/out/debug)
#	file(COPY ${SHADERS_FILES}  DESTINATION  ${CMAKE_BINARY_DIR}/out/release)
#endif()