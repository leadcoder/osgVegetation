

#include "XBFVegetationTile.h"

#include <osg/Version>
#include <osg/Notify>

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
	VegetationReadFileCallback(const std::vector<XBFVegetationData> &data) : m_VegData(data)
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

		int lod_level = extractLODLevelFromFileName(filename);

		for (size_t i = 0; i < m_VegData.size(); i++)
		{
			if (lod_level == m_VegData[i].TargetLODLevel && rr.validNode())
			{
				TerrainTileVisitor ttv;
				rr.getNode()->accept(ttv);

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
		return rr;
	}
protected:
	virtual ~VegetationReadFileCallback() {}
	std::vector<XBFVegetationData> m_VegData;
};

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

	viewer.getCamera()->setSmallFeatureCullingPixelSize(-1.0f);

	viewer.getCamera()->getGraphicsContext()->getState()->setUseModelViewAndProjectionUniforms(true);

	//Add sample data path
	osgDB::Registry::instance()->getDataFilePathList().push_back("../data");
	osgDB::Registry::instance()->getDataFilePathList().push_back("./data"); //hack to be able to run from GCC out dir

	std::vector<XBFVegetationData> data;
	
	XBFVegetationData tree_data(1000.0f, 2, 2);
	tree_data.MeshLODs.push_back(XBFVegetationData::XBFLOD("trees/fir01_l0.osg", 400));
	tree_data.MeshLODs.push_back(XBFVegetationData::XBFLOD("trees/fir01_l1.osg"));
	data.push_back(tree_data);

	osgDB::Registry::instance()->setReadFileCallback(new VegetationReadFileCallback(data));

	osg::ref_ptr<osg::Node> rootnode = osgDB::readNodeFile("D:/terrain/vpb/us/us-terrain.zip/us-terrain.osg");
	PrepareTerrain(rootnode);
	//osg::ref_ptr<osgTerrain::Terrain> terrain = findTopMostNodeOfType<osgTerrain::Terrain>(rootnode.get());
	viewer.setSceneData(rootnode);
	viewer.run();
}

#else
int main(int argc, char** argv)
{
	OSG_WARN << "Sorry, but XFB requires at least OSG 3.4.0." << std::endl;
	return -1;
}
#endif