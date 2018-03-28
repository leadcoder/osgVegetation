

#include "XBFVegetationTile.h"
#include "ov_BillboardTile.h"
#include "ov_Utils.h"

#include <osg/Version>
#include <osg/Notify>
#include <osg/Fog>

#if OSG_VERSION_GREATER_OR_EQUAL(3,4,0)

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include <osgGA/StateSetManipulator>
#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>
#include <osgGA/KeySwitchMatrixManipulator>
#include <osgGA/StateSetManipulator>
#include <osgGA/AnimationPathManipulator>
#include <osgGA/TerrainManipulator>

#include <osg/Geometry>
#include <osg/Depth>
#include <osg/Point>
#include <osg/VertexAttribDivisor>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osgUtil/Optimizer>

#include <osgTerrain/Terrain>
#include <osgTerrain/Layer>
#include <osg/PositionAttitudeTransform>

template<class T>
class FindTopMostNodeOfTypeVisitor : public osg::NodeVisitor
{
public:
	FindTopMostNodeOfTypeVisitor() :
		osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
		_foundNode(0)
	{}

	void apply(osg::Node& node)
	{
		T* result = dynamic_cast<T*>(&node);
		if (result)
		{
			_foundNode = result;
		}
		else
		{
			traverse(node);
		}
	}

	T* _foundNode;
};

template<class T>
T* findTopMostNodeOfType(osg::Node* node)
{
	if (!node) return 0;

	FindTopMostNodeOfTypeVisitor<T> fnotv;
	node->accept(fnotv);

	return fnotv._foundNode;
}

class TerrainTileVisitor : public osg::NodeVisitor
{
public:
	std::vector<osgTerrain::TerrainTile*> Tiles;
	osg::Group* MainGroup;
	TerrainTileVisitor() :
		osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) {
	}

	void apply(osg::Node& node)
	{
		osgTerrain::TerrainTile* tile = dynamic_cast<osgTerrain::TerrainTile*>(&node);
		if (tile)
		{
			Tiles.push_back(tile);
		}
		else
		{
			traverse(node);
		}
	}
};

class VegetationReadFileCallback : public osgDB::ReadFileCallback
{
public:
	VegetationReadFileCallback(const std::vector<XBFVegetationData> &data, std::vector<osgVegetation::BillboardLayer>& bb_layers) : m_VegData(data),
		m_BBLayers(bb_layers)
	{

	}

	class TerrainTileVisitor : public osg::NodeVisitor
	{
	public:
		std::vector<osgTerrain::TerrainTile*> Tiles;
		osg::Group* MainGroup;
		TerrainTileVisitor() :
			osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) {
		}

		void apply(osg::Node& node)
		{
			osgTerrain::TerrainTile* tile = dynamic_cast<osgTerrain::TerrainTile*>(&node);
			if (tile)
			{
				Tiles.push_back(tile);
			}
			else
			{
				traverse(node);
			}
		}
	};

	static int extractLODLevelFromFileName(const std::string& filename)
	{
		std::string clean_filename = filename;
		std::size_t found = clean_filename.find_last_of("/\\");
		if (found != std::string::npos)
			clean_filename = clean_filename.substr(found + 1);

		found = clean_filename.find("_L");

		int lod_level = -1;
		if (found != std::string::npos)
		{
			std::string level = clean_filename.substr(found + 2);
			found = level.find("_X");
			if (found != std::string::npos)
			{
				level = level.substr(0, found);
				lod_level = atoi(level.c_str());
			}
		}
		return lod_level;
	}

	virtual osgDB::ReaderWriter::ReadResult readNode(const std::string& filename, const osgDB::Options* options)
	{
		osgDB::ReaderWriter::ReadResult rr = ReadFileCallback::readNode(filename, options);

#ifdef OV_USE_TILE_ID_LOD_LEVEL
		const int lod_level = ttv.Tiles.size() > 0 ? ttv.Tiles[0]->getTileID().level - 1 : 0;
#else
		const int lod_level = extractLODLevelFromFileName(filename);
#endif
		TerrainTileVisitor ttv;
		rr.getNode()->accept(ttv);

		for (size_t i = 0; i < m_VegData.size(); i++)
		{
			if (lod_level == m_VegData[i].TargetLODLevel && rr.validNode())
			{
				osg::Group* group = dynamic_cast<osg::Group*>(rr.getNode());
				osg::PagedLOD* plod = dynamic_cast<osg::PagedLOD*>(rr.getNode());
				if (group && plod == 0)
				{
					osg::Group* veg_layer = new osg::Group();
					group->addChild(veg_layer);
					
					for (size_t j = 0; j < ttv.Tiles.size(); j++)
					{
						veg_layer->addChild(new XBFVegetationTile(ttv.Tiles[j], m_VegData[i]));
					}
				}
			}
		}

		for (size_t i = 0; i < m_BBLayers.size(); i++)
		{
			if (lod_level == m_BBLayers[i].LODLevel && rr.validNode())
			{
				osg::Group* group = dynamic_cast<osg::Group*>(rr.getNode());
				osg::PagedLOD* plod = dynamic_cast<osg::PagedLOD*>(rr.getNode());
				if (group && plod == 0)
				{
					osg::Group* veg_layer = new osg::Group();
					group->addChild(veg_layer);
					for (size_t j = 0; j < ttv.Tiles.size(); j++)
					{
						osg::Node* veg_node = osgVegetation::BillboardLayerHelper::CreateVegNodeFromTerrainTile(ttv.Tiles[j], m_BBLayers[i]);
						veg_layer->addChild(veg_node);
					}
				}
			}
		}

		return rr;
	}
protected:
	virtual ~VegetationReadFileCallback() {}
	std::vector<XBFVegetationData> m_VegData;
	std::vector<osgVegetation::BillboardLayer> m_BBLayers;
};
/*
void PrepareTerrain(osg::Node* terrain)
{
	osg::Program* program = new osg::Program;
	terrain->getOrCreateStateSet()->setAttribute(program);
	terrain->getOrCreateStateSet()->addUniform(new osg::Uniform("baseTexture", 0));
	terrain->getOrCreateStateSet()->addUniform(new osg::Uniform("landCoverTexture", 1));
	terrain->getOrCreateStateSet()->addUniform(new osg::Uniform("detailTexture0", 2));
	terrain->getOrCreateStateSet()->addUniform(new osg::Uniform("detailTexture1", 3));

	osg::Texture2D *d0_tex = new osg::Texture2D;
	d0_tex->setWrap(osg::Texture2D::WRAP_S, osg::Texture2D::REPEAT);
	d0_tex->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::REPEAT);
	d0_tex->setImage(osgDB::readRefImageFile("terrain/detail/detail_grass_mossy.dds"));
	terrain->getOrCreateStateSet()->setTextureAttributeAndModes(2, d0_tex, osg::StateAttribute::ON);

	osg::Texture2D *d1_tex = new osg::Texture2D;
	d1_tex->setWrap(osg::Texture2D::WRAP_S, osg::Texture2D::REPEAT);
	d1_tex->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::REPEAT);
	d1_tex->setImage(osgDB::readRefImageFile("terrain/detail/detail_dirt.dds"));
	terrain->getOrCreateStateSet()->setTextureAttributeAndModes(3, d1_tex, osg::StateAttribute::ON);
	program->addShader(osg::Shader::readShaderFile(osg::Shader::FRAGMENT, osgDB::findDataFile("sample_terrain_fragment.glsl")));
}
*/
int
main(int argc, char** argv)
{
	osg::ArgumentParser arguments(&argc, argv);
	osgViewer::Viewer viewer(arguments);
	
	osg::ref_ptr<osgGA::KeySwitchMatrixManipulator> keyswitchManipulator = new osgGA::KeySwitchMatrixManipulator;

	keyswitchManipulator->addMatrixManipulator('1', "Trackball", new osgGA::TrackballManipulator());
	keyswitchManipulator->addMatrixManipulator('2', "Flight", new osgGA::FlightManipulator());
	keyswitchManipulator->addMatrixManipulator('3', "Drive", new osgGA::DriveManipulator());
	keyswitchManipulator->addMatrixManipulator('4', "Terrain", new osgGA::TerrainManipulator());

	viewer.setCameraManipulator(keyswitchManipulator.get());
	viewer.addEventHandler(new osgViewer::StatsHandler());
	viewer.addEventHandler(new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()));

	osg::DisplaySettings::instance()->setNumMultiSamples(8);
	//viewer.getCamera()->setSmallFeatureCullingPixelSize(-1.0f);

	viewer.getCamera()->getGraphicsContext()->getState()->setUseModelViewAndProjectionUniforms(true);

	//Add sample data path
	osgDB::Registry::instance()->getDataFilePathList().push_back("../data");
	osgDB::Registry::instance()->getDataFilePathList().push_back("./data"); //hack to be able to run from GCC out dir

	std::vector<XBFVegetationData> data;
	
	XBFVegetationData tree_data(1500.0f, 40, 2 , 10.0);
	tree_data.MeshLODs.push_back(XBFVegetationData::XBFLOD("trees/fir01_l0.osg", 140));
	tree_data.MeshLODs.push_back(XBFVegetationData::XBFLOD("trees/fir01_l1_bb.osg"));
	data.push_back(tree_data);

	/*osgVegetation::BillboardLayer grass_data1(240, 8, 1.0, 0.8, 0.1, 5);
	grass_data1.Type = osgVegetation::BillboardLayer::BLT_GRASS;
	grass_data1.Billboards.push_back(osgVegetation::BillboardLayer::Billboard("billboards/veg_plant03.png", osg::Vec2f(4, 2), 0.9, 0.02));
	grass_data1.Billboards.push_back(osgVegetation::BillboardLayer::Billboard("billboards/veg_plant01.png", osg::Vec2f(2, 2), 0.9, 0.02));
	grass_data1.Billboards.push_back(osgVegetation::BillboardLayer::Billboard("billboards/grass2.png", osg::Vec2f(2, 1), 1.0, 1.0));
	*/

	osgVegetation::BillboardLayer grass_data1(240, 2, 1.0, 0.3, 0.1, 5);
	grass_data1.Type = osgVegetation::BillboardLayer::BLT_GRASS;
	grass_data1.Billboards.push_back(osgVegetation::BillboardLayer::Billboard("billboards/veg_plant03.png", osg::Vec2f(4, 2), 0.9, 0.008));
	grass_data1.Billboards.push_back(osgVegetation::BillboardLayer::Billboard("billboards/veg_plant01.png", osg::Vec2f(2, 2), 0.9, 0.002));
	grass_data1.Billboards.push_back(osgVegetation::BillboardLayer::Billboard("billboards/grass2.png", osg::Vec2f(2, 1), 1.0, 1.0));

	/*osgVegetation::BillboardLayer grass_data2(100, 16, 1.0, 0.8, 0.1, 5);
	grass_data2.Type = osgVegetation::BillboardLayer::BLT_GRASS;
	grass_data2.Billboards.push_back(osgVegetation::BillboardLayer::Billboard("billboards/grass2.png", osg::Vec2f(1, 1), 1.0, 1.0));
	*/

	std::vector<osgVegetation::BillboardLayer> bb_layers;
	bb_layers.push_back(grass_data1);
	//bb_layers.push_back(grass_data2);

	osgDB::Registry::instance()->setReadFileCallback(new VegetationReadFileCallback(data, bb_layers));

	osg::ref_ptr<osg::Node> rootnode = osgDB::readNodeFile("terrain/us-terrain.zip/us-terrain.osg");
	//osg::ref_ptr<osg::Node> rootnode = osgDB::readNodeFile("D:/terrain/vpb/us/us-terrain.zip/us-terrain.osg");
	osgVegetation::PrepareTerrainForDetailMapping(rootnode);
	

	//Add light and shadows
	osg::Light* pLight = new osg::Light;
	pLight->setDiffuse(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
	osg::Vec4 lightPos(1, 1.0, 1, 0);
	pLight->setPosition(lightPos);		// last param	w = 0.0 directional light (direction)
	osg::Vec3f lightDir(-lightPos.x(), -lightPos.y(), -lightPos.z());
	lightDir.normalize();
	pLight->setDirection(lightDir);
	pLight->setAmbient(osg::Vec4(0.5f, 0.5f, 0.5f, 1.0f));

	osg::LightSource* pLightSource = new osg::LightSource;
	pLightSource->setLight(pLight);

	rootnode->asGroup()->addChild(pLightSource);

	bool use_fog = true;
	if (use_fog)
	{
		const osg::Vec4 fog_color(0.5, 0.6, 0.7, 1.0);
		//Add fog
		osg::StateSet* state = rootnode->getOrCreateStateSet();
		osg::ref_ptr<osg::Fog> fog = new osg::Fog();
		state->setMode(GL_FOG, osg::StateAttribute::ON);
		state->setAttributeAndModes(fog.get());
		fog->setMode(osg::Fog::EXP2);
		fog->setDensity(0.0005);
		fog->setColor(fog_color);
		viewer.getCamera()->setClearColor(fog_color);
		osg::StateSet::DefineList& defineList = state->getDefineList();
		defineList["FM_EXP2"].second = (osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
	}


	viewer.setUpViewInWindow(100, 100, 800, 600);

	viewer.getCamera()->getGraphicsContext()->getState()->setUseModelViewAndProjectionUniforms(true);

	viewer.setSceneData(rootnode);
	while (!viewer.done())
	{
		//animate light if shadows enabled
		//if(shadow_type != osgVegetation::SM_DISABLED)
		{
			float t = viewer.getFrameStamp()->getSimulationTime() * 0.5;
			//lightPos.set(sinf(t), cosf(t), 0.5 + 0.45*cosf(t), 0.0f);
			lightPos.set(sinf(t), cosf(t), 1.0f/*0.5 + 0.45*cosf(t)*/, 0.0f);
			//lightPos.set(1.0, 0, 0.5 + 0.45*cosf(t), 0.0f);
			//lightPos.set(0.2f,0,1.1 + cosf(t),0.0f);
			pLight->setPosition(lightPos);
			lightDir.set(-lightPos.x(), -lightPos.y(), -lightPos.z());
			lightDir.normalize();
			pLight->setDirection(lightDir);
		}
		viewer.frame();
	}
	return 0;
}

#else
int main(int argc, char** argv)
{
	OSG_WARN << "Sorry, but XFB requires at least OSG 3.4.0." << std::endl;
	return -1;
}
#endif