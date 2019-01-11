SET(APP_NAME "sample7")
SET(CPP_FILES sample7.cpp AggregateGeometryVisitor.cpp)
SET(H_FILES AggregateGeometryVisitor.h ov_MeshTileGenerator.h ov_MeshTileGeneratorConfig.h ov_GPUCullData.h)
SET(SHADERS_FILES ov_mesh_cull_vertex.glsl
ov_mesh_cull_tess_eval.glsl
ov_mesh_cull_tess_ctrl.glsl
ov_mesh_cull_geometry.glsl
ov_mesh_cull_fragment.glsl
ov_mesh_render_vertex.glsl
ov_mesh_render_fragment.glsl)

include(OSGDep)
find_package(OpenGL)

source_group("Shaders" FILES ${SHADERS_FILES})
ADD_EXECUTABLE(${APP_NAME} ${SHADERS_FILES} ${CPP_FILES} ${H_FILES})

SET_TARGET_PROPERTIES(${APP_NAME} PROPERTIES DEBUG_POSTFIX _d)
SET_TARGET_PROPERTIES(${APP_NAME} PROPERTIES FOLDER "Samples") 
TARGET_LINK_LIBRARIES(${APP_NAME} ${OPENSCENEGRAPH_LIBRARIES}  ${OPENGL_gl_LIBRARY})
INCLUDE_DIRECTORIES(${OPENSCENEGRAPH_INCLUDE_DIRS} ${PROJECT_SOURCE_DIR}/osgVegetationRuntime)
INSTALL(TARGETS ${APP_NAME}  RUNTIME DESTINATION bin)
INSTALL(FILES ${SHADERS_FILES} DESTINATION bin)
if(WIN32)
	FILE(COPY ${SHADERS_FILES}  DESTINATION  ${CMAKE_BINARY_DIR}/out/debug)
	FILE(COPY ${SHADERS_FILES}  DESTINATION  ${CMAKE_BINARY_DIR}/out/release)
endif()






