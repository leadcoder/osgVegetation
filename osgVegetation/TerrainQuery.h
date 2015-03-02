#pragma once
#include "Common.h"
#include <osg/BoundingBox>
#include <osg/Referenced>
#include <osg/Vec4>
#include <osg/Vec3>
#include <osg/Vec2>
#include <osg/Node>
#include <osg/Texture>
#include <osg/ref_ptr>
#include <osgUtil/LineSegmentIntersector>
#include <vector>
#include "ITerrainQuery.h"
#include "CoverageColor.h"
#include "CoverageData.h"

namespace osgSim {class DatabaseCacheReadCallback;}

namespace osgVegetation
{
	/*
		Standard terrain query implementation.
	*/
	class osgvExport TerrainQuery : public ITerrainQuery
	{
	public:
		TerrainQuery(osg::Node* terrain,const CoverageData &cd);

		//ITerrainQuery interface
		/**
			Get terrain data for provided location
		*/
		bool getTerrainData(osg::Vec3d& location, osg::Vec4 &texture_color, std::string &coverage_name, CoverageColor &coverage_color, osg::Vec3d &inter);
	public:

		/**
			Set suffix used to generate coverage texture filename.
			The suffix is appended to extension-less terrain base texture filename.
		*/
		void setCoverageTextureSuffix(const std::string &value) {m_CoverageTextureSuffix=value;}

		/**
			Get suffix used to generate coverage texture filename.
		*/
		std::string getCoverageTextureSuffix() const {return m_CoverageTextureSuffix;}

		/**
			Set explicit coverage texture filename
		*/
		void setCoverageTexture(const std::string &value) {m_CoverageTexture=value;}

		/**
			Get explicit coverage texture filename
		*/
		std::string getCoverageTexture() const {return m_CoverageTexture;}

		/**
			Flip coverage texture coordinates
		*/
		void setFlipCoverageCoordinates(bool value) {m_FlipCoverageCoordinates=value;}
		
		/**
			Flip coverage texture coordinates
		*/
		bool getFlipCoverageCoordinates() const {return m_FlipCoverageCoordinates;}
	private:
		osg::Image* _loadImage(const std::string &filename);
		osg::Texture* _getTexture(const osgUtil::LineSegmentIntersector::Intersection& intersection,osg::Vec3& tc) const;

		osg::Node* m_Terrain;
		osgUtil::IntersectionVisitor m_IntersectionVisitor;
		typedef std::map<std::string,osg::ref_ptr<osg::Image> > ImageCacheMap;
		ImageCacheMap m_ImageCache;
		osgSim::DatabaseCacheReadCallback* m_MeshCache;
		std::string m_CoverageTextureSuffix;
		std::string m_CoverageTexture;
		CoverageData m_CoverageData;
		bool m_FlipCoverageCoordinates;
	};
}
