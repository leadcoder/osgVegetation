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

#include "ov_BillboardTile.h"
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

#include <osgTerrain/Terrain>
#include <osgTerrain/TerrainTile>
#include <osgTerrain/GeometryTechnique>
#include <osg/Version>
#include <osgTerrain/Layer>

#include <osgFX/MultiTextureControl>
#include <osg/PositionAttitudeTransform>
#include <osg/PatchParameter>

#include <iostream>



#ifndef OSG_VERSION_GREATER_OR_EQUAL
#define OSG_VERSION_GREATER_OR_EQUAL(MAJOR, MINOR, PATCH) ((OPENSCENEGRAPH_MAJOR_VERSION>MAJOR) || (OPENSCENEGRAPH_MAJOR_VERSION==MAJOR && (OPENSCENEGRAPH_MINOR_VERSION>MINOR || (OPENSCENEGRAPH_MINOR_VERSION==MINOR && OPENSCENEGRAPH_PATCH_VERSION>=PATCH))))
#endif

#if OSG_VERSION_GREATER_OR_EQUAL( 3, 5, 1 )
#include <osgTerrain/DisplacementMappingTechnique>
#endif


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


class VegetationReadFileCallback : public osgDB::ReadFileCallback
{
public:
	VegetationReadFileCallback(const std::vector<osgVegetation::BillboardLayer> &data) : m_VegData(data)
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
#define OV_USE_TILE_ID_LOD_LEVEL
	virtual osgDB::ReaderWriter::ReadResult readNode(const std::string& filename, const osgDB::Options* options)
	{
		osgDB::ReaderWriter::ReadResult rr = ReadFileCallback::readNode(filename, options);

		TerrainTileVisitor ttv;
		rr.getNode()->accept(ttv);

#ifdef OV_USE_TILE_ID_LOD_LEVEL
		const int lod_level = ttv.Tiles.size() > 0 ? ttv.Tiles[0]->getTileID().level - 1 : 0;
#else
		const int lod_level = extractLODLevelFromFileName(filename);
#endif
		for (size_t i = 0; i < m_VegData.size(); i++)
		{
			if (lod_level == m_VegData[i].LODLevel && rr.validNode())
			{
				osg::Group* group = dynamic_cast<osg::Group*>(rr.getNode());
				osg::PagedLOD* plod = dynamic_cast<osg::PagedLOD*>(rr.getNode());
				if (group && plod == 0)
				{
					osg::Group* veg_layer = new osg::Group();
					group->addChild(veg_layer);
					for (size_t j = 0; j < ttv.Tiles.size(); j++)
					{
						osgVegetation::BillboardTile* bb_tile = new osgVegetation::BillboardTile(m_VegData[i], ttv.Tiles[j]);
						veg_layer->addChild(bb_tile);
					}
				}
			}
		}
		return rr;
	}
protected:
	virtual ~VegetationReadFileCallback() {}
	std::vector<osgVegetation::BillboardLayer> m_VegData;
};

int main(int argc, char** argv)
{
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

		std::string pathfile;
		char keyForAnimationPath = '5';
		while (arguments.read("-p", pathfile))
		{
			osgGA::AnimationPathManipulator* apm = new osgGA::AnimationPathManipulator(pathfile);
			if (apm || !apm->valid())
			{
				unsigned int num = keyswitchManipulator->getNumMatrixManipulators();
				keyswitchManipulator->addMatrixManipulator(keyForAnimationPath, "Path", apm);
				keyswitchManipulator->selectMatrixManipulator(num);
				++keyForAnimationPath;
			}
		}

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

	// obtain the vertical scale
	float verticalScale = 1.0f;
	while (arguments.read("-v", verticalScale)) {}

	// obtain the sample ratio
	float sampleRatio = 1.0f;
	while (arguments.read("-r", sampleRatio)) {}

	bool useDisplacementMappingTechnique = false;// arguments.read("--dm");
	if (useDisplacementMappingTechnique)
	{
		//osgDB::Registry::instance()->setReadFileCallback(new VegetationReadFileCallback());
	}

	bool setDatabaseThreadAffinity = false;
	unsigned int cpuNum = 0;
	while (arguments.read("--db-affinity", cpuNum)) { setDatabaseThreadAffinity = true; }

	osgVegetation::BillboardLayer grass_data(100, 16, 1.0,1.0, 0.1, 5);
	grass_data.Billboards.push_back(osgVegetation::BillboardLayer::Billboard("billboards/grass0.png", osg::Vec2f(1, 1),0.9));

	std::vector<osgVegetation::BillboardLayer> data;
	data.push_back(grass_data);

	osgVegetation::BillboardLayer tree_data(1740, 3, 0.5, 0.7, 0.1, 2);
	tree_data.Billboards.push_back(osgVegetation::BillboardLayer::Billboard("billboards/fir01_bb.png", osg::Vec2f(4, 8),1.4));
	data.push_back(tree_data);

	osgDB::Registry::instance()->setReadFileCallback(new VegetationReadFileCallback(data));
	// load the nodes from the commandline arguments.

#if OSG_VERSION_GREATER_OR_EQUAL(3,5,1)
	osg::ref_ptr<osg::Node> rootnode = osgDB::readRefNodeFiles(arguments);
#else
	osg::ref_ptr<osg::Node> rootnode = osgDB::readNodeFiles(arguments);
#endif

	
	if (!rootnode)
	{
		osg::notify(osg::NOTICE) << "Warning: no valid data loaded, please specify a database on the command line." << std::endl;
		return 1;
	}

	osg::ref_ptr<osgTerrain::Terrain> terrain = findTopMostNodeOfType<osgTerrain::Terrain>(rootnode.get());
	if (!terrain)
	{
		// no Terrain node present insert one above the loaded model.
		terrain = new osgTerrain::Terrain;

		// if CoordinateSystemNode is present copy it's contents into the Terrain, and discard it.
		osg::CoordinateSystemNode* csn = findTopMostNodeOfType<osg::CoordinateSystemNode>(rootnode.get());
		if (csn)
		{
			terrain->set(*csn);
			for (unsigned int i = 0; i < csn->getNumChildren(); ++i)
			{
				terrain->addChild(csn->getChild(i));
			}
		}
		else
		{
			terrain->addChild(rootnode.get());
		}

		rootnode = terrain.get();
	}

	//Add sample data path
	osgDB::Registry::instance()->getDataFilePathList().push_back("../data");

	osgVegetation::PrepareTerrainForDetailMapping(terrain);

	terrain->setSampleRatio(sampleRatio);
	terrain->setVerticalScale(verticalScale);
#if OSG_VERSION_GREATER_OR_EQUAL( 3, 5, 1 )
	if (useDisplacementMappingTechnique)
	{
		terrain->setTerrainTechniquePrototype(new osgTerrain::DisplacementMappingTechnique());
	}
#endif

	// add a viewport to the viewer and attach the scene graph.
	viewer.setSceneData(rootnode.get());

	// if required set the DatabaseThread affinity, note must call after viewer.setSceneData() so that the osgViewer::Scene object is constructed with it's DatabasePager.
	if (setDatabaseThreadAffinity)
	{
		for (unsigned int i = 0; i < viewer.getDatabasePager()->getNumDatabaseThreads(); ++i)
		{
			osgDB::DatabasePager::DatabaseThread* thread = viewer.getDatabasePager()->getDatabaseThread(i);
			thread->setProcessorAffinity(cpuNum);
			OSG_NOTICE << "Settings affinity of DatabaseThread=" << thread << " isRunning()=" << thread->isRunning() << " cpuNum=" << cpuNum << std::endl;
		}
	}

	// following are tests of the #pragma(tic) shader composition
	//terrain->getOrCreateStateSet()->setDefine("NUM_LIGHTS", "1");
	//terrain->getOrCreateStateSet()->setDefine("LIGHTING"); // , osg::StateAttribute::OFF|osg::StateAttribute::OVERRIDE);
	//terrain->getOrCreateStateSet()->setDefine("COMPUTE_DIAGONALS"); // , osg::StateAttribute::OFF|osg::StateAttribute::OVERRIDE);
	viewer.getCamera()->getGraphicsContext()->getState()->setUseModelViewAndProjectionUniforms(true);
	//viewer.getCamera()->getGraphicsContext()->getState()->resetVertexAttributeAlias(false, 8);
	//viewer.getCamera()->getGraphicsContext()->getState()->setUseVertexAttributeAliasing(true);
	// run the viewers main loop
	return viewer.run();

}
