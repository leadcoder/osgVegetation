cmake_minimum_required(VERSION 3.13)

set(OSGV_VERSION_MAJOR 2)
set(OSGV_VERSION_MINOR 0)
set(OSGV_VERSION_PATCH 0)
set(OSGV_VERSION ${OSGV_VERSION_MAJOR}.${OSGV_VERSION_MINOR}.${OSGV_VERSION_PATCH})

project(osgVegetation VERSION ${OSGV_VERSION})

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_MODULE_PATH "${osgVegetation_SOURCE_DIR}/CMakeModules;${CMAKE_MODULE_PATH}")
include(osgv-macros)

find_package(OpenSceneGraph REQUIRED osgUtil osgDB osgGA osgText osgShadow osgViewer osgSim osgTerrain)
include(osg-version)

option(OSGV_BUILD_SAMPLES "Build sample" ON)
option(OSGV_BUILD_PLUGIN "Build plugin" ON)
option(OSGV_BUILD_APPLICATIONS "Build applications" ON)

set(OSGV_INCLUDE_INSTALL_DIR include/osgVegetation)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/out/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/out/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/out/bin)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/out/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/out/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/out/bin)

set(CMAKE_DEBUG_POSTFIX  "d" CACHE STRING "add a debug postfix, usually d on windows")

add_subdirectory(osgVegetation)

if(OSGV_BUILD_PLUGIN)
	add_subdirectory(osgVegetationPlugin)
endif()

if(OSGV_BUILD_APPLICATIONS)
	add_subdirectory(applications)
endif()

if(OSGV_BUILD_SAMPLES)
	add_subdirectory(samples)
endif()
