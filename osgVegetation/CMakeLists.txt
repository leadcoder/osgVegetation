#set lib name here!
set(LIB_NAME "osgVegetation")

include(OSGDep)

SET(CPP_FILES 
	BillboardQuadTreeScattering.cpp
	BRTGeometryShader.cpp
	BRTShaderInstancing.cpp
	MRTShaderInstancing.cpp
	Serializer.cpp	
	TerrainQuery.cpp
	MeshQuadTreeScattering.cpp
	VegetationUtils.cpp
	tinystr.cpp
	tinyxml.cpp
	tinyxmlerror.cpp
	tinyxmlparser.cpp
)

SET(H_FILES
	BillboardData.h
	BillboardLayer.h
	BillboardObject.h
	BillboardQuadTreeScattering.h
	BRTGeometryShader.h
	BRTShaderInstancing.h
	Common.h
	CoverageColor.h
	CoverageData.h
	EnvironmentSettings.h
	IBillboardRenderingTech.h
	IMeshRenderingTech.h
	MeshLayer.h
	MeshData.h
	MeshObject.h
	MeshQuadTreeScattering.h
	MRTShaderInstancing.h
	Serializer.h
	ITerrainQuery.h
	TerrainQuery.h
	VegetationUtils.h
	Tessellation.h
)

SET(SHADERS_FILES
	shaders/brt_fragment.glsl
	shaders/brt_geometry.glsl
	shaders/brt_vertex.glsl
	shaders/veg_vertex.glsl
	shaders/veg_tess_ctrl.glsl
	shaders/veg_tess_eval.glsl
	shaders/veg_geometry.glsl
	shaders/veg_fragment.glsl
	shaders/terrain_vertex.glsl
	shaders/terrain_tess_ctrl.glsl
	shaders/terrain_tess_eval.glsl
	shaders/terrain_fragment.glsl
)

include_directories(${OPENSCENEGRAPH_INCLUDE_DIRS})

#Enable class export 
add_definitions( -DOSG_VEGETATION_EXPORTS)

add_library (${LIB_NAME} SHARED  ${CPP_FILES} ${H_FILES})
target_link_libraries(${LIB_NAME} ${OPENSCENEGRAPH_LIBRARIES})

INSTALL(TARGETS ${LIB_NAME}
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION bin
  ARCHIVE DESTINATION lib)

INSTALL(FILES ${H_FILES} DESTINATION include)
INSTALL(FILES ${SHADERS_FILES} DESTINATION bin/shaders)

if(WIN32)
	FILE(COPY ${SHADERS_FILES}  DESTINATION  ${CMAKE_BINARY_DIR}/out/debug/shaders)
	FILE(COPY ${SHADERS_FILES}  DESTINATION  ${CMAKE_BINARY_DIR}/out/release/shaders)
endif()