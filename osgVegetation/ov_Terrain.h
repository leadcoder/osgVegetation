#pragma once
#include "ov_Common.h"
//#include "ov_BillboardLayer.h"
#include <osg/Vec2>
#include <vector>

namespace osgVegetation
{
	class DetailLayer
	{
	public:
		DetailLayer(const std::string &texture, float scale = 1) : DetailTexture(texture) , Scale(scale){}
		std::string DetailTexture;
		float Scale;
	};

	class TerrainTextureUnitSettings
	{
	public:
		TerrainTextureUnitSettings() : ColorTextureUnit(-1),
			SplatTextureUnit(-1), 
			ElevationTextureUnit(-1),
			DetailTextureUnit(-1)
		{}
		int ColorTextureUnit;
		int SplatTextureUnit;
		int ElevationTextureUnit;
		int DetailTextureUnit;
	};

	class TerrainShadingConfiguration
	{
	public:
		TerrainShadingConfiguration(const TerrainTextureUnitSettings& tex_units) :
			TextureUnits(tex_units),
			UseTessellation(false)
		{

		}
		std::vector<DetailLayer> DetailLayers;
		bool UseTessellation;
		TerrainTextureUnitSettings TextureUnits;
	};
}