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
	osgDB::Registry::instance()->getDataFilePathList().push_back("E:/temp/detail_mapping/Grid0/tiles");
	osgDB::Registry::instance()->getDataFilePathList().push_back("E:/temp/detail_mapping/Grid0/material_textures");  
	osgDB::Registry::instance()->getDataFilePathList().push_back("E:/temp/detail_mapping/Grid0/color_textures");  
	//osg::ref_ptr<osg::Node> terrain = osgDB::readNodeFile("C:/temp/kvarn/Grid0/tiles/0x1_3_3x3.ive.osg");
	//osg::ref_ptr<osg::Node> terrain = osgDB::readNodeFile("C:/temp/kvarn/Grid0/tiles/0x0_0_0x0.ive");
	osg::ref_ptr<osg::Node> terrain = osgDB::readNodeFile("E:/temp/detail_mapping/proxy.osg");

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
	
	osgVegetation::BillboardLayer  grass_l0("Images/veg_grass02.dds",50); 
	grass_l0.Density = 3.2;
	grass_l0.Height.set(0.5,0.6);
	grass_l0.Width.set(0.5,0.6);
	grass_l0.Scale.set(1.5,3);
	grass_l0.ColorIntensity.set(0.1,0.1);
	grass_l0.MixInColorRatio = 3.0;
	grass_l0.MixInIntensity = true;
	grass_l0.Materials.push_back(material_map[GRASS]);
	grass_l0.Materials.push_back(material_map[WOODS]);
	
	osgVegetation::BillboardLayer  grass_l1 = grass_l0;
	grass_l1.ViewDistance *= 0.5; 
	grass_l1.Density *= 4;
	grass_l1.Scale *= 0.8;

	osgVegetation::BillboardLayer  grass_l2 = grass_l1;
	grass_l2.ViewDistance *= 0.5; 
	grass_l2.Density *= 4;
	grass_l2.Scale *= 0.8;

	osgVegetation::BillboardLayer plant_l0("Images/veg_plant03.dds",100); 
	plant_l0.Density = 0.1;
	plant_l0.Height.set(0.6,1.2);
	plant_l0.Width.set(0.5,0.7);
	plant_l0.Scale.set(1.5,3);
	plant_l0.ColorIntensity.set(0.1,0.1);
	plant_l0.MixInColorRatio = 2.5;
	plant_l0.MixInIntensity = true;
	plant_l0.Materials.push_back(material_map[GRASS]);
	plant_l0.Materials.push_back(material_map[WOODS]);

	osgVegetation::BillboardLayer  plant_l1 = plant_l0;
	plant_l1.ViewDistance *= 0.5; 
	plant_l1.Density *= 4;
	plant_l1.Scale *= 0.8;

	
	osgVegetation::BillboardLayerVector ug_layers;
	ug_layers.push_back(grass_l0);
	ug_layers.push_back(grass_l1);
	ug_layers.push_back(grass_l2);
	ug_layers.push_back(plant_l0);
	ug_layers.push_back(plant_l1);
	
	osgVegetation::BillboardData undergrowth_data(ug_layers,true,0.4,false);
	
	//Tree layers
	osgVegetation::BillboardLayer  spruce_l0("Images/spruce01.dds",2000);
	spruce_l0.Density = 0.02;
	spruce_l0.Height.set(5,5);
	spruce_l0.Width.set(2,2);
	spruce_l0.Scale.set(2,3);
	spruce_l0.ColorIntensity.set(0.5, 0.5);
	spruce_l0.MixInColorRatio = 2.0;
	spruce_l0.MixInIntensity = true;
	spruce_l0.Materials.push_back(material_map[WOODS]);
	
	osgVegetation::BillboardLayer pine_l0("Images/pine01.dds",2000); 
	pine_l0.Density = 0.02;
	pine_l0.Height.set(5,5);
	pine_l0.Width.set(2,2);
	pine_l0.Scale.set(2,3);
	pine_l0.ColorIntensity.set(0.5, 0.5);
	pine_l0.MixInColorRatio = 2.0;
	pine_l0.MixInIntensity = true;
	pine_l0.Materials.push_back(material_map[WOODS]);

	osgVegetation::BillboardLayer  birch_l0("Images/birch01.dds",2000);
	birch_l0.Density = 0.012;
	birch_l0.Height.set(4,4);
	birch_l0.Width.set(4,4);
	birch_l0.Scale.set(2,3);
	birch_l0.ColorIntensity.set(0.5, 0.5);
	birch_l0.MixInColorRatio = 2.0;
	birch_l0.MixInIntensity = true;
	birch_l0.Materials.push_back(material_map[WOODS]);

	osgVegetation::BillboardLayerVector og_layers;
	og_layers.push_back(spruce_l0);
	og_layers.push_back(pine_l0);
	og_layers.push_back(birch_l0);

	osgVegetation::BillboardData tree_data(og_layers,false,0.08,false);

	std::string save_path("c:/temp/paged/");
	//add path to enable viewer to find LODS
	osgDB::Registry::instance()->getDataFilePathList().push_back(save_path);  
	
	osg::ComputeBoundsVisitor  cbv;
	osg::BoundingBox &bb(cbv.getBoundingBox());
	terrain->accept(cbv);


	//test to use smaller bb
	const double bb_scale = 0.1;
	osg::Vec3 bb_size = (bb._max - bb._min)*bb_scale;
	osg::Vec3 bb_center = (bb._max + bb._min)*0.5;
	osg::BoundingBox new_bb;
	new_bb._min = bb_center - bb_size*0.5;
	new_bb._max = bb_center + bb_size*0.5;
	//bb._min.set(new_bb._min.x(),new_bb._min.y(),bb._min.z());
	//bb._max.set(new_bb._max.x(),new_bb._max.y(),bb._max.z());

	osgVegetation::TerrainQuery tq(terrain.get());

	tq.setMaterialTextureSuffix("_material.tga");
	osgVegetation::QuadTreeScattering scattering(&tq);
	osg::Node* ug_node = scattering.generate(bb,undergrowth_data,save_path, "ug_");
	group->addChild(ug_node);
	osgVegetation::QuadTreeScattering scattering2(&tq);
	osg::Node* tree_node = scattering2.generate(bb,tree_data,save_path,"og_");
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
