#pragma once
#include "Common.h"
#include "BillboardData.h"
#include "CoverageData.h"
#include <osg/BoundingBox>
#include <osg/Referenced>
#include <osg/vec4>
#include <osg/vec3>
#include <osg/vec2>
#include <osg/Node>
#include <vector>
class TiXmlElement;
namespace osgVegetation
{
	class ITerrainQuery;

	class osgvExport Serializer
	{
	public:
		typedef std::map<std::string,CoverageColor> MaterialMapping;
		Serializer(){}
		virtual ~Serializer(){}
		std::vector<BillboardData> loadBillboardData(const std::string &filename) const;
		BillboardData loadBillboardData(TiXmlElement *bd_elem) const;
		osg::ref_ptr<ITerrainQuery> loadTerrainQuery(osg::Node* terrain, const std::string &filename) const;
		CoverageData loadCoverageData(TiXmlElement *cd_elem) const;
	};
}