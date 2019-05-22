#include "ov_VPBVegetationInjection.h"
#include "ov_Utils.h"
#include "ov_TerrainSplatShadingStateSet.h"
#include <osg/ArgumentParser>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>
#include <osgGA/KeySwitchMatrixManipulator>
#include <osgGA/StateSetManipulator>
#include <osgGA/AnimationPathManipulator>
#include <osgGA/TerrainManipulator>
#include <osg/Version>
#include <osg/Fog>
#include "ov_DemoShadow.h"
#include <iostream>

int main(int argc, char** argv)
{
	osgVegetation::SceneConfiguration config;
	config.ShadowMode = osgVegetation::SM_LISPSM;
	config.FogMode = osgVegetation::FM_EXP2;

	osg::ArgumentParser arguments(&argc, argv);

	// construct the viewer.
	osgViewer::Viewer viewer(arguments);

	// set up the camera manipulators.
	{
		osg::ref_ptr<osgGA::KeySwitchMatrixManipulator> keyswitchManipulator = new osgGA::KeySwitchMatrixManipulator;

		keyswitchManipulator->addMatrixManipulator('1', "Trackball", new osgGA::TrackballManipulator());
		keyswitchManipulator->addMatrixManipulator('2', "Flight", new osgGA::FlightManipulator());
		keyswitchManipulator->addMatrixManipulator('3', "Drive", new osgGA::DriveManipulator());
		keyswitchManipulator->addMatrixManipulator('4', "Terrain", new osgGA::TerrainManipulator());
		viewer.setCameraManipulator(keyswitchManipulator.get());
	}

	// add the state manipulator
	viewer.addEventHandler(new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()));

	// add the stats handler
	viewer.addEventHandler(new osgViewer::StatsHandler);

	// add the record camera path handler
	viewer.addEventHandler(new osgViewer::RecordCameraPathHandler);

	// add the window size toggle handler
	viewer.addEventHandler(new osgViewer::WindowSizeHandler);

	osg::DisplaySettings::instance()->setNumMultiSamples(4);

	//Add sample data path
	osgDB::Registry::instance()->getDataFilePathList().push_back("../data");

	//setup vegetation layers
	std::vector<osgVegetation::BillboardLayer> layers;
	osgVegetation::BillboardLayer grass_data(140, 0.2, 1.0, 0.1, 5);
	grass_data.Type = osgVegetation::BillboardLayer::BLT_GRASS;
	grass_data.Billboards.push_back(osgVegetation::BillboardLayer::Billboard("billboards/veg_plant03.png", osg::Vec2f(4, 2), 0.9, 0.008));
	grass_data.Billboards.push_back(osgVegetation::BillboardLayer::Billboard("billboards/veg_plant01.png", osg::Vec2f(2, 2), 0.9, 0.002));
	grass_data.Billboards.push_back(osgVegetation::BillboardLayer::Billboard("billboards/grass2.png", osg::Vec2f(2, 1), 1.0, 1.0));
	osg::Vec4 grass_splat_color_filter(0.1, -1, -1, -1);
	grass_data.Filter.SplatFilter = osgVegetation::PassFilter::GenerateSplatFilter(grass_splat_color_filter, "<");

	layers.push_back(grass_data);

	
	osgVegetation::BillboardLayer tree_data(740, 0.004, 0.7, 0.1, 2);
	tree_data.Type = osgVegetation::BillboardLayer::BLT_ROTATED_QUAD;
	tree_data.Filter.ColorFilter = "if(length(base_color.xyz) > 0.5) return false;";
	tree_data.Billboards.push_back(osgVegetation::BillboardLayer::Billboard("billboards/fir01_bb.png", osg::Vec2f(10, 16), 1.0, 1.0));
	//tree_data.Billboards.push_back(osgVegetation::BillboardLayer::Billboard("billboards/tree0.rgba", osg::Vec2f(8, 16), 1.2));
	layers.push_back(tree_data);

	osg::ref_ptr<osg::Group> root_node = CreateShadowNode(config.ShadowMode);

	//Setup texture units
	//osgVegetation::TerrainTextureUnitSettings terrain_tu;
	//terrain_tu.ColorTextureUnit = 0;
	//terrain_tu.SplatTextureUnit = 1;

	osgVegetation::BillboardNodeGeneratorConfig bbconfig(layers,12);

	osgDB::Registry::instance()->setReadFileCallback(new osgVegetation::VPBVegetationInjection(bbconfig));
#if 0
	osg::ref_ptr<osg::Node> terrain_node = osgDB::readNodeFile("terrain/us-terrain.zip/us-terrain.osg");
#else
	//osg::ref_ptr<osg::Node> terrain_node = osgDB::readNodeFile("D:/terrain/vpb/us/final/us-terrain.osgb");
	osg::ref_ptr<osg::Node> terrain_node = osgDB::readNodeFile("D:/terrain/vpb/us/final/us-terrain.osg");
#endif
	
	osgVegetation::TerrainSplatShadingConfig terrain_shading;
	terrain_shading.ColorTexture.TexUnit = 0;
	terrain_shading.SplatTexture.TexUnit = 1;
	//terrain_shading.DetailLayers.push_back(osgVegetation::DetailLayer(std::string("terrain/detail/detail_grass_mossy.dds"), 0.2));
	//terrain_shading.DetailLayers.push_back(osgVegetation::DetailLayer(std::string("terrain/detail/detail_dirt.dds"), 0.2));
	//terrain_shading.DetailLayers.push_back(osgVegetation::DetailLayer(std::string("terrain/detail/detail_grass_mossy.dds"), 0.2));
	//terrain_shading.DetailLayers.push_back(osgVegetation::DetailLayer(std::string("terrain/detail/detail_dirt.dds"), 0.2));
	//terrain_shading.DetailTextureUnit = 2;

	terrain_shading.DetailLayers.push_back(osgVegetation::DetailLayer(std::string("terrain/detail/detail_dirt.dds"), 0.05));
	terrain_shading.DetailLayers.push_back(osgVegetation::DetailLayer(std::string("terrain/detail/detail_grass_mossy.dds"), 0.05));
	terrain_shading.DetailLayers.push_back(osgVegetation::DetailLayer(std::string("terrain/detail/detail_grass_mossy.dds"), 0.05));
	terrain_shading.DetailLayers.push_back(osgVegetation::DetailLayer(std::string("terrain/detail/detail_grass_mossy.dds"), 0.05));
	terrain_shading.DetailTextureUnit = 3;
	terrain_shading.NoiseTexture = osgVegetation::TextureConfig("terrain/detail/noise.png", 4);

	osg::ref_ptr <osgVegetation::TerrainSplatShadingStateSet> terrain_shading_ss = new osgVegetation::TerrainSplatShadingStateSet(terrain_shading);
	osg::ref_ptr<osg::Group> terrain_shading_effect = new osg::Group();
	terrain_shading_effect->setStateSet(terrain_shading_ss);
	terrain_shading_effect->addChild(terrain_node);
	

	root_node->addChild(terrain_shading_effect);
	
	if (!terrain_node)
	{
		osg::notify(osg::NOTICE) << "Warning: no valid data loaded, please specify a database on the command line." << std::endl;
		return 1;
	}

	//Add directional light source
	osg::Light* light = new osg::Light;
	light->setDiffuse(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
	osg::Vec4 light_pos(1, 1.0, 1, 0);
	light->setPosition(light_pos);		// last param	w = 0.0 directional light (direction)
	osg::Vec3f light_dir(-light_pos.x(), -light_pos.y(), -light_pos.z());
	light_dir.normalize();
	light->setDirection(light_dir);
	light->setAmbient(osg::Vec4(0.4f, 0.4f, 0.4f, 1.0f));
	//light->setSpecular(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));

	osg::LightSource* light_source = new osg::LightSource;
	light_source->setLight(light);
	root_node->addChild(light_source);

	if (config.FogMode != osgVegetation::FM_DISABLED) //Add fog effect?
	{
		const osg::Vec4 fog_color(0.5, 0.6, 0.7, 1.0);
		osg::StateSet* state = root_node->getOrCreateStateSet();
		osg::ref_ptr<osg::Fog> fog = new osg::Fog();
		state->setMode(GL_FOG, osg::StateAttribute::ON);
		state->setAttributeAndModes(fog.get());
		fog->setMode(osg::Fog::Mode(config.FogMode));
		fog->setDensity(0.0005);
		fog->setColor(fog_color);
		viewer.getCamera()->setClearColor(fog_color);
	}

	osgVegetation::SetSceneDefinitions(root_node->getOrCreateStateSet(), config);

	// add a viewport to the viewer and attach the scene graph.
	viewer.setSceneData(root_node.get());
	viewer.setUpViewInWindow(100, 100, 800, 600);
	viewer.getCamera()->getGraphicsContext()->getState()->setUseModelViewAndProjectionUniforms(true);
	viewer.getCamera()->getGraphicsContext()->getState()->setUseVertexAttributeAliasing(true);
	// run the viewers main loop
	while (!viewer.done())
	{
		float t = viewer.getFrameStamp()->getSimulationTime() * 0.5;
		light_pos.set(sinf(t), cosf(t), 1, 0.0f);
		light->setPosition(light_pos);
		light_dir.set(-light_pos.x(), -light_pos.y(), -light_pos.z());
		light_dir.normalize();
		light->setDirection(light_dir);
		viewer.frame();
	}
	return 0;
}
