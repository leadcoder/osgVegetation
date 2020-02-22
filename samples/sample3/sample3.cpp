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
#include "ov_Demo.h"
#include <iostream>

namespace osgVegetation
{
	GlobalRegister Register;
}

int main(int argc, char** argv)
{
	osgVegetation::Register.TexUnits.AddUnit(0, OV_TERRAIN_COLOR_TEXTURE_ID);
	osgVegetation::Register.TexUnits.AddUnit(1, OV_TERRAIN_SPLAT_TEXTURE_ID);

	Demo demo(argc, argv);
	//demo.EnableFog(osg::Fog::EXP2);
	//Setup billboard grass layer
	osg::ref_ptr<osgVegetation::BillboardLayerConfig> grass_layer = new osgVegetation::BillboardLayerConfig();
	grass_layer->MaxDistance = 140;
	grass_layer->Density = 0.2;
	grass_layer->ColorImpact = 1.0;
	grass_layer->CastShadow = false;
	grass_layer->Type = osgVegetation::BillboardLayerConfig::BLT_GRASS;
	grass_layer->Billboards.push_back(osgVegetation::BillboardLayerConfig::Billboard("billboards/veg_plant03.png", osg::Vec2f(4, 2), 0.9, 0.008));
	grass_layer->Billboards.push_back(osgVegetation::BillboardLayerConfig::Billboard("billboards/veg_plant01.png", osg::Vec2f(2, 2), 0.9, 0.002));
	grass_layer->Billboards.push_back(osgVegetation::BillboardLayerConfig::Billboard("billboards/grass2.png", osg::Vec2f(2, 1), 1.0, 1.0));
	//osg::Vec4 grass_splat_color_filter(0.1, -1, -1, -1);
	grass_layer->Filter.SplatFilter = "if(splat_color.x > 0.5) return false;"; //osgVegetation::PassFilter::GenerateSplatFilter(grass_splat_color_filter, "<");

	//Setup billboard tree layer
	osg::ref_ptr<osgVegetation::BillboardLayerConfig> tree_layer1 = new osgVegetation::BillboardLayerConfig();
	tree_layer1->MaxDistance = 740;
	tree_layer1->Density = 0.001;
	tree_layer1->ColorImpact = 0.0;
	tree_layer1->Type = osgVegetation::BillboardLayerConfig::BLT_ROTATED_QUAD;
	tree_layer1->Filter.ColorFilter = "if(length(base_color.xyz) > 0.5) return false;";
	tree_layer1->Filter.SplatFilter = "if(splat_color.x > 0.5) return false;";
	tree_layer1->Billboards.push_back(osgVegetation::BillboardLayerConfig::Billboard("billboards/fir01_bb.png", osg::Vec2f(10, 16), 1.0, 1.0));

	//Setup mesh tree layer
	osgVegetation::MeshLayerConfig* tree_layer2 = new osgVegetation::MeshLayerConfig(3000);
	tree_layer2->Filter.ColorFilter = tree_layer1->Filter.ColorFilter;
	tree_layer2->Filter.SplatFilter = tree_layer1->Filter.SplatFilter;
	{
		//Add mesh-LODs
		osgVegetation::MeshTypeConfig mesh;
		const float end_dist = 740;
		mesh.MeshLODs.push_back(osgVegetation::MeshTypeConfig::MeshLODConfig("trees/fir01_l0.osg", osg::Vec4(0.0f, 0.0f, 100.0f, 110.0f)));
		mesh.MeshLODs.push_back(osgVegetation::MeshTypeConfig::MeshLODConfig("trees/fir01_l1_bb.osg", osg::Vec4(100.0f, 110.0f, end_dist, end_dist + 10),1));
		tree_layer2->MeshTypes.push_back(mesh);
	}

	//Add layers at suitable lod
	osgVegetation::VPBInjectionLODConfig grass_terrain_lod(5);
	grass_terrain_lod.Layers.push_back(grass_layer);

	osgVegetation::VPBInjectionLODConfig tree_terrain_lod(2);
	//tree_terrain_lod.Layers.push_back(tree_layer1);
	tree_terrain_lod.Layers.push_back(tree_layer2);

	//Create the final config
	std::vector<osgVegetation::VPBInjectionLODConfig> vpb_config;
	vpb_config.push_back(grass_terrain_lod);
	vpb_config.push_back(tree_terrain_lod);

	//Register ReadFileCallback to catch PagedLod
	osgDB::Registry::instance()->setReadFileCallback(new osgVegetation::VPBVegetationInjection(vpb_config));

#if 1
	osg::ref_ptr<osg::Node> terrain_node = osgDB::readNodeFile("D:/terrain/vpb/us/deploy/utm/us-terrain.osg");
	//osg::ref_ptr<osg::Node> terrain_node = osgDB::readNodeFile("terrain/us-terrain.zip/us-terrain.osg");
#else
	//osg::ref_ptr<osg::Node> terrain_node = osgDB::readNodeFile("D:/terrain/vpb/us/final/us-terrain.osgb");
	osg::ref_ptr<osg::Node> terrain_node = osgDB::readNodeFile("D:/terrain/vpb/us/final/us-terrain.osg");
#endif

	//Add some terrain shading
	osgVegetation::TerrainSplatShadingConfig terrain_shading;
	terrain_shading.DetailLayers.push_back(osgVegetation::DetailLayer(std::string("terrain/detail/detail_grass_mossy.dds"), 0.05));
	terrain_shading.DetailLayers.push_back(osgVegetation::DetailLayer(std::string("terrain/detail/detail_dirt.dds"), 0.05));
	terrain_shading.DetailLayers.push_back(osgVegetation::DetailLayer(std::string("terrain/detail/detail_dirt.dds"), 0.05));
	terrain_shading.DetailLayers.push_back(osgVegetation::DetailLayer(std::string("terrain/detail/detail_dirt.dds"), 0.05));
	terrain_shading.NoiseTexture.File = "terrain/detail/noise.png";
	terrain_shading.ColorModulateRatio = 0.5;
	osg::ref_ptr <osgVegetation::TerrainSplatShadingStateSet> terrain_shading_ss = new osgVegetation::TerrainSplatShadingStateSet(terrain_shading);

	osg::ref_ptr<osg::Group> terrain_shading_effect = new osg::Group();
	terrain_shading_effect->setStateSet(terrain_shading_ss);
	terrain_shading_effect->addChild(terrain_node);

	demo.GetSceneRoot()->addChild(terrain_shading_effect);
	demo.Run();
	return 0;
}
