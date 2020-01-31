#set lib name here!
set(LIB_NAME "osgVegetation")

SET(H_FILES
	ov_ILayerConfig.h
	ov_BillboardLayerConfig.h
	ov_Common.h
	ov_Utils.h
	ov_VPBVegetationInjection.h
	ov_VPBVegetationInjectionConfig.h
	ov_Register.h
	ov_TextureConfig.h
	ov_TerrainShadingStateSet.h
	ov_TerrainSplatShadingStateSet.h
	ov_TerrainStateSet.h
	ov_TextureUnits.h
	ov_Scene.h
	ov_Shadow.h
	ov_Fog.h
	ov_BillboardMultiLayerEffect.h
	ov_BillboardLayerStateSet.h
	ov_PassFilter.h
	AggregateGeometryVisitor.h
	ov_LayerGenerator.h
	ov_MeshLayerGenerator.h 
	ov_MeshLayerConfig.h 
	ov_GPUCullData.h
	ov_CustomVegetationInjection.h
)

SET(SHADERS_FILES
	ov_billboard_vertex.glsl
	ov_billboard_tess_ctrl.glsl
	ov_billboard_tess_eval.glsl
	ov_billboard_geometry.glsl
	ov_billboard_fragment.glsl
	ov_common_vertex.glsl
	ov_common_fragment.glsl
	ov_terrain_color.glsl
	ov_terrain_elevation.glsl
	ov_terrain_vertex.glsl
	ov_terrain_tess_eval.glsl
	ov_terrain_tess_ctrl.glsl
	ov_terrain_fragment.glsl
	ov_terrain_pass_filter.glsl
	ov_mesh_cull_vertex.glsl
	ov_mesh_cull_tess_eval.glsl
	ov_mesh_cull_tess_ctrl.glsl
	ov_mesh_cull_geometry.glsl
	ov_mesh_cull_fragment.glsl
	ov_mesh_render_vertex.glsl
	ov_mesh_render_fragment.glsl
)

cmake_policy(SET CMP0076 NEW)

add_library(${LIB_NAME} INTERFACE)
target_include_directories(${LIB_NAME} INTERFACE $<BUILD_INTERFACE:${OPENSCENEGRAPH_INCLUDE_DIRS}>)
target_include_directories(${LIB_NAME} INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_sources(${LIB_NAME} INTERFACE $<BUILD_INTERFACE:${H_FILES};${SHADERS_FILES}>)
target_link_libraries(${LIB_NAME} INTERFACE ${OPENSCENEGRAPH_LIBRARIES})

if(MSVC)
	source_group("Shaders" FILES ${SHADERS_FILES})
    add_custom_target(${LIB_NAME}.IDE SOURCES ${H_FILES} ${SHADERS_FILES})
endif()

#  INSTALL(TARGETS ${LIB_NAME}
#  RUNTIME DESTINATION bin
#  LIBRARY DESTINATION bin
#  ARCHIVE DESTINATION lib)

INSTALL(FILES ${H_FILES} DESTINATION include)
INSTALL(FILES ${SHADERS_FILES} DESTINATION bin)

if(WIN32)
	FILE(COPY ${SHADERS_FILES}  DESTINATION  ${CMAKE_BINARY_DIR}/out/debug)
	FILE(COPY ${SHADERS_FILES}  DESTINATION  ${CMAKE_BINARY_DIR}/out/release)
endif()