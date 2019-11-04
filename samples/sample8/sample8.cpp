#include "ov_BillboardLayerConfig.h"
#include "ov_BillboardLayerStateSet.h"
#include "ov_TerrainSplatShadingStateSet.h"
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
#include <osg/ShapeDrawable>
#include <iostream>
#include "ov_DemoTerrain.h"
#include "ov_Demo.h"


osg::ref_ptr<osg::Image> LoadElevationImage(const std::string &heightFile, float height_scale)
{
	osg::ref_ptr <osg::Image> heightMap = osgDB::readImageFile(heightFile);
	osg::ref_ptr <osg::Image> terrainImage = new osg::Image;
	terrainImage->allocateImage(heightMap->s(), heightMap->t(), 1, GL_LUMINANCE, GL_FLOAT);
	terrainImage->setInternalTextureFormat(GL_LUMINANCE_FLOAT32_ATI);

	for (int r = 0; r < heightMap->t(); ++r)
	{
		for (int c = 0; c < heightMap->s(); ++c)
		{
			*((float*)(terrainImage->data(c, r))) = heightMap->getColor(c, r).r() * height_scale;
		}
	}
	return terrainImage;
}

osg::ref_ptr <osg::Texture2D> CreateElevationTexture(osg::ref_ptr<osg::Image> elevation_image)
{
	osg::ref_ptr <osg::Texture2D> elevation_texture = new osg::Texture2D;
	elevation_texture->setImage(elevation_image);
	elevation_texture->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
	elevation_texture->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);
	elevation_texture->setWrap(osg::Texture2D::WRAP_S, osg::Texture2D::CLAMP_TO_EDGE);
	elevation_texture->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::CLAMP_TO_EDGE);
	elevation_texture->setResizeNonPowerOfTwoHint(false);
	return elevation_texture;
}

struct BoundingBoxCB : public osg::Drawable::ComputeBoundingBoxCallback
{
	BoundingBoxCB() {}
	BoundingBoxCB(const osg::BoundingBox &bbox) : _bbox(bbox) {};
	osg::BoundingBox computeBound(const osg::Drawable&) const { return _bbox; }
private:
	osg::BoundingBox _bbox;
};

struct BoundingSphereCB : public osg::Drawable::ComputeBoundingSphereCallback
{
	BoundingSphereCB() {}
	BoundingSphereCB(const osg::BoundingSphere &bs) : _bounds(bs) {};
	osg::BoundingSphere computeBound(const osg::Node&) const { return _bounds; }
private:
	osg::BoundingSphere _bounds;
};

osg::ref_ptr<osg::Node> createFlatTerrainPatch(double terrain_size, int samples)
{
	osg::ref_ptr <osg::HeightField> heightField = new osg::HeightField();
	heightField->allocate(samples, samples);
	heightField->setOrigin(osg::Vec3(-terrain_size / 2.0, -terrain_size / 2.0, 0));
	heightField->setXInterval(terrain_size / double(samples - 1));
	heightField->setYInterval(terrain_size / double(samples - 1));
	heightField->setSkirtHeight(0.0f);

	for (unsigned int r = 0; r < heightField->getNumRows(); r++)
	{
		for (unsigned int c = 0; c < heightField->getNumColumns(); c++)
		{
			heightField->setHeight(c, r, 0);
		}
	}

	osg::ref_ptr <osg::Geode> geode = new osg::Geode();
	osg::ShapeDrawable* sd = new osg::ShapeDrawable(heightField);
	sd->setComputeBoundingBoxCallback(new BoundingBoxCB(osg::BoundingBox(osg::Vec3d(-terrain_size / 2.0, -terrain_size / 2.0, 0),
	osg::Vec3d(terrain_size / 2.0, terrain_size / 2.0, 1000))));
	geode->addDrawable(sd);
	osgVegetation::ConvertToPatches(geode);
	return geode;
}

osg::ref_ptr<osg::Node> createTerrainPatch(double terrain_size, osg::ref_ptr<osg::Image> elevation_image)
{
	osg::ref_ptr <osg::HeightField> heightField = new osg::HeightField();
	heightField->allocate(elevation_image->s(), elevation_image->t());
	heightField->setOrigin(osg::Vec3(-terrain_size / 2.0, -terrain_size / 2.0, 0));
	heightField->setXInterval(terrain_size / double(elevation_image->s() - 1));
	heightField->setYInterval(terrain_size / double(elevation_image->t() - 1));
	heightField->setSkirtHeight(0.0f);

	for (unsigned int r = 0; r < heightField->getNumRows(); r++)
	{
		for (unsigned int c = 0; c < heightField->getNumColumns(); c++)
		{
			heightField->setHeight(c, r, elevation_image->getColor(c, r).r());
		}
	}

	osg::ref_ptr<osg::Geode> geode = new osg::Geode();
	geode->addDrawable(new osg::ShapeDrawable(heightField));
	return geode;
}

std::vector<osgVegetation::BillboardLayerConfig> GetVegetationLayers()
{
	osg::Vec4 grass_splat_threashold(0.5, 0.5, 0.5, 0.5);
	std::vector<osgVegetation::BillboardLayerConfig> layers;
	osgVegetation::BillboardLayerConfig grass_data(osgVegetation::BillboardLayerConfig::BLT_GRASS);
	grass_data.MaxDistance = 100;
	grass_data.ColorImpact = 1.0;
	grass_data.Density = 0.1;
	grass_data.CastShadow = false;
	grass_data.Filter.SplatFilter = "if (length(splat_color) < 0.5) return false;";
	grass_data.Billboards.push_back(osgVegetation::BillboardLayerConfig::Billboard("billboards/veg_plant03.png", osg::Vec2f(4, 2), 1.0, 0.008));
	grass_data.Billboards.push_back(osgVegetation::BillboardLayerConfig::Billboard("billboards/veg_plant01.png", osg::Vec2f(2, 2), 1.0, 0.002));
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
	tree_data.Density = 0.001;
	tree_data.ColorImpact = 0.1;
	tree_data.Filter.SplatFilter = osgVegetation::PassFilter::GenerateSplatFilter(osg::Vec4(-1, -1, 0.5, -1), "<");
	tree_data.Billboards.push_back(osgVegetation::BillboardLayerConfig::Billboard("billboards/fir01_bb.png", osg::Vec2f(10, 16), 1.6, 1.0));
	layers.push_back(tree_data);
	return layers;
}

int main(int argc, char** argv)
{
	osgVegetation::Register.Scene.Shadow.Mode = osgVegetation::SM_LISPSM;
	osgVegetation::Register.Scene.FogMode = osgVegetation::FM_EXP2;
	
	osgVegetation::Register.TexUnits.AddUnit(6, OV_SHADOW_TEXTURE0_ID);
	osgVegetation::Register.TexUnits.AddUnit(7, OV_SHADOW_TEXTURE1_ID);

	Demo demo(argc, argv, osgVegetation::Register.Scene);

	osg::ref_ptr<osg::Group> root_node = new osg::Group();
	
	const std::string HEIGHT_MAP = "terrain/hm/heightmap.png";
	const double HEIGHT_MAP_SCALE = (1386.67 - 607.0);
	const double TERRAIN_SIZE = 4000;
	osg::ref_ptr<osg::Image> elev_image = LoadElevationImage(HEIGHT_MAP, HEIGHT_MAP_SCALE);
	osg::ref_ptr<osg::Texture2D> elev_tex = CreateElevationTexture(elev_image);
	osg::ref_ptr<osg::Node> terrain = createFlatTerrainPatch(TERRAIN_SIZE, 50);
	terrain->setNodeMask(osgVegetation::Register.Scene.Shadow.ReceivesShadowTraversalMask);

	osgVegetation::TerrainSplatShadingConfig terrain_shading_config;

	terrain_shading_config.UseTessellation = true;

	terrain_shading_config.ColorTexture.File = "terrain/hm/colormap.png";
	terrain_shading_config.NormalTexture.File = "terrain/hm/normalmap.png";
	terrain_shading_config.ElevationTexture.Texture = elev_tex;

	terrain_shading_config.SplatTexture.File = "terrain/hm/splatmap.png";
	terrain_shading_config.DetailLayers.push_back(osgVegetation::DetailLayer(std::string("terrain/detail/detail_dirt.dds"), 0.05));
	terrain_shading_config.DetailLayers.push_back(osgVegetation::DetailLayer(std::string("terrain/detail/detail_grass_mossy.dds"), 0.05));
	terrain_shading_config.DetailLayers.push_back(osgVegetation::DetailLayer(std::string("terrain/detail/detail_grass_mossy.dds"), 0.05));
	terrain_shading_config.DetailLayers.push_back(osgVegetation::DetailLayer(std::string("terrain/detail/detail_grass_mossy.dds"), 0.05));
	terrain_shading_config.NoiseTexture.File = "terrain/detail/noise.png";
	terrain_shading_config.MaxDistance = 400;
	terrain_shading_config.ColorModulateRatio = 0.2;
	osg::ref_ptr<osgVegetation::TerrainSplatShadingEffect> terrain_shading_effect = new osgVegetation::TerrainSplatShadingEffect(terrain_shading_config);
	terrain_shading_effect->addChild(terrain);
	root_node->addChild(terrain_shading_effect);

	//Add invisible collision geometry for camera manipulator intersection tests
	{
		const int COLLSION_TERRAIN_MASK = 0x8;
		osg::ref_ptr<osg::Node> collision_terrain = createTerrainPatch(TERRAIN_SIZE, elev_image);
		collision_terrain->setNodeMask(COLLSION_TERRAIN_MASK);
		root_node->addChild(collision_terrain);
		demo.GetViewer().getCamera()->setCullMask(~COLLSION_TERRAIN_MASK);
	}

	//Add vegetation to terrain

	osg::ref_ptr<osg::Node> terrain_geometry = createFlatTerrainPatch(TERRAIN_SIZE, 100);

	//Add all billboard layers
	for (size_t i = 0; i < GetVegetationLayers().size(); i++)
	{
		osgVegetation::BillboardLayerConfig layer_config = GetVegetationLayers().at(i);
		osg::ref_ptr<osgVegetation::BillboardLayerEffect> bb_layer = new osgVegetation::BillboardLayerEffect(layer_config);
		bb_layer->addChild(terrain_geometry);
		terrain_shading_effect->addChild(bb_layer);
	}
	demo.GetSceneRoot()->addChild(root_node);
	demo.Run();
	return 0;
}
