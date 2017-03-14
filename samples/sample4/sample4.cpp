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
#include "VegGeometryTechnique.h"
#include "VegetationUtils.h"

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


//#include "vegTerrainTech.h"
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
		//osgDB::Registry::instance()->setReadFileCallback(new CleanTechniqueReadFileCallback());
	}

	bool setDatabaseThreadAffinity = false;
	unsigned int cpuNum = 0;
	while (arguments.read("--db-affinity", cpuNum)) { setDatabaseThreadAffinity = true; }

	//MyTileLoadedCallback* cb = new MyTileLoadedCallback();
	//osgTerrain::TerrainTile::setTileLoadedCallback(cb);

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

	std::vector<std::string> tex_names;
	tex_names.push_back("billboards/grass0.png");
	//osg::ref_ptr<osg::Texture2DArray> textures = osgVegetation::Utils::loadTextureArray(tex_names);

	terrain->setTerrainTechniquePrototype(new VegGeometryTechnique());
	terrain->setSampleRatio(sampleRatio);
	terrain->setVerticalScale(verticalScale);
#if OSG_VERSION_GREATER_OR_EQUAL( 3, 5, 1 )
	if (useDisplacementMappingTechnique)
	{
		terrain->setTerrainTechniquePrototype(new osgTerrain::DisplacementMappingTechnique());
	}
#endif
	// register our custom handler for adjust Terrain settings
	//viewer.addEventHandler(new TerrainHandler(terrain.get(), findTopMostNodeOfType<osgFX::MultiTextureControl>(rootnode.get())));

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
