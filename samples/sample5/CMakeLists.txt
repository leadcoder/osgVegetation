SET(APP_NAME "sample5")
SET(CPP_FILES sample5.cpp)
SET(H_FILES XBFVegetationTile.h XBFVegetationData.h XBFInstanceGenerator.h XBFInstance.h)
SET(SHADERS_FILES
	xbf_points_tess_ctrl.glsl
	xbf_points_tess_eval.glsl
	xbf_points_geometry.glsl
	xbf_points_vertex.glsl
	xbf_render_vertex.glsl
	xbf_render_fragment.glsl
	sample_terrain_fragment.glsl
)

include(OSGDep)
find_package(OpenGL)

source_group("Shaders" FILES ${SHADERS_FILES})
ADD_EXECUTABLE(${APP_NAME} ${SHADERS_FILES} ${CPP_FILES} ${H_FILES})

SET_TARGET_PROPERTIES(${APP_NAME} PROPERTIES DEBUG_POSTFIX _d)
SET_TARGET_PROPERTIES(${APP_NAME} PROPERTIES FOLDER "Samples") 
TARGET_LINK_LIBRARIES(${APP_NAME} ${OPENSCENEGRAPH_LIBRARIES}  ${OPENGL_gl_LIBRARY} osgVegetation)
INCLUDE_DIRECTORIES(${OPENSCENEGRAPH_INCLUDE_DIRS} ${PROJECT_SOURCE_DIR}/osgVegetation)
INSTALL(TARGETS ${APP_NAME}  RUNTIME DESTINATION bin)
INSTALL(FILES ${SHADERS_FILES} DESTINATION bin)
if(WIN32)
	FILE(COPY ${SHADERS_FILES}  DESTINATION  ${CMAKE_BINARY_DIR}/out/debug)
	FILE(COPY ${SHADERS_FILES}  DESTINATION  ${CMAKE_BINARY_DIR}/out/release)
endif()






