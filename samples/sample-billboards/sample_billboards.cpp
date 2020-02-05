#include "ov_LayerGenerator.h"
#include "ov_Utils.h"
#include <osg/ArgumentParser>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgGA/TrackballManipulator>
#include <osgGA/KeySwitchMatrixManipulator>
#include <osgGA/StateSetManipulator>
#include <osgGA/TerrainManipulator>
#include <osg/Version>
#include "ov_Demo.h"
#include "ov_DemoTerrain.h"

namespace osgVegetation 
{
	GlobalRegister Register;
}

std::vector<osg::ref_ptr<osgVegetation::ILayerConfig> > createBillboardLayers()
{
	std::vector<osg::ref_ptr<osgVegetation::ILayerConfig> > layers;
	osg::ref_ptr <osgVegetation::BillboardLayerConfig> tree_layer = new osgVegetation::BillboardLayerConfig();
	tree_layer->MaxDistance = 740;
	tree_layer->Density = 0.001;
	tree_layer->ColorImpact = 0.0;
	tree_layer->Type = osgVegetation::BillboardLayerConfig::BLT_ROTATED_QUAD;
	tree_layer->Billboards.push_back(osgVegetation::BillboardLayerConfig::Billboard("billboards/fir01_bb.png", osg::Vec2f(10, 16), 1.0, 1.0));
	layers.push_back(tree_layer);
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
	osg::ref_ptr<osg::Group> root = new osg::Group;

	osg::ref_ptr<osg::Group> root_node = new osg::Group();

	//Create the terrain geometry and add it to scene
	osg::ref_ptr<osg::Node> terrain = createFlatGrid(4000, 50);
	root_node->addChild(terrain);

	std::vector < osg::ref_ptr<osgVegetation::ILayerConfig> > layers =  createBillboardLayers();
	osgVegetation::LayerGenerator generator(layers);

	//Make a clone of the terrain model and prepare the copy for tessellation (convert it to use GL_PATCHES)
	osg::ref_ptr<osg::Node> vegetation_terrain = osgVegetation::CloneAndConvertToPatches(terrain);
	osg::ref_ptr<osg::Group> vegetation_node = generator.CreateVegetationNode(vegetation_terrain);
	root_node->addChild(vegetation_node);

	viewer.setSceneData(root_node);

	viewer.setUpViewInWindow(100, 100, 800, 600);

	viewer.getCamera()->getGraphicsContext()->getState()->setUseModelViewAndProjectionUniforms(true);
	return viewer.run();
}

