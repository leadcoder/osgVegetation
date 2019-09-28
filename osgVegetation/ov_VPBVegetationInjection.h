#pragma once
#include "ov_BillboardLayerConfig.h"
#include "ov_Utils.h"
#include "ov_TerrainHelper.h"
#include "ov_BillboardLayerStateSet.h"
#include "ov_VPBVegetationInjectionConfig.h"
#include "ov_BillboardMultiLayerEffect.h"
#include "ov_MeshLayerGenerator.h"
#include <osg/PagedLOD>
#include <osgTerrain/Terrain>
#include <osgTerrain/TerrainTile>

namespace osgVegetation
{

#if 0
	class BillboardNodeGeneratorConfig
	{
	public:
		BillboardNodeGeneratorConfig(const std::vector<BillboardLayer> &layers = std::vector<BillboardLayer>(),
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
				if (m_Config.Layers[i].Type == BillboardLayerConfig::BLT_GRASS)
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
#endif

	class MeshMultiLayerGenerator
	{
	public:
		MeshMultiLayerGenerator(std::vector<MeshLayerConfig> layers)
		{
			for (size_t i = 0; i < layers.size(); i++)
			{
				m_MeshGenerators.push_back(MeshLayerGenerator(layers[i]));
			}
		}

		osg::ref_ptr<osg::Group> CreateMeshNode(osg::ref_ptr<osg::Node> terrain_geometry)
		{
			osg::ref_ptr<osg::Group> root = new osg::Group();
			for (size_t i = 0; i < m_MeshGenerators.size(); i++)
			{
				osg::Group* mesh_node = m_MeshGenerators[i].CreateMeshNode(terrain_geometry);
				root->addChild(mesh_node);
			}
			return root;
		}
	private:
		std::vector<MeshLayerGenerator> m_MeshGenerators;
	};

	class LayerGenerator
	{
	public:
		LayerGenerator(std::vector<osg::ref_ptr<ILayerConfig>> layers)
		{
			//Sort layers
			std::vector<MeshLayerConfig> mesh_layers;
			std::vector<BillboardLayerConfig> billboard_layers;
			for (size_t i = 0; i < layers.size(); i++)
			{
				if (MeshLayerConfig* mesh_layer = dynamic_cast<MeshLayerConfig*>(layers[i].get()))
					mesh_layers.push_back(*mesh_layer);
				if (BillboardLayerConfig* bb_layer = dynamic_cast<BillboardLayerConfig*>(layers[i].get()))
					billboard_layers.push_back(*bb_layer);
			}
			m_BBEffect = new BillboardMultiLayerEffect(billboard_layers);
			m_MeshGenerator = new MeshMultiLayerGenerator(mesh_layers);
		}

		osg::ref_ptr<osg::Group> CreateVegetationNode(osg::ref_ptr<osg::Node> terrain_geometry)
		{
			osg::ref_ptr<osg::Group> root = new osg::Group();
			osg::ref_ptr<BillboardMultiLayerEffect> bb_layers = m_BBEffect->createInstance(terrain_geometry);
			root->addChild(bb_layers);
			osg::ref_ptr<osg::Group> mesh_layers = m_MeshGenerator->CreateMeshNode(terrain_geometry);
			root->addChild(mesh_layers);
			return root;
		}
		int TargetLevel;
	private:
		osg::ref_ptr<BillboardMultiLayerEffect> m_BBEffect;
		MeshMultiLayerGenerator* m_MeshGenerator;
	};


	class VPBInjectionLOD
	{
	public:
		VPBInjectionLOD(VPBInjectionLODConfig config) :
			TargetLevel(config.TargetLevel),
			m_Generator(config.Layers)
		{
		}

		osg::ref_ptr<osg::Group> CreateVegetationNode(osg::ref_ptr<osg::Node> terrain_geometry)
		{
			return m_Generator.CreateVegetationNode(terrain_geometry);
		}
		int TargetLevel;
	private:
		LayerGenerator m_Generator;
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
	
		VPBVegetationInjection(const VPBVegetationInjectionConfig &config)
		{
			for (size_t i = 0; i < config.TerrainLODs.size(); i++)
			{
				m_Levels.push_back(VPBInjectionLOD(config.TerrainLODs[i]));
				//m_LODLayers.insert(std::pair<int, osg::ref_ptr<BillboardMultiLayerEffect> >(config.TerrainLODs[i].TargetLevel, new BillboardMultiLayerEffect(config.TerrainLODs[i].Layers, config.BillboardTexUnit)));
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
			else //Trian tile
			{
				int x1, x2, y1, y2;
				int lod = -1;
				int num_args = sscanf(clean_filename.c_str(), "tile_%dx%d_%d_%dx%d", &x1, &y1, &lod, &x2, &y2);
				if (num_args == 5)
					lod_level = lod;
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
			VPBInjectionLOD* injector = GetTargetLevel(lod_level);
			//std::map<int, osg::ref_ptr<BillboardMultiLayerEffect> >::const_iterator iter = m_LODLayers.find(lod_level);
			//if (iter != m_LODLayers.end())
			if (injector)
			{
				osg::Group* root_node = dynamic_cast<osg::Group*>(rr.getNode());
				osg::PagedLOD* plod = dynamic_cast<osg::PagedLOD*>(root_node);
				//check that root node is a group-node, also check if PagedLOD (leaf nodes?), then not possible to attach 
				if (root_node && plod == NULL) 
				{
					//Create/clone terrain geometry
					osg::Group* terrain_geometry = CreateTerrainGeometry(root_node);
					osg::ref_ptr<osg::Node> veg_node = injector->CreateVegetationNode(terrain_geometry);
					root_node->addChild(veg_node);
				}
			}
			return rr;
		}
		VPBInjectionLOD* GetTargetLevel(int level)
		{
			for (size_t i = 0; i < m_Levels.size(); i++)
			{
				if (m_Levels[i].TargetLevel == level)
					return &m_Levels[i];
			}
			return NULL;
		}
	protected:
		virtual ~VPBVegetationInjection() {}
		std::vector<VPBInjectionLOD> m_Levels;
		//std::map<int, osg::ref_ptr<BillboardMultiLayerEffect> > m_LODLayers;
	};
}