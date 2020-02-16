#pragma once
#include "ov_Register.h"
#include "ov_Scene.h"

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
#include <osg/Version>
#include <osg/PositionAttitudeTransform>
#include <osg/Fog>

#include <osgShadow/ShadowedScene>
#include <osgShadow/ShadowMap>
#include <osgShadow/ParallelSplitShadowMap>
#include <osgShadow/LightSpacePerspectiveShadowMap>
#include <osgShadow/StandardShadowMap>
#include <osgShadow/ViewDependentShadowMap>

class Demo
{
public:

	enum ShadowModeEnum
	{
		SM_DISABLED,
		SM_UNDEFINED,
		SM_LISPSM,
		SM_VDSM1, //one texture
		SM_VDSM2, //two textures
	};

	Demo(int argc, char** argv) : m_Viewer(osg::ArgumentParser(&argc, argv)),
		m_ReceivesShadowTraversalMask(0x2),
		m_CastsShadowTraversalMask(0x4)
	{
		osg::ref_ptr<osgGA::KeySwitchMatrixManipulator> keyswitchManipulator = new osgGA::KeySwitchMatrixManipulator;
		keyswitchManipulator->addMatrixManipulator('1', "Trackball", new osgGA::TrackballManipulator());
		keyswitchManipulator->addMatrixManipulator('2', "Flight", new osgGA::FlightManipulator());
		keyswitchManipulator->addMatrixManipulator('3', "Drive", new osgGA::DriveManipulator());
		keyswitchManipulator->addMatrixManipulator('4', "Terrain", new osgGA::TerrainManipulator());
		m_Viewer.setCameraManipulator(keyswitchManipulator.get());

		// add the state manipulator
		m_Viewer.addEventHandler(new osgGA::StateSetManipulator(m_Viewer.getCamera()->getOrCreateStateSet()));

		// add the stats handler
		m_Viewer.addEventHandler(new osgViewer::StatsHandler);

		// add the record camera path handler
		m_Viewer.addEventHandler(new osgViewer::RecordCameraPathHandler);

		// add the window size toggle handler
		m_Viewer.addEventHandler(new osgViewer::WindowSizeHandler);

		m_Viewer.setThreadingModel(osgViewer::ViewerBase::SingleThreaded);

		osg::DisplaySettings::instance()->setNumMultiSamples(4);

		//Add sample data path
		osgDB::Registry::instance()->getDataFilePathList().push_back("../shaders");
		osgDB::Registry::instance()->getDataFilePathList().push_back("../sample-data");

		m_SceneData = new osg::Group();

		//Add light
		m_Light = new osg::Light;
		m_Light->setDiffuse(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
		osg::Vec4 light_pos(1, 1.0, 1, 0);
		m_Light->setPosition(light_pos);		// last param	w = 0.0 directional light (direction)
		osg::Vec3f light_dir(-light_pos.x(), -light_pos.y(), -light_pos.z());
		light_dir.normalize();
		m_Light->setDirection(light_dir);
		m_Light->setAmbient(osg::Vec4(0.4f, 0.4f, 0.4f, 1.0f));

		osg::LightSource* light_source = new osg::LightSource;
		light_source->setLight(m_Light);
		m_SceneData->addChild(light_source);
		//apply scene settings
		//config.Apply(m_SceneData->getOrCreateStateSet());
	}

	void EnableFog(osg::Fog::Mode fog_mode)
	{
		const osg::Vec4 fog_color(0.5, 0.6, 0.7, 1.0);
		osg::StateSet* state = m_SceneData->getOrCreateStateSet();
		osg::ref_ptr<osg::Fog> fog = new osg::Fog();
		state->setMode(GL_FOG, osg::StateAttribute::ON);
		state->setAttributeAndModes(fog.get());
		fog->setMode(osg::Fog::Mode(fog_mode));
		fog->setDensity(0.0005);
		fog->setColor(fog_color);
		if (fog_mode == osg::Fog::Mode::LINEAR)
		{
			fog->setStart(0);
			fog->setEnd(500);
		}
		m_Viewer.getCamera()->setClearColor(fog_color);

		osgVegetation::Scene::EnableFog(m_SceneData->getOrCreateStateSet(), fog_mode);
	}

	void EnableShadow(ShadowModeEnum shadow_mode)
	{
		m_ShadowScene = _CreateShadowedScene(shadow_mode);
		m_SceneData->addChild(m_ShadowScene);
		osgVegetation::Scene::EnableShadowMapping(m_ShadowScene->getOrCreateStateSet(), m_ShadowScene->getShadowSettings()->getNumShadowMapsPerLight());
	}

	void Run()
	{
		m_Viewer.setSceneData(m_SceneData);
		m_Viewer.setUpViewInWindow(100, 100, 800, 600);

		m_Viewer.realize();
		m_Viewer.getCamera()->getGraphicsContext()->getState()->setUseModelViewAndProjectionUniforms(true);
		// run the viewers main loop
		while (!m_Viewer.done())
		{
			const float t = m_Viewer.getFrameStamp()->getSimulationTime() * 0.5;
			const osg::Vec4 light_pos(sinf(t), cosf(t), 1, 0.0f);
			m_Light->setPosition(light_pos);
			osg::Vec3 light_dir(-light_pos.x(), -light_pos.y(), -light_pos.z());
			light_dir.normalize();
			m_Light->setDirection(light_dir);
			m_Viewer.frame();
		}
	}
	osg::ref_ptr<osg::Group> GetSceneRoot() const { return m_ShadowScene.valid() ? m_ShadowScene->asGroup() : m_SceneData; }
	osgViewer::Viewer& GetViewer() { return m_Viewer; }
private:
	osg::ref_ptr<osgShadow::ShadowedScene> _CreateShadowedSceneLiSPSM()
	{
		osgVegetation::Register.TexUnits.AddUnitIfNotPresent(6, OV_SHADOW_TEXTURE0_ID);
		int shadowTexUnit = osgVegetation::Register.TexUnits.GetUnit(OV_SHADOW_TEXTURE0_ID);
		osg::ref_ptr<osgShadow::ShadowedScene> shadowedScene = new osgShadow::ShadowedScene;
		osgShadow::ShadowSettings* settings = shadowedScene->getShadowSettings();
		//settings->setShadowMapProjectionHint(osgShadow::ShadowSettings::ORTHOGRAPHIC_SHADOW_MAP);
		int mapres = 2048;
		osg::ref_ptr<osgShadow::LightSpacePerspectiveShadowMapVB> lispsm = new osgShadow::LightSpacePerspectiveShadowMapVB;
		osg::ref_ptr<osgShadow::MinimalShadowMap> sm = lispsm;
		float minLightMargin = 20.f;
		float maxFarPlane = 400;
		int baseTexUnit = 0;
		sm->setMinLightMargin(minLightMargin);
		sm->setMaxFarPlane(maxFarPlane);
		sm->setTextureSize(osg::Vec2s(mapres, mapres));
		sm->setBaseTextureCoordIndex(baseTexUnit);
		sm->setBaseTextureUnit(baseTexUnit);
		sm->setShadowTextureCoordIndex(shadowTexUnit);
		sm->setShadowTextureUnit(shadowTexUnit);
		//sm->setMainVertexShader( NULL );
		//sm->setShadowVertexShader(NULL);

		shadowedScene->setReceivesShadowTraversalMask(m_ReceivesShadowTraversalMask);
		shadowedScene->setCastsShadowTraversalMask(m_CastsShadowTraversalMask);

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

		//Add texture sampler and unit uniforms,to match vdms and our shadow shaders
		osg::Uniform* shadowTextureUnit = new osg::Uniform(osg::Uniform::INT, "shadowTextureUnit0");
		shadowTextureUnit->set(shadowTexUnit);
		shadowedScene->getOrCreateStateSet()->addUniform(shadowTextureUnit);

		osg::Uniform* shadowTextureSampler = new osg::Uniform(osg::Uniform::INT, "shadowTexture0");
		shadowTextureSampler->set(shadowTexUnit);
		shadowedScene->getOrCreateStateSet()->addUniform(shadowTextureSampler);

		return shadowedScene;
	}

	osg::ref_ptr<osgShadow::ShadowedScene> _CreateShadowedSceneVDSM(int numShadowMaps)
	{
		osgVegetation::Register.TexUnits.AddUnitIfNotPresent(6, OV_SHADOW_TEXTURE0_ID);
		osgVegetation::Register.TexUnits.AddUnitIfNotPresent(7, OV_SHADOW_TEXTURE1_ID);
		int shadowTexUnit = osgVegetation::Register.TexUnits.GetUnit(OV_SHADOW_TEXTURE0_ID);
		osg::ref_ptr<osgShadow::ShadowedScene> shadowedScene = new osgShadow::ShadowedScene;
		int mapres = 2048;
		osgShadow::ShadowSettings* settings = shadowedScene->getShadowSettings();
		settings->setReceivesShadowTraversalMask(m_ReceivesShadowTraversalMask | 0x1);
		settings->setCastsShadowTraversalMask(m_CastsShadowTraversalMask);
		//settings->setShadowMapProjectionHint(osgShadow::ShadowSettings::PERSPECTIVE_SHADOW_MAP);
		//settings->setShadowMapProjectionHint(osgShadow::ShadowSettings::ORTHOGRAPHIC_SHADOW_MAP);
		settings->setBaseShadowTextureUnit(shadowTexUnit);
		double n = 0.5;
		settings->setMinimumShadowMapNearFarRatio(n);
		settings->setNumShadowMapsPerLight(numShadowMaps);
		//settings->setMultipleShadowMapHint(osgShadow::ShadowSettings::PARALLEL_SPLIT);
		//settings->setMultipleShadowMapHint(osgShadow::ShadowSettings::CASCADED);
		settings->setMaximumShadowMapDistance(800);
		settings->setTextureSize(osg::Vec2s(mapres, mapres));
		//settings->setComputeNearFarModeOverride(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);
		//settings->setComputeNearFarModeOverride(osg::CullSettings::COMPUTE_NEAR_FAR_USING_PRIMITIVES);
		settings->setComputeNearFarModeOverride(osg::CullSettings::COMPUTE_NEAR_FAR_USING_BOUNDING_VOLUMES);
		//settings->setShaderHint(osgShadow::ShadowSettings::NO_SHADERS);
		settings->setShaderHint(osgShadow::ShadowSettings::PROVIDE_VERTEX_AND_FRAGMENT_SHADER);
		osg::ref_ptr<osgShadow::ViewDependentShadowMap> vdsm = new osgShadow::ViewDependentShadowMap;
		shadowedScene->setShadowTechnique(vdsm.get());
		
		return shadowedScene;
	}
		
	osg::ref_ptr<osgShadow::ShadowedScene> _CreateShadowedScene(ShadowModeEnum shadow_mode)
	{
		osg::ref_ptr<osgShadow::ShadowedScene> ss;
		if (shadow_mode == SM_LISPSM)
			ss = _CreateShadowedSceneLiSPSM();
		else if (shadow_mode == SM_VDSM1)
			ss = _CreateShadowedSceneVDSM(1);
		else if (shadow_mode == SM_VDSM2)
			ss = _CreateShadowedSceneVDSM(2);
		return ss;
	}

	

	osg::ref_ptr<osg::Group> m_SceneData;
	//osg::ref_ptr<osg::Group> m_RootNode;
	osg::ref_ptr<osgShadow::ShadowedScene> m_ShadowScene;
	osgViewer::Viewer m_Viewer;
	osg::Light* m_Light;
	unsigned int m_ReceivesShadowTraversalMask;
	unsigned int m_CastsShadowTraversalMask;
};