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
//#include "VegGeometryTechnique.h"
#include "VegetationUtils.h"
#include "Tessellation.h"

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

/*
class MyTileLoadedCallback : public osgTerrain::TerrainTile::TileLoadedCallback
{
public:
	MyTileLoadedCallback() {}
	virtual bool deferExternalLayerLoading() const { return false; }
	virtual void loaded(osgTerrain::TerrainTile* tile, const osgDB::ReaderWriter::Options* options) const
	{
		osgTerrain::HeightFieldLayer* layer = dynamic_cast<osgTerrain::HeightFieldLayer*>(tile->getElevationLayer());
		if (layer)
		{
			osg::HeightField* hf = layer->getHeightField();
			if (hf)
			{
				unsigned int numRows = hf->getNumColumns();
				unsigned int numColumns = hf->getNumRows();
				float columnCoordDelta = hf->getXInterval();
				float rowCoordDelta = hf->getYInterval();

				osg::Geometry* geometry = new osg::Geometry;

				osg::Vec3Array& v = *(new osg::Vec3Array(numColumns*numRows));
				osg::Vec4ubArray& color = *(new osg::Vec4ubArray(1));

				color[0].set(255, 255, 255, 255);


				float rowTexDelta = 1.0f / (float)(numRows - 1);
				float columnTexDelta = 1.0f / (float)(numColumns - 1);
				osg::Vec3 origin = hf->getOrigin();
				origin.z() += 10;
				osg::Vec3 pos(0, 0, origin.z());
				osg::Vec2 tex(0.0f, 0.0f);
				int vi = 0;
				for (int r = 0; r < numRows; ++r)
				{
					pos.x() = 0.0;// origin.x();
					tex.x() = 0.0f;
					for (int c = 0; c < numColumns; ++c)
					{
						float h = hf->getHeight(c, r);
						v[vi].set(pos.x(), pos.y(), h + 10);
						pos.x() += columnCoordDelta;
						tex.x() += columnTexDelta;
						++vi;
					}
					pos.y() += rowCoordDelta;
					tex.y() += rowTexDelta;
				}

				geometry->setVertexArray(&v);
				geometry->setColorArray(&color, osg::Array::BIND_OVERALL);

			

				osg::DrawElementsUShort& drawElements = *(new osg::DrawElementsUShort(GL_PATCHES, 2 * 3 * (numColumns*numRows)));
				geometry->addPrimitiveSet(&drawElements);
				int ei = 0;
				for (int r = 0; r < numRows - 1; ++r)
				{
					for (int c = 0; c < numColumns - 1; ++c)
					{
			
						drawElements[ei++] = (r)*numColumns + c;
						drawElements[ei++] = (r)*numColumns + c + 1;
						drawElements[ei++] = (r + 1)*numColumns + c + 1;

						drawElements[ei++] = (r + 1)*numColumns + c + 1;
						drawElements[ei++] = (r + 1)*numColumns + c;
						drawElements[ei++] = (r)*numColumns + c;
					}
				}


				//osg::Vec3 size(numColumns*columnCoordDelta, numRows*rowCoordDelta, 100);
				osg::Vec3 size(FLT_MAX, FLT_MAX, FLT_MAX);
				//geometry->setInitialBound(osg::BoundingBox(origin, origin + size));
				geometry->setInitialBound(osg::BoundingBox(-size, size));
				geometry->setUseDisplayList(false);

				osg::Geode* geode = new osg::Geode();
				geode->addDrawable(geometry);
				geode->getOrCreateStateSet()->setDataVariance(osg::Object::DYNAMIC);
				osg::PositionAttitudeTransform* pat = new osg::PositionAttitudeTransform();
				pat->setPosition(origin);
				pat->addChild(geode);

				if (tile && tile->getNumParents() > 0)
				{
					osg::PagedLOD* plod = dynamic_cast<osg::PagedLOD*>(tile->getParent(0));
					//osg::ref_ptr<osg::MatrixTransform> trans = dynamic_cast<osg::MatrixTransform*>(buffer._transform->clone(osg::CopyOp::SHALLOW_COPY));

					if (plod)
					{
						osg::Group* plod_group = dynamic_cast<osg::Group*>(plod->getParent(0));
						if (plod_group)
							plod_group->addChild(pat);
					}
				}
				
				//tile->addChild(geode);
				//geode->setName("-----------hejsan");
			}
		}
	}
protected:
	virtual ~MyTileLoadedCallback() {};
};
*/

class CleanTechniqueReadFileCallback : public osgDB::ReadFileCallback
{

public:
	CleanTechniqueReadFileCallback(const std::vector<osgVegetation::VegetationData> &data) : m_VegData(data)
	{

	}
	

	class TerrainTileVisitor : public osg::NodeVisitor
	{
	public:
		std::vector<osgTerrain::TerrainTile*> Tiles;
		osg::Group* MainGroup;
		TerrainTileVisitor() :
			osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN){
		}

		void apply(osg::Node& node)
		{
			osgTerrain::TerrainTile* tile = dynamic_cast<osgTerrain::TerrainTile*>(&node);
			if (tile)
			{
				Tiles.push_back(tile);
				//osgTerrain::TileID id = tile->getTileID();
				/*
				osgTerrain::HeightFieldLayer* layer = dynamic_cast<osgTerrain::HeightFieldLayer*>(tile->getElevationLayer());
				if (layer)
				{
					osg::HeightField* hf = layer->getHeightField();
					if (hf)
					{
						unsigned int numColumns = hf->getNumColumns();
						unsigned int numRows = hf->getNumRows();
						float columnCoordDelta = hf->getXInterval();
						float rowCoordDelta = hf->getYInterval();

						osg::Geometry* geometry = new osg::Geometry;

						osg::Vec3Array& v = *(new osg::Vec3Array(numColumns*numRows));
						osg::Vec2Array& t = *(new osg::Vec2Array(numColumns*numRows));
						osg::Vec4ubArray& color = *(new osg::Vec4ubArray(1));

						color[0].set(255, 255, 255, 255);


						float rowTexDelta = 1.0f / (float)(numRows - 1);
						float columnTexDelta = 1.0f / (float)(numColumns - 1);

						osg::Vec3 local_origin(0, 0, 0);
						//local_origin.x() = -(columnCoordDelta*(numColumns - 1)) / 2.0;
						//local_origin.y() = -(rowCoordDelta*(numRows - 1)) / 2.0;
						//origin.z() += 1;

						osg::Vec3 pos(local_origin.x(), local_origin.y(), local_origin.z());
						osg::Vec2 tex(0.0f, 0.0f);
						int vi = 0;
						for (int r = 0; r < numRows; ++r)
						{
							pos.x() = local_origin.x();
							tex.x() = 0.0f;
							for (int c = 0; c < numColumns; ++c)
							{
								float h = hf->getHeight(c, r);
								v[vi].set(pos.x(), pos.y(), h);
								t[vi].set(tex.x(), tex.y());
								pos.x() += columnCoordDelta;
								tex.x() += columnTexDelta;
								++vi;
							}
							pos.y() += rowCoordDelta;
							tex.y() += rowTexDelta;
						}

				
						geometry->setVertexArray(&v);
						geometry->setTexCoordArray(0, &t);
						geometry->setColorArray(&color, osg::Array::BIND_OVERALL);

					
						osg::DrawElementsUShort& drawElements = *(new osg::DrawElementsUShort(GL_PATCHES, 2 * 3 * (numColumns*numRows)));
						geometry->addPrimitiveSet(&drawElements);
						int ei = 0;
						for (int r = 0; r < numRows - 1; ++r)
						{
							for (int c = 0; c < numColumns - 1; ++c)
							{

								drawElements[ei++] = (r)*numColumns + c;
								drawElements[ei++] = (r)*numColumns + c + 1;
								drawElements[ei++] = (r + 1)*numColumns + c + 1;

								drawElements[ei++] = (r + 1)*numColumns + c + 1;
								drawElements[ei++] = (r + 1)*numColumns + c;
								drawElements[ei++] = (r)*numColumns + c;
							}
						}

						//osg::Vec3 size(numColumns*columnCoordDelta, numRows*rowCoordDelta, 100);
						osg::Vec3 size(FLT_MAX, FLT_MAX, FLT_MAX);
						//geometry->setInitialBound(osg::BoundingBox(origin, origin + size));
						//geometry->setInitialBound(osg::BoundingBox(-size, size));
						geometry->setUseDisplayList(false);

						osg::Geode* geode = new osg::Geode();
						geode->addDrawable(geometry);
						geode->getOrCreateStateSet()->setDataVariance(osg::Object::DYNAMIC);
						*/
						
					/*	osg::Geode* hf_geom = osgVegetation::GeomConvert::HeightFieldToTriPatches(hf);
						osgVegetation::VegetationData::PrepareVegLayer(hf_geom->getOrCreateStateSet(), VegData);

						//Add color texture
						osgTerrain::Layer* colorLayer = tile->getColorLayer(0);
						if (colorLayer)
						{
							osg::Image* image = colorLayer->getImage();
							osg::Texture2D* texture = new osg::Texture2D(image);
							hf_geom->getOrCreateStateSet()->setTextureAttributeAndModes(0, texture, osg::StateAttribute::ON);
						}
						osg::PositionAttitudeTransform* pat = new osg::PositionAttitudeTransform();
						pat->setPosition(hf->getOrigin());
						pat->addChild(hf_geom);
						Tiles.push_back(pat);
						*/
						/*if (tile && tile->getNumParents() > 0)
						{
							osg::PagedLOD* plod = dynamic_cast<osg::PagedLOD*>(tile->getParent(0));
							//osg::ref_ptr<osg::MatrixTransform> trans = dynamic_cast<osg::MatrixTransform*>(buffer._transform->clone(osg::CopyOp::SHALLOW_COPY));

							if (plod && plod->getNumParents() > 0)
							{
								RetGroup = dynamic_cast<osg::Group*>(plod->getParent(0));
							}
						}*/
					//}
				//}
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
			if (lod_level == m_VegData[i].LODLevel && rr.validNode())
			{
				TerrainTileVisitor ttv;
				rr.getNode()->accept(ttv);

				osg::Group* group = dynamic_cast<osg::Group*>(rr.getNode());
				osg::PagedLOD* plod = dynamic_cast<osg::PagedLOD*>(rr.getNode());
				if (group && plod == 0)
				{
					osg::Group* veg_layer = new osg::Group();
					group->addChild(veg_layer);
					osgVegetation::VegetationData::PrepareVegLayer(veg_layer->getOrCreateStateSet(), m_VegData[i]);
					for (size_t j = 0; j < ttv.Tiles.size(); j++)
					{
						osg::Node* veg_geometry_node = osgVegetation::GeomConvert::ToTriPatchGeometry(ttv.Tiles[j]);
						veg_layer->addChild(veg_geometry_node);
					}
				}
			
			}
		}
		return rr;
	}

protected:
	virtual ~CleanTechniqueReadFileCallback() {}

	std::vector<osgVegetation::VegetationData> m_VegData;
};


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


	osgVegetation::VegetationData grass_data(100, 16, 4);
	grass_data.Billboards.push_back(osgVegetation::Billboard("billboards/grass0.png", osg::Vec2f(1, 1)));

	std::vector<osgVegetation::VegetationData> data;
	data.push_back(grass_data);

	osgVegetation::VegetationData tree_data(1740, 2, 2);
	tree_data.Billboards.push_back(osgVegetation::Billboard("billboards/fir01_bb.png", osg::Vec2f(4, 8)));
//	data.push_back(tree_data);

	osgDB::Registry::instance()->setReadFileCallback(new CleanTechniqueReadFileCallback(data));
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

	//osg::ref_ptr<osg::Texture2DArray> textures = osgVegetation::Utils::loadTextureArray(tex_names);

	//terrain->setTerrainTechniquePrototype(new VegGeometryTechnique());

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
