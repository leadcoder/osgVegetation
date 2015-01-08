#pragma once
#include "BillboardLayer.h"
#include <osg/Referenced>
#include <osg/vec4>
#include <osg/vec3>
#include <osg/vec2>
#include <osg/Fog>
#include <osg/Vec4ub>
#include <osg/ref_ptr>

namespace osgVegetation
{

	enum BillboardType
	{
		BT_SCREEN_ALIGNED,
		BT_CROSS_QUADS
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
			Type(BT_CROSS_QUADS)
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
	};
}