#set lib name here!
set(LIB_NAME "osgdb_ovt")

include(OSGDep)

SET(CPP_FILES 
	ov_Serializer.cpp
	ReaderWriterVegTerrain.cpp
	tinystr.cpp
	tinyxml.cpp
	tinyxmlerror.cpp
	tinyxmlparser.cpp
)

SET(H_FILES
	ov_Serializer.h
	ReaderWriterVegTerrain.h
)

SET(CMAKE_DEBUG_POSTFIX  "d")

include_directories(${OPENSCENEGRAPH_INCLUDE_DIRS} ${PROJECT_SOURCE_DIR}/osgVegetation)

#Enable class export 
#add_definitions( -DOSG_VEGETATION_EXPORTS)

add_library(${LIB_NAME} SHARED  ${CPP_FILES} ${H_FILES} ${SHADERS_FILES})
target_link_libraries(${LIB_NAME} ${OPENSCENEGRAPH_LIBRARIES})

INSTALL(TARGETS ${LIB_NAME}
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION bin
  ARCHIVE DESTINATION lib)

INSTALL(FILES ${H_FILES} DESTINATION include)

if(WIN32)
	FILE(COPY ${SHADERS_FILES}  DESTINATION  ${CMAKE_BINARY_DIR}/out/debug)
	FILE(COPY ${SHADERS_FILES}  DESTINATION  ${CMAKE_BINARY_DIR}/out/release)
endif()