#pragma once
#include "ov_VPBVegetationInjection.h"
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
		TerrainSplatShadingConfig SplatConfig;
		BillboardNodeGeneratorConfig BillboardConfig;
	};
	
	class XMLSerializer
	{
	public:
		XMLSerializer(){}
		virtual ~XMLSerializer(){}
		static TerrainConfig ReadTerrainData(const std::string &filename);
	};
}
