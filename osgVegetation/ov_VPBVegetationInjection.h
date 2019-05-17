#pragma once
#include "ov_Terrain.h"
#include "ov_BillboardLayer.h"
#include "ov_Utils.h"
#include "ov_TerrainHelper.h"
#include "ov_BillboardLayerStateSet.h"
#include <osg/PagedLOD>
#include <osgTerrain/Terrain>
#include <osgTerrain/TerrainTile>

namespace osgVegetation
{

	class BillboardNodeGeneratorConfig
	{
	public:
		BillboardNodeGeneratorConfig(const std::vector<BillboardLayer> &layers,
			//TerrainTextureUnitSettings terrrain_texture_units = TerrainTextureUnitSettings(),
			int billboard_texture_unit = 2,
			int receives_shadow_mask = 0x1,
			int cast_shadow_mask = 0x2) : Layers(layers),
			//TerrainTextureUnits(terrrain_texture_units),
			BillboardTexUnit(billboard_texture_unit),
			ReceivesShadowTraversalMask(receives_shadow_mask),
			CastShadowTraversalMask(cast_shadow_mask)
		{}
		std::vector<BillboardLayer> Layers;
		//TerrainTextureUnitSettings TerrainTextureUnits;
		int BillboardTexUnit;
		int ReceivesShadowTraversalMask;
		int CastShadowTraversalMask;
	};

	class BillboardNodeGenerator
	{
	public:
		BillboardNodeGenerator(const BillboardNodeGeneratorConfig &config, osg::ref_ptr <osg::StateSet> terrain_state_set) : m_Config(config),
			m_TerrainStateSet(terrain_state_set)
		{
			for (size_t i = 0; i < config.Layers.size(); i++)
			{
				m_Layers.push_back(new BillboardLayerStateSet(config.Layers[i], config.BillboardTexUnit));
			}
		}

		osg::ref_ptr<osg::Node> CreateNode(osg::Node* terrain) const
		{
			osg::ref_ptr<osg::Group> layers = new osg::Group();
			if (m_TerrainStateSet)
				layers->getOrCreateStateSet()->merge(*m_TerrainStateSet);

			for (size_t i = 0; i < m_Layers.size(); i++)
			{
				osg::ref_ptr<osg::Group> layer_node = new osg::Group();
				layer_node->setStateSet(m_Layers[i]);
				layer_node->addChild(terrain);

				//Disable shadow casting for grass, TODO make this optional
				if (m_Config.Layers[i].Type == BillboardLayer::BLT_GRASS)
					layer_node->setNodeMask(m_Config.ReceivesShadowTraversalMask);
				else
					layer_node->setNodeMask(m_Config.ReceivesShadowTraversalMask | m_Config.CastShadowTraversalMask);

				layers->addChild(layer_node);
			}
			return layers;
		}
	private:
		std::vector<osg::ref_ptr<BillboardLayerStateSet> > m_Layers;
		BillboardNodeGeneratorConfig m_Config;
		osg::ref_ptr <osg::StateSet> m_TerrainStateSet;
	};

	class BillboardMultiLayerEffect : public osg::Group
	{
	public:
		BillboardMultiLayerEffect(const std::vector<BillboardLayer> &layers, int tex_unit)
		{
			for (size_t i = 0; i < layers.size(); i++)
			{
				osg::ref_ptr<BillboardLayerEffect> layer = new BillboardLayerEffect(layers[i], tex_unit);
				layer->setNodeMask(0x1);
				//layer_node->setNodeMask(0x1 | m_Config.CastShadowTraversalMask);
				addChild(layer);
			}
		}

		BillboardMultiLayerEffect(const BillboardMultiLayerEffect& rhs, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY) : osg::Group(rhs, copyop)
		{

		}

		virtual Object* cloneType() const { return new Group(); }
		virtual Object* clone(const osg::CopyOp& copyop) const { return new BillboardMultiLayerEffect(*this, copyop); }

		osg::ref_ptr<BillboardMultiLayerEffect> createInstance(osg::ref_ptr<osg::Node> terrain_geometry) const
		{
			osg::ref_ptr<BillboardMultiLayerEffect>  effect = dynamic_cast<BillboardMultiLayerEffect*>(clone(osg::CopyOp::DEEP_COPY_NODES));
			effect->insertTerrain(terrain_geometry);
			return effect;
		}
	
		void insertTerrain(osg::ref_ptr<osg::Node> terrain_geometry)
		{
			for (unsigned int i = 0; i < getNumChildren(); i++)
			{
				getChild(i)->asGroup()->addChild(terrain_geometry);
			}
		}
	};



	//Inject vegetation layers into VirtualPlanetBuilder (VPB) PLOD terrains,
	//ie terrain created with --PagedLOD, --POLYGONAL or --TERRAIN.
	//Both flat and geocentric terrains should work.
	class VPBVegetationInjection : public osgDB::ReadFileCallback
	{
	public:
		class TerrainNodeMaskVisitor : public osg::NodeVisitor
		{
		private:
			int m_NodeMask;
		public:
			TerrainNodeMaskVisitor(int node_mask) : m_NodeMask(node_mask),
				osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) {
			}

			void apply(osg::Node& node)
			{
				osgTerrain::TerrainTile* tile = dynamic_cast<osgTerrain::TerrainTile*>(&node);
				if (tile)
				{
					node.setNodeMask(m_NodeMask);
				}
				else
				{
					traverse(node);
				}
			}

			void apply(osg::Geode& geode)
			{
				geode.setNodeMask(m_NodeMask);
			}
		};

		//Vistor that create geometry from found terrain tiles and add them to provided root group
		class TerrainTilesToGeometryVisitor : public osg::NodeVisitor
		{
		private:
			osg::Group* m_RootNode;
		public:
			TerrainTilesToGeometryVisitor(osg::Group* root) : m_RootNode(root),
				osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) {
			}

			void apply(osg::Node& node)
			{
				osgTerrain::TerrainTile* tile = dynamic_cast<osgTerrain::TerrainTile*>(&node);
				if (tile)
				{
					if (osg::Node* terrain_tile = TerrainHelper::CreateTerrainNodeFromTerrainTile(tile))
						m_RootNode->addChild(terrain_tile);
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
					//reset node mask
					geode_copy->setNodeMask(~0);
					osg::MatrixTransform* new_matrix = new osg::MatrixTransform();
					osg::NodePath nodePath = getNodePath();
					new_matrix->setMatrix(osg::computeLocalToWorld(nodePath));
					new_matrix->addChild(geode_copy);
					m_RootNode->addChild(new_matrix);
				}
			}
		};

		class CopyTerrainGeodeToVisitor : public osg::NodeVisitor
		{
		private:
			osg::Group* m_RootNode;
		public:
			CopyTerrainGeodeToVisitor(osg::Group* root) : m_RootNode(root),
				osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) {
			}

			void apply(osg::Geode& geode)
			{
				if (osg::Geode* geode_copy = dynamic_cast<osg::Geode*>(geode.clone(osg::CopyOp::DEEP_COPY_ALL)))
				{
					osg::MatrixTransform* new_matrix = new osg::MatrixTransform();
					osg::NodePath nodePath = getNodePath();
					new_matrix->setMatrix(osg::computeLocalToWorld(nodePath));
					new_matrix->addChild(geode_copy);
					m_RootNode->addChild(new_matrix);
				}
			}
		};
	
		VPBVegetationInjection(const BillboardNodeGeneratorConfig &config)
		{
			typedef std::map<int, std::vector<BillboardLayer> > LODBuckets;
			LODBuckets lod_buckets;
			for (size_t i = 0; i < config.Layers.size(); i++)
			{
				lod_buckets[config.Layers[i].LODLevel].push_back(config.Layers[i]);
			}

			for (LODBuckets::const_iterator iter = lod_buckets.begin(); iter != lod_buckets.end(); iter++)
			{
				BillboardNodeGeneratorConfig lod_config = config;
				lod_config.Layers = iter->second;
				m_LODLayers.insert(std::pair<int, osg::ref_ptr<BillboardMultiLayerEffect> >(iter->first, new BillboardMultiLayerEffect(lod_config.Layers, lod_config.BillboardTexUnit)));
			}
		}
	
		static int ExtractLODLevelFromFileName(const std::string& filename)
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

		osg::Group* CreateTerrainGeometry(osg::Node* terrain)
		{
			osg::Group* terrain_geometry = new osg::Group();

			//TODO: find out what type of terrain --POLYGONAL or --TERRAIN
			TerrainTilesToGeometryVisitor tt_visitor(terrain_geometry);
			terrain->accept(tt_visitor);

			CopyTerrainGeodeToVisitor copy_visitor(terrain_geometry);
			terrain->accept(copy_visitor);

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

			//disable terrain self shadowing
			TerrainNodeMaskVisitor mask_visitor(0x1);
			rr.getNode()->accept(mask_visitor);

#ifdef OV_USE_TILE_ID_LOD_LEVEL
			const int lod_level = ttv.Tiles.size() > 0 ? ttv.Tiles[0]->getTileID().level - 1 : 0;
#else
			const int lod_level = ExtractLODLevelFromFileName(filename);
#endif
			std::map<int, osg::ref_ptr<BillboardMultiLayerEffect> >::const_iterator iter = m_LODLayers.find(lod_level);
			if (iter != m_LODLayers.end())
			{
				osg::Group* root_node = dynamic_cast<osg::Group*>(rr.getNode());
				osg::PagedLOD* plod = dynamic_cast<osg::PagedLOD*>(root_node);
				//check that root node is a group-node, also check if PagedLOD (leaf nodes?), then not possible to attach 
				if (root_node && plod == NULL) 
				{
					//Create/clone terrain geometry
					osg::Group* terrain_geometry = CreateTerrainGeometry(root_node);
					osg::ref_ptr<BillboardMultiLayerEffect> veg_node = iter->second->createInstance(terrain_geometry);
					root_node->addChild(veg_node);
				}
			}
			return rr;
		}
	protected:
		virtual ~VPBVegetationInjection() {}
		std::map<int, osg::ref_ptr<BillboardMultiLayerEffect> > m_LODLayers;
	};
}