#pragma once
#include "MaterialColor.h"
#include <osg/Referenced>
#include <osg/vec4>
#include <osg/vec3>
#include <osg/vec2>
#include <osg/Vec4ub>
#include <osg/ref_ptr>

namespace osgVegetation
{

	struct MeshLod
	{
		MeshLod(const std::string &mesh, double max_dist):MeshName(mesh),
			MaxDistance(max_dist){}
		double MaxDistance;
		std::string MeshName;
		std::string ImageName;
	};

	struct MeshLayer
	{
		double Density;
		std::vector<MeshLod> MeshLODs;
		osg::Vec2 IntensitySpan;
		osg::Vec2 Height;
		osg::Vec2 Width;
	
		std::vector<MaterialColor> Materials;
		bool hasMaterial(const MaterialColor& mat) const
		{
			for(size_t i = 0 ; i < Materials.size(); i++)
			{
				if(Materials[i] == mat)
					return true;
			}
			return false;
		}
	};

	typedef std::vector<MeshLayer> MeshLayerVector;
}