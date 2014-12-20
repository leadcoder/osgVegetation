#pragma once
#include "MaterialColor.h"
#include <osg/Referenced>
#include <osg/vec4>
#include <osg/vec3>
#include <osg/vec2>
#include <osg/ref_ptr>

namespace osgVegetation
{
	/**
		Struct holding data for single billboard layer.
	*/
	struct BillboardLayer
	{
	public:
		BillboardLayer(const std::string &tex_name) : TextureName(tex_name),
			Height(1.0, 1.0),
			Width(1.0, 1.0),
			Scale(1.0, 1.0),
			ColorIntensity(1.0, 1.0),
			Density(1.0),
			MixInColorRatio(0.0),
			_TextureIndex(0)

		{

		}


		bool hasMaterial(const MaterialColor& mat) const
		{
			for(size_t i = 0 ; i < Materials.size(); i++)
			{
				if(Materials[i] == mat)
					return true;
			}
			return false;
		}
		
		/**
			2D Texture used in this billboard layers (note that all textures for 
			all layers should be in same resolution
		*/
		std::string TextureName;
		
		/**
			Height interval (x=min, y=max) for all billboards populated in this layers
		*/
		osg::Vec2 Height;

		/**
			Width interval (x=min, y=max) for all billboards populated in this layers
		*/
		osg::Vec2 Width;

		/**
			Overall scale interval (x=min, y=max) for all billboards populated in this layers (will effect both height and width)
		*/
		osg::Vec2 Scale;
		
		/**
			Density for this layer (billboards/m2)
		*/
		double Density;

		/**
			Color intensity interval
		*/
		osg::Vec2 ColorIntensity;

		/**
			Percentage to of ground texture color to use for billboard coloring
		*/
		double MixInColorRatio;

		/**
			Material vector that specify where to scatter billboards
		*/
		std::vector<MaterialColor> Materials;
		
		//internal data holding texture index inside texture array
		int _TextureIndex;
	};
	typedef std::vector<BillboardLayer> BillboardLayerVector;
}