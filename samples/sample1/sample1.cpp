#include <osg/AlphaFunc>
#include <osg/Billboard>
#include <osg/BlendFunc>
#include <osg/Depth>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Material>
#include <osg/Math>
#include <osg/MatrixTransform>
#include <osg/PolygonOffset>
#include <osg/Projection>
#include <osg/ShapeDrawable>
#include <osg/StateSet>
#include <osg/Switch>
#include <osg/Texture2D>
#include <osg/TextureBuffer>
#include <osg/Image>
#include <osg/TexEnv>
#include <osg/VertexProgram>
#include <osg/FragmentProgram>
#include <osg/ComputeBoundsVisitor>
#include <osgDB/WriteFile>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osg/Texture2DArray>
#include <osgUtil/LineSegmentIntersector>
#include <osgUtil/IntersectionVisitor>
#include <osgUtil/SmoothingVisitor>
#include <osgText/Text>
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
#include <osgGA/SphericalManipulator>

#include <iostream>
#include <sstream>
// for the grid data..
#include "terrain_coords.h"
#include "MeshScattering.h"
#include "MRTShaderInstancing.h"
#include "QuadTreeScattering.h"

int main( int argc, char **argv )
{
	// use an ArgumentParser object to manage the program arguments.
	osg::ArgumentParser arguments(&argc,argv);

	// construct the viewer.
	osgViewer::Viewer viewer(arguments);
	
	// add the stats handler
	viewer.addEventHandler(new osgViewer::StatsHandler);

	//viewer.addEventHandler(new TechniqueEventHandler(ttm.get()));
	viewer.addEventHandler(new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()));


	osg::ref_ptr<osgGA::KeySwitchMatrixManipulator> keyswitchManipulator = new osgGA::KeySwitchMatrixManipulator;

	keyswitchManipulator->addMatrixManipulator( '1', "Trackball", new osgGA::TrackballManipulator() );
	keyswitchManipulator->addMatrixManipulator( '2', "Flight", new osgGA::FlightManipulator() );
	keyswitchManipulator->addMatrixManipulator( '3', "Drive", new osgGA::DriveManipulator() );
	keyswitchManipulator->addMatrixManipulator( '4', "Terrain", new osgGA::TerrainManipulator() );
	keyswitchManipulator->addMatrixManipulator( '5', "Orbit", new osgGA::OrbitManipulator() );
	keyswitchManipulator->addMatrixManipulator( '6', "FirstPerson", new osgGA::FirstPersonManipulator() );
	keyswitchManipulator->addMatrixManipulator( '7', "Spherical", new osgGA::SphericalManipulator() );
	viewer.setCameraManipulator( keyswitchManipulator.get() );
	
	//Add texture search paths
	//osgDB::Registry::instance()->getDataFilePathList().push_back("C:/temp/kvarn/Grid0/tiles");
	osgDB::Registry::instance()->getDataFilePathList().push_back("C:/dev/OSGLab/git/forest_test/data");  
	
	//osg::ref_ptr<osg::Node> terrain = osgDB::readNodeFile("C:/temp/kvarn/Grid0/tiles/0x1_3_3x3.ive.osg");
	//osg::ref_ptr<osg::Node> terrain = osgDB::readNodeFile("C:/temp/kvarn/Grid0/tiles/0x0_0_0x0.ive");
	//osg::ref_ptr<osg::Node> terrain = osgDB::readNodeFile("C:/temp/kvarn/proxy.osg");
	osg::ref_ptr<osg::Node> terrain = osgDB::readNodeFile("lz.osg");
	osg::Group* group = new osg::Group;
	group->addChild(terrain);

	enum MaterialEnum
	{
		GRASS,
		ROAD,
		WOODS,
		DIRT
	};
	std::map<MaterialEnum,osgVegetation::MaterialColor> material_map;
	material_map[GRASS] = osgVegetation::MaterialColor(0,0,1,1);
	material_map[WOODS] = osgVegetation::MaterialColor(1,1,1,1);
	material_map[ROAD] = osgVegetation::MaterialColor(0,0,1,1);
	material_map[DIRT] = osgVegetation::MaterialColor(1,0,0,1);

	osgVegetation::MeshLayer treelayer1; 
	treelayer1.Density = 0.03;
	treelayer1.Height.set(0.7,1.2);
	treelayer1.Width.set(0.9,1.1);
	treelayer1.IntensitySpan.set(1.1,1.5);
	
	treelayer1.MeshLODs.push_back(osgVegetation::MeshLod("trees/pine01_no_alpha.osg.osg.osg",300));
	treelayer1.MeshLODs.push_back(osgVegetation::MeshLod("trees/pine01_no_alpha.osg.osg",150));
	treelayer1.MeshLODs.push_back(osgVegetation::MeshLod("trees/pine01_no_alpha.osg",75));
	treelayer1.Materials.push_back(material_map[WOODS]);

	osgVegetation::MeshLayer treelayer2; 
	treelayer2.Density = 0.01;
	treelayer2.Height.set(7,12);
	treelayer2.Width.set(9,11);
	//	grass1.TextureName = "Images/veg_grass02.dds";
	treelayer2.IntensitySpan.set(0.7,1.0);
	//grass1.MeshName = "cube_mapped_torus.osgt";
	treelayer2.MeshLODs.push_back(osgVegetation::MeshLod("C:/temp/osgearth/osgearth/data/pinetree.ive",300));
	treelayer2.Materials.push_back(material_map[WOODS]);
	//grass1.MeshNames.push_back("pine01.ive");
	//grass1.MeshName = "C:/temp/osgearth/osgearth/data/pinetree.ive";
	//grass1.MeshName = "C:/temp/osgearth/osgearth/data/loopix/tree7.osgb";
	//grass1.MeshName = "C:/dev/GASSData/gfx/osg/3dmodels/genesis/patria.3ds";

	osgVegetation::MeshLayerVector layers;
	//layers.push_back(treelayer1);
	layers.push_back(treelayer2);
	
	osgVegetation::BillboardData undergrowth_data(50,true,0.5,true);
	osgVegetation::BillboardLayer  grass2("billboards/tree0.rgba"); 
	grass2.Density = 3.5;
	grass2.Height.set(0.3,0.6);
	grass2.Width.set(0.25,0.35);
	grass2.Scale.set(1.5,3);
	grass2.Materials.push_back(material_map[GRASS]);
	grass2.Materials.push_back(material_map[WOODS]);
	undergrowth_data.Layers.push_back(grass2);
	
	osgVegetation::BillboardData tree_data(400,false,0.08,false);
	osgVegetation::BillboardLayer  spruce("billboards/tree0.rgba");
	spruce.Density = 0.3;
	spruce.Height.set(3,5);
	spruce.Width.set(2,2);
	spruce.Scale.set(0.3,0.9);
	spruce.Materials.push_back(material_map[WOODS]);
	tree_data.Layers.push_back(spruce);
	
	osgVegetation::QuadTreeScattering scattering(terrain.get());
	osg::Node* tree_node = scattering.create(tree_data);
	group->addChild(tree_node);

	//osgDB::writeNodeFile(*bb_node,"c:/temp/bbveg.ive");

	// osg:Light allows us to set default parameter values to be used by osg::LightSource
	// note: we do not use light number 0, because we do not want to override the osg default headlights
	osg::Light* pLight = new osg::Light;
	//pLight->setLightNum( 4 );						
	pLight->setDiffuse( osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f) );
	pLight->setPosition( osg::Vec4(1,0,1,0) );		// last param	w = 0.0 directional light (direction)
	//				w = 1.0 point light (position)
	// light source
	osg::LightSource* pLightSource = new osg::LightSource;    
	pLightSource->setLight( pLight );

	group->addChild( pLightSource );
	
	viewer.setSceneData(group);
	
	return viewer.run();
}
