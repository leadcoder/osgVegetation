#include "ov_MeshLayerGenerator.h"
#include "ov_VPBVegetationInjection.h"
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
#include <osgShadow/ShadowedScene>
#include <osgShadow/ShadowMap>
#include <osgShadow/ParallelSplitShadowMap>
#include <osgShadow/LightSpacePerspectiveShadowMap>
#include <osgShadow/StandardShadowMap>
#include <osgShadow/ViewDependentShadowMap>
#include "ov_Utils.h"


namespace osgVegetation
{
	GlobalRegister Register;
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
	osgDB::Registry::instance()->getDataFilePathList().push_back("../shaders");
	osgDB::Registry::instance()->getDataFilePathList().push_back("../sample-data");

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



	//Control texture slots
	osgVegetation::Register.TexUnits.AddUnit(0, OV_TERRAIN_COLOR_TEXTURE_ID);
	//osgVegetation::Register.TexUnits.AddUnit(2, OV_TERRAIN_NORMAL_TEXTURE_ID);
	osgVegetation::Register.TexUnits.AddUnit(1, OV_TERRAIN_SPLAT_TEXTURE_ID);
	osgVegetation::Register.TexUnits.AddUnit(2, OV_TERRAIN_DETAIL_TEXTURE_ID);
	osgVegetation::Register.TexUnits.AddUnit(3, OV_BILLBOARD_TEXTURE_ID);
	osgVegetation::Register.TexUnits.AddUnit(6, OV_SHADOW_TEXTURE0_ID);
	osgVegetation::Register.TexUnits.AddUnit(7, OV_SHADOW_TEXTURE1_ID);

	//Add light and shadows
	osg::Light* pLight = new osg::Light;
	pLight->setDiffuse(osg::Vec4(0.7f, 0.7f, 0.7f, 1.0f));
	osg::Vec4 lightPos(1.0, 1.0, 1.0, 0);
	pLight->setPosition(lightPos);		// last param	w = 0.0 directional light (direction)
	osg::Vec3f lightDir(-lightPos.x(), -lightPos.y(), -lightPos.z());
	lightDir.normalize();
	pLight->setDirection(lightDir);
	pLight->setAmbient(osg::Vec4(0.6f, 0.6f, 0.6f, 1.0f));

	osg::LightSource* pLightSource = new osg::LightSource;
	pLightSource->setLight(pLight);
	root->addChild(pLightSource);

#if 0
	osgVegetation::MeshLayerConfig layer(1000);
	osgVegetation::MeshTypeConfig mesh_data1;
	float end_dist = 200.0f;
	mesh_data1.MeshLODs.push_back(osgVegetation::MeshTypeConfig::MeshLODConfig("trees/fir01_l0.osg", osg::Vec4(0.0f, 0.0f, 100.0f, 110.0f)));
	mesh_data1.MeshLODs.push_back(osgVegetation::MeshTypeConfig::MeshLODConfig("trees/fir01_l1.osg", osg::Vec4(100.0f, 110.0f, end_dist, end_dist + 10)));
	//mesh_data.MeshLODs.push_back(MeshTypeConfig::MeshLODConfig("LOD2", osg::Vec4(500.0f, 510.0f, 1200.0f, 1210.0f)));
	layer.MeshTypes.push_back(mesh_data1);
	//osgVegetation::MeshTypeConfig  mesh_data2;
	//mesh_data2.MeshLODs.push_back(osgVegetation::MeshTypeConfig::MeshLODConfig("trees/test.osg", osg::Vec4(0.0f, 0.0f, 2000.0f, 2500.0f)));
	//layer.MeshTypes.push_back(mesh_data2);

	
	//osg::ref_ptr<osgVegetation::MeshLayerConfig> mesh_layer = new osgVegetation::MeshLayerConfig(layer);

	//Setup layers in terrain injection lods
	std::vector<osgVegetation::VPBInjectionLODConfig> terrain_lods;
	osgVegetation::VPBInjectionLODConfig tree_terrain_lod(3);
	
	tree_terrain_lod.Layers.push_back(&layer);
	terrain_lods.push_back(tree_terrain_lod);
	osgVegetation::VPBVegetationInjectionConfig vpbconfig(terrain_lods);

	osgDB::Registry::instance()->setReadFileCallback(new osgVegetation::VPBVegetationInjection(vpbconfig));
#else
	//setup vegetation layers
	std::vector<osgVegetation::BillboardLayerConfig> layers;
	osgVegetation::BillboardLayerConfig grass_layer;
	grass_layer.MaxDistance = 140;
	grass_layer.Density = 0.2;
	grass_layer.ColorImpact = 1.0;
	grass_layer.CastShadow = false;
	grass_layer.Type = osgVegetation::BillboardLayerConfig::BLT_GRASS;
	grass_layer.Billboards.push_back(osgVegetation::BillboardLayerConfig::Billboard("billboards/veg_plant03.png", osg::Vec2f(4, 2), 0.9, 0.008));
	grass_layer.Billboards.push_back(osgVegetation::BillboardLayerConfig::Billboard("billboards/veg_plant01.png", osg::Vec2f(2, 2), 0.9, 0.002));
	grass_layer.Billboards.push_back(osgVegetation::BillboardLayerConfig::Billboard("billboards/grass2.png", osg::Vec2f(2, 1), 1.0, 1.0));
	//osg::Vec4 grass_splat_color_filter(0.1, -1, -1, -1);
	grass_layer.Filter.SplatFilter = "if(splat_color.x > 0.5) return false;"; //osgVegetation::PassFilter::GenerateSplatFilter(grass_splat_color_filter, "<");

	osgVegetation::BillboardLayerConfig tree_layer;
	tree_layer.MaxDistance = 740;
	tree_layer.Density = 0.001;
	tree_layer.ColorImpact = 0.0;
	tree_layer.Type = osgVegetation::BillboardLayerConfig::BLT_ROTATED_QUAD;
	tree_layer.Filter.ColorFilter = "if(length(base_color.xyz) > 0.5) return false;";
	tree_layer.Filter.SplatFilter = "if(splat_color.x > 0.5) return false;";
	tree_layer.Billboards.push_back(osgVegetation::BillboardLayerConfig::Billboard("billboards/fir01_bb.png", osg::Vec2f(10, 16), 1.0, 1.0));
	//tree_data.Billboards.push_back(osgVegetation::BillboardLayerConfig::Billboard("billboards/tree0.rgba", osg::Vec2f(8, 16), 1.2));

	//Setup layers in terrain injection lods
	std::vector<osgVegetation::VPBInjectionLODConfig> terrain_lods;
	osgVegetation::VPBInjectionLODConfig grass_terrain_lod(5);
	grass_terrain_lod.Layers.push_back(&grass_layer);
	terrain_lods.push_back(grass_terrain_lod);

	osgVegetation::VPBInjectionLODConfig tree_terrain_lod(2);
	//tree_terrain_lod.Layers.push_back(&tree_layer);
	

	osg::ref_ptr <osgVegetation::MeshLayerConfig> tree_layer2 = new osgVegetation::MeshLayerConfig(1000);
	osgVegetation::MeshTypeConfig mesh;
	const float end_dist = 200.0f;
	mesh.MeshLODs.push_back(osgVegetation::MeshTypeConfig::MeshLODConfig("trees/fir01_l0.osg", osg::Vec4(0.0f, 0.0f, 100.0f, 110.0f)));
	mesh.MeshLODs.push_back(osgVegetation::MeshTypeConfig::MeshLODConfig("trees/fir01_l1.osg", osg::Vec4(100.0f, 110.0f, end_dist, end_dist + 10)));
	tree_layer2->MeshTypes.push_back(mesh);

	tree_terrain_lod.Layers.push_back(tree_layer2);
	terrain_lods.push_back(tree_terrain_lod);

	osgDB::Registry::instance()->setReadFileCallback(new osgVegetation::VPBVegetationInjection(terrain_lods));
#endif
	osg::ref_ptr<osg::Node> terrain_node = osgDB::readNodeFile("terrain/us-terrain.zip/us-terrain.osg");
	root->addChild(terrain_node);

	if (false)
	{
		osg::ref_ptr<osgShadow::ShadowedScene> shadowedScene = new osgShadow::ShadowedScene;
		int mapres = 2048;
		osg::ref_ptr<osgShadow::LightSpacePerspectiveShadowMapVB> lispsm = new osgShadow::LightSpacePerspectiveShadowMapVB;

		osg::ref_ptr<osgShadow::MinimalShadowMap> sm = lispsm;
		float minLightMargin = 20.f;
		float maxFarPlane = 1400;
		int baseTexUnit = 0;
		int shadowTexUnit = 6;
		sm->setMinLightMargin(minLightMargin);
		sm->setMaxFarPlane(maxFarPlane);
		sm->setTextureSize(osg::Vec2s(mapres, mapres));

		sm->setBaseTextureCoordIndex(baseTexUnit);
		sm->setBaseTextureUnit(baseTexUnit);

		sm->setShadowTextureCoordIndex(shadowTexUnit);
		sm->setShadowTextureUnit(shadowTexUnit);

		//sm->setMainVertexShader( NULL );
		//sm->setShadowVertexShader(NULL);

		
		shadowedScene->setReceivesShadowTraversalMask(osgVegetation::Register.ReceivesShadowTraversalMask);
		shadowedScene->setCastsShadowTraversalMask(osgVegetation::Register.CastsShadowTraversalMask);

		//sm->setMainFragmentShader(NULL);
		osg::Shader* mainFragmentShader = new osg::Shader(osg::Shader::FRAGMENT,
			" // following expressions are auto modified - do not change them:       \n"
			" // gl_TexCoord[0]  0 - can be subsituted with other index              \n"
			"                                                                        \n"
			"float DynamicShadow( );                                                 \n"
			"                                                                        \n"
			"uniform sampler2D baseTexture;                                          \n"
			"                                                                        \n"
			"void main(void)                                                         \n"
			"{                                                                       \n"
			"  vec4 colorAmbientEmissive = gl_FrontLightModelProduct.sceneColor;     \n"
			"  // Add ambient from Light of index = 0                                \n"
			"  colorAmbientEmissive += gl_FrontLightProduct[0].ambient;              \n"
			"  vec4 color = texture2D( baseTexture, gl_TexCoord[0].xy );             \n"
			"  color *= mix( colorAmbientEmissive, gl_Color, DynamicShadow() );      \n"
			"    float depth = gl_FragCoord.z / gl_FragCoord.w;\n"
			"    float fogFactor = exp(-pow((gl_Fog.density * depth), 2.0));\n"
			"    fogFactor = clamp(fogFactor, 0.0, 1.0);\n"
			"    //color.rgb = mix( gl_Fog.color.rgb, color.rgb, fogFactor );            \n"
			"    gl_FragColor = color;                                                 \n"
			"} \n");

		sm->setMainFragmentShader(mainFragmentShader);
		shadowedScene->setShadowTechnique(sm);
		shadowedScene->addChild(root);

		osg::StateSet::DefineList& defineList = root->getOrCreateStateSet()->getDefineList();
		defineList["SM_LISPSM"].second = (osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

		osg::Uniform* shadowTextureUnit = new osg::Uniform(osg::Uniform::INT, "shadowTextureUnit0");
		shadowTextureUnit->set(shadowTexUnit);
		shadowedScene->getOrCreateStateSet()->addUniform(shadowTextureUnit);

		viewer.setSceneData(shadowedScene.get());
	}
	else if (false)
	{
		osg::ref_ptr<osgShadow::ShadowedScene> shadowedScene = new osgShadow::ShadowedScene;
		int mapres = 2048;

		static int ReceivesShadowTraversalMask = 0x1;
		static int CastsShadowTraversalMask = 0x2;


		osgShadow::ShadowSettings* settings = shadowedScene->getShadowSettings();
		settings->setReceivesShadowTraversalMask(ReceivesShadowTraversalMask);
		settings->setCastsShadowTraversalMask(CastsShadowTraversalMask);
		//settings->setShadowMapProjectionHint(osgShadow::ShadowSettings::PERSPECTIVE_SHADOW_MAP);
		int shadowTexUnit = 6;
		settings->setBaseShadowTextureUnit(shadowTexUnit);

		double n = 0.8;
		settings->setMinimumShadowMapNearFarRatio(n);

		unsigned int numShadowMaps = 2;
		settings->setNumShadowMapsPerLight(numShadowMaps);
		//settings->setMultipleShadowMapHint(osgShadow::ShadowSettings::PARALLEL_SPLIT);
		settings->setMultipleShadowMapHint(osgShadow::ShadowSettings::CASCADED);
		settings->setMaximumShadowMapDistance(400);
		settings->setTextureSize(osg::Vec2s(mapres, mapres));
		//settings->setShaderHint(osgShadow::ShadowSettings::PROVIDE_VERTEX_AND_FRAGMENT_SHADER);
		osg::ref_ptr<osgShadow::ViewDependentShadowMap> vdsm = new osgShadow::ViewDependentShadowMap;
		shadowedScene->setShadowTechnique(vdsm.get());

		shadowedScene->addChild(root);

		osg::StateSet::DefineList& defineList = root->getOrCreateStateSet()->getDefineList();
		if (numShadowMaps == 1)
			defineList["SM_VDSM1"].second = (osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
		else
			defineList["SM_VDSM2"].second = (osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

		osg::Uniform* shadowTextureUnit0 = new osg::Uniform(osg::Uniform::INT, "shadowTextureUnit0");
		shadowTextureUnit0->set(shadowTexUnit);
		root->getOrCreateStateSet()->addUniform(shadowTextureUnit0);
		osg::Uniform* shadowTextureUnit1 = new osg::Uniform(osg::Uniform::INT, "shadowTextureUnit1");
		shadowTextureUnit1->set(shadowTexUnit + 1);
		root->getOrCreateStateSet()->addUniform(shadowTextureUnit1);

		viewer.setSceneData(shadowedScene.get());
	}
	else
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
	while (!viewer.done())
	{
		//animate light if shadows enabled
		//if(shadow_type != osgVegetation::SM_DISABLED)
		{
			float t = viewer.getFrameStamp()->getSimulationTime() * 0.5;
			lightPos.set(sinf(t), cosf(t), 1.0f/*0.5 + 0.45*cosf(t)*/, 0.0f);
			//lightPos.set(1.0, 0, 0.5 + 0.45*cosf(t), 0.0f);
			 //lightPos.set(0.2f,0,1.1 + cosf(t),0.0f);
			pLight->setPosition(lightPos);
			lightDir.set(-lightPos.x(), -lightPos.y(), -lightPos.z());
			lightDir.normalize();
			pLight->setDirection(lightDir);
		}
		viewer.frame();
	}
	//return viewer.run();
}
