#pragma once
#include "Common.h"
#include <osg/BoundingBox>
#include <osg/Referenced>
#include <osg/vec4>
#include <osg/vec3>
#include <osg/vec2>
#include <osg/Node>
#include <osg/Texture>
#include <osg/ref_ptr>
#include <osgUtil/LineSegmentIntersector>
#include <vector>
#include "ITerrainQuery.h"

namespace osgSim {class DatabaseCacheReadCallback;}

namespace osgVegetation
{
	/*
		Standard terrain query implementation.
	*/
	class osgvExport TerrainQuery : public osg::Referenced, public ITerrainQuery
	{
	public:
		TerrainQuery(osg::Node* terrain);
		
		//ITerrainQuery interface 
		/**
			Get terrain data for provided location
		*/
		bool getTerrainData(osg::Vec3& location, osg::Vec4 &color, osg::Vec4 &material_color, osg::Vec3 &inter);
	public:

		/**
			Set suffix used to generate material texture filename. 
			The suffix is appended to extension-less terrain texture filename.
		*/
		void setMaterialTextureSuffix(const std::string &value) {m_MaterialTextureSuffix=value;}
		
		/**
			Get suffix used to generate material texture filename. 
		*/
		std::string getMaterialTextureSuffix() const {return m_MaterialTextureSuffix;}
		
		/**
			Set explicit material texture filename
		*/
		void setMaterialTexture(const std::string &value) {m_MaterialTexture=value;}
		
		/**
			Get explicit material texture filename
		*/
		std::string getMaterialTexture() const {return m_MaterialTexture;}
	private:
		osg::Image* _loadImage(const std::string &filename);
		osg::Texture* _getTexture(const osgUtil::LineSegmentIntersector::Intersection& intersection,osg::Vec3& tc) const;
		
		osg::Node* m_Terrain;
		osgUtil::IntersectionVisitor m_IntersectionVisitor;
		typedef std::map<std::string,osg::ref_ptr<osg::Image> > MaterialCacheMap; 
		MaterialCacheMap m_MaterialCache;
		osgSim::DatabaseCacheReadCallback* m_Cache;
		std::string m_MaterialTextureSuffix;
		std::string m_MaterialTexture;
	};
}