#include <osgDB/ReadFile>
#include <osgUtil/Optimizer>
#include <osg/CoordinateSystemNode>
#include <osg/Fog>
#include <osgText/Text>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>
#include <osgGA/KeySwitchMatrixManipulator>
#include <osgGA/StateSetManipulator>
#include <osgGA/AnimationPathManipulator>
#include <osgGA/TerrainManipulator>
#include <osgGA/SphericalManipulator>
#include <osgGA/Device>
#include <osgShadow/ShadowedScene>
#include <osgShadow/ShadowVolume>
#include <osgShadow/ShadowMap>
#include <osgShadow/LightSpacePerspectiveShadowMap>
#include <osgShadow/StandardShadowMap>
#include <osgShadow/ViewDependentShadowMap>
#include <osg/Version>

#ifndef OSG_VERSION_GREATER_OR_EQUAL
#define OSG_VERSION_GREATER_OR_EQUAL(MAJOR, MINOR, PATCH) ((OPENSCENEGRAPH_MAJOR_VERSION>MAJOR) || (OPENSCENEGRAPH_MAJOR_VERSION==MAJOR && (OPENSCENEGRAPH_MINOR_VERSION>MINOR || (OPENSCENEGRAPH_MINOR_VERSION==MINOR && OPENSCENEGRAPH_PATCH_VERSION>=PATCH))))
#endif


#if OSG_VERSION_GREATER_OR_EQUAL( 3, 5, 1 )
	#include <osg/Types>
#endif

int main(int argc, char **argv)
{
	//std::string opt_env= "OSG_OPTIMIZER=COMBINE_ADJACENT_LODS SHARE_DUPLICATE_STATE MERGE_GEOMETRY MAKE_FAST_GEOMETRY CHECK_GEOMETRY OPTIMIZE_TEXTURE_SETTINGS STATIC_OBJECT_DETECTION";
//#ifdef WIN32
	//_putenv(opt_env.c_str());
//#else
	

	//const std::string shadow_type = "VDSM";

	osg::DisplaySettings::instance()->setNumMultiSamples(4);
	// use an ArgumentParser object to manage the program arguments.
	osg::ArgumentParser arguments(&argc, argv);

	arguments.getApplicationUsage()->setApplicationName(arguments.getApplicationName());
	arguments.getApplicationUsage()->setDescription(arguments.getApplicationName() + " is the standard OpenSceneGraph example which loads and visualises 3d models.");
	arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName() + " [options] filename ...");
	arguments.getApplicationUsage()->addCommandLineOption("--image <filename>", "Load an image and render it on a quad");
	arguments.getApplicationUsage()->addCommandLineOption("--dem <filename>", "Load an image/DEM and render it on a HeightField");
	arguments.getApplicationUsage()->addCommandLineOption("--login <url> <username> <password>", "Provide authentication information for http file access.");
	arguments.getApplicationUsage()->addCommandLineOption("-p <filename>", "Play specified camera path animation file, previously saved with 'z' key.");
	arguments.getApplicationUsage()->addCommandLineOption("--speed <factor>", "Speed factor for animation playing (1 == normal speed).");
	arguments.getApplicationUsage()->addCommandLineOption("--device <device-name>", "add named device to the viewer");
	arguments.getApplicationUsage()->addCommandLineOption("--fov <value>", "Field of view");
	arguments.getApplicationUsage()->addCommandLineOption("--enable_fog", "Use Fog");
	arguments.getApplicationUsage()->addCommandLineOption("--shadow_type <value>", "Set Shadow type NONE,LISPSM or VDSM");

	osgViewer::Viewer viewer(arguments);

	unsigned int helpType = 0;
	if ((helpType = arguments.readHelpType()))
	{
		arguments.getApplicationUsage()->write(std::cout, helpType);
		return 1;
	}

	// report any errors if they have occurred when parsing the program arguments.
	if (arguments.errors())
	{
		arguments.writeErrorMessages(std::cout);
		return 1;
	}

	if (arguments.argc() <= 1)
	{
		arguments.getApplicationUsage()->write(std::cout, osg::ApplicationUsage::COMMAND_LINE_OPTION);
		return 1;
	}

	std::string url, username, password;
	while (arguments.read("--login", url, username, password))
	{
		if (!osgDB::Registry::instance()->getAuthenticationMap())
		{
			osgDB::Registry::instance()->setAuthenticationMap(new osgDB::AuthenticationMap);
			osgDB::Registry::instance()->getAuthenticationMap()->addAuthenticationDetails(
				url,
				new osgDB::AuthenticationDetails(username, password)
				);
		}
	}

	std::string device;
	while (arguments.read("--device", device))
	{
#if OSG_VERSION_GREATER_OR_EQUAL(3,5,1)
		osg::ref_ptr<osgGA::Device> dev = osgDB::readRefFile<osgGA::Device>(device);
#else
		osg::ref_ptr<osgGA::Device> dev = osgDB::readFile<osgGA::Device>(device);
#endif
		if (dev.valid())
		{
			viewer.addDevice(dev);
		}
	}

	double fov = 35;
	while (arguments.read("--fov", fov))
	{

	}

	bool use_fog = false;
	while (arguments.read("--enable_fog"))
	{
		use_fog =  true;
	}

	bool enableShadows = true;
	std::string shadow_type = "NONE";
	while (arguments.read("--shadow_type", shadow_type))
	{

	}

	if(shadow_type == "NONE")
		enableShadows = false;


	// set up the camera manipulators.
	{
		osg::ref_ptr<osgGA::KeySwitchMatrixManipulator> keyswitchManipulator = new osgGA::KeySwitchMatrixManipulator;

		keyswitchManipulator->addMatrixManipulator('1', "Trackball", new osgGA::TrackballManipulator());
		keyswitchManipulator->addMatrixManipulator('2', "Flight", new osgGA::FlightManipulator());
		keyswitchManipulator->addMatrixManipulator('3', "Drive", new osgGA::DriveManipulator());
		keyswitchManipulator->addMatrixManipulator('4', "Terrain", new osgGA::TerrainManipulator());
		keyswitchManipulator->addMatrixManipulator('5', "Orbit", new osgGA::OrbitManipulator());
		keyswitchManipulator->addMatrixManipulator('6', "FirstPerson", new osgGA::FirstPersonManipulator());
		keyswitchManipulator->addMatrixManipulator('7', "Spherical", new osgGA::SphericalManipulator());

		std::string pathfile;
		double animationSpeed = 1.0;
		while (arguments.read("--speed", animationSpeed)) {}
		char keyForAnimationPath = '8';
		while (arguments.read("-p", pathfile))
		{
			osgGA::AnimationPathManipulator* apm = new osgGA::AnimationPathManipulator(pathfile);
			if (apm || !apm->valid())
			{
				apm->setTimeScale(animationSpeed);

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

	// add the thread model handler
	viewer.addEventHandler(new osgViewer::ThreadingHandler);

	// add the window size toggle handler
	viewer.addEventHandler(new osgViewer::WindowSizeHandler);

	// add the stats handler
	viewer.addEventHandler(new osgViewer::StatsHandler);

	// add the help handler
	viewer.addEventHandler(new osgViewer::HelpHandler(arguments.getApplicationUsage()));

	// add the record camera path handler
	viewer.addEventHandler(new osgViewer::RecordCameraPathHandler);

	// add the LOD Scale handler
	viewer.addEventHandler(new osgViewer::LODScaleHandler);

	// add the screen capture handler
	viewer.addEventHandler(new osgViewer::ScreenCaptureHandler);

	// load the data
#if OSG_VERSION_GREATER_OR_EQUAL(3,5,1)
	osg::ref_ptr<osg::Node> loadedModel = osgDB::readRefNodeFiles(arguments);
#else
	osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFiles(arguments);
#endif
	if (!loadedModel)
	{
		std::cout << arguments.getApplicationName() << ": No data loaded" << std::endl;
		return 1;
	}

	// any option left unread are converted into errors to write out later.
	arguments.reportRemainingOptionsAsUnrecognized();

	// report any errors if they have occurred when parsing the program arguments.
	if (arguments.errors())
	{
		arguments.writeErrorMessages(std::cout);
		return 1;
	}


	// optimize the scene graph, remove redundant nodes and state etc.
	osgUtil::Optimizer optimizer;
	optimizer.optimize(loadedModel);

	//Create root node
	osg::Group* group = new osg::Group;

	if (use_fog)
	{
		//Add fog
		const osg::Fog::Mode fog_mode = osg::Fog::LINEAR;
		osg::StateSet* state = group->getOrCreateStateSet();
		osg::ref_ptr<osg::Fog> fog = new osg::Fog();
		state->setMode(GL_FOG, osg::StateAttribute::ON);
		state->setAttributeAndModes(fog.get());
		fog->setMode(fog_mode);
		fog->setDensity(0.000005);
		fog->setEnd(800);
		fog->setStart(30);
		fog->setColor(osg::Vec4(1.0, 1.0, 1.0, 1.0));
	}

	//Add light and shadows
	osg::Light* pLight = new osg::Light;
	pLight->setDiffuse(osg::Vec4(0.6f, 0.6f, 0.6f, 0.6f));
	osg::Vec4 lightPos(1, 1.0, 1, 0);
	pLight->setPosition(lightPos);		// last param	w = 0.0 directional light (direction)
	osg::Vec3f lightDir(-lightPos.x(), -lightPos.y(), -lightPos.z());
	lightDir.normalize();
	pLight->setDirection(lightDir);
	pLight->setAmbient(osg::Vec4(0.4f, 0.4f, 0.4f, 1.0f));
	//pLight->setAmbient(osg::Vec4(0.0f, 0.0f, 0.0f, 1.0f) );

	osg::LightSource* pLightSource = new osg::LightSource;
	pLightSource->setLight(pLight);
	group->addChild(pLightSource);
	group->addChild(loadedModel);

	
	double nearClip = 10;
	double farClip = 10000;
	viewer.getCamera()->setComputeNearFarMode(osgUtil::CullVisitor::DO_NOT_COMPUTE_NEAR_FAR);
	viewer.getCamera()->setProjectionResizePolicy(osg::Camera::HORIZONTAL);
	double aspectRatio = 4.0 / 3.0;

	if (viewer.getCamera()->getViewport())
		aspectRatio = viewer.getCamera()->getViewport()->width() / viewer.getCamera()->getViewport()->height();
	viewer.getCamera()->setProjectionMatrixAsPerspective(fov, aspectRatio, nearClip, farClip);

	static int ReceivesShadowTraversalMask = 0x1;
	static int CastsShadowTraversalMask = 0x2;

	group->setNodeMask(CastsShadowTraversalMask | ReceivesShadowTraversalMask);

	osg::ref_ptr<osgShadow::ShadowedScene> shadowedScene = new osgShadow::ShadowedScene;
	int mapres = 2048;
	if (shadow_type == "LISPSM")
	{
		osg::ref_ptr<osgShadow::MinimalShadowMap> sm = new osgShadow::LightSpacePerspectiveShadowMapCB;
		float minLightMargin = 20.f;
		float maxFarPlane = farClip;
		int baseTexUnit = 0;
		int shadowTexUnit = 6;
		sm->setMinLightMargin(minLightMargin);
		sm->setMaxFarPlane(maxFarPlane);
		sm->setTextureSize(osg::Vec2s(mapres, mapres));
		sm->setShadowTextureCoordIndex(shadowTexUnit);
		sm->setShadowTextureUnit(shadowTexUnit);
		sm->setBaseTextureCoordIndex(baseTexUnit);
		sm->setBaseTextureUnit(baseTexUnit);

		//sm->setMainVertexShader(NULL);
		//sm->setShadowVertexShader(NULL);

		shadowedScene->setReceivesShadowTraversalMask(ReceivesShadowTraversalMask);
		shadowedScene->setCastsShadowTraversalMask(CastsShadowTraversalMask);


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
	}
	else if (shadow_type == "VDSM")
	{

		osgShadow::ShadowSettings* settings = shadowedScene->getShadowSettings();
		settings->setReceivesShadowTraversalMask(ReceivesShadowTraversalMask);
		settings->setCastsShadowTraversalMask(CastsShadowTraversalMask);
		//settings->setShadowMapProjectionHint(osgShadow::ShadowSettings::PERSPECTIVE_SHADOW_MAP);
		unsigned int unit = 2;
		settings->setBaseShadowTextureUnit(unit);

		double n = 0.8;
		settings->setMinimumShadowMapNearFarRatio(n);

		unsigned int numShadowMaps = 2;
		settings->setNumShadowMapsPerLight(numShadowMaps);

		//settings->setMultipleShadowMapHint(osgShadow::ShadowSettings::PARALLEL_SPLIT);
		settings->setMultipleShadowMapHint(osgShadow::ShadowSettings::CASCADED);


		settings->setTextureSize(osg::Vec2s(mapres, mapres));
		//settings->setShaderHint(osgShadow::ShadowSettings::PROVIDE_VERTEX_AND_FRAGMENT_SHADER);
		osg::ref_ptr<osgShadow::ViewDependentShadowMap> vdsm = new osgShadow::ViewDependentShadowMap;
		shadowedScene->setShadowTechnique(vdsm.get());
	}


	if (enableShadows)
	{
		shadowedScene->addChild(group);
		viewer.setSceneData(shadowedScene);
	}
	else
	{
		viewer.setSceneData(group);
	}
	viewer.realize();

	while (!viewer.done())
	{
		//animate light if shadows enabled
	//	if (enableShadows)
		{
			float t = viewer.getFrameStamp()->getSimulationTime()*0.04;
			lightPos.set(sinf(t), cosf(t), 0.7f, 0.0f);
			//lightPos.set(0.2f,0,1.1 + cosf(t),0.0f);
			pLight->setPosition(lightPos);
			lightDir.set(-lightPos.x(), -lightPos.y(), -lightPos.z());
			lightDir.normalize();
			pLight->setDirection(lightDir);
		}
		viewer.frame();
	}
	return 1;
}
