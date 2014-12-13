#pragma once
#include "Common.h"
#include <osg/BoundingBox>
#include <osg/Referenced>
#include <osg/vec4>
#include <osg/vec3>
#include <osg/vec2>
#include <osg/Vec4ub>
#include <osg/Node>
#include <osg/Texture>
#include <osg/ref_ptr>
#include <osgUtil/LineSegmentIntersector>
#include <vector>

namespace osgSim {class DatabaseCacheReadCallback;}

namespace osgVegetation
{
	class osgvExport TerrainQuery : public osg::Referenced
	{
	public:
		TerrainQuery(osg::Node* terrain);
		bool getTerrainData(osg::Vec3& location, osg::Vec4 &color, osg::Vec4 &material_color, osg::Vec3 &inter);
	private:
		osg::Image* _loadImage(const std::string &filename);
		osg::Texture* _getTexture(const osgUtil::LineSegmentIntersector::Intersection& intersection,osg::Vec3& tc) const;
		osg::Node* m_Terrain;
		osgUtil::IntersectionVisitor m_IntersectionVisitor;
		typedef std::map<std::string,osg::ref_ptr<osg::Image> > MaterialCacheMap; 
		MaterialCacheMap m_MaterialCache;
		osgSim::DatabaseCacheReadCallback* m_Cache;
	};
}