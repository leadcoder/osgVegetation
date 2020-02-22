/* OpenSceneGraph example, osgterrain.
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
*  THE SOFTWARE.
*/

#include "ov_BillboardLayerConfig.h"
#include "ov_BillboardLayerStateSet.h"
#include "ov_TerrainSplatShadingStateSet.h"
#include "ov_BillboardMultiLayerEffect.h"
#include "ov_Utils.h"
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
#include <osg/PositionAttitudeTransform>
#include <osg/Fog>

#include <iostream>
#include "ov_DemoTerrain.h"
#include "ov_Demo.h"

namespace osgVegetation
{
	GlobalRegister Register;
}


std::vector<osgVegetation::BillboardLayerConfig> CreateVegetationConfig()
{
	std::vector<osgVegetation::BillboardLayerConfig> layers;
	osgVegetation::BillboardLayerConfig grass_data(osgVegetation::BillboardLayerConfig::BLT_GRASS);
	grass_data.MaxDistance = 100;
	grass_data.Density = 0.1;
	grass_data.ColorImpact = 1.0;
	grass_data.CastShadow = false;
	grass_data.Filter.SplatFilter = osgVegetation::PassFilter::GenerateSplatFilter(osg::Vec4(-1, 0.5, -1, -1), "<");
	//grass_data.Type = osgVegetation::BillboardLayerConfig::BLT_CROSS_QUADS;
	grass_data.Billboards.push_back(osgVegetation::BillboardLayerConfig::Billboard("billboards/veg_plant03.png", osg::Vec2f(4, 2), 0.9, 0.008));
	grass_data.Billboards.push_back(osgVegetation::BillboardLayerConfig::Billboard("billboards/veg_plant01.png", osg::Vec2f(2, 2), 0.9, 0.002));
	grass_data.Billboards.push_back(osgVegetation::BillboardLayerConfig::Billboard("billboards/grass2.png", osg::Vec2f(2, 1), 1.0, 1.0));
	layers.push_back(grass_data);

	osgVegetation::BillboardLayerConfig grass_data2(osgVegetation::BillboardLayerConfig::BLT_GRASS);
	grass_data2.MaxDistance = 30;
	grass_data2.Density = 0.4;
	grass_data2.ColorImpact = 1.0;
	grass_data2.CastShadow = false;
	grass_data2.Filter.SplatFilter = grass_data.Filter.SplatFilter;
	grass_data2.Billboards.push_back(osgVegetation::BillboardLayerConfig::Billboard("billboards/grass2.png", osg::Vec2f(2, 1), 1.0, 1.0));
	layers.push_back(grass_data2);
	
	osgVegetation::BillboardLayerConfig tree_data(osgVegetation::BillboardLayerConfig::BLT_ROTATED_QUAD);
	tree_data.MaxDistance = 740;
	tree_data.Density = 0.01;
	tree_data.ColorImpact = 0.0;
	tree_data.Filter.SplatFilter = osgVegetation::PassFilter::GenerateSplatFilter(osg::Vec4(-1, 0.5, -1, -1), "<");
	tree_data.Filter.ColorFilter = "if(length(base_color.xyz) > 0.5) return false;";
	//tree_data.Type = osgVegetation::BillboardLayerConfig::BLT_CROSS_QUADS;
	tree_data.Billboards.push_back(osgVegetation::BillboardLayerConfig::Billboard("billboards/fir01_bb.png", osg::Vec2f(10, 16), 1.5, 1.0));
	layers.push_back(tree_data);
	return layers;
}

osgVegetation::TerrainSplatShadingConfig CreateTerrainShaderConfig(bool tess)
{
	//Create terrain layer node
	osgVegetation::TerrainSplatShadingConfig tsc;
	tsc.ColorTexture.File = "Images/lz.rgb";
	tsc.SplatTexture.File = "Images/lz_coverage.png";
	tsc.DetailLayers.push_back(osgVegetation::DetailLayer(std::string("terrain/detail/detail_dirt.dds"), 0.08));
	tsc.DetailLayers.push_back(osgVegetation::DetailLayer(std::string("terrain/detail/detail_dirt.dds"), 0.08));
	tsc.DetailLayers.push_back(osgVegetation::DetailLayer(std::string("terrain/detail/detail_grass_mossy.dds"), 0.08));
	tsc.DetailLayers.push_back(osgVegetation::DetailLayer(std::string("terrain/detail/detail_grass_mossy.dds"), 0.08));
	tsc.NoiseTexture.File = "terrain/detail/noise.png";
	tsc.UseTessellation = tess;
	tsc.ColorModulateRatio = 0.2;
	return tsc;
}

#if 0
osg::ref_ptr<osg::Node> CreateVegetationNode(osg::ref_ptr<osg::Node> terrain_geometry)
{
	osg::ref_ptr<osgVegetation::BillboardMultiLayerEffect> layers = new osgVegetation::BillboardMultiLayerEffect(GetVegetationLayers());
	layers->insertTerrain(terrain_geometry);
	return layers;
}

osg::ref_ptr<osg::Group> CreateTerrainPatches(double terrain_size)
{
	//Create terrain geometry used for both terrain layer and  vegetation layers
	osg::ref_ptr<osg::Node> terrain_geometry = CreateDemoTerrain(terrain_size);
	osgVegetation::ConvertToPatches(terrain_geometry);
	osg::ref_ptr<osgVegetation::TerrainSplatShadingEffect> terrain_shading_effect = new osgVegetation::TerrainSplatShadingEffect(GetTerrainShaderConfig(true));
	//Disable terrain self shadowning
	//terrain_shading_effect->setNodeMask(ReceivesShadowTraversalMask);
	terrain_shading_effect->addChild(terrain_geometry);
	//Create vegetation layer node
	terrain_shading_effect->addChild( CreateVegetationNode(terrain_geometry));
	return terrain_shading_effect;
}

osg::ref_ptr<osg::Group> CreateTerrain(double terrain_size)
{
	//Create terrain geometry used for both terrain layer and  vegetation layers
	osg::ref_ptr<osg::Node> terrain_geometry = CreateDemoTerrain(terrain_size);
	const bool apply_shader = true;
	osg::ref_ptr<osg::Group> terrain_shading_effect = apply_shader ? new osgVegetation::TerrainSplatShadingEffect(GetTerrainShaderConfig(false)) : new osg::Group();
	//Disable terrain self shadowning
	//terrain_shading_effect->setNodeMask(ReceivesShadowTraversalMask);
	terrain_shading_effect->addChild(terrain_geometry);

	//Create vegetation layer node
	//minimal copy for our sample data
	osg::ref_ptr<osg::Node> terrain_patch_geometry = dynamic_cast<osg::Node*>(terrain_geometry->clone(osg::CopyOp::DEEP_COPY_PRIMITIVES | osg::CopyOp::DEEP_COPY_DRAWABLES));
	osgVegetation::ConvertToPatches(terrain_patch_geometry);
	terrain_shading_effect->addChild( CreateVegetationNode(terrain_patch_geometry));
	return terrain_shading_effect;
}
#endif

int main(int argc, char** argv)
{
	Demo demo(argc, argv);
	demo.EnableFog(osg::Fog::EXP2);
	demo.EnableShadow(Demo::SM_VDSM2);
	const double terrain_size = 2000;

//#define REUSE_TERRAIN
#ifdef REUSE_TERRAIN
	osg::ref_ptr<osg::Node> terrain_geometry =  CreateDemoTerrain(terrain_size);
	osgVegetation::ConvertToPatches(terrain_geometry);
	
	osg::ref_ptr<osgVegetation::BillboardMultiLayerEffect> veg_effect = new osgVegetation::BillboardMultiLayerEffect(CreateVegetationConfig());
	veg_effect->insertTerrain(terrain_geometry);

	//Create terrain shading effect
	osg::ref_ptr<osg::Group> terrain_shading_effect = new osgVegetation::TerrainSplatShadingEffect(CreateTerrainShaderConfig(true));
	
	//Apply terrain shading to both terrain and vegetation
	terrain_shading_effect->addChild(terrain_geometry);
	terrain_shading_effect->addChild(veg_effect);
#else
	osg::ref_ptr<osg::Node> terrain_geometry = CreateDemoTerrain(terrain_size);

	osg::ref_ptr<osg::Node> vegetation_terrain = dynamic_cast<osg::Node*>(terrain_geometry->clone(osg::CopyOp::DEEP_COPY_PRIMITIVES | osg::CopyOp::DEEP_COPY_DRAWABLES));
	osgVegetation::ConvertToPatches(vegetation_terrain);

	osg::ref_ptr<osgVegetation::BillboardMultiLayerEffect> veg_effect = new osgVegetation::BillboardMultiLayerEffect(CreateVegetationConfig());
	veg_effect->insertTerrain(vegetation_terrain);

	//Create terrain shading effect
	osg::ref_ptr<osg::Group> terrain_shading_effect = new osgVegetation::TerrainSplatShadingEffect(CreateTerrainShaderConfig(false));

	//osg::ref_ptr<osg::Group> terrain_shading_effect = new osg::Group();
	//Apply terrain shading to both terrain and vegetation
	terrain_shading_effect->addChild(terrain_geometry);
	terrain_shading_effect->addChild(veg_effect);

#endif
	demo.GetSceneRoot()->addChild(terrain_shading_effect);

	demo.Run();
	return 0;
}
