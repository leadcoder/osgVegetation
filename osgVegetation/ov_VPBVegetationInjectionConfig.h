#pragma once

#include "ov_BillboardLayerConfig.h"
#include "ov_BillboardLayerStateSet.h"
#include "ov_MeshLayerConfig.h"
#include "ov_TerrainSplatShadingStateSet.h"

namespace osgVegetation
{

	class VPBInjectionLODConfig
	{
	public:
		VPBInjectionLODConfig(int target_level = 0) : TargetLevel(target_level)
		{
		}
		int TargetLevel;
		std::vector<osg::ref_ptr<ILayerConfig> > Layers;
	};

	class VPBVegetationInjectionConfig
	{
	public:
		VPBVegetationInjectionConfig() {}
		VPBVegetationInjectionConfig(const std::vector<VPBInjectionLODConfig> &lods) : TerrainLODs(lods)
		{}
		//std::string Filename;
		//std::string TerrainType;
		bool ObjectsCastShadow;
		bool TerrainCastShadow;
		TerrainSplatShadingConfig SplatConfig;
		std::vector<VPBInjectionLODConfig> TerrainLODs;
	};
}