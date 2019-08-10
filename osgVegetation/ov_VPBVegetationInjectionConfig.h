#pragma once

#include "ov_BillboardLayer.h"
#include "ov_BillboardLayerStateSet.h"
#include "ov_MeshTileGeneratorConfig.h"

namespace osgVegetation
{

	class VPBInjectionLODConfig
	{
	public:
		VPBInjectionLODConfig(int target_level = 0, const std::vector<BillboardLayer> &layers = std::vector<BillboardLayer>()) : TargetLevel(target_level),
			Layers(layers)
		{
		}
		int TargetLevel;
		std::vector<BillboardLayer> Layers;
		std::vector<MeshLayerConfig> MeshLayers;
	};

	class VPBVegetationInjectionConfig
	{
	public:
		VPBVegetationInjectionConfig(const std::vector<VPBInjectionLODConfig> &lods = std::vector<VPBInjectionLODConfig>(),
			int billboard_texture_unit = 2,
			int receives_shadow_mask = 0x1,
			int cast_shadow_mask = 0x2) : TerrainLODs(lods),
			BillboardTexUnit(billboard_texture_unit),
			ReceivesShadowTraversalMask(receives_shadow_mask),
			CastShadowTraversalMask(cast_shadow_mask)
		{}
		int BillboardTexUnit;
		int ReceivesShadowTraversalMask;
		int CastShadowTraversalMask;
		std::vector<VPBInjectionLODConfig> TerrainLODs;
	};
}