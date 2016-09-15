#pragma once
#include "Common.h"
#include <osg/Vec2>
#include <vector>

namespace osgVegetation
{
	/**
		Struct holding data for billboard layer.
	*/
	struct BillboardLayer
	{
	public:
		BillboardLayer(const std::string &tex_name, double min_tile_size) : TextureName(tex_name),
			MinTileSize(min_tile_size),
			Height(1.0, 1.0),
			Width(1.0, 1.0),
			Scale(1.0, 1.0),
			ColorIntensity(1.0, 1.0),
			Density(1.0),
			TerrainColorRatio(0.0),
			UseTerrainIntensity(false),
			_TextureIndex(-1),
			_QTLevel(-1)
		{

		}

		/**
			2D Texture used in this billboard layers (note that all textures for
			all layers should be in same resolution.
		*/
		std::string TextureName;

		/**
			Min tile size. This layer .
			The exact distance is depending on other layers view distance (and quad tree tile cutoff).
		*/
		double MinTileSize;

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
			Percentage to of ground texture color/intensity to use for billboard coloring
		*/
		double TerrainColorRatio;

		/**
			Only use intensity from ground texture (i.e (r+g+b)/3.0) when generating billboard color.
			This is false by the fault which means that each color
			channel from the ground texture is respected.
		*/
		bool UseTerrainIntensity;

		/**
			Coverage material vector that specify where to scatter billboards
		*/
		std::vector<std::string> CoverageMaterials;


		//internal data holding texture index inside texture array
		int _TextureIndex;
		//internal data holding quad tree level for this layer
		int _QTLevel;

		/**
			Helper function to check is this layer hold coverage material
		*/
		bool hasCoverage(const std::string& name) const
		{
			for(size_t i = 0 ; i < CoverageMaterials.size(); i++)
			{
				if(CoverageMaterials[i] == name)
					return true;
			}
			return false;
		}
	};
	typedef std::vector<BillboardLayer> BillboardLayerVector;
}
