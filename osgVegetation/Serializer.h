#pragma once
#include "Common.h"
#include "BillboardData.h"
#include "CoverageData.h"
#include "EnvironmentSettings.h"
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
		EnvironmentSettings loadEnvironmentSettings(const std::string &filename);
		EnvironmentSettings loadEnvironmentSettingsImpl(TiXmlElement *es_elem) const;

	};
}
