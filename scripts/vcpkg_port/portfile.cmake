vcpkg_fail_port_install(ON_TARGET "UWP")

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO leadcoder/osgVegetation
    REF 635ae01af898772d1608ac470b596ec210619148
    SHA512 6cf8b73eb189d7f422032a318d43ed05c717b0c142f6374c92ea032862a7c62a13695edb3539cad994816d51c29f08ef3897cf78042f9b422974310b8cb7517a
    HEAD_REF osgv-2.0
)

set(VCPKG_POLICY_DLLS_WITHOUT_LIBS enabled)
set(VCPKG_POLICY_DLLS_WITHOUT_EXPORTS enabled)

vcpkg_configure_cmake(
    SOURCE_PATH ${SOURCE_PATH}
    PREFER_NINJA
    OPTIONS
        -DOSGV_BUILD_SAMPLES=FALSE
		-DOSGV_BUILD_APPLICATIONS=FALSE
		-DOSGV_BUILD_PLUGIN=TRUE
)

vcpkg_install_cmake()

vcpkg_copy_pdbs()

vcpkg_fixup_cmake_targets(CONFIG_PATH "cmake")

file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/debug/include ${CURRENT_PACKAGES_DIR}/debug/share)

set(OSGV_TOOL_PATH_REL ${CURRENT_PACKAGES_DIR}/tools/osgvegetation)
set(OSGV_TOOL_PATH_DBG ${CURRENT_PACKAGES_DIR}/debug/tools/osgvegetation)

file(GLOB OSGV_TOOLS ${CURRENT_PACKAGES_DIR}/bin/*.exe)
if(OSGV_TOOLS)
	file(COPY ${OSGV_TOOLS} DESTINATION ${OSGV_TOOL_PATH_REL})
	file(REMOVE_RECURSE ${OSGV_TOOLS})
endif()

file(GLOB OSGV_TOOLS_DBG ${CURRENT_PACKAGES_DIR}/debug/bin/*.exe)

if(OSGV_TOOLS_DBG)
	file(REMOVE_RECURSE ${OSGV_TOOLS_DBG})
endif()

#move cmake-files to share
file(GLOB OSGV_CMAKE_FILES ${CURRENT_PACKAGES_DIR}/cmake/*.cmake)
file(COPY ${OSGV_CMAKE_FILES} DESTINATION ${CURRENT_PACKAGES_DIR}/share/osgvegetation/cmake)
file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/cmake ${CURRENT_PACKAGES_DIR}/debug/cmake)

#move shader-files
file(GLOB OSGV_SHADER_FILES ${CURRENT_PACKAGES_DIR}/shaders/*.*)
file(COPY ${OSGV_SHADER_FILES} DESTINATION ${CURRENT_PACKAGES_DIR}/share/osgvegetation/shaders)
file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/shaders ${CURRENT_PACKAGES_DIR}/debug/shaders)

#install copyright
file(INSTALL ${SOURCE_PATH}/README.md DESTINATION ${CURRENT_PACKAGES_DIR}/share/${PORT} RENAME copyright)

