#pragma once
#include "Common.h"
#include <osg/Referenced>
#include <osg/Vec4>
#include <osg/Vec3>
#include <osg/Vec2>
#include <osg/Fog>

namespace osgVegetation
{
	enum OSGShadowMode
	{
		SM_DISABLED,
		SM_LISPSM,
		SM_VDSM1, //one texture
		SM_VDSM2, //two textures
	};

	struct EnvironmentSettings
	{
		EnvironmentSettings() :
			UseFog(false),
			FogMode(osg::Fog::LINEAR),
			ShadowMode(SM_DISABLED),
			BaseShadowTextureUnit(6)
		{

		}
		/**
		Indicates if fog should be inject in shaders,
		*/
		bool UseFog;

		/**
		Fog mode that should be used if UseFog is true
		*/
		osg::Fog::Mode FogMode;

		/**
		This will be used when generating shadow map look up in the shader generator
		*/
		OSGShadowMode ShadowMode;

		/**
		
		*/
		int BaseShadowTextureUnit;
	};
}