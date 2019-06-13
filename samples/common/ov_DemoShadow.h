#pragma once
#include "ov_Scene.h"
#include <osgShadow/ShadowedScene>
#include <osgShadow/ShadowMap>
#include <osgShadow/ParallelSplitShadowMap>
#include <osgShadow/LightSpacePerspectiveShadowMap>
#include <osgShadow/StandardShadowMap>
#include <osgShadow/ViewDependentShadowMap>


static int ReceivesShadowTraversalMask = 0x1;
static int CastsShadowTraversalMask = 0x2;

osg::ref_ptr<osg::Group> CreateShadowNode(osgVegetation::ShadowModeEnum type)
{
	int shadowTexUnit = 6;
	if (type == osgVegetation::SM_LISPSM)
	{
		osg::ref_ptr<osgShadow::ShadowedScene> shadowedScene = new osgShadow::ShadowedScene;
		int mapres = 2048;
		osg::ref_ptr<osgShadow::LightSpacePerspectiveShadowMapVB> lispsm = new osgShadow::LightSpacePerspectiveShadowMapVB;

		osg::ref_ptr<osgShadow::MinimalShadowMap> sm = lispsm;
		float minLightMargin = 20.f;
		float maxFarPlane = 1400;
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
	
		shadowedScene->setReceivesShadowTraversalMask(ReceivesShadowTraversalMask);
		shadowedScene->setCastsShadowTraversalMask(CastsShadowTraversalMask);

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
	else if (type == osgVegetation::SM_VDSM1 ||
			 type == osgVegetation::SM_VDSM2)
	{
		osg::ref_ptr<osgShadow::ShadowedScene> shadowedScene = new osgShadow::ShadowedScene;
		int mapres = 2048;

	
		osgShadow::ShadowSettings* settings = shadowedScene->getShadowSettings();
		settings->setReceivesShadowTraversalMask(ReceivesShadowTraversalMask);
		settings->setCastsShadowTraversalMask(CastsShadowTraversalMask);
		settings->setShadowMapProjectionHint(osgShadow::ShadowSettings::PERSPECTIVE_SHADOW_MAP);
		
		settings->setBaseShadowTextureUnit(shadowTexUnit);

		double n = 0.8;
		settings->setMinimumShadowMapNearFarRatio(n);

		const unsigned int numShadowMaps = (type == osgVegetation::SM_VDSM1) ? 1 : 2;
		settings->setNumShadowMapsPerLight(numShadowMaps);
		//settings->setMultipleShadowMapHint(osgShadow::ShadowSettings::PARALLEL_SPLIT);
		settings->setMultipleShadowMapHint(osgShadow::ShadowSettings::CASCADED);
		settings->setMaximumShadowMapDistance(400);
		settings->setTextureSize(osg::Vec2s(mapres, mapres));
		//settings->setShaderHint(osgShadow::ShadowSettings::PROVIDE_VERTEX_AND_FRAGMENT_SHADER);
		osg::ref_ptr<osgShadow::ViewDependentShadowMap> vdsm = new osgShadow::ViewDependentShadowMap;
		shadowedScene->setShadowTechnique(vdsm.get());
		
		osg::Uniform* shadowTextureUnit0 = new osg::Uniform(osg::Uniform::INT, "shadowTextureUnit0");
		shadowTextureUnit0->set(shadowTexUnit);
		shadowedScene->getOrCreateStateSet()->addUniform(shadowTextureUnit0);
		
		if (numShadowMaps > 1)
		{
			osg::Uniform* shadowTextureUnit1 = new osg::Uniform(osg::Uniform::INT, "shadowTextureUnit1");
			int shadowTexUnit1 = shadowTexUnit + 1;
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
