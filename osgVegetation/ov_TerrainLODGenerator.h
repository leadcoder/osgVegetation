#pragma once
#include "ov_Utils.h"
#include "ov_TerrainHelper.h"
#include "ov_LayerGenerator.h"
#include "ov_XMLLayerReader.h"
#include <osgTerrain/Terrain>
#include <osgTerrain/TerrainTile>
#include <osgDB/WriteFile>
#include <osgDB/FileNameUtils>

namespace osgVegetation
{
	class TerrainLODGeneratorConfig
	{
	public:
		int TargetLevel;
		std::vector<osg::ref_ptr<ILayerConfig> > Layers;

		TerrainLODGeneratorConfig(int target_level = 0) :TargetLevel(target_level)
		{
		}

		static TerrainLODGeneratorConfig ReadXML(osgDB::XmlNode* xmlNode)
		{
			TerrainLODGeneratorConfig config;
			QueryIntAttribute(xmlNode, "TargetLevel", config.TargetLevel);
			if (osgDB::XmlNode* layers_node = getFirstNodeByName(xmlNode, "Layers"))
			{
				config.Layers = readLayers(layers_node);
			}
			return config;
		}
	};

	class TerrainLODGenerator
	{
	public:
		TerrainLODGenerator(TerrainLODGeneratorConfig config) :
			TargetLevel(config.TargetLevel),
			m_Generator(config.Layers)
		{

		}
		osg::ref_ptr<osg::Group> CreateVegetationNode(osg::ref_ptr<osg::Node> terrain_geometry) const
		{
			return m_Generator.CreateVegetationNode(terrain_geometry);
		}
		int TargetLevel;
	private:
		LayerGenerator m_Generator;
	};
	
}