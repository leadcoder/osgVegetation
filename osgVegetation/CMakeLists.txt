#set lib name here!
set(LIB_NAME "osgVegetation")

set(H_FILES
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
	ov_BillboardMultiLayerEffect.h
	ov_BillboardLayerStateSet.h
	ov_PassFilter.h
	AggregateGeometryVisitor.h
	ov_LayerGenerator.h
	ov_MeshLayerGenerator.h 
	ov_MeshLayerConfig.h 
	ov_GPUCullData.h
	ov_CustomVegetationInjection.h
	ov_XMLUtils.h
	ov_XMLLayerReader.h
	ov_XMLTerrainShadingReader.h
	ov_XMLVPBReader.h
)

set(SHADERS_FILES
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
target_include_directories(${LIB_NAME} INTERFACE 
	$<BUILD_INTERFACE:${OPENSCENEGRAPH_INCLUDE_DIRS}>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
	$<INSTALL_INTERFACE:${OSGV_INCLUDE_INSTALL_DIR}>)

target_link_libraries(${LIB_NAME} INTERFACE ${OPENSCENEGRAPH_LIBRARIES})

if(MSVC)
	source_group("Shaders" FILES ${SHADERS_FILES})
    add_custom_target(${LIB_NAME}.IDE SOURCES ${H_FILES} ${SHADERS_FILES})
endif()

install(TARGETS ${LIB_NAME} EXPORT ${LIB_NAME}Target)

install(EXPORT ${LIB_NAME}Target
       FILE ${LIB_NAME}Target.cmake
      NAMESPACE osgv::
      DESTINATION cmake)

install(FILES ${H_FILES} DESTINATION ${OSGV_INCLUDE_INSTALL_DIR})
install(FILES ${SHADERS_FILES} DESTINATION shaders)
install(FILES ${PROJECT_SOURCE_DIR}/CMakeModules/osgvegetation-config.cmake DESTINATION cmake)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/osgvegetation-config-version.cmake
  VERSION ${OSGV_VERSION_MAJOR}
  COMPATIBILITY SameMajorVersion)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/osgvegetation-config-version.cmake DESTINATION cmake)

if(WIN32)
	file(COPY ${SHADERS_FILES}  DESTINATION  ${CMAKE_BINARY_DIR}/out/shaders)
endif()