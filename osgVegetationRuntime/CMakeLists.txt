#set lib name here!
set(LIB_NAME "osgVegetationRuntime")

include(OSGDep)

SET(CPP_FILES 
	ov_Runtime.cpp
)

SET(H_FILES
	ov_BillboardLayer.h
	ov_BillboardTile.h
	ov_Common.h
	ov_Utils.h
)

SET(SHADERS_FILES
	ov_billboard_vertex.glsl
	ov_billboard_tess_ctrl.glsl
	ov_billboard_tess_eval.glsl
	ov_billboard_geometry.glsl
	ov_billboard_fragment.glsl
	ov_terrain_detail_fragment.glsl
	#terrain_vertex.glsl
	#terrain_tess_ctrl.glsl
	#terrain_tess_eval.glsl
	#terrain_fragment.glsl
)

include_directories(${OPENSCENEGRAPH_INCLUDE_DIRS})

#Enable class export 
add_definitions( -DOSG_VEGETATION_EXPORTS)

source_group("Shaders" FILES ${SHADERS_FILES})
add_library(${LIB_NAME} SHARED  ${CPP_FILES} ${H_FILES} ${SHADERS_FILES})
target_link_libraries(${LIB_NAME} ${OPENSCENEGRAPH_LIBRARIES})

INSTALL(TARGETS ${LIB_NAME}
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION bin
  ARCHIVE DESTINATION lib)

INSTALL(FILES ${H_FILES} DESTINATION include)
INSTALL(FILES ${SHADERS_FILES} DESTINATION bin)

if(WIN32)
	FILE(COPY ${SHADERS_FILES}  DESTINATION  ${CMAKE_BINARY_DIR}/out/debug)
	FILE(COPY ${SHADERS_FILES}  DESTINATION  ${CMAKE_BINARY_DIR}/out/release)
endif()