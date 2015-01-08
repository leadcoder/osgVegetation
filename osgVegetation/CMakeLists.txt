#set lib name here!
set(LIB_NAME "osgVegetation")

include(OSGDep)

SET(CPP_FILES 
	BillboardQuadTreeScattering.cpp
	BRTGeometryShader.cpp
	BRTShaderInstancing.cpp
	MRTShaderInstancing.cpp  
	TerrainQuery.cpp
	MeshQuadTreeScattering.cpp
	VegetationUtils.cpp
)

SET(H_FILES
	BillboardData.h
	BillboardLayer.h
	BillboardObject.h
	BillboardQuadTreeScattering.h
	BRTGeometryShader.h
	BRTShaderInstancing.h
	Common.h
	IBillboardRenderingTech.h
	IMeshRenderingTech.h
	MaterialColor.h	
	MeshLayer.h
	MeshData.h
	MeshObject.h
	MeshQuadTreeScattering.h
	ITerrainQuery.h
	TerrainQuery.h
	VegetationUtils.h
)

include_directories(${OSG_INC_DIR})

#Enable class export 
add_definitions( -DOSG_VEGETATION_EXPORTS)

#Add some folders where to search for dependency libraries
link_directories(${OSG_LIB_DIR})
add_library (${LIB_NAME} SHARED  ${CPP_FILES} ${H_FILES})
target_link_libraries(${LIB_NAME} ${OSG_LINK_LIBRARIES})

INSTALL(TARGETS ${LIB_NAME}
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION bin
  ARCHIVE DESTINATION lib)

INSTALL(FILES ${H_FILES} DESTINATION include)


