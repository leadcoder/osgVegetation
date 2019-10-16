#pragma once
#include "ov_Common.h"
#include <osg/Texture2D>
#include <osg/Texture2DArray>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>

//predefined texture ids
#define OV_TERRAIN_COLOR_TEXTURE_ID "ov_TerrainColorTexture"
#define OV_TERRAIN_NORMAL_TEXTURE_ID "ov_TerrainNormalTexture"
#define OV_TERRAIN_ELEVATION_TEXTURE_ID "ov_TerrainElevationTexture"
#define OV_TERRAIN_SPLAT_TEXTURE_ID "ov_TerrainSplatTexture"
#define OV_TERRAIN_DETAIL_TEXTURE_ID "ov_TerrainDetailTexture"
#define OV_TERRAIN_NOISE_TEXTURE_ID "ov_TerrainNoiseTexture"
#define OV_BILLBOARD_TEXTURE_ID "ov_BillboardTexture"
#define OV_MESH_COLOR_TEXTURE_ID "ov_MeshColorTexture"
#define OV_SHADOW_TEXTURE0_ID "ov_ShadowTexture0"
#define OV_SHADOW_TEXTURE1_ID "ov_ShadowTexture1"

namespace osgVegetation
{
	class TextureUnits
	{
	public:
		struct NamedUnit
		{
			std::string Name;
			int Index;
		};

		int CreateOrGetUnit(const std::string &name)
		{
			int unit = GetUnit(name);
			if (unit >= 0)
				return unit;

			//get next free slot unit
			int new_unit = 0;
			while (HasUnit(new_unit))
			{
				new_unit++;
			}
			AddUnit(new_unit, name);
			return new_unit;
		}

		void AddUnit(int index, const std::string name = "")
		{
			NamedUnit named_unit;
			named_unit.Name = name;
			named_unit.Index = index;
			m_Units.push_back(named_unit);
		}

		int GetUnit(const std::string &name)
		{
			for (size_t i = 0; i < m_Units.size(); i++)
			{
				if (m_Units[i].Name == name)
				{
					return m_Units[i].Index;
				}
			}
			return -1;
		}

		bool HasUnit(int unit)
		{
			for (size_t i = 0; i < m_Units.size(); i++)
			{
				if (m_Units[i].Index == unit)
				{
					return true;
				}
			}
			return false;
		}
	private:
		std::vector<NamedUnit> m_Units;
	};
}