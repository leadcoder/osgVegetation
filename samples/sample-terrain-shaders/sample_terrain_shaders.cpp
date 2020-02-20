
#include "ov_LayerGenerator.h"
#include "ov_Utils.h"
#include "ov_Scene.h"
#include "ov_TerrainSplatShadingStateSet.h"
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

std::vector<osg::ref_ptr<osgVegetation::ILayerConfig> > createVegetionConfig()
{
	std::vector<osg::ref_ptr<osgVegetation::ILayerConfig> > layers;
	
	osg::ref_ptr <osgVegetation::MeshLayerConfig> tree_layer = new osgVegetation::MeshLayerConfig(1000);
	tree_layer->BackFaceCulling = true;
	tree_layer->CastShadow = true;
	tree_layer->ReceiveShadow = false;
	tree_layer->Filter.SplatFilter = "if(splat_color.g < 0.5) return false;";
	osgVegetation::MeshTypeConfig mesh;
	const float end_dist = 1000.0f;
	const float mix_dist = 10.0f;
	const float lod_dist = 80.0f;
	const float intensity = 1.2;
	mesh.MeshLODs.push_back(osgVegetation::MeshTypeConfig::MeshLODConfig("trees/fir01_l0.osg", osg::Vec4(0.0f, 0.0f, lod_dist, lod_dist + mix_dist),0, intensity));
	mesh.MeshLODs.push_back(osgVegetation::MeshTypeConfig::MeshLODConfig("trees/fir01_l1.osg", osg::Vec4(lod_dist - mix_dist, lod_dist + mix_dist, end_dist-100, end_dist), 0, intensity));
	mesh.DiffuseIntensity = 1.0;
	mesh.IntensityVariation = 0.6;
	mesh.ScaleVariation = 0.2;
	tree_layer->MeshTypes.push_back(mesh);
	layers.push_back(tree_layer);

	osg::ref_ptr <osgVegetation::BillboardLayerConfig> grass_layer_1 = new osgVegetation::BillboardLayerConfig();
	grass_layer_1->Type = osgVegetation::BillboardLayerConfig::BLT_GRASS;
	grass_layer_1->MaxDistance = 100;
	grass_layer_1->Density = 0.4;
	grass_layer_1->ColorImpact = 0.7;
	grass_layer_1->CastShadow = false;
	grass_layer_1->Filter.SplatFilter = "if(splat_color.g < 0.5  && splat_color.r < 0.5) return false;";
	grass_layer_1->Billboards.push_back(osgVegetation::BillboardLayerConfig::Billboard("billboards/veg_plant03.png", osg::Vec2f(4, 2), 1.0, 0.008));
	grass_layer_1->Billboards.push_back(osgVegetation::BillboardLayerConfig::Billboard("billboards/veg_plant01.png", osg::Vec2f(2, 2), 1.0, 0.002));
	grass_layer_1->Billboards.push_back(osgVegetation::BillboardLayerConfig::Billboard("billboards/grass2.png", osg::Vec2f(2, 1), 1.0, 1.0));
	layers.push_back(grass_layer_1);

	return layers;
}

osgVegetation::TerrainSplatShadingConfig createSplatShadingConfig()
{
	osgVegetation::TerrainSplatShadingConfig terrain_shading;
	terrain_shading.DetailLayers.push_back(osgVegetation::DetailLayer(std::string("terrain/detail/detail_dirt.jpg"), 0.05));
	terrain_shading.DetailLayers.push_back(osgVegetation::DetailLayer(std::string("terrain/detail/detail_patch_grass.jpg"), 0.09));
	terrain_shading.DetailLayers.push_back(osgVegetation::DetailLayer(std::string("terrain/detail/detail_grass.jpg"), 0.09));
	terrain_shading.DetailLayers.push_back(osgVegetation::DetailLayer(std::string("terrain/detail/detail_stones.jpg"), 0.05));
	terrain_shading.NoiseTexture.File = "terrain/detail/noise.png";
	terrain_shading.SplatTexture.File = "terrain/terrain_splat.png";
	terrain_shading.ColorModulateRatio = 0;
	return terrain_shading;
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
	
	//Create the shading node that will be parent to the terrain and the vegetation node
	osg::ref_ptr<osg::Group> splat_shading = new osg::Group();
	splat_shading->setStateSet(new osgVegetation::TerrainSplatShadingStateSet(createSplatShadingConfig()));
	root_node->addChild(splat_shading);

	//Create the terrain geometry
	osg::ref_ptr<osg::Node> terrain = ovSampleUtils::createFlatGrid(4000, 50);
	splat_shading->addChild(terrain);

	//create the layer generator from our configuration
	osgVegetation::LayerGenerator layer_generator(createVegetionConfig());
	osg::ref_ptr<osg::Group> vegetation = layer_generator.CreateVegetationNode(osgVegetation::CloneAndConvertToPatches(terrain));
	
	//Note; by adding the vegetation-node to the splat-shading-node vegetation shaders will get access to terrain textures 
	//that can be used for color matching or placement filtering
	splat_shading->addChild(vegetation);

	viewer.setSceneData(root_node);

	//viewer.setUpViewAcrossAllScreens();
	viewer.setUpViewInWindow(100, 100, 800, 600);

	viewer.getCamera()->getGraphicsContext()->getState()->setUseModelViewAndProjectionUniforms(true);
	return viewer.run();
}

