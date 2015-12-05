#pragma once
#include "Common.h"
#include "BillboardLayer.h"
#include <osg/Referenced>
#include <osg/Vec4>
#include <osg/Vec3>
#include <osg/Vec2>
#include <osg/Fog>
#include <osg/Vec4ub>
#include <osg/ref_ptr>

namespace osgVegetation
{
	enum BillboardType
	{
		BT_ROTATED_QUAD,
		BT_CROSS_QUADS
	};

	enum OSGShadowMode
	{
		SM_LISPSM,
		SM_VDSM1, //one texture
		SM_VDSM2, //two textures
	};

	/**
		Struct holding billboard collection and settings common
		for all billboard layers used by the scattering stage
	*/
	struct BillboardData
	{
		BillboardData(const BillboardLayerVector &layers,
			bool use_alpha_blend,
			float alpha_ref_value,
			bool terrain_normal):
			Layers(layers),
			UseAlphaBlend(use_alpha_blend),
			AlphaRefValue(alpha_ref_value),
			TerrainNormal(terrain_normal),
			ReceiveShadows(false),
			CastShadows(false),
			UseFog(false),
			FogMode(osg::Fog::LINEAR),
			Type(BT_CROSS_QUADS),
			TilePixelSize(0),
			ShadowMode(SM_VDSM2)
		{

		}

		/**
			Enable alpha blending for all layers
		*/
		bool UseAlphaBlend;

		/**
			Set alpha rejection value for all layers
		*/
		float AlphaRefValue;


		/**
			Enable terrain normals, this will replace regular billboard normals (perpendicular to the billboard)
			with the terrain normal under the billboard. This is useful for small translucent billboards like
			plants and grass.
		*/
		bool TerrainNormal;

		/**
			Should geometry receive shadows or not.
		*/
		bool ReceiveShadows;

		/*
			This will be used when generating shadow map look up in the shader generator
		*/
		OSGShadowMode ShadowMode;

		/**
			Should geometry cast shadows or not.
		*/
		bool CastShadows;

		/**
			The billboard collection
		*/
		BillboardLayerVector Layers;

		/**
			Indicates if fog should be inject in shaders,
		*/
		bool UseFog;

		/**
			Fog mode that should be used if UseFog is true
		*/
		osg::Fog::Mode FogMode;

		/**
			Type of billboard
		*/
		BillboardType Type;

		/**
			Set this value if you want to override default distance based range mode
			If this value is 0 (default) osg::LOD::DISTANCE_FROM_EYE_POINT range mode 
			is used otherwise osg::LOD::PIXEL_SIZE_ON_SCREEN is used.
		*/
		int TilePixelSize;

		
	};
}
