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
		OVTConfig() : ObjectsCastShadow(false), TerrainCastShadow(false){}
		std::string Filename;
		std::string TerrainType;
		bool ObjectsCastShadow;
		bool TerrainCastShadow;
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
