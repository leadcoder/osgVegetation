#pragma once
#include <osg/Referenced>
#include <osg/vec4>
#include <osg/vec3>
#include <osg/vec2>
#include <osg/Vec4ub>
#include <osg/ref_ptr>

namespace osgVegetation
{
	typedef osg::Vec4 MaterialColor;
	struct BillboardVegetationLayer
	{
		std::string TextureName;
		osg::Vec2 Height;
		osg::Vec2 Width;
		double Density;
		//TextureUnit
		int TextureUnit;
		std::vector<MaterialColor> Materials;
		osg::Vec3 MinColor;
		osg::Vec3 MaxColor;
		bool HasMaterial(const MaterialColor& mat) const
		{
			for(size_t i = 0 ; i < Materials.size(); i++)
			{
				if(Materials[i] == mat)
					return true;
			}
			return false;
		}

	};
	typedef std::vector<BillboardVegetationLayer> BillboardVegetationLayerVector;

	struct MeshLod
	{
		MeshLod(const std::string &mesh, double max_dist):MeshName(mesh),
			MaxDistance(max_dist){}
		double MaxDistance;
		std::string MeshName;
		std::string ImageName;
	};

	struct MeshVegetationLayer
	{
		double Density;
		std::vector<MeshLod> MeshLODs;
		osg::Vec2 IntensitySpan;
		osg::Vec2 Height;
		osg::Vec2 Width;
		std::vector<MaterialColor> Materials;
		bool HasMaterial(const MaterialColor& mat) const
		{
			for(size_t i = 0 ; i < Materials.size(); i++)
			{
				if(Materials[i] == mat)
					return true;
			}
			return false;
		}
	};

	typedef std::vector<MeshVegetationLayer> MeshVegetationLayerVector;
}