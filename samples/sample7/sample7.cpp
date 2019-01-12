#include "ov_MeshTileGenerator.h"
#include <osg/Vec4i>
#include <osg/Quat>
#include <osg/Geometry>
#include <osg/CullFace>
#include <osg/Image>
#include <osg/Texture>
#include <osg/LightSource>
#include <osg/MatrixTransform>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osgDB/WriteFile>
#include <osgGA/StateSetManipulator>
#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>
#include <osgGA/TerrainManipulator>
#include <osgGA/KeySwitchMatrixManipulator>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgTerrain/TerrainTile>
#include <osg/PositionAttitudeTransform>

namespace osgVegetation
{

	class VegetationTile : public osg::PositionAttitudeTransform
	{
	public:
		VegetationTile(MeshTileGenerator& gen, osgTerrain::TerrainTile* tile)
		{
			osgTerrain::HeightFieldLayer* layer = dynamic_cast<osgTerrain::HeightFieldLayer*>(tile->getElevationLayer());
			if (layer)
			{
				osg::HeightField* hf = layer->getHeightField();
				if (hf)
				{
					osg::Geometry* hf_geom = _CreateGeometryFromHeightField(hf);
					setPosition(hf->getOrigin());
					addChild(gen.CreateNode(hf_geom));
				}
			}
		}
	private:
		static osg::Geometry* _CreateGeometryFromHeightField(osg::HeightField* hf)
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

			osg::Vec3 pos(local_origin.x(), local_origin.y(), local_origin.z());
			osg::Vec2 tex(0.0f, 0.0f);
			int vi = 0;
			for (unsigned int r = 0; r < numRows; ++r)
			{
				pos.x() = local_origin.x();
				tex.x() = 0.0f;
				for (unsigned int c = 0; c < numColumns; ++c)
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
			for (unsigned int r = 0; r < numRows - 1; ++r)
			{
				for (unsigned int c = 0; c < numColumns - 1; ++c)
				{
					// Try to imitate how GeometryTechnique::generateGeometry optimize 
					// which way to put the diagonal by choosing to
					// place it between the two corners that have the least curvature
					// relative to each other.
					// Due to how normals are calculated we don't get exact match...fix this by using same normal calulations

					osg::Vec3 n00 = hf->getNormal(c, r);
					osg::Vec3 n01 = hf->getNormal(c, r + 1);
					osg::Vec3 n10 = hf->getNormal(c + 1, r);
					osg::Vec3 n11 = hf->getNormal(c + 1, r + 1);
					float dot_00_11 = n00 * n11;
					float dot_01_10 = n01 * n10;
					if (dot_00_11 > dot_01_10)
					{
						drawElements[ei++] = (r)*numColumns + c;
						drawElements[ei++] = (r)*numColumns + c + 1;
						drawElements[ei++] = (r + 1)*numColumns + c + 1;

						drawElements[ei++] = (r + 1)*numColumns + c + 1;
						drawElements[ei++] = (r + 1)*numColumns + c;
						drawElements[ei++] = (r)*numColumns + c;
					}
					else
					{
						drawElements[ei++] = (r)*numColumns + c;
						drawElements[ei++] = (r)*numColumns + c + 1;
						drawElements[ei++] = (r + 1)*numColumns + c;

						drawElements[ei++] = (r)*numColumns + c + 1;
						drawElements[ei++] = (r + 1)*numColumns + c + 1;
						drawElements[ei++] = (r + 1)*numColumns + c;
					}
				}
			}
			geometry->setUseDisplayList(false);
			return geometry;
		}
	};

	class VegetationReadFileCallback : public osgDB::ReadFileCallback
	{
	public:
		VegetationReadFileCallback(const std::vector<MeshTileGeneratorConfig> &data) : m_VegData(data), m_Generator(data[0])
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
							veg_layer->addChild(new VegetationTile(m_Generator, ttv.Tiles[j]));
						}
					}
				}
			}
			return rr;
		}
	protected:
		virtual ~VegetationReadFileCallback() {}
		std::vector<MeshTileGeneratorConfig> m_VegData;
		MeshTileGenerator m_Generator;
	};
}

int main(int argc, char **argv)
{
	// use an ArgumentParser object to manage the program arguments.
	osg::ArgumentParser arguments(&argc, argv);

	arguments.getApplicationUsage()->setDescription(arguments.getApplicationName() + "");
	arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName() + " [options] ");
	arguments.getApplicationUsage()->addCommandLineOption("-h or --help", "Display this information");
	
	if (arguments.read("-h") || arguments.read("--help"))
	{
		arguments.getApplicationUsage()->write(std::cout);
		return 1;
	}

	//Add sample data path
	osgDB::Registry::instance()->getDataFilePathList().push_back("../data");

	// construct the viewer.
	osgViewer::Viewer viewer(arguments);
	//Use multisampling
	osg::DisplaySettings::instance()->setNumMultiSamples(4);

	// Add basic event handlers and manipulators
	viewer.addEventHandler(new osgViewer::StatsHandler);
	viewer.addEventHandler(new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()));
	viewer.addEventHandler(new osgViewer::ThreadingHandler);

	osg::ref_ptr<osgGA::KeySwitchMatrixManipulator> keyswitchManipulator = new osgGA::KeySwitchMatrixManipulator;

	keyswitchManipulator->addMatrixManipulator('1', "Trackball", new osgGA::TrackballManipulator());
	keyswitchManipulator->addMatrixManipulator('2', "Flight", new osgGA::FlightManipulator());
	keyswitchManipulator->addMatrixManipulator('3', "Drive", new osgGA::DriveManipulator());
	keyswitchManipulator->addMatrixManipulator('4', "Terrain", new osgGA::TerrainManipulator());
	viewer.setCameraManipulator(keyswitchManipulator.get());

	// create root node
	osg::ref_ptr<osg::Group> root = new osg::Group;

	// add lightsource to root node
	osg::ref_ptr<osg::LightSource> lSource = new osg::LightSource;
	lSource->getLight()->setPosition(osg::Vec4(1.0, -1.0, 1.0, 0.0));
	lSource->getLight()->setDiffuse(osg::Vec4(0.9, 0.9, 0.9, 1.0));
	lSource->getLight()->setAmbient(osg::Vec4(0.1, 0.1, 0.1, 1.0));
	root->addChild(lSource.get());

	osgVegetation::MeshTileGeneratorConfig layer(10000, 5);
	osgVegetation::MeshTypeConfig mesh_data1;
	mesh_data1.MeshLODs.push_back(osgVegetation::MeshTypeConfig::MeshLODConfig("trees/fir01_l0.osg", osg::Vec4(0.0f, 0.0f, 100.0f, 110.0f)));
	mesh_data1.MeshLODs.push_back(osgVegetation::MeshTypeConfig::MeshLODConfig("trees/fir01_l1.osg", osg::Vec4(100.0f, 110.0f, 2500.0f, 2510.0f)));
	//mesh_data.MeshLODs.push_back(MeshTypeConfig::MeshLODConfig("LOD2", osg::Vec4(500.0f, 510.0f, 1200.0f, 1210.0f)));
	layer.MeshTypes.push_back(mesh_data1);
	
	osgVegetation::MeshTypeConfig  mesh_data2;
	mesh_data2.MeshLODs.push_back(osgVegetation::MeshTypeConfig::MeshLODConfig("trees/test.osg", osg::Vec4(0.0f, 0.0f, 2000.0f, 2500.0f)));
	layer.MeshTypes.push_back(mesh_data2);

	std::vector<osgVegetation::MeshTileGeneratorConfig> layers;
	layers.push_back(layer);

	osgDB::Registry::instance()->setReadFileCallback(new osgVegetation::VegetationReadFileCallback(layers));
	osg::ref_ptr<osg::Node> terrain_node = osgDB::readNodeFile("terrain/us-terrain.zip/us-terrain.osg");
	root->addChild(terrain_node);

	viewer.setSceneData(root.get());
	//viewer.realize();

	viewer.setUpViewInWindow(100, 100, 800, 600);
	viewer.getCamera()->getGraphicsContext()->getState()->setUseModelViewAndProjectionUniforms(true);

	// shaders use osg_ variables so we must do the following
	/*osgViewer::Viewer::Windows windows;
	viewer.getWindows(windows);
	for (osgViewer::Viewer::Windows::iterator itr = windows.begin(); itr != windows.end(); ++itr)
		(*itr)->getState()->setUseModelViewAndProjectionUniforms(true);
		*/
	return viewer.run();
}