#pragma once
#include "Common.h"
#include "BillboardLayer.h"

namespace osgVegetation
{
	enum BillboardType
	{
		BT_ROTATED_QUAD,
		BT_CROSS_QUADS,
		BT_GRASS
	};
	
	/**
		Rendering technique enumeration
		This list the type of technique used to realize the billboard rendering.
		For example if we should use shader instancing (BRTShaderInstancing) or 
		geometry shaders (BRTGeometryShader) .
	*/
	enum BillboardRenderingTechnique
	{
		BRT_SHADER_INSTANCING,
		BRT_GEOMETRY_SHADER
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
			Type(BT_CROSS_QUADS),
			TilePixelSize(0),
			Technique(BRT_SHADER_INSTANCING),
			UseMultiSample(false)
		{

		}

		/**
			Enable alpha blending for all layers
		*/
		bool UseAlphaBlend;

		/**
		Enable multi sampling
		*/
		bool UseMultiSample;

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
			Should geometry receive shadows or not. Default to false
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
			Type of billboard, Default to BT_CROSS_QUADS
		*/
		BillboardType Type;

		/**
			Set this value if you want to override default distance based range mode
			If this value is 0 (default) osg::LOD::DISTANCE_FROM_EYE_POINT range mode 
			is used otherwise osg::LOD::PIXEL_SIZE_ON_SCREEN is used.
			Default to 0
		*/
		int TilePixelSize;


		/**
			Rendering Technique, default to BRT_SHADER_INSTANCING
		*/
		BillboardRenderingTechnique Technique;
		
	};
}
