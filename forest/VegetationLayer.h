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
	struct VegetationLayer
	{
		std::string TextureName;
		osg::Vec2 Height;
		osg::Vec2 Width;
		double Density;
		//TextureUnit
		int TextureUnit;
		std::vector<std::string> MeshNames;
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
	typedef std::vector<VegetationLayer> VegetationLayerVector;
}