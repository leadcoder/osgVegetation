#pragma once
#include "Common.h"
#include <osg/Vec4>
#include <osg/Vec3>
#include <osg/Node>
#include <osg/Texture>
#include <osg/ref_ptr>
#include <osgUtil/LineSegmentIntersector>
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
			Set suffix used to generate alternative color texture filename when terrain texture is stored as dds.
			The suffix is appended to extension-less terrain base texture filename.
		*/
		void setColorTextureSuffix(const std::string &value) {m_ColorTextureSuffix=value;}

		/**
			Get suffix used to generate alternative color texture filename.
		*/
		std::string getColorTextureSuffix() const {return m_ColorTextureSuffix;}


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

		/**
			Flip color texture coordinates
		*/
		void setFlipColorCoordinates(bool value) {m_FlipColorCoordinates=value;}
		
		/**
			Flip color texture coordinates
		*/
		bool getFlipColorCoordinates() const {return m_FlipColorCoordinates;}
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
		std::string m_ColorTextureSuffix;

		CoverageData m_CoverageData;
		bool m_FlipCoverageCoordinates;
		bool m_FlipColorCoordinates;
	};
}
