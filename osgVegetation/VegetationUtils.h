#pragma once
#include "Common.h"
#include <osg/ref_ptr>
#include <osg/Texture2DArray>
#include <cstdlib>

namespace osgVegetation
{
	struct BillboardData;
	class Utils
	{
	public:
		static double random(double min,double max) { return min + (max-min)*static_cast<double>(rand())/ static_cast<double>(RAND_MAX); }
		static int random(int min,int max) { return min + static_cast<int>((static_cast<double>(max-min)*static_cast<double>(rand())/ static_cast<double>(RAND_MAX)) + 0.5); }

		/**
			Helper function that load all layer textures into the returning Texture2DArray.
			This function will also save texture index into the texture array for each layer (_TextureIndex)
		*/
		static osg::ref_ptr<osg::Texture2DArray> loadTextureArray(BillboardData &data);
	};
}
