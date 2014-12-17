#pragma once
#include "Common.h"
#include <osg/Referenced>
#include <osg/vec4>
#include <osg/vec3>
#include <osg/vec2>

namespace osgVegetation
{
	/**
		Interface for terrain queries
	*/
	class osgvExport ITerrainQuery 
	{
	public:
		virtual ~ITerrainQuery(){};
		/**
			Get terrain data for provided location
		*/
		virtual bool getTerrainData(osg::Vec3& location, osg::Vec4 &color, osg::Vec4 &material_color, osg::Vec3 &inter) = 0;
	};
}