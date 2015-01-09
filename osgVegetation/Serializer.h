#pragma once
#include "Common.h"
#include "BillboardData.h"
#include <osg/BoundingBox>
#include <osg/Referenced>
#include <osg/vec4>
#include <osg/vec3>
#include <osg/vec2>
#include <vector>
class TiXmlElement;
namespace osgVegetation
{
	class osgvExport Serializer
	{
	public:
		typedef std::map<std::string,MaterialColor> MaterialMapping;
		Serializer(){}
		virtual ~Serializer(){}
		BillboardData loadBillboardData(const std::string &filename) const;
		BillboardData loadBillboardData(TiXmlElement *bd_elem,const MaterialMapping& mapping) const;
		MaterialMapping loadMaterialMapping(TiXmlElement *mm_elem) const;
	};
}