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
	class MeshVegetationObject : public osg::Referenced
	{
	public:
		MeshVegetationObject():
		  Color(255,255,255,255),
			  Width(1.0f),
			  Height(1.0f){}
		  MeshVegetationObject(const osg::Vec3& position, const osg::Vec4& color, float width, float height, unsigned int type):
		    Color(color),
			  Width(width),
			  Height(height)
			  {Position = position;}
		  osg::Vec3       Position;
		  osg::Quat		  Rotation;
		  osg::Vec4       Color;
		  float           Width;
		  float           Height;
	};
	typedef std::vector<osg::ref_ptr<MeshVegetationObject> > MeshVegetationObjectVector;
	typedef std::map<int, MeshVegetationObjectVector> MeshVegetationMap;
}