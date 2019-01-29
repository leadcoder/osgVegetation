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
	class PLODTerrainTileInjection : public osgDB::ReadFileCallback
	{
	public:
		class PLODGeometryCreator : public osg::NodeVisitor
		{
		public:
			osg::Group* CurrentParent;
			PLODGeometryCreator(osg::Group* root) : CurrentParent(root),
				osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) {
			}

			void apply(osg::Node& node)
			{
				osgTerrain::TerrainTile* tile = dynamic_cast<osgTerrain::TerrainTile*>(&node);
				if (tile)
				{
					if (osg::Node* terrain_tile = TerrainHelper::CreateTerrainNodeFromTerrainTile(tile))
						CurrentParent->addChild(terrain_tile);
				}
				else
				{
					traverse(node);
				}
			}

			void apply(osg::Geode& geode)
			{
				if (osg::Geode* geode_copy = dynamic_cast<osg::Geode*>(geode.clone(osg::CopyOp::DEEP_COPY_ALL)))
				{
					osg::MatrixTransform* new_matrix = new osg::MatrixTransform();
					osg::NodePath nodePath = getNodePath();
					new_matrix->setMatrix(osg::computeLocalToWorld(nodePath));
					new_matrix->addChild(geode_copy);
					CurrentParent->addChild(new_matrix);
				}
			}
		};


		PLODTerrainTileInjection(const std::map<int, BillboardNodeGenerator> &lod_layers) : m_LODLayers(lod_layers)
		{

		}

		PLODTerrainTileInjection(const std::vector<BillboardLayer> &layers)
		{
			typedef std::map<int, std::vector<BillboardLayer> > LODBuckets;
			LODBuckets lod_buckets;
			for (size_t i = 0; i < layers.size(); i++)
			{
				lod_buckets[layers[i].LODLevel].push_back(layers[i]);
			}

			for (LODBuckets::const_iterator iter = lod_buckets.begin(); iter != lod_buckets.end(); iter++)
			{
				m_LODLayers.insert(std::pair<int, BillboardNodeGenerator>(iter->first, BillboardNodeGenerator(iter->second)));
			}
		}
	

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

		osg::Group*  CreateTerrainGeometry(osg::Node* terrain)
		{
			osg::Group* terrain_geometry = new osg::Group();
			PLODGeometryCreator visitor(terrain_geometry);
			terrain->accept(visitor);

			//Prepare for tesselation shaders, change PrimitiveSet draw mode to GL_PATCHES
			ConvertToPatchesVisitor cv;
			terrain_geometry->accept(cv);
			return terrain_geometry;
		}

		//#define OV_USE_TILE_ID_LOD_LEVEL
		virtual osgDB::ReaderWriter::ReadResult readNode(const std::string& filename, const osgDB::Options* options)
		{
			osgDB::ReaderWriter::ReadResult rr = ReadFileCallback::readNode(filename, options);

			if (!rr.getNode())
				return rr;

			if (!rr.validNode())
				return rr;

#ifdef OV_USE_TILE_ID_LOD_LEVEL
			const int lod_level = ttv.Tiles.size() > 0 ? ttv.Tiles[0]->getTileID().level - 1 : 0;
#else
			const int lod_level = extractLODLevelFromFileName(filename);
#endif
			std::map<int, BillboardNodeGenerator>::const_iterator iter = m_LODLayers.find(lod_level);
			if (iter != m_LODLayers.end())
			{
				osg::Group* root_node = dynamic_cast<osg::Group*>(rr.getNode());
				osg::PagedLOD* plod = dynamic_cast<osg::PagedLOD*>(root_node);  
				if (root_node && plod == NULL) //check that root node is a group-node, also check if PagedLOD (leaf nodes?), then not possible to attach 
				{
					//Create/clone terrain geometry
					osg::Group* terrain_geometry = CreateTerrainGeometry(root_node);
					osg::ref_ptr<osg::Node> veg_node = iter->second.CreateNode(terrain_geometry);
					root_node->addChild(veg_node);
				}
			}
			return rr;
		}
	protected:
		virtual ~PLODTerrainTileInjection() {}
		std::map<int, BillboardNodeGenerator> m_LODLayers;
	};
}