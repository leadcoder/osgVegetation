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
	public:
		BillboardVegetationLayer(const std::string &tex_name) : TextureName(tex_name),
			Height(1,1),
			Width(1,1),
			Scale(1,1),
			Density(1),
			_TextureUnit(0)

		{

		}

		bool HasMaterial(const MaterialColor& mat) const
		{
			for(size_t i = 0 ; i < Materials.size(); i++)
			{
				if(Materials[i] == mat)
					return true;
			}
			return false;
		}
		
		std::string TextureName;
		osg::Vec2 Height;
		osg::Vec2 Width;
		osg::Vec2 Scale;
		double Density;
		std::vector<MaterialColor> Materials;
		osg::Vec3 MinColor;
		osg::Vec3 MaxColor;
		//internal data
		int _TextureUnit;
	};
	typedef std::vector<BillboardVegetationLayer> BillboardVegetationLayerVector;

	struct BillboardVegetationData
	{
		BillboardVegetationData(double view_distance, bool use_alpha_blend, float alpha_ref_value, bool terrain_normal) : 
			ViewDistance(view_distance),		
			UseAlphaBlend(use_alpha_blend), 
			AlphaRefValue(alpha_ref_value),
			TerrainNormal(terrain_normal)
		{

		}
		bool UseAlphaBlend;
		float AlphaRefValue;
		double ViewDistance;
		bool TerrainNormal;
		BillboardVegetationLayerVector Layers;
	};

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