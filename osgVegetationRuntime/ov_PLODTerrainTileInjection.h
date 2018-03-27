#pragma once
#include "ov_Terrain.h"
#include "ov_BillboardLayer.h"
#include "ov_BillboardTile.h"
#include "ov_Utils.h"
#include <osg/PagedLOD>
#include <osgTerrain/Terrain>
#include <osgTerrain/TerrainTile>

namespace osgVegetation
{
	class PLODTerrainTileInjection : public osgDB::ReadFileCallback
	{
	public:
		PLODTerrainTileInjection(const Terrain &data) : m_Terrain(data)
		{

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

			TerrainTileVisitor ttv;
			rr.getNode()->accept(ttv);
			
			for (size_t i = 0; i < ttv.Tiles.size(); i++)
			{
				osgVegetation::PrepareTerrainForDetailMapping(ttv.Tiles[i],m_Terrain);
			}

			
#ifdef OV_USE_TILE_ID_LOD_LEVEL
			const int lod_level = ttv.Tiles.size() > 0 ? ttv.Tiles[0]->getTileID().level - 1 : 0;
#else
			const int lod_level = extractLODLevelFromFileName(filename);
#endif
			for (size_t i = 0; i < m_Terrain.BillboardLayers.size(); i++)
			{
				if (lod_level == m_Terrain.BillboardLayers[i].LODLevel && rr.validNode())
				{
					osg::Group* group = dynamic_cast<osg::Group*>(rr.getNode());
					osg::PagedLOD* plod = dynamic_cast<osg::PagedLOD*>(rr.getNode());
					if (group && plod == 0)
					{
						osg::Group* veg_layer = new osg::Group();
						group->addChild(veg_layer);
						for (size_t j = 0; j < ttv.Tiles.size(); j++)
						{
							osg::Node* veg_node = BillboardLayerHelper::CreateVegNodeFromTerrainTile(ttv.Tiles[j], m_Terrain.BillboardLayers[i]);
							veg_layer->addChild(veg_node);
						}
					}
				}
			}
			return rr;
		}
	protected:
		virtual ~PLODTerrainTileInjection() {}
		Terrain m_Terrain;
	};
}