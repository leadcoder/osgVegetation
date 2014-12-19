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
#include "MeshScattering.h"
#include "MRTShaderInstancing.h"
#include "QuadTreeScattering.h"
#include "TerrainQuery.h"

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
	
	//Add data path
	osgDB::Registry::instance()->getDataFilePathList().push_back("C:/temp/OpenSceneGraph-Data-3.0.0");  

	//Add texture search paths
	osgDB::Registry::instance()->getDataFilePathList().push_back("F:/temp/detail_mapping/Grid0/tiles");
	osgDB::Registry::instance()->getDataFilePathList().push_back("F:/temp/detail_mapping/Grid0/material_textures");  
	osgDB::Registry::instance()->getDataFilePathList().push_back("F:/temp/detail_mapping/Grid0/color_textures");  
	//osg::ref_ptr<osg::Node> terrain = osgDB::readNodeFile("C:/temp/kvarn/Grid0/tiles/0x1_3_3x3.ive.osg");
	//osg::ref_ptr<osg::Node> terrain = osgDB::readNodeFile("C:/temp/kvarn/Grid0/tiles/0x0_0_0x0.ive");
	osg::ref_ptr<osg::Node> terrain = osgDB::readNodeFile("F:/temp/detail_mapping/proxy.osg");

	osg::Group* group = new osg::Group;
	group->addChild(terrain);


	//setup optimization variables
	std::string opt_env= "OSG_OPTIMIZER=COMBINE_ADJACENT_LODS SHARE_DUPLICATE_STATE MERGE_GEOMETRY MAKE_FAST_GEOMETRY CHECK_GEOMETRY OPTIMIZE_TEXTURE_SETTINGS STATIC_OBJECT_DETECTION";
#ifdef WIN32
	_putenv(opt_env.c_str());
#else
	char * writable = new char[opt_env.size() + 1];
	std::copy(opt_env.begin(), opt_env.end(), writable);
	writable[opt_env.size()] = '\0'; // don't forget the terminating 0
	putenv(writable);
	delete[] writable;
#endif


	enum MaterialEnum
	{
		GRASS,
		ROAD,
		WOODS,
		DIRT
	};
	std::map<MaterialEnum,osgVegetation::MaterialColor> material_map;
	material_map[GRASS] = osgVegetation::MaterialColor(0,0,0,1);
	material_map[WOODS] = osgVegetation::MaterialColor(0,1,0,1);
	material_map[ROAD] = osgVegetation::MaterialColor(0,0,1,1);
	material_map[DIRT] = osgVegetation::MaterialColor(1,0,0,1);

	//osgVegetation::BillboardData undergrowth_data(50,true,0.4,true);
	osgVegetation::BillboardData undergrowth_data(100,true,0.4,true);

	osgVegetation::BillboardLayer  grass2("Images/veg_grass02.dds"); 
	grass2.Density = 0.6;
	grass2.Height.set(0.3,0.6);
	grass2.Width.set(0.25,0.35);
	grass2.Scale.set(1.5,3);
	grass2.ColorIntensity.set(0.6,0.6);
	
	grass2.Materials.push_back(material_map[GRASS]);
	grass2.Materials.push_back(material_map[WOODS]);
	undergrowth_data.Layers.push_back(grass2);

	osgVegetation::BillboardLayer  grass3("Images/veg_plant03.dds"); 
	grass3.Density = 0.1;
	grass3.Height.set(0.6,1.2);
	grass3.Width.set(0.5,0.7);
	grass3.Scale.set(1.5,3);
	grass3.ColorIntensity.set(0.6,0.6);
	grass3.Materials.push_back(material_map[GRASS]);
	grass3.Materials.push_back(material_map[WOODS]);
	undergrowth_data.Layers.push_back(grass3);

	osgVegetation::BillboardData tree_data(600,false,0.08,false);

	osgVegetation::BillboardLayer  spruce("Images/spruce01.dds");
	spruce.Density = 0.03;
	spruce.Height.set(5,5);
	spruce.Width.set(2,2);
	spruce.Scale.set(2,3);
	spruce.ColorIntensity.set(1.0,1.0);
	spruce.Materials.push_back(material_map[WOODS]);
	tree_data.Layers.push_back(spruce);

	osgVegetation::BillboardLayer  pine("Images/pine01.dds"); 
	pine.Density = 0.03;
	pine.Height.set(5,5);
	pine.Width.set(2,2);
	pine.Scale.set(2,3);
	pine.ColorIntensity.set(1.0,1.0);
	pine.Materials.push_back(material_map[WOODS]);
	tree_data.Layers.push_back(pine);

	osgVegetation::BillboardLayer  birch("Images/birch01.dds");
	birch.Density = 0.03;
	birch.Height.set(4,4);
	birch.Width.set(4,4);
	birch.Scale.set(2,3);
	birch.Materials.push_back(material_map[WOODS]);
	tree_data.Layers.push_back(birch);


	std::string save_path("c:/temp/paged/");
	//add path to enable viewer to find LODS
	osgDB::Registry::instance()->getDataFilePathList().push_back(save_path);  
	
	osgVegetation::TerrainQuery tq(terrain.get());
	tq.setMaterialTextureSuffix(".tga");
	osgVegetation::QuadTreeScattering scattering(terrain.get(),&tq);
	osg::Node* ug_node = scattering.create(undergrowth_data,save_path, "ug_");
	group->addChild(ug_node);
	osgVegetation::QuadTreeScattering scattering2(terrain.get(),&tq);
	osg::Node* tree_node = scattering2.create(tree_data,save_path,"og_");
	group->addChild(tree_node);
	osgDB::writeNodeFile(*group, save_path + "terrain_and_veg.ive");
	//osgDB::writeNodeFile(*tree_node,"c:/temp/tree_veg.ive");
	
	osg::Light* pLight = new osg::Light;
	//pLight->setLightNum( 4 );						
	pLight->setDiffuse( osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f) );
	pLight->setPosition( osg::Vec4(1,0,1,0) );		// last param	w = 0.0 directional light (direction)
	pLight->setAmbient(osg::Vec4(0.7f, 0.7f, 0.7f, 1.0f) );

	// light source
	osg::LightSource* pLightSource = new osg::LightSource;    
	pLightSource->setLight( pLight );
	group->addChild( pLightSource );
	viewer.setSceneData(group);

	return viewer.run();
}
