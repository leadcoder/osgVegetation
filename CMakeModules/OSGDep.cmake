
if(EXISTS "${GASS_DEPENDENCIES_DIR}/OpenSceneGraph-v3.2.1")
	set(OSG_DIR  "${GASS_DEPENDENCIES_DIR}/OpenSceneGraph-v3.2.1" CACHE PATH "OSG folder")
else()
	set(OSG_DIR $ENV{OSG_HOME} CACHE PATH "OSG folder")
endif()

set(OSG_BIN_DIR ${OSG_DIR}/bin)
set(OSG_INC_DIR ${OSG_DIR}/include)
set(OSG_LIB_DIR ${OSG_DIR}/lib)

set(OSG_LINK_LIBRARIES debug OpenThreadsd
			debug osgd
			debug osgDBd
			debug osgUtild
			debug osgGAd
			debug osgViewerd
			debug osgTextd
			debug osgShadowd
			debug osgSimd
			debug osgTerraind
			optimized OpenThreads
			optimized osg
			optimized osgDB
			optimized osgUtil
			optimized osgGA
			optimized osgViewer
			optimized osgText
			optimized osgShadow
			optimized osgSim
			optimized osgTerrain
			opengl32
			glu32)

			

set(OSG_PLUGINS_DIR_NAME osgPlugins-3.2.1)
set(OSG_PLUGINS_DIR	${OSG_BIN_DIR}/${OSG_PLUGINS_DIR_NAME})

FILE(GLOB OSG_PLUGINS "${OSG_PLUGINS_DIR}/*.dll")

set(OSG_BIN_FILES_DEBUG ${OSG_BIN_DIR}/ot20-OpenThreadsd.dll
${OSG_BIN_DIR}/zlibd.dll
${OSG_BIN_DIR}/libpng16d.dll
${OSG_BIN_DIR}/osg100-osgd.dll
${OSG_BIN_DIR}/osg100-osgDBd.dll
${OSG_BIN_DIR}/osg100-osgGAd.dll
${OSG_BIN_DIR}/osg100-osgViewerd.dll
${OSG_BIN_DIR}/osg100-osgTextd.dll
${OSG_BIN_DIR}/osg100-osgUtild.dll
${OSG_BIN_DIR}/osg100-osgSimd.dll
${OSG_BIN_DIR}/osg100-osgFXd.dll
${OSG_BIN_DIR}/osg100-osgTerraind.dll
${OSG_BIN_DIR}/osg100-osgShadowd.dll
${OSG_BIN_DIR}/osg100-osgAnimationd.dll
${OSG_BIN_DIR}/osg100-osgManipulatord.dll
${OSG_BIN_DIR}/osg100-osgParticled.dll
${OSG_BIN_DIR}/osg100-osgVolumed.dll
)

set(OSG_BIN_FILES_RELEASE ${OSG_BIN_DIR}/ot20-OpenThreads.dll
${OSG_BIN_DIR}/zlib.dll
${OSG_BIN_DIR}/libpng16.dll
${OSG_BIN_DIR}/osg100-osg.dll
${OSG_BIN_DIR}/osg100-osgDB.dll
${OSG_BIN_DIR}/osg100-osgGA.dll
${OSG_BIN_DIR}/osg100-osgViewer.dll
${OSG_BIN_DIR}/osg100-osgText.dll
${OSG_BIN_DIR}/osg100-osgUtil.dll
${OSG_BIN_DIR}/osg100-osgSim.dll
${OSG_BIN_DIR}/osg100-osgFX.dll
${OSG_BIN_DIR}/osg100-osgTerrain.dll
${OSG_BIN_DIR}/osg100-osgShadow.dll
${OSG_BIN_DIR}/osg100-osgAnimation.dll
${OSG_BIN_DIR}/osg100-osgManipulator.dll
${OSG_BIN_DIR}/osg100-osgParticle.dll
${OSG_BIN_DIR}/osg100-osgVolume.dll
)

#set(OSG_LIB_DIR ${OSG_BIN_DIR}/lib)
#set(OSG_INC_DIR ${OSG_BIN_DIR}/include)
