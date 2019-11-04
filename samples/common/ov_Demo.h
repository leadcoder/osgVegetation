#pragma once
#include "ov_Register.h"
#include <osgShadow/ShadowedScene>
#include <osgShadow/ShadowMap>
#include <osgShadow/ParallelSplitShadowMap>
#include <osgShadow/LightSpacePerspectiveShadowMap>
#include <osgShadow/StandardShadowMap>
#include <osgShadow/ViewDependentShadowMap>

class Demo
{
public:
	Demo(int argc, char** argv, osgVegetation::SceneConfig& config) : m_Viewer(osg::ArgumentParser(&argc, argv))
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
		osgDB::Registry::instance()->getDataFilePathList().push_back("../data");

		m_SceneData = _CreateShadowNode(config.Shadow);

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

		if (config.FogMode != osgVegetation::FM_DISABLED) //Add fog effect?
		{
			const osg::Vec4 fog_color(0.5, 0.6, 0.7, 1.0);
			osg::StateSet* state = m_SceneData->getOrCreateStateSet();
			osg::ref_ptr<osg::Fog> fog = new osg::Fog();
			state->setMode(GL_FOG, osg::StateAttribute::ON);
			state->setAttributeAndModes(fog.get());
			fog->setMode(osg::Fog::Mode(config.FogMode));
			fog->setDensity(0.0005);
			fog->setColor(fog_color);
			if (config.FogMode == osgVegetation::FM_LINEAR)
			{
				fog->setStart(0);
				fog->setEnd(1000);
			}
			m_Viewer.getCamera()->setClearColor(fog_color);
			//apply scene settings
			config.Apply(m_SceneData->getOrCreateStateSet());
		}
	}

	void Run()
	{
		m_Viewer.setSceneData(m_SceneData);
		m_Viewer.setUpViewInWindow(100, 100, 800, 600);

		m_Viewer.realize();
		m_Viewer.getCamera()->getGraphicsContext()->getState()->setUseModelViewAndProjectionUniforms(true);
		//viewer.getCamera()->getGraphicsContext()->getState()->setUseVertexAttributeAliasing(true);
		//m_Viewer.getCamera()->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);
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
	osg::ref_ptr<osg::Group> GetSceneRoot() const { return m_SceneData; }
	osgViewer::Viewer& GetViewer() { return m_Viewer; }
private:
	osg::ref_ptr<osg::Group> _CreateShadowNode(osgVegetation::ShadowSettings config)
	{
		osgVegetation::Register.TexUnits.AddUnit(6, OV_SHADOW_TEXTURE0_ID);
		osgVegetation::Register.TexUnits.AddUnit(7, OV_SHADOW_TEXTURE1_ID);
		int shadowTexUnit = osgVegetation::Register.TexUnits.GetUnit(OV_SHADOW_TEXTURE0_ID);
		if (config.Mode == osgVegetation::SM_LISPSM)
		{
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

			shadowedScene->setReceivesShadowTraversalMask(config.ReceivesShadowTraversalMask);
			shadowedScene->setCastsShadowTraversalMask(config.CastsShadowTraversalMask);

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

			osg::Uniform* shadowTextureUnit = new osg::Uniform(osg::Uniform::INT, "shadowTextureUnit");
			shadowTextureUnit->set(shadowTexUnit);
			shadowedScene->getOrCreateStateSet()->addUniform(shadowTextureUnit);
			return shadowedScene;
		}
		else if (config.Mode == osgVegetation::SM_VDSM1 ||
			config.Mode == osgVegetation::SM_VDSM2)
		{
			osg::ref_ptr<osgShadow::ShadowedScene> shadowedScene = new osgShadow::ShadowedScene;
			int mapres = 2048;
			osgShadow::ShadowSettings* settings = shadowedScene->getShadowSettings();
			settings->setReceivesShadowTraversalMask(config.ReceivesShadowTraversalMask | 0x1);
			settings->setCastsShadowTraversalMask(config.CastsShadowTraversalMask);
			//settings->setShadowMapProjectionHint(osgShadow::ShadowSettings::PERSPECTIVE_SHADOW_MAP);
			//settings->setShadowMapProjectionHint(osgShadow::ShadowSettings::ORTHOGRAPHIC_SHADOW_MAP);

			settings->setBaseShadowTextureUnit(shadowTexUnit);

			double n = 0.5;
			settings->setMinimumShadowMapNearFarRatio(n);

			const unsigned int numShadowMaps = (config.Mode == osgVegetation::SM_VDSM1) ? 1 : 2;
			settings->setNumShadowMapsPerLight(numShadowMaps);

			//settings->setMultipleShadowMapHint(osgShadow::ShadowSettings::PARALLEL_SPLIT);
			//settings->setMultipleShadowMapHint(osgShadow::ShadowSettings::CASCADED);
			settings->setMaximumShadowMapDistance(800);
			settings->setTextureSize(osg::Vec2s(mapres, mapres));
			//settings->setComputeNearFarModeOverride(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);
			settings->setComputeNearFarModeOverride(osg::CullSettings::COMPUTE_NEAR_FAR_USING_PRIMITIVES);
			settings->setShaderHint(osgShadow::ShadowSettings::NO_SHADERS);
			//settings->setShaderHint(osgShadow::ShadowSettings::PROVIDE_VERTEX_AND_FRAGMENT_SHADER);
			osg::ref_ptr<osgShadow::ViewDependentShadowMap> vdsm = new osgShadow::ViewDependentShadowMap;
			shadowedScene->setShadowTechnique(vdsm.get());

			osg::Uniform* shadowTextureUnit0 = new osg::Uniform(osg::Uniform::INT, "shadowTextureUnit0");
			shadowTextureUnit0->set(shadowTexUnit);
			shadowedScene->getOrCreateStateSet()->addUniform(shadowTextureUnit0);

			if (numShadowMaps > 1)
			{
				osg::Uniform* shadowTextureUnit1 = new osg::Uniform(osg::Uniform::INT, "shadowTextureUnit1");
				int shadowTexUnit1 = osgVegetation::Register.TexUnits.GetUnit(OV_SHADOW_TEXTURE1_ID);
				shadowTextureUnit1->set(shadowTexUnit1);
				shadowedScene->getOrCreateStateSet()->addUniform(shadowTextureUnit1);
			}

			return shadowedScene;
		}
		else
		{
			return new osg::Group();
		}
	}
	osg::ref_ptr<osg::Group> m_SceneData;
	osgViewer::Viewer m_Viewer;
	osg::Light* m_Light;
};