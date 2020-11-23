#pragma once
#include <osg/Geometry>
#include <osg/Geode>
#include <osg/LightSource>
#include <osg/Fog>
#include <osgShadow/ShadowedScene>
#include <osgShadow/LightSpacePerspectiveShadowMap>
#include <osgShadow/ViewDependentShadowMap>

#include "ovSampleUtils_StateSetManipulator.h"

namespace ovSampleUtils
{
	struct BoundingBoxCB : public osg::Drawable::ComputeBoundingBoxCallback
	{
		BoundingBoxCB() {}
		BoundingBoxCB(const osg::BoundingBox& bbox) : _bbox(bbox) {};
		osg::BoundingBox computeBound(const osg::Drawable&) const { return _bbox; }
	private:
		osg::BoundingBox _bbox;
	};

	osg::ref_ptr<osg::LightSource> createSunLight()
	{
		osg::ref_ptr<osg::Light> light = new osg::Light;
		light->setDiffuse(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
		osg::Vec4 light_pos(0.5, -0.5, 0.7, 0);
		light->setPosition(light_pos);		// last param	w = 0.0 directional light (direction)
		osg::Vec3f light_dir(-light_pos.x(), -light_pos.y(), -light_pos.z());
		light_dir.normalize();
		light->setDirection(light_dir);
		light->setAmbient(osg::Vec4(0.5f, 0.5f, 0.5f, 1.0f));
		osg::ref_ptr <osg::LightSource> light_source = new osg::LightSource;
		light_source->setLight(light);
		return light_source;
	}

	osg::ref_ptr<osg::Fog> createDefaultFog(osg::Fog::Mode fog_mode = osg::Fog::Mode::LINEAR)
	{
		const osg::Vec4 fog_color(0.5, 0.6, 0.7, 1.0);
		osg::ref_ptr<osg::Fog> fog = new osg::Fog();
		fog->setMode(osg::Fog::Mode(fog_mode));
		fog->setDensity(0.001);
		fog->setColor(fog_color);
		if (fog_mode == osg::Fog::Mode::LINEAR)
		{
			fog->setStart(0);
			fog->setEnd(500);
		}
		return fog;
	}

	osg::ref_ptr<osgShadow::ShadowedScene> createShadowedSceneLiSPSM()
	{
		const int mapres = 2048;
		const float minLightMargin = 20.f;
		const float maxFarPlane = 400;
		const int baseTexUnit = 0;
		const int shadowTexUnit = 6;

		osg::ref_ptr<osgShadow::ShadowedScene> shadowedScene = new osgShadow::ShadowedScene;
		osgShadow::ShadowSettings* settings = shadowedScene->getShadowSettings();
		
		osg::ref_ptr<osgShadow::LightSpacePerspectiveShadowMapVB> lispsm = new osgShadow::LightSpacePerspectiveShadowMapVB;
		osg::ref_ptr<osgShadow::MinimalShadowMap> sm = lispsm;
		sm->setMinLightMargin(minLightMargin);
		sm->setMaxFarPlane(maxFarPlane);
		sm->setTextureSize(osg::Vec2s(mapres, mapres));
		sm->setBaseTextureCoordIndex(baseTexUnit);
		sm->setBaseTextureUnit(baseTexUnit);
		sm->setShadowTextureCoordIndex(shadowTexUnit);
		sm->setShadowTextureUnit(shadowTexUnit);
		shadowedScene->setReceivesShadowTraversalMask(0x2);
		shadowedScene->setCastsShadowTraversalMask(0x4);

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

		bool receive_shadow = true;
		shadowedScene->getOrCreateStateSet()->addUniform(new osg::Uniform("ov_receive_shadow", receive_shadow));
		shadowedScene->getOrCreateStateSet()->addUniform(new osg::Uniform("osg_shadowMaxDistance", float(maxFarPlane)));
		shadowedScene->getOrCreateStateSet()->addUniform(new osg::Uniform("osg_shadowSoftness", float(10.0f)));

		return shadowedScene;
	}

	osg::ref_ptr<osgShadow::ShadowedScene> createShadowedSceneVDSM(int numShadowMaps = 2)
	{
		const int shadowTexUnit = 6;
		const int mapres = 2048;
		const double n = 0.5;
		const double maxDistance = 800;
		osg::ref_ptr<osgShadow::ShadowedScene> shadowedScene = new osgShadow::ShadowedScene;
		osgShadow::ShadowSettings* settings = shadowedScene->getShadowSettings();
		settings->setReceivesShadowTraversalMask(0x2);
		settings->setCastsShadowTraversalMask(0x4);
		settings->setBaseShadowTextureUnit(shadowTexUnit);
		settings->setMinimumShadowMapNearFarRatio(n);
		settings->setNumShadowMapsPerLight(numShadowMaps);
		//settings->setMultipleShadowMapHint(osgShadow::ShadowSettings::PARALLEL_SPLIT);
		//settings->setMultipleShadowMapHint(osgShadow::ShadowSettings::CASCADED);
		settings->setMaximumShadowMapDistance(maxDistance);
		settings->setTextureSize(osg::Vec2s(mapres, mapres));
		//settings->setComputeNearFarModeOverride(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);
		//settings->setComputeNearFarModeOverride(osg::CullSettings::COMPUTE_NEAR_FAR_USING_PRIMITIVES);
		settings->setComputeNearFarModeOverride(osg::CullSettings::COMPUTE_NEAR_FAR_USING_BOUNDING_VOLUMES);
		//settings->setShaderHint(osgShadow::ShadowSettings::NO_SHADERS);
		settings->setShaderHint(osgShadow::ShadowSettings::PROVIDE_VERTEX_AND_FRAGMENT_SHADER);
		settings->setDebugDraw(false);
		osg::ref_ptr<osgShadow::ViewDependentShadowMap> vdsm = new osgShadow::ViewDependentShadowMap;
		shadowedScene->setShadowTechnique(vdsm.get());

		bool receive_shadow = true;
		shadowedScene->getOrCreateStateSet()->addUniform(new osg::Uniform("ov_receive_shadow", receive_shadow));
		shadowedScene->getOrCreateStateSet()->addUniform(new osg::Uniform("osg_shadowMaxDistance", float(maxDistance)));
		shadowedScene->getOrCreateStateSet()->addUniform(new osg::Uniform("osg_shadowSoftness", float(10.0f)));

		return shadowedScene;
	}

	osg::ref_ptr<osg::Node> createFlatGrid(double terrain_size, int samples, bool tess = false)
	{
		const osg::Vec3 origin(-terrain_size / 2.0, -terrain_size / 2.0, 0.0);
		const unsigned int numColumns = samples;
		const unsigned int numRows = samples;
		const osg::Vec2 size(terrain_size, terrain_size);
		osg::Geometry* geometry = new osg::Geometry;

		osg::Vec3Array& v = *(new osg::Vec3Array(numColumns * numRows));
		osg::Vec2Array& t = *(new osg::Vec2Array(numColumns * numRows));
		osg::Vec4ubArray& color = *(new osg::Vec4ubArray(1));

		color[0].set(255, 255, 255, 255);

		float rowCoordDelta = size.y() / (float)(numRows - 1);
		float columnCoordDelta = size.x() / (float)(numColumns - 1);

		float rowTexDelta = 1.0f / (float)(numRows - 1);
		float columnTexDelta = 1.0f / (float)(numColumns - 1);

		osg::Vec3 pos = origin;
		osg::Vec2 tex(0.0f, 0.0f);
		int vi = 0;
		for (unsigned int r = 0; r < numRows; ++r)
		{
			pos.x() = origin.x();
			tex.x() = 0.0f;
			for (unsigned int c = 0; c < numColumns; ++c)
			{
				v[vi].set(pos.x(), pos.y(), 0);
				t[vi].set(tex.x(), tex.y());
				pos.x() += columnCoordDelta;
				tex.x() += columnTexDelta;
				++vi;
			}
			pos.y() += rowCoordDelta;
			tex.y() += rowTexDelta;
		}

		geometry->setVertexArray(&v);
		geometry->setColorArray(&color);
		geometry->setColorBinding(osg::Geometry::BIND_OVERALL);
		geometry->setTexCoordArray(0, &t);
		if (tess)
			geometry->setUseDisplayList(false);

		osg::DrawElementsUShort& drawElements = *(new osg::DrawElementsUShort(tess ? GL_PATCHES : GL_TRIANGLES, 3 * 2 * numRows * numColumns));
		geometry->addPrimitiveSet(&drawElements);
		int ei = 0;
		for (unsigned int r = 0; r < numRows - 1; ++r)
		{
			for (unsigned int c = 0; c < numColumns - 1; ++c)
			{
				drawElements[ei++] = (r + 1) * numColumns + c + 1;
				drawElements[ei++] = (r + 1) * numColumns + c;
				drawElements[ei++] = r * numColumns + c;
				
				drawElements[ei++] = r * numColumns + c + 1;
				drawElements[ei++] = (r + 1) * numColumns + c + 1;
				drawElements[ei++] = r * numColumns + c;
			}
		}
		osg::ref_ptr < osg::Geode> geode = new osg::Geode();
		geode->addDrawable(geometry);

		geometry->setComputeBoundingBoxCallback(new BoundingBoxCB(osg::BoundingBox(osg::Vec3d(-terrain_size / 2.0, -terrain_size / 2.0, 0),
			osg::Vec3d(terrain_size / 2.0, terrain_size / 2.0, 1))));
		return geode;
	}
}