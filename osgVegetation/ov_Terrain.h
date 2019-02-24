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

	class TerrainConfiguration
	{
	public:
		/*enum OSGShadowMode
		{
			SM_DISABLED,
			SM_UNDEFINED,
			SM_LISPSM,
			SM_VDSM1, //one texture
			SM_VDSM2, //two textures
		};*/

		enum TerrainType
		{
			TT_PLOD_TERRAIN,
			TT_PLOD_GEODE,
		};

		TerrainConfiguration() :UseTessellation(true)//: Type(TT_PLOD_TERRAIN), ShadowMode(SM_DISABLED)
		{

		}

		std::string Filename;
		std::string VertexShader;
		std::string FragmentShader;
		std::vector<DetailLayer> DetailLayers;
		bool UseTessellation;
	//	std::vector<BillboardLayer> BillboardLayers;
		//OSGShadowMode ShadowMode;
		//TerrainType Type;
	};
}