#pragma once
#include "Common.h"
#include "BillboardData.h"
#include <osg/BoundingBox>
#include <osg/Referenced>
#include <osg/vec4>
#include <osg/vec3>
#include <osg/vec2>
#include <vector>

namespace osgVegetation
{
	class osgvExport Serializer
	{
	public:
		Serializer(){}
		virtual ~Serializer(){}
		BillboardData loadBillboardData(const std::string &filename);
	};
}