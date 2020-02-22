
#include "ov_LayerGenerator.h"
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

namespace osgVegetation { osgVegetation::GlobalRegister Register; }

std::vector<osg::ref_ptr<osgVegetation::ILayerConfig> > createBillboardLayers()
{
	std::vector<osg::ref_ptr<osgVegetation::ILayerConfig> > layers;
	osg::ref_ptr <osgVegetation::BillboardLayerConfig> tree_layer = new osgVegetation::BillboardLayerConfig();
	tree_layer->Type = osgVegetation::BillboardLayerConfig::BLT_ROTATED_QUAD;
	tree_layer->MaxDistance = 740;
	tree_layer->Density = 0.001;
	tree_layer->ColorImpact = 0.0;
	tree_layer->Billboards.push_back(osgVegetation::BillboardLayerConfig::Billboard("billboards/fir01_bb.png", osg::Vec2f(10, 16), 1.0, 1.0));
	layers.push_back(tree_layer);

	osg::ref_ptr <osgVegetation::BillboardLayerConfig> grass_layer_1 = new osgVegetation::BillboardLayerConfig();
	grass_layer_1->Type = osgVegetation::BillboardLayerConfig::BLT_GRASS;
	grass_layer_1->MaxDistance = 100;
	grass_layer_1->Density = 0.1;
	grass_layer_1->ColorImpact = 0.0;
	grass_layer_1->CastShadow = false;
	grass_layer_1->Billboards.push_back(osgVegetation::BillboardLayerConfig::Billboard("billboards/veg_plant03.png", osg::Vec2f(4, 2), 1.0, 0.008));
	grass_layer_1->Billboards.push_back(osgVegetation::BillboardLayerConfig::Billboard("billboards/veg_plant01.png", osg::Vec2f(2, 2), 1.0, 0.002));
	grass_layer_1->Billboards.push_back(osgVegetation::BillboardLayerConfig::Billboard("billboards/grass1.png", osg::Vec2f(2, 1), 1.0, 1.0));
	layers.push_back(grass_layer_1);
	return layers;
}

int main(int argc, char** argv)
{
	// use an ArgumentParser object to manage the program arguments.
	osg::ArgumentParser arguments(&argc, argv);

	//Add sample data path
	osgDB::Registry::instance()->getDataFilePathList().push_back("../shaders");
	osgDB::Registry::instance()->getDataFilePathList().push_back("../sample-data");

	// construct the viewer.
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
	osg::ref_ptr<osg::Group> root_node = new osg::Group();

	
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

	
	//Create the terrain geometry and add it to scene
	osg::ref_ptr<osg::Node> terrain = ovSampleUtils::createFlatGrid(4000, 50);
	root_node->addChild(terrain);

	//create the layer generator from our configuration
	osgVegetation::LayerGenerator layer_generator(createBillboardLayers());

	//Make a clone of the terrain model and prepare the copy for tessellation (convert it to use GL_PATCHES)
	osg::ref_ptr<osg::Node> vegetation_terrain = osgVegetation::CloneAndConvertToPatches(terrain);
	osg::ref_ptr<osg::Group> vegetation_node = layer_generator.CreateVegetationNode(vegetation_terrain);
	root_node->addChild(vegetation_node);

	viewer.setSceneData(root_node);

	//viewer.setUpViewAcrossAllScreens();
	viewer.setUpViewInWindow(100, 100, 800, 600);

	viewer.getCamera()->getGraphicsContext()->getState()->setUseModelViewAndProjectionUniforms(true);
	return viewer.run();
}

