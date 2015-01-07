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

#include <osgShadow/ShadowedScene>
#include <osgShadow/ShadowVolume>
#include <osgShadow/ShadowTexture>
#include <osgShadow/ShadowMap>
#include <osgShadow/SoftShadowMap>
#include <osgShadow/ParallelSplitShadowMap>
#include <osgShadow/LightSpacePerspectiveShadowMap>
#include <osgShadow/StandardShadowMap>
#include <osgShadow/ViewDependentShadowMap>


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
	osgDB::Registry::instance()->getDataFilePathList().push_back("../data");  
	osgDB::Registry::instance()->getDataFilePathList().push_back("c:/temp/paged");  
	osg::ref_ptr<osg::Node> terrain = osgDB::readNodeFile("lz.osg");
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

	//char* opt_var = getenv( "OSG_OPTIMIZER" ); // C4996
	const bool enableShadows = false;
	const bool use_paged_LOD = false;
	const bool use_fog = true;
	const osg::Fog::Mode fog_mode = osg::Fog::LINEAR;

	if(use_fog)
	{
		osg::StateSet* state = group->getOrCreateStateSet();
		osg::ref_ptr<osg::Fog> fog = new osg::Fog();
		state->setMode(GL_FOG, osg::StateAttribute::ON);
		state->setAttributeAndModes(fog.get());
		fog->setMode(fog_mode);
		fog->setDensity(0.00001);
		fog->setEnd(800);
		fog->setStart(30);
		fog->setColor(osg::Vec4(1.0, 1.0, 1.0,1.0));
	}


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

	osgVegetation::BillboardLayer  tree_l0("billboards/tree0.rgba",400);
	tree_l0.Density = 0.005;
	tree_l0.Height.set(5,5);
	tree_l0.Width.set(2,2);
	tree_l0.Scale.set(0.8,0.9);
	tree_l0.ColorIntensity.set(2,2);
	tree_l0.MixInColorRatio = 1.0;
	tree_l0.Materials.push_back(material_map[WOODS]);

	//second LOD
	osgVegetation::BillboardLayer  tree_l1 = tree_l0;
	tree_l1.Density *= 4;
	tree_l1.Scale *= 0.8;
	tree_l1.ViewDistance *= 0.5;

	//third LOD
	osgVegetation::BillboardLayer  tree_l2 = tree_l1;
	tree_l2.Density *= 4;
	tree_l2.Scale *= 0.8;
	tree_l2.ViewDistance *= 0.5;

	//add all layers, the order is not important
	osgVegetation::BillboardLayerVector layers;
	layers.push_back(tree_l2);
	layers.push_back(tree_l0);
	layers.push_back(tree_l1);

	//create billboard data by supplying layers and rendering settings.
	osgVegetation::BillboardData tree_data(layers, false,0.08,false);
	tree_data.ReceiveShadows = enableShadows;
	
	std::string save_path;
	if(use_paged_LOD)
	{
		save_path = "c:/temp/paged/";
		osgDB::Registry::instance()->getDataFilePathList().push_back(save_path); 
	}

	osg::ComputeBoundsVisitor  cbv;
	osg::BoundingBox &bb(cbv.getBoundingBox());
	terrain->accept(cbv);

	//down size bb for faster generation
	osg::Vec3 bb_size = bb._max - bb._min;
	bb._min = bb._min + bb_size*0.1;
	bb._max = bb._max - bb_size*0.1;

	osgVegetation::TerrainQuery tq(terrain.get());
	osgVegetation::QuadTreeScattering scattering(&tq,use_fog,fog_mode);
	osg::Node* tree_node = scattering.generate(bb,tree_data,save_path);
	group->addChild(tree_node);
	
	//osgDB::writeNodeFile(*group, save_path + "terrain_and_veg.ive");
	osgDB::writeNodeFile(*group, "c:/temp/terrain_and_veg.osgt");
	
	osg::Light* pLight = new osg::Light;
	//pLight->setLightNum( 4 );						
	pLight->setDiffuse( osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f) );
	osg::Vec4 lightPos(1,0.5,1,0); 
	pLight->setPosition(lightPos);		// last param	w = 0.0 directional light (direction)
	osg::Vec3f lightDir(-lightPos.x(),-lightPos.y(),-lightPos.z());
	lightDir.normalize();
	pLight->setDirection(lightDir);
	pLight->setAmbient(osg::Vec4(0.4f, 0.4f, 0.4f, 1.0f) );
	// light source
	osg::LightSource* pLightSource = new osg::LightSource;    
	pLightSource->setLight( pLight );
	group->addChild( pLightSource );


	static int ReceivesShadowTraversalMask = 0x1;
	static int CastsShadowTraversalMask = 0x2;

	osg::ref_ptr<osgShadow::ShadowedScene> shadowedScene = new osgShadow::ShadowedScene;
	osgShadow::ShadowSettings* settings = shadowedScene->getShadowSettings();
	settings->setReceivesShadowTraversalMask(ReceivesShadowTraversalMask);
	settings->setCastsShadowTraversalMask(CastsShadowTraversalMask);
	//settings->setShadowMapProjectionHint(osgShadow::ShadowSettings::PERSPECTIVE_SHADOW_MAP);

	
	unsigned int unit=2;
	settings->setBaseShadowTextureUnit(unit);

	double n=0.8;
	settings->setMinimumShadowMapNearFarRatio(n);

	unsigned int numShadowMaps = 2;
	settings->setNumShadowMapsPerLight(numShadowMaps);

	//if (arguments.read("--parallel-split") || arguments.read("--ps") ) settings->setMultipleShadowMapHint(osgShadow::ShadowSettings::PARALLEL_SPLIT);
	//if (arguments.read("--cascaded")) settings->setMultipleShadowMapHint(osgShadow::ShadowSettings::CASCADED);
	//settings->setMultipleShadowMapHint(osgShadow::ShadowSettings::CASCADED);

	int mapres = 1024;
	settings->setTextureSize(osg::Vec2s(mapres,mapres));
	//settings->setShaderHint(osgShadow::ShadowSettings::PROVIDE_VERTEX_AND_FRAGMENT_SHADER);

	osg::ref_ptr<osgShadow::ViewDependentShadowMap> vdsm = new osgShadow::ViewDependentShadowMap;
	shadowedScene->setShadowTechnique(vdsm.get());
	terrain->setNodeMask(ReceivesShadowTraversalMask);
	tree_node->setNodeMask(CastsShadowTraversalMask | ReceivesShadowTraversalMask);

	if(enableShadows)
	{
		shadowedScene->addChild(group);
		viewer.setSceneData(shadowedScene);
	}
	else
	{
		viewer.setSceneData(group);
	}
	

	return viewer.run();
}
