#pragma once
#include "Common.h"
#include <osg/Referenced>
#include <osg/Vec4>
#include "CoverageColor.h"

namespace osgVegetation
{
	/**
		Interface for terrain queries
	*/
	class osgvExport ITerrainQuery : public osg::Referenced
	{
	public:
		virtual ~ITerrainQuery(){};
		/**
			Get terrain data for provided location
		*/
		virtual bool getTerrainData(osg::Vec3d& location, osg::Vec4 &color, std::string &coverage_name , CoverageColor &coverage_color, osg::Vec3d &inter) = 0;
	};
}
