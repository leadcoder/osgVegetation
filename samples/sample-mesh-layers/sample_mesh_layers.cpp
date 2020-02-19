
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

namespace osgVegetation { GlobalRegister Register; }

std::vector<osg::ref_ptr<osgVegetation::ILayerConfig> > createBillboardLayers()
{
	std::vector<osg::ref_ptr<osgVegetation::ILayerConfig> > layers;
	
	osg::ref_ptr <osgVegetation::MeshLayerConfig> tree_layer = new osgVegetation::MeshLayerConfig(1000);
	tree_layer->BackFaceCulling = true;
	tree_layer->CastShadow = false;
	tree_layer->ReceiveShadow = false;
	osgVegetation::MeshTypeConfig mesh;
	const float end_dist = 1000.0f;
	const float mix_dist = 10.0f;
	const float lod_dist = 80.0f;
	//mesh.MeshLODs.push_back(osgVegetation::MeshTypeConfig::MeshLODConfig("trees/maple/maple.obj", osg::Vec4(0, 0, 50, 61), 0U, 1.0));
	//mesh.MeshLODs.push_back(osgVegetation::MeshTypeConfig::MeshLODConfig("trees/maple/maple_bb.osg", osg::Vec4(50, 60, end_dist, end_dist + 10), 1, 1.0));
	mesh.MeshLODs.push_back(osgVegetation::MeshTypeConfig::MeshLODConfig("trees/fir01_l0.osg", osg::Vec4(0.0f, 0.0f, lod_dist, lod_dist + mix_dist)));
	mesh.MeshLODs.push_back(osgVegetation::MeshTypeConfig::MeshLODConfig("trees/fir01_l1.osg", osg::Vec4(lod_dist - mix_dist, lod_dist + mix_dist, end_dist-100, end_dist)));
	mesh.DiffuseIntensity = 1.0;
	mesh.IntensityVariation = 0;
	tree_layer->MeshTypes.push_back(mesh);
	layers.push_back(tree_layer);
	return layers;
}

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
		osgVegetation::Register.CastsShadowTraversalMask = shadow_scene->getShadowSettings()->getCastsShadowTraversalMask();
		osgVegetation::Register.ReceivesShadowTraversalMask = 0x1;
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

