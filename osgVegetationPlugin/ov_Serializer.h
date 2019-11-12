#pragma once
#include "ov_VPBVegetationInjectionConfig.h"
#include "ov_TerrainSplatShadingStateSet.h"
#include <osg/Node>
#include <vector>

class TiXmlElement;
namespace osgVegetation
{

	struct TerrainConfig
	{
	public:
		std::string Filename;
		std::string TerrainType;
		TerrainSplatShadingConfig SplatConfig;
		VPBVegetationInjectionConfig BillboardConfig;
	};
	
	class XMLSerializer
	{
	public:
		XMLSerializer(){}
		virtual ~XMLSerializer(){}
		static TerrainConfig ReadTerrainData(const std::string &filename);
	};
}
