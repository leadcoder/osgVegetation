#pragma once
#include <osg/BoundingBox>
#include <osg/Referenced>
#include <osg/vec4>
#include <osg/vec3>
#include <osg/Vec4ub>
#include <osg/ref_ptr>
#include <vector>
#include <map>
#include "VegetationLayer.h"

namespace osgVegetation
{
	class BillboardVegetationObject : public osg::Referenced
	{
	public:
		BillboardVegetationObject():
		  Color(255,255,255,255),
			  Width(1.0f),
			  Height(1.0f),
			  TextureUnit(0) {}

		  BillboardVegetationObject(const osg::Vec3& position, const osg::Vec4ub& color, float width, float height, unsigned int type):

		  Color(color),
			  Width(width),
			  Height(height),
			  TextureUnit(type) {Position = position;}

		  osg::Vec3		  Position;
		  osg::Vec4ub     Color;
		  float           Width;
		  float           Height;
		  unsigned int    TextureUnit;
		  std::vector<std::string> MeshNames;
	};
	typedef std::vector< osg::ref_ptr<BillboardVegetationObject> > BillboardVegetationObjectVector;
}