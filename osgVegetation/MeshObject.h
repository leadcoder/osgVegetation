#pragma once
#include <osg/BoundingBox>
#include <osg/Referenced>
#include <osg/vec4>
#include <osg/vec3>
#include <osg/Vec4ub>
#include <osg/ref_ptr>
#include <vector>
#include <map>
#include "BillboardLayer.h"


namespace osgVegetation
{
	class MeshObject : public osg::Referenced
	{
	public:
		MeshObject():
		  Color(255,255,255,255),
			  Width(1.0f),
			  Height(1.0f){}
		  MeshObject(const osg::Vec3& position, const osg::Vec4& color, float width, float height, unsigned int type):
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
	typedef std::vector<osg::ref_ptr<MeshObject> > MeshVegetationObjectVector;
	typedef std::map<int, MeshVegetationObjectVector> MeshVegetationMap;

	class MeshObjectClass : public osg::Referenced
	{
	public:
		MeshObjectClass()
		{

		}
	};
}