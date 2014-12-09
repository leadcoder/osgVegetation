#pragma once
#include "BillboardLayer.h"
#include <osg/Referenced>
#include <osg/vec4>
#include <osg/vec3>
#include <osg/vec2>
#include <osg/Vec4ub>
#include <osg/ref_ptr>

namespace osgVegetation
{
	/**
		Struct holding billboard collection and settings common 
		for all billboard layers used by the scattering stage
	*/
	struct BillboardData
	{
		BillboardData(double view_distance, bool use_alpha_blend, float alpha_ref_value, bool terrain_normal) : 
			ViewDistance(view_distance),		
			UseAlphaBlend(use_alpha_blend), 
			AlphaRefValue(alpha_ref_value),
			TerrainNormal(terrain_normal)
		{

		}
		/*
			Enable alpha blending for all layers
		*/
		bool UseAlphaBlend;

		/*
			Set alpha rejection value for all layers
		*/
		float AlphaRefValue;

		/*
			Max view distance
		*/
		double ViewDistance;

		/*
			Enable terrain normals, this will replace regular billboard normals (perpendicular to the billboard)
			with the terrain normal under the billboard. This is useful for small translucent billboards like 
			plants and grass. 
		*/
		bool TerrainNormal;

		/**
			The billboard collection
		*/
		BillboardLayerVector Layers;
	};
}