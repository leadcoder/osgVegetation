
set(LIB_NAME "osgdb_ovt")

set(CPP_FILES 
	ReaderWriterVegTerrain.cpp
)

set(H_FILES
	ReaderWriterVegTerrain.h
)

add_library(${LIB_NAME} SHARED  ${CPP_FILES} ${H_FILES} ${SHADERS_FILES})
target_link_libraries(${LIB_NAME} osgVegetation)
set_target_properties(${LIB_NAME} PROPERTIES DEBUG_POSTFIX d)
set_target_properties(${LIB_NAME} PROPERTIES PREFIX "")

install(TARGETS ${LIB_NAME} RUNTIME DESTINATION bin)
