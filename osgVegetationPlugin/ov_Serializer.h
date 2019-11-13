#pragma once
#include "ov_VPBVegetationInjectionConfig.h"
#include "ov_TerrainSplatShadingStateSet.h"
#include <osg/Node>
#include <vector>

class TiXmlElement;
namespace osgVegetation
{
	struct OVTConfig
	{
	public:
		std::string Filename;
		std::string TerrainType;
		TerrainSplatShadingConfig SplatConfig;
		VPBVegetationInjectionConfig VPBConfig;
	};
	
	class XMLSerializer
	{
	public:
		XMLSerializer(){}
		virtual ~XMLSerializer(){}
		static OVTConfig ReadOVTConfig(const std::string &filename);
	};
}
