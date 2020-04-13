#include "ov_Utils.h"
#include "ov_Scene.h"
#include "ovSampleUtils.h"
#include <osg/ArgumentParser>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>
#include <osgGA/KeySwitchMatrixManipulator>
#include <osgGA/StateSetManipulator>
#include <osgGA/TerrainManipulator>
#include <osg/Version>
#include <osgDB/FileNameUtils>

//namespace osgVegetation { GlobalRegister Register; }

int main(int argc, char** argv)
{
	osg::ArgumentParser arguments(&argc, argv);

	//Add sample data path
	osgDB::Registry::instance()->getDataFilePathList().push_back("../shaders");
	osgDB::Registry::instance()->getDataFilePathList().push_back("../sample-data");

	osgViewer::Viewer viewer(arguments);
	
	//Use multisampling
	osg::DisplaySettings::instance()->setNumMultiSamples(4);

	// Add basic event handlers and manipulators
	viewer.addEventHandler(new osgViewer::StatsHandler);
	viewer.addEventHandler(new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()));
	
	osg::ref_ptr<osgGA::KeySwitchMatrixManipulator> keyswitchManipulator = new osgGA::KeySwitchMatrixManipulator;
	keyswitchManipulator->addMatrixManipulator('1', "Trackball", new osgGA::TrackballManipulator());
	keyswitchManipulator->addMatrixManipulator('2', "Flight", new osgGA::FlightManipulator());
	keyswitchManipulator->addMatrixManipulator('3', "Drive", new osgGA::DriveManipulator());
	keyswitchManipulator->addMatrixManipulator('4', "Terrain", new osgGA::TerrainManipulator());
	viewer.setCameraManipulator(keyswitchManipulator.get());

	// create root node
	osg::ref_ptr<osg::Group> root_node= new osg::Group();
	if (true)
	{
		osg::ref_ptr<osgShadow::ShadowedScene> shadow_scene = ovSampleUtils::createShadowedSceneVDSM();
		osgVegetation::Scene::EnableShadowMapping(shadow_scene->getOrCreateStateSet(), shadow_scene->getShadowSettings()->getNumShadowMapsPerLight());
		//osgVegetation::Register.CastsShadowTraversalMask = shadow_scene->getShadowSettings()->getCastsShadowTraversalMask();
		//osgVegetation::Register.ReceivesShadowTraversalMask = 0x1;
		root_node = shadow_scene;
	}

	//add osg light
	osg::ref_ptr<osg::LightSource> light_source = ovSampleUtils::createSunLight();
	root_node->addChild(light_source);
	//enable lighting in ov
	osgVegetation::Scene::SetLighting(root_node->getOrCreateStateSet(), true);

	//add osg fog
	osg::ref_ptr<osg::Fog> fog = ovSampleUtils::createDefaultFog(osg::Fog::Mode::EXP2);
	root_node->getOrCreateStateSet()->setAttributeAndModes(fog.get());
	root_node->getOrCreateStateSet()->setMode(GL_FOG, osg::StateAttribute::ON);
	viewer.getCamera()->setClearColor(fog->getColor());

	//enable fog in ov
	osgVegetation::Scene::EnableFog(root_node->getOrCreateStateSet(), fog->getMode());

	viewer.addEventHandler(new ovSampleUtils::StateSetManipulator(root_node->getOrCreateStateSet(), fog));

	osg::ref_ptr<osg::Node> terrain = osgDB::readNodeFile("terrain.osg.ovt");

	root_node->addChild(terrain);
	
	viewer.setSceneData(root_node);
	
	viewer.setUpViewInWindow(100, 100, 800, 600);

	viewer.getCamera()->getGraphicsContext()->getState()->setUseModelViewAndProjectionUniforms(true);
	return viewer.run();
}