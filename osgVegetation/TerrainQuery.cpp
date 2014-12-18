#include "TerrainQuery.h"
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Math>
#include <osg/Texture2D>
#include <osg/TexMat>
#include <osg/TextureBuffer>
#include <osg/Image>
#include <osgDB/WriteFile>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <osgUtil/LineSegmentIntersector>
#include <osgUtil/IntersectionVisitor>
#include <osgSim/LineOfSight>
#include <iostream>
#include <sstream>
#include "VegetationUtils.h"

namespace osgVegetation
{
	struct PagedReaderCallback : public osgUtil::IntersectionVisitor::ReadCallback
	{
		std::vector<osg::ref_ptr<osg::Node>  > pagedLODVec;

		virtual osg::Node* readNodeFile( const std::string& filename )
		{ 
			osg::ref_ptr<osg::Node> node = osgDB::readNodeFile(filename); 
			pagedLODVec.push_back(node);
			if(pagedLODVec.size() > 100) //flush cache
				pagedLODVec.clear();
			return node.get();
		}
	};

	TerrainQuery::TerrainQuery(osg::Node* terrain): m_Terrain(terrain),
		m_MaterialTextureSuffix("_material.png")
	{
		m_Cache = new osgSim::DatabaseCacheReadCallback;
		m_IntersectionVisitor.setReadCallback(m_Cache);
		m_IntersectionVisitor.setLODSelectionMode(osgUtil::IntersectionVisitor::USE_HIGHEST_LEVEL_OF_DETAIL);
	}

	bool TerrainQuery::getTerrainData(osg::Vec3& location, osg::Vec4 &color, osg::Vec4 &material_color, osg::Vec3 &inter)
	{
		osg::Vec3 start_location(location.x(),location.y(), -100);
		osg::ref_ptr<osgUtil::LineSegmentIntersector> intersector =	new osgUtil::LineSegmentIntersector(start_location,start_location + osg::Vec3(0.0f,0.0f,300));
		m_IntersectionVisitor.setIntersector(intersector.get());
		m_Terrain->accept(m_IntersectionVisitor);
		if (intersector->containsIntersections())
		{
			osgUtil::LineSegmentIntersector::Intersections& intersections = intersector->getIntersections();
			for(osgUtil::LineSegmentIntersector::Intersections::iterator itr = intersections.begin();
				itr != intersections.end();
				++itr)
			{
				const osgUtil::LineSegmentIntersector::Intersection& intersection = *itr;
				osg::Vec3 tc;
				osg::Texture* texture = _getTexture(intersection,tc);

				if(texture && texture->getImage(0))
				{
					std::string tex_filename = osgDB::getSimpleFileName(texture->getImage(0)->getFileName());
				    //check if dds, if so we will try to load alternative image file because we have no utils to decompress dds
					if(osgDB::getFileExtension(tex_filename) == "dds")
					{
						tex_filename = osgDB::getNameLessExtension(tex_filename) + ".png";
						//first check cache
						osg::Image* image = _loadImage(tex_filename);
						if(image)
						{
							osg::Vec3 tc2(tc.x(),1.0 - tc.y(),tc.z());
							color = image->getColor(tc2);
						}
					}
					else
						color = texture->getImage(0)->getColor(tc);

					//get material texture
					//const std::string mat_image_filename = osgDB::getNameLessExtension(osgDB::getSimpleFileName(tex_filename)) + "_material.png";
					std::string mat_image_filename;
					if(m_MaterialTexture != "")
						mat_image_filename = m_MaterialTexture;
					else
						mat_image_filename = osgDB::getNameLessExtension(osgDB::getSimpleFileName(tex_filename)) + m_MaterialTextureSuffix;

					osg::Image* image = _loadImage(mat_image_filename);
					if(image)
					{
						osg::Vec3 tc2(tc.x(),1.0 - tc.y(),tc.z());
						material_color = image->getColor(tc2);
					}
					else
						material_color = color;
				}
				inter = intersection.getWorldIntersectPoint();
				return true;
			}
		}
		return false;
	}

	osg::Image* TerrainQuery::_loadImage(const std::string &filename)
	{
		osg::Image* image = NULL;
		MaterialCacheMap::iterator iter = m_MaterialCache.find(filename);
		if(iter != m_MaterialCache.end())
		{
			image = iter->second.get();
		}
		else
		{
			if(m_MaterialCache.size() > 100)
				m_MaterialCache.clear();

			m_MaterialCache[filename] = osgDB::readImageFile(filename);
			image = m_MaterialCache[filename].get();
		}
		return image;
	}

	osg::Texture* TerrainQuery::_getTexture(const osgUtil::LineSegmentIntersector::Intersection& intersection,osg::Vec3& tc) const
	{
		osg::Geometry* geometry = intersection.drawable.valid() ? intersection.drawable->asGeometry() : 0;
		osg::Vec3Array* vertices = geometry ? dynamic_cast<osg::Vec3Array*>(geometry->getVertexArray()) : 0;

		if (vertices)
		{
			if (intersection.indexList.size()==3 && intersection.ratioList.size()==3)
			{
				unsigned int i1 = intersection.indexList[0];
				unsigned int i2 = intersection.indexList[1];
				unsigned int i3 = intersection.indexList[2];

				float r1 = intersection.ratioList[0];
				float r2 = intersection.ratioList[1];
				float r3 = intersection.ratioList[2];

				osg::Array* texcoords = (geometry->getNumTexCoordArrays()>0) ? geometry->getTexCoordArray(0) : 0;
				osg::FloatArray* texcoords_FloatArray = dynamic_cast<osg::FloatArray*>(texcoords);
				osg::Vec2Array* texcoords_Vec2Array = dynamic_cast<osg::Vec2Array*>(texcoords);
				osg::Vec3Array* texcoords_Vec3Array = dynamic_cast<osg::Vec3Array*>(texcoords);
				if (texcoords_FloatArray)
				{
					// we have tex coord array so now we can compute the final tex coord at the point of intersection.
					float tc1 = (*texcoords_FloatArray)[i1];
					float tc2 = (*texcoords_FloatArray)[i2];
					float tc3 = (*texcoords_FloatArray)[i3];
					tc.x() = tc1*r1 + tc2*r2 + tc3*r3;
				}
				else if (texcoords_Vec2Array)
				{
					// we have tex coord array so now we can compute the final tex coord at the point of intersection.
					const osg::Vec2& tc1 = (*texcoords_Vec2Array)[i1];
					const osg::Vec2& tc2 = (*texcoords_Vec2Array)[i2];
					const osg::Vec2& tc3 = (*texcoords_Vec2Array)[i3];
					tc.x() = tc1.x()*r1 + tc2.x()*r2 + tc3.x()*r3;
					tc.y() = tc1.y()*r1 + tc2.y()*r2 + tc3.y()*r3;
				}
				else if (texcoords_Vec3Array)
				{
					// we have tex coord array so now we can compute the final tex coord at the point of intersection.
					const osg::Vec3& tc1 = (*texcoords_Vec3Array)[i1];
					const osg::Vec3& tc2 = (*texcoords_Vec3Array)[i2];
					const osg::Vec3& tc3 = (*texcoords_Vec3Array)[i3];
					tc.x() = tc1.x()*r1 + tc2.x()*r2 + tc3.x()*r3;
					tc.y() = tc1.y()*r1 + tc2.y()*r2 + tc3.y()*r3;
					tc.z() = tc1.z()*r1 + tc2.z()*r2 + tc3.z()*r3;
				}
				else
				{
					return 0;
				}
			}

			const osg::TexMat* activeTexMat = 0;
			const osg::Texture* activeTexture = 0;

			if (intersection.drawable->getStateSet())
			{
				const osg::TexMat* texMat = dynamic_cast<osg::TexMat*>(intersection.drawable->getStateSet()->getTextureAttribute(0,osg::StateAttribute::TEXMAT));
				if (texMat) activeTexMat = texMat;

				const osg::Texture* texture = dynamic_cast<osg::Texture*>(intersection.drawable->getStateSet()->getTextureAttribute(0,osg::StateAttribute::TEXTURE));
				if (texture) activeTexture = texture;
			}

			for(osg::NodePath::const_reverse_iterator itr = intersection.nodePath.rbegin();
				itr != intersection.nodePath.rend() && (!activeTexMat || !activeTexture);
				++itr)
			{
				const osg::Node* node = *itr;
				if (node->getStateSet())
				{
					if (!activeTexMat)
					{
						const osg::TexMat* texMat = dynamic_cast<const osg::TexMat*>(node->getStateSet()->getTextureAttribute(0,osg::StateAttribute::TEXMAT));
						if (texMat) activeTexMat = texMat;
					}

					if (!activeTexture)
					{
						const osg::Texture* texture = dynamic_cast<const osg::Texture*>(node->getStateSet()->getTextureAttribute(0,osg::StateAttribute::TEXTURE));
						if (texture) activeTexture = texture;
					}
				}
			}

			if (activeTexMat)
			{
				osg::Vec4 tc_transformed = osg::Vec4(tc.x(), tc.y(), tc.z() ,0.0f) * activeTexMat->getMatrix();
				tc.x() = tc_transformed.x();
				tc.y() = tc_transformed.y();
				tc.z() = tc_transformed.z();

				if (activeTexture && activeTexMat->getScaleByTextureRectangleSize())
				{
					tc.x() *= static_cast<float>(activeTexture->getTextureWidth());
					tc.y() *= static_cast<float>(activeTexture->getTextureHeight());
					tc.z() *= static_cast<float>(activeTexture->getTextureDepth());
				}
			}

			return const_cast<osg::Texture*>(activeTexture);

		}
		return 0;
	}

}
