#include <osg/Geometry>
#include <osg/Math>
#include <osg/MatrixTransform>
#include <osg/StateSet>
#include <osg/ComputeBoundsVisitor>
#include <osgDB/WriteFile>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
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
#include "BillboardQuadTreeScattering.h"
#include "TerrainQuery.h"
#include "CoverageData.h"

//Define some coverage material names
#define WOODS "WOODS"
#define GRASS "GRASS"
#define ROAD "ROAD"
#define DIRT "DIRT"

int main( int argc, char **argv )
{
	//Global settings
	const bool enableShadows = false;
	const bool use_fog = true;
	const osg::Fog::Mode fog_mode = osg::Fog::LINEAR;

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

	//Add sample data path
	osgDB::Registry::instance()->getDataFilePathList().push_back("../data");  

	//load terrain
	osg::ref_ptr<osg::Node> terrain = osgDB::readNodeFile("lz.osg");

	//Create root node
	osg::Group* group = new osg::Group;
	group->addChild(terrain);

	//Create billboard layers

	//First LOD start at 400m
	osgVegetation::BillboardLayer  tree_l0("billboards/fir01_bb.dds", 400);

	tree_l0.Density = 0.01;
	tree_l0.Height.set(5,5);
	tree_l0.Width.set(2,2);
	tree_l0.Scale.set(0.8,0.9);
	tree_l0.ColorIntensity.set(3.0,3.0);
	tree_l0.TerrainColorRatio = 0.7;
	tree_l0.UseTerrainIntensity = false;
	tree_l0.CoverageMaterials.push_back(WOODS);

	//second LOD start at 200m, also increase density and decrease scale
	osgVegetation::BillboardLayer  tree_l1 = tree_l0;
	tree_l1.Density *= 4; //scale density 4 times to match quad tree structure
	tree_l1.Scale *= 0.8;
	tree_l1.ViewDistance *= 0.5;

	//third LOD start at 100m
	osgVegetation::BillboardLayer  tree_l2 = tree_l1;
	tree_l2.Density *= 4;
	tree_l2.Scale *= 0.8;
	tree_l2.ViewDistance *= 0.5;

	//add all layers, the order is not important, 
	//the layers will be sorted by distance before generation
	osgVegetation::BillboardLayerVector layers;
	layers.push_back(tree_l2);
	layers.push_back(tree_l0);
	layers.push_back(tree_l1);

	//create billboard data by supplying layers and rendering settings.
	osgVegetation::BillboardData tree_data(layers, false,0.3,false);
	tree_data.ReceiveShadows = enableShadows;
	tree_data.CastShadows = enableShadows;
	tree_data.UseFog = use_fog;
	tree_data.FogMode = fog_mode;
	tree_data.TerrainNormal = false;

	//if(enableShadows) // For correct shadow casting we need to use cross quads...no solution for this right now
	//	tree_data.Type = osgVegetation::BT_CROSS_QUADS;
	//else
	tree_data.Type = osgVegetation::BT_CROSS_QUADS;

	osg::ComputeBoundsVisitor  cbv;
	osg::BoundingBox &bb(cbv.getBoundingBox());
	terrain->accept(cbv);

	//down size bb for faster generation...useful for testing purpose
	osg::Vec3 bb_size = bb._max - bb._min;
	bb._min = bb._min + bb_size*0.2;
	bb._max = bb._max - bb_size*0.2;

	//coverage data used by the terrain query class
	osgVegetation::CoverageData cd;
	//...add the materials, here we match material name with colors
	cd.CoverageMaterials.push_back(osgVegetation::CoverageData::CoverageMaterial(GRASS,osgVegetation::CoverageColor(0,0,1,1)));
	cd.CoverageMaterials.push_back(osgVegetation::CoverageData::CoverageMaterial(WOODS,osgVegetation::CoverageColor(1,1,1,1)));
	cd.CoverageMaterials.push_back(osgVegetation::CoverageData::CoverageMaterial(ROAD,osgVegetation::CoverageColor(0,0,1,1)));
	cd.CoverageMaterials.push_back(osgVegetation::CoverageData::CoverageMaterial(DIRT,osgVegetation::CoverageColor(1,0,0,1)));

	//Create terrain query class and by feeding terrain and coverage data
	osgVegetation::TerrainQuery tq(terrain.get(),cd);

	//create scattering class
	osgVegetation::BillboardQuadTreeScattering scattering(&tq);

	osg::Node* tree_node = NULL;

	try{
		//Start generation
		tree_node = scattering.generate(bb,tree_data);
	}
	catch(std::exception& e)
	{
		std::cerr << e.what();
		return 0;
	}

	//We are done, add vegetation to root node
	group->addChild(tree_node);

	if(use_fog)
	{
		//Add fog
		osg::StateSet* state = group->getOrCreateStateSet();
		osg::ref_ptr<osg::Fog> fog = new osg::Fog();
		state->setMode(GL_FOG, osg::StateAttribute::ON);
		state->setAttributeAndModes(fog.get());
		fog->setMode(fog_mode);
		fog->setDensity(0.000005);
		fog->setEnd(800);
		fog->setStart(30);
		fog->setColor(osg::Vec4(1.0, 1.0, 1.0,1.0));
	}


	//Add light and shadows
	osg::Light* pLight = new osg::Light;
	pLight->setDiffuse( osg::Vec4(0.6f, 0.6f, 0.6f, 0.6f) );
	osg::Vec4 lightPos(1,1.0,1,0); 
	pLight->setPosition(lightPos);		// last param	w = 0.0 directional light (direction)
	osg::Vec3f lightDir(-lightPos.x(),-lightPos.y(),-lightPos.z());
	lightDir.normalize();
	pLight->setDirection(lightDir);
	pLight->setAmbient(osg::Vec4(0.4f, 0.4f, 0.4f, 1.0f) );


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

	while (!viewer.done())
	{
		//animate light if shadows enabled
		if(enableShadows)
		{
			float t = viewer.getFrameStamp()->getSimulationTime()*0.4;
			lightPos.set(sinf(t),cosf(t),0.7f,0.0f);
			pLight->setPosition(lightPos);
			osg::Vec3f lightDir(-lightPos.x(),-lightPos.y(),-lightPos.z());
			lightDir.normalize();
			pLight->setDirection(lightDir);
		}	
		viewer.frame();
	}
	return 1;
}
