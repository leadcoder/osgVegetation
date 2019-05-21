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

#include "ov_BillboardLayer.h"
#include "ov_BillboardLayerStateSet.h"
#include "ov_TerrainShadingStateSet.h"
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
#include "ov_DemoShadow.h"


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

std::vector<osgVegetation::BillboardLayer> GetVegetationLayers()
{
	osg::Vec4 grass_splat_threashold(0.5, 0.5, 0.5, 0.5);
	std::vector<osgVegetation::BillboardLayer> layers;
	osgVegetation::BillboardLayer grass_data(100, 0.1, 1.0, 1);
	grass_data.Type = osgVegetation::BillboardLayer::BLT_GRASS;
	grass_data.Filter.SplatFilter = "if (length(splat_color) < 0.5) return false;";
	//grass_data.Type = osgVegetation::BillboardLayer::BLT_CROSS_QUADS;
	grass_data.Billboards.push_back(osgVegetation::BillboardLayer::Billboard("billboards/veg_plant03.png", osg::Vec2f(4, 2), 1.0, 0.008));
	grass_data.Billboards.push_back(osgVegetation::BillboardLayer::Billboard("billboards/veg_plant01.png", osg::Vec2f(2, 2), 1.0, 0.002));
	grass_data.Billboards.push_back(osgVegetation::BillboardLayer::Billboard("billboards/grass2.png", osg::Vec2f(2, 1), 1.0, 1.0));
	layers.push_back(grass_data);

	osgVegetation::BillboardLayer grass_data2(30, 0.4, 1.0, 5);
	grass_data2.Type = osgVegetation::BillboardLayer::BLT_GRASS;
	grass_data2.Filter.SplatFilter = grass_data.Filter.SplatFilter;
	grass_data2.Billboards.push_back(osgVegetation::BillboardLayer::Billboard("billboards/grass2.png", osg::Vec2f(2, 1), 1.0, 1.0));
	layers.push_back(grass_data2);

	//osgVegetation::BillboardLayer tree_data(2740, 0.001, 0.5, 0.7, 0.1, 2);
	osgVegetation::BillboardLayer tree_data(740, 0.001, 0.7, 0.1, 2);
	//tree_data.Type = osgVegetation::BillboardLayer::BLT_CROSS_QUADS;
	tree_data.Type = osgVegetation::BillboardLayer::BLT_ROTATED_QUAD;
	tree_data.Filter.SplatFilter = osgVegetation::PassFilter::GenerateSplatFilter(osg::Vec4(-1, -1, 0.5, -1), "<");
	tree_data.Billboards.push_back(osgVegetation::BillboardLayer::Billboard("billboards/fir01_bb.png", osg::Vec2f(10, 16), 1.6, 1.0));
	layers.push_back(tree_data);
	return layers;
}

class Terrain : public osg::Group
{
private:
	osg::Node* _createGrid(double terrain_size, int res)
	{
		osg::HeightField* heightField = new osg::HeightField();
		heightField->allocate(res, res);
		heightField->setOrigin(osg::Vec3(-terrain_size / 2.0, -terrain_size / 2.0, 0));
		heightField->setXInterval(terrain_size / double(res - 1));
		heightField->setYInterval(terrain_size / double(res - 1));
		heightField->setSkirtHeight(0.0f);

		for (unsigned int r = 0; r < heightField->getNumRows(); r++)
		{
			for (unsigned int c = 0; c < heightField->getNumColumns(); c++)
			{
				heightField->setHeight(c, r, 0);
			}
		}

		osg::Geode* geode = new osg::Geode();
		osg::ShapeDrawable* sd = new osg::ShapeDrawable(heightField);
		sd->setComputeBoundingBoxCallback(new BoundingBoxCB(osg::BoundingBox(osg::Vec3d(-terrain_size / 2.0, -terrain_size / 2.0, 0),
			osg::Vec3d(terrain_size / 2.0, terrain_size / 2.0, 1000))));
		//sd->setComputeBoundingSphereCallback(new BoundingSphereCB());
		geode->addDrawable(sd);
		//geode->setComputeBoundingBoxCallback(new BoundingBoxCB());
		//geode->setComputeBoundingSphereCallback(new BoundingSphereCB());
		//geode->setInitialBound(osg::BoundingSphere(osg::Vec3(-terrain_size / 2.0, -terrain_size / 2.0, 0),));
		return geode;
	}

	osg::Node* _createTerrainGeometry(osg::ref_ptr<osg::Image> elevation_image, double terrain_size, int res)
	{
		osg::HeightField* heightField = new osg::HeightField();
		heightField->allocate(elevation_image->s(), elevation_image->t());
		heightField->setOrigin(osg::Vec3(-terrain_size / 2.0, -terrain_size / 2.0, 0));
		heightField->setXInterval(terrain_size / double(elevation_image->s() - 1));
		heightField->setYInterval(terrain_size / double(elevation_image->t() - 1));
		heightField->setSkirtHeight(0.0f);

		for (unsigned int r = 0; r < heightField->getNumRows(); r++) {
			for (unsigned int c = 0; c < heightField->getNumColumns(); c++) {
				heightField->setHeight(c, r, elevation_image->getColor(c, r).r());
			}
		}

		osg::Geode* geode = new osg::Geode();
		geode->addDrawable(new osg::ShapeDrawable(heightField));
		return geode;
	}

	/*osg::ref_ptr<osg::Image> _loadElevationImage(const std::string &heightFile, float height_scale)
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
	}*/

	 void _SetupElevationTexture(osg::ref_ptr<osg::Image> elevation_image)
	{
		if (elevation_image)
		{
			m_ElevationTexture = new osg::Texture2D;
			m_ElevationTexture->setImage(elevation_image);
			m_ElevationTexture->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
			m_ElevationTexture->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);
			m_ElevationTexture->setWrap(osg::Texture2D::WRAP_S, osg::Texture2D::CLAMP_TO_EDGE);
			m_ElevationTexture->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::CLAMP_TO_EDGE);
			m_ElevationTexture->setResizeNonPowerOfTwoHint(false);
		}
	}

	std::string HEIGHT_MAP = "terrain/hm/heightmap.png";
	double HEIGHT_MAP_SCALE = (1386.67 - 607.0);
	double TERRAIN_SIZE = 4000;
	osg::Texture2D* m_ElevationTexture = NULL;
	bool m_UseTessellation;
public:
	osg::Texture2D*  GetElevationTexture() const
	{
		return m_ElevationTexture;
	}

	double GetSize() const
	{
		return TERRAIN_SIZE;
	}

	/*osg::ref_ptr<osg::Node> GetTerrainGeometry()
	{
		return m_ElevationTexture ? _createGrid(TERRAIN_SIZE, 100) : _createTerrainGeometry(HEIGHT_MAP, TERRAIN_SIZE, 8, HEIGHT_MAP_SCALE);
	}*/

	Terrain(osg::ref_ptr<osg::Image> elevation_image) : m_UseTessellation(false)
	{
		_SetupElevationTexture(elevation_image);
		osg::ref_ptr<osg::Node> terrain_geometry = _createTerrainGeometry(elevation_image, TERRAIN_SIZE, 1);
		if (m_UseTessellation)
			osgVegetation::ConvertToPatches(terrain_geometry);
		addChild(terrain_geometry);
		terrain_geometry->setNodeMask(ReceivesShadowTraversalMask);
	}
};

//Create ID mapping

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
	
	osg::ref_ptr<osg::Group> root_node = CreateShadowNode(config.ShadowMode);
	
	std::string HEIGHT_MAP = "terrain/hm/heightmap.png";
	double HEIGHT_MAP_SCALE = (1386.67 - 607.0);
	double TERRAIN_SIZE = 4000;
	osg::ref_ptr<osg::Image> elev_image = LoadElevationImage(HEIGHT_MAP, HEIGHT_MAP_SCALE);
	osg::ref_ptr<osg::Texture2D> elev_tex =  CreateElevationTexture(elev_image);
	osg::ref_ptr<osg::Node> terrain = createFlatTerrainPatch(TERRAIN_SIZE, 50);
	terrain->setNodeMask(ReceivesShadowTraversalMask);
	
	//osgVegetation::ConvertToPatches(terrain);

	osgVegetation::TerrainShadingConfiguration terrain_shading_config;
	
	terrain_shading_config.UseTessellation = true;
	terrain_shading_config.ColorTexture = osgVegetation::TextureConfig("terrain/hm/colormap.png", 0);
	terrain_shading_config.SplatTexture = osgVegetation::TextureConfig("terrain/hm/splatmap.png", 1);
	terrain_shading_config.ElevationTexture = osgVegetation::TextureConfig(elev_tex, 2);
	terrain_shading_config.DetailLayers.push_back(osgVegetation::DetailLayer(std::string("terrain/detail/detail_dirt.dds"), 0.05));
	terrain_shading_config.DetailLayers.push_back(osgVegetation::DetailLayer(std::string("terrain/detail/detail_grass_mossy.dds"), 0.05));
	terrain_shading_config.DetailLayers.push_back(osgVegetation::DetailLayer(std::string("terrain/detail/detail_grass_mossy.dds"), 0.05));
	terrain_shading_config.DetailLayers.push_back(osgVegetation::DetailLayer(std::string("terrain/detail/detail_grass_mossy.dds"), 0.05));
	terrain_shading_config.DetailTextureUnit = 3;
	terrain_shading_config.NoiseTexture = osgVegetation::TextureConfig("terrain/detail/noise.png", 4);
	
	osg::ref_ptr<osgVegetation::TerrainShadingEffect> terrain_shading_effect = new osgVegetation::TerrainShadingEffect(terrain_shading_config);
	terrain_shading_effect->addChild(terrain);
	root_node->addChild(terrain_shading_effect);

	//Add invisible collision geometry for camera manipulator intersection tests
	{
		const int COLLSION_TERRAIN_MASK = 0x4;
		osg::ref_ptr<osg::Node> collision_terrain = createTerrainPatch(TERRAIN_SIZE, elev_image);
		collision_terrain->setNodeMask(COLLSION_TERRAIN_MASK);
		root_node->addChild(collision_terrain);
		viewer.getCamera()->setCullMask(~COLLSION_TERRAIN_MASK);
	}
	
	//Add vegetation to terrain
#if 1
	osg::ref_ptr<osg::Node> terrain_geometry = createFlatTerrainPatch(TERRAIN_SIZE, 100);
	
	//osgVegetation::ConvertToPatches(terrain_geometry);
	//terrain_geometry->setNodeMask(ReceivesShadowTraversalMask | CastsShadowTraversalMask);
	//Add all billboard layers
	int billboard_tex_unit = 12;
	for (size_t i = 0; i < GetVegetationLayers().size(); i++)
	{
		osgVegetation::BillboardLayer layer_config = GetVegetationLayers().at(i);
		osg::ref_ptr<osgVegetation::BillboardLayerEffect> bb_layer = new osgVegetation::BillboardLayerEffect(layer_config, billboard_tex_unit);
		//bb_layer->setTerrainTextures(terrain->GetTextureUnits());
		bb_layer->addChild(terrain_geometry);
		//bb_layer->setNodeMask(0x1 | 0x2);
		if (layer_config.Type == osgVegetation::BillboardLayer::BLT_GRASS)
			bb_layer->setNodeMask(ReceivesShadowTraversalMask);
		else
			bb_layer->setNodeMask(ReceivesShadowTraversalMask | CastsShadowTraversalMask);
		terrain_shading_effect->addChild(bb_layer);
	}
#endif	
	//apply scene settings to terrain
	osgVegetation::SetSceneDefinitions(terrain_shading_effect->getOrCreateStateSet(), config);
	
	
	if (!root_node)
	{
		osg::notify(osg::NOTICE) << "Warning: no valid data loaded, please specify a database on the command line." << std::endl;
		return 1;
	}
	
	//Add light and shadows
	osg::Light* light = new osg::Light;
	light->setDiffuse(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
	osg::Vec4 light_pos(1, 1.0, 1, 0);
	light->setPosition(light_pos);		// last param	w = 0.0 directional light (direction)
	osg::Vec3f light_dir(-light_pos.x(), -light_pos.y(), -light_pos.z());
	light_dir.normalize();
	light->setDirection(light_dir);
	light->setAmbient(osg::Vec4(0.3f, 0.3f, 0.3f, 1.0f));

	osg::LightSource* light_source = new osg::LightSource;
	light_source->setLight(light);
	root_node->addChild(light_source);

	viewer.setThreadingModel(osgViewer::ViewerBase::SingleThreaded);

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
		if (config.FogMode == osgVegetation::FM_LINEAR)
		{
			fog->setStart(0);
			fog->setEnd(1000);
		}
		
		viewer.getCamera()->setClearColor(fog_color);
	}

	viewer.setSceneData(root_node);
	viewer.setUpViewInWindow(100, 100, 800, 600);

	viewer.getCamera()->getGraphicsContext()->getState()->setUseModelViewAndProjectionUniforms(true);
	//viewer.getCamera()->getGraphicsContext()->getState()->setUseVertexAttributeAliasing(true);

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
