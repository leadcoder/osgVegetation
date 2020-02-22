
if(NOT OPENSCENEGRAPH_FOUND)
  return()
endif()

macro(osg_extract_version var str)
STRING(REGEX REPLACE ".*VERSION\ *([0-9]+)" "\\1" ${var} ${str})
endmacro(osg_extract_version)

find_file(OSG_VERSION_FILE osg/Version ${OPENSCENEGRAPH_INCLUDE_DIRS})

if(NOT OSG_VERSION_FILE)
	message("Found no osg version header")
else() 
	file(STRINGS "${OSG_VERSION_FILE}" OSG_MAJOR REGEX "#define OPENSCENEGRAPH_MAJOR_VERSION")
	file(STRINGS "${OSG_VERSION_FILE}" OSG_MINOR REGEX "#define OPENSCENEGRAPH_MINOR_VERSION")
	file(STRINGS "${OSG_VERSION_FILE}" OSG_PATCH REGEX "#define OPENSCENEGRAPH_PATCH_VERSION")
	file(STRINGS "${OSG_VERSION_FILE}" OSG_SOVERSION REGEX "#define OPENSCENEGRAPH_SOVERSION")
	osg_extract_version(OSG_MAJOR ${OSG_MAJOR})
	osg_extract_version(OSG_MINOR ${OSG_MINOR})
	osg_extract_version(OSG_PATCH ${OSG_PATCH})
	osg_extract_version(OSG_SOVERSION ${OSG_SOVERSION})
	#set(OSG_VERSION "${OSG_MAJOR}.${OSG_MINOR}.${OSG_PATCH}" CACHE STRING "OpenSceneGraph Version")
	#set(OSG_SHARED_PREFIX "osg${OSG_SOVERSION}-" CACHE STRING "OpenSceneGraph so-version")
	
	#try to find binary folder
	if (WIN32)
		set(OSG_SHARED_LIB_EXT .dll)
	else() #assume linux
		set(OSG_SHARED_LIB_EXT .so)
	endif()

	#move this to helper file
	find_path(OSG_BIN_DIR_REL NAMES osg${OSG_SOVERSION}-osg${OSG_SHARED_LIB_EXT} HINTS ${OPENSCENEGRAPH_INCLUDE_DIRS}/../ PATH_SUFFIXES bin)
	find_path(OSG_BIN_DIR_DBG NAMES osg${OSG_SOVERSION}-osgd${OSG_SHARED_LIB_EXT} HINTS ${OPENSCENEGRAPH_INCLUDE_DIRS}/../ PATH_SUFFIXES bin)
    set(OSG_PLUGIN_FOLDER_NAME "osgPlugins-${OSG_MAJOR}.${OSG_MINOR}.${OSG_PATCH}")
	find_path(OSG_PLUGIN_DIR_REL NAMES osgdb_3ds${OSG_SHARED_LIB_EXT} 
		HINTS ${OPENSCENEGRAPH_INCLUDE_DIRS}/../ 
		PATH_SUFFIXES bin bin/${OSG_PLUGIN_FOLDER_NAME} tools/osg/${OSG_PLUGIN_FOLDER_NAME})
		
	find_path(OSG_PLUGIN_DIR_DBG NAMES osgdb_3dsd${OSG_SHARED_LIB_EXT} 
		HINTS ${OPENSCENEGRAPH_INCLUDE_DIRS}/../ 
		PATH_SUFFIXES bin bin/${OSG_PLUGIN_FOLDER_NAME} tools/osg/${OSG_PLUGIN_FOLDER_NAME})
endif()