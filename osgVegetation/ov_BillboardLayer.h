#pragma once
#include "ov_Common.h"
#include <osg/Texture2DArray>
#include <osg/Multisample>
#include <osg/Program>
#include <osg/AlphaFunc>
#include <osg/BlendFunc>
#include <osg/Texture2D>
#include <osg/Multisample>
#include <osg/CullFace>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osg/PatchParameter>
#include <osgDB/ReadFile>

namespace osgVegetation
{
	class BillboardLayer
	{
	public:

		enum BillboardLayerType
		{
			BLT_ROTATED_QUAD,
			BLT_CROSS_QUADS,
			BLT_GRASS
		};

		class Billboard
		{
		public:
			Billboard(const std::string &texture, const osg::Vec2f &size, float intensity, float probability) : TextureName(texture),
				Size(size),
				Intensity(intensity),
				Probability(probability)
			{

			}

			~Billboard()
			{

			}
			std::string TextureName;
			osg::Vec2f Size;
			float Intensity;
			float Probability;

		private:
		};

		BillboardLayer(float max_dist = 150, float density = 8, float color_threshold = 0.2, float color_impact = 1.0, float lc_id = -1, int lod_level = -1) : MaxDistance(max_dist),
			Density(density), 
			ColorThreshold(color_threshold),
			ColorImpact(color_impact),
			LandCoverID(lc_id),
			AlphaRejectValue(0.1),
			LODLevel(lod_level),
			Type(BLT_ROTATED_QUAD)
		{
		}

		~BillboardLayer()
		{

		}

		float MaxDistance;
		float Density;
		float ColorThreshold;
		float ColorImpact;
		float LandCoverID;
		float AlphaRejectValue;
		int LODLevel;
		BillboardLayerType Type;
		std::vector<float> Filter;
		std::vector<Billboard> Billboards;
	private:
	};
}
