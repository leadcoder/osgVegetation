#pragma once
#include "ov_Common.h"
#include "ov_PassFilter.h"
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

		BillboardLayer(BillboardLayerType type = BLT_ROTATED_QUAD) : Type(type),
			MaxDistance(150),
			Density(0.02), 
			ColorImpact(1.0),
			AlphaRejectValue(0.1)
		{

		}

		~BillboardLayer()
		{

		}

		float MaxDistance;
		float Density;
		float ColorImpact;
		float AlphaRejectValue;
		BillboardLayerType Type;
		PassFilter Filter;
		std::vector<Billboard> Billboards;
	private:
	};



}