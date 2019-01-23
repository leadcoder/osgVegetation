#pragma once
#include "ov_Terrain.h"
#include "ov_BillboardLayer.h"
#include "ov_Utils.h"
#include "ov_TerrainHelper.h"
#include "ov_BillboardNodeGenerator.h"
#include <osg/PagedLOD>
#include <osgTerrain/Terrain>
#include <osgTerrain/TerrainTile>


namespace osgVegetation
{
	class LODLayer
	{
	public:
		LODLayer(int lod_level, std::vector<BillboardLayer> layers) : LODLevel(lod_level),
			Generator(layers) {}
		int LODLevel;
		BillboardNodeGenerator Generator;
	};

	class PLODTerrainTileInjection : public osgDB::ReadFileCallback
	{
	public:

		PLODTerrainTileInjection(const std::vector<LODLayer> &lod_layers) : m_LODLayers(lod_layers)
		{
			
		}
	
		PLODTerrainTileInjection(const std::vector<BillboardLayer> &layers) 
		{
			typedef std::map<int, std::vector<BillboardLayer>> LODBuckets;
			LODBuckets lod_buckets;
			for (size_t i = 0; i < layers.size(); i++)
			{
				lod_buckets[layers[i].LODLevel].push_back(layers[i]);
			}
			
			for (LODBuckets::const_iterator iter = lod_buckets.begin(); iter != lod_buckets.end(); iter++)
			{
				m_LODLayers.push_back(LODLayer(iter->first, iter->second));
			}
		}

		class TerrainTileVisitor : public osg::NodeVisitor
		{
		public:
			std::vector<osgTerrain::TerrainTile*> Tiles;
			osg::Group* MainGroup;
			TerrainTileVisitor() :
				osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) {
			}

			void apply(osg::Node& node)
			{
				osgTerrain::TerrainTile* tile = dynamic_cast<osgTerrain::TerrainTile*>(&node);
				if (tile)
				{
					Tiles.push_back(tile);
				}
				else
				{
					traverse(node);
				}
			}
		};

		static int extractLODLevelFromFileName(const std::string& filename)
		{
			std::string clean_filename = filename;
			std::size_t found = clean_filename.find_last_of("/\\");
			if (found != std::string::npos)
				clean_filename = clean_filename.substr(found + 1);

			found = clean_filename.find("_L");

			int lod_level = -1;
			if (found != std::string::npos)
			{
				std::string level = clean_filename.substr(found + 2);
				found = level.find("_X");
				if (found != std::string::npos)
				{
					level = level.substr(0, found);
					lod_level = atoi(level.c_str());
				}
			}
			return lod_level;
		}

#define OV_USE_TILE_ID_LOD_LEVEL
		virtual osgDB::ReaderWriter::ReadResult readNode(const std::string& filename, const osgDB::Options* options)
		{
			osgDB::ReaderWriter::ReadResult rr = ReadFileCallback::readNode(filename, options);

			if (!rr.getNode())
				return rr;
			TerrainTileVisitor ttv;
			rr.getNode()->accept(ttv);
			
			for (size_t i = 0; i < ttv.Tiles.size(); i++)
			{
				//osgVegetation::PrepareTerrainForDetailMapping(ttv.Tiles[i],m_Terrain);
			}
			
#ifdef OV_USE_TILE_ID_LOD_LEVEL
			const int lod_level = ttv.Tiles.size() > 0 ? ttv.Tiles[0]->getTileID().level - 1 : 0;
#else
			const int lod_level = extractLODLevelFromFileName(filename);
#endif
			for (size_t i = 0; i < m_LODLayers.size(); i++)
			{
				if (lod_level == m_LODLayers[i].LODLevel && rr.validNode())
				{
					osg::Group* group = dynamic_cast<osg::Group*>(rr.getNode());
					osg::PagedLOD* plod = dynamic_cast<osg::PagedLOD*>(rr.getNode());
					if (group && plod == 0)
					{
						osg::Group* terrain_node_group = new osg::Group();
						for (size_t j = 0; j < ttv.Tiles.size(); j++)
						{
							if(osg::Node* terrain_tile = TerrainHelper::CreateTerrainNodeFromTerrainTile(ttv.Tiles[j]))
								terrain_node_group->addChild(terrain_tile);
						}
						osg::ref_ptr<osg::Node> veg_node = m_LODLayers[i].Generator.CreateNode(terrain_node_group);
						group->addChild(veg_node);
					}
				}
			}
			return rr;
		}
	protected:
		virtual ~PLODTerrainTileInjection() {}
		std::vector<LODLayer> m_LODLayers;
	};
}