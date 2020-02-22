#pragma once

#include "ov_BillboardLayerConfig.h"
#include "ov_BillboardLayerStateSet.h"
#include "ov_MeshLayerConfig.h"

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
		VPBVegetationInjectionConfig(const std::vector<VPBInjectionLODConfig> &lods = std::vector<VPBInjectionLODConfig>()) : TerrainLODs(lods)
		{}
		std::vector<VPBInjectionLODConfig> TerrainLODs;
	};
}