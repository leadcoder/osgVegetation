#pragma once
#include "ov_Utils.h"
#include "ov_TerrainHelper.h"
#include "ov_VPBVegetationInjectionConfig.h"
#include "ov_LayerGenerator.h"
#include <osg/PagedLOD>
#include <osgTerrain/Terrain>
#include <osgTerrain/TerrainTile>
#include <osgDB/WriteFile>

namespace osgVegetation
{
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
	class VPBVegetationInjection  : public osgDB::ReadFileCallback
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

		//Vistor that create geometry from found terrain tiles and add them to provided root group
		class ApplyPseudoLoader : public osg::NodeVisitor
		{
		private:
			std::string m_PseudoLoaderExt;
		public:
			ApplyPseudoLoader(const std::string ext) : m_PseudoLoaderExt(ext),
				osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) {
			}

			void apply(osg::PagedLOD& node)
			{
				for (size_t i = 0; i < node.getNumFileNames(); i++)
				{
					const std::string name = node.getFileName(i);
					if(name != "")
						node.setFileName(i, name + "." + m_PseudoLoaderExt);
				}
			}
		};
	
		VPBVegetationInjection(const VPBVegetationInjectionConfig &config)
		{
			for (size_t i = 0; i < config.TerrainLODs.size(); i++)
			{
				m_Levels.push_back(VPBInjectionLOD(config.TerrainLODs[i]));
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

		osg::Group* CreateTerrainGeometry(osg::Node* terrain) const
		{
			osg::Group* terrain_geometry = new osg::Group();

			//TODO: find out what type of terrain --POLYGONAL or --TERRAIN
			TerrainTilesToGeometryVisitor tt_visitor(terrain_geometry);
			terrain->accept(tt_visitor);

			//osgDB::writeNodeFile(*terrain_geometry,"c:/temp/ovt_tile.osg");
			//CopyTerrainGeodeToVisitor copy_visitor(terrain_geometry);
			//terrain->accept(copy_visitor);

			//Prepare for tesselation shaders, change PrimitiveSet draw mode to GL_PATCHES
			ConvertToPatchesVisitor cv;
			terrain_geometry->accept(cv);
			return terrain_geometry;
		}

//#define OV_USE_TILE_ID_LOD_LEVEL

		VPBInjectionLOD* GetTargetLevel(int level) 
		{
			for (size_t i = 0; i < m_Levels.size(); i++)
			{
				if (m_Levels[i].TargetLevel == level)
					return &m_Levels[i];
			}
			return NULL;
		}

		virtual osgDB::ReaderWriter::ReadResult readNode(
			const std::string& filename,
			const osgDB::ReaderWriter::Options* options)
		{
			const bool use_pseudo_loader = m_PseudoLoaderExt != "" ? true : false;
			
			osgDB::ReaderWriter::ReadResult rr = use_pseudo_loader ? osgDB::readNodeFile(filename, options) : ReadFileCallback::readNode(filename, options);
		
			if (!rr.getNode())
				return rr;

			if (!rr.validNode())
				return rr;

			if (use_pseudo_loader)
			{
				ApplyPseudoLoader pseudo_loader_visitor(m_PseudoLoaderExt);
				rr.getNode()->accept(pseudo_loader_visitor);
			}

			//disable terrain self shadowing
			TerrainNodeMaskVisitor mask_visitor(~Register.Scene.Shadow.CastsShadowTraversalMask);
			rr.getNode()->accept(mask_visitor);

#ifdef OV_USE_TILE_ID_LOD_LEVEL
			const int lod_level = ttv.Tiles.size() > 0 ? ttv.Tiles[0]->getTileID().level - 1 : 0;
#else
			const int lod_level = ExtractLODLevelFromFileName(filename);
#endif
			VPBInjectionLOD* injector = GetTargetLevel(lod_level);
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

		void SetPseudoLoaderExt(const std::string& ext)
		{
			m_PseudoLoaderExt = ext;
		}

		void SetTerrainStateSet(osg::ref_ptr <osg::StateSet> state_set)
		{
			m_TerrainStateSet = state_set;
		}
	protected:
		virtual ~VPBVegetationInjection() {}
		std::vector<VPBInjectionLOD> m_Levels;
		std::string m_PseudoLoaderExt;
		osg::ref_ptr <osg::StateSet> m_TerrainStateSet;
	};

	class CustomTerrain : public VPBVegetationInjection
	{
	public:
		//Find the group holding the terrain geometry/geode 
		class PLODTerrainHelper
		{
		private:
			class PLODVisitor : public osg::NodeVisitor
			{
			public:
				std::vector<osg::PagedLOD*> PLODVec;
				PLODVisitor() : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) {	}
				void apply(osg::PagedLOD& plod) {
					PLODVec.push_back(&plod);
				}
			};

			class GroupVisitor : public osg::NodeVisitor
			{
			public:
				std::vector<osg::Group*> Groups;
				GroupVisitor() : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) {}
				void apply(osg::PagedLOD& node) { traverse(node); }
				void apply(osg::LOD& node) { traverse(node); }
				void apply(osg::Geode& node) {}
				void apply(osg::Geometry& node) {}
				void apply(osg::Group& node) { Groups.push_back(&node); }
			};
		public:
			//Get all paged lods;
			static std::vector<osg::PagedLOD*> GetPagedLODNodes(osg::Node* node)
			{
				PLODVisitor plod_vist;
				node->accept(plod_vist);
				return plod_vist.PLODVec;
			}

			static std::vector<osg::Group*> GetGroups(osg::Node* node)
			{
				PLODTerrainHelper::GroupVisitor gv;
				node->accept(gv);
				return gv.Groups;
			}
		};

		CustomTerrain(const VPBVegetationInjectionConfig &config) : VPBVegetationInjection(config)
		{

		}

		static void ApplyTerrainStateSet(osg::ref_ptr <osg::StateSet> state_set, osg::Node* root_node)
		{
			//terrain is under plod node
			std::vector<osg::PagedLOD*> plods = PLODTerrainHelper::GetPagedLODNodes(root_node);
			for (size_t i = 0; i < plods.size(); i++)
			{
				//get all terrain tiles under this plod, placed in group
				std::vector<osg::Group*> groups = PLODTerrainHelper::GetGroups(plods[i]);
				for (size_t j = 0; j < groups.size(); j++)
				{
					groups[j]->getOrCreateStateSet()->merge(*state_set);
				}
			}
		}

		static void ApplyNodeMaskToObjects(int node_mask, osg::Node* root_node)
		{
			//Objects are under root node
			if (osg::Group* group_root = dynamic_cast<osg::Group*>(root_node))
			{
				for (size_t i = 0; i < group_root->getNumChildren(); i++)
				{
					osg::Node* child = group_root->getChild(i);
					std::vector<osg::Group*> groups = PLODTerrainHelper::GetGroups(child);
					for (size_t j = 0; j < groups.size(); j++)
					{
						groups[j]->setNodeMask(node_mask);
					}
				}
			}
		}

		osgDB::ReaderWriter::ReadResult readNode(
			const std::string& filename,
			const osgDB::ReaderWriter::Options* options)
		{
			const bool use_pseudo_loader = m_PseudoLoaderExt != "" ? true : false;
			osgDB::ReaderWriter::ReadResult rr = use_pseudo_loader ? osgDB::readNodeFile(filename, options) : ReadFileCallback::readNode(filename, options);
			if (!rr.getNode())
				return rr;

			if (!rr.validNode())
				return rr;

			if (use_pseudo_loader)
			{
				ApplyPseudoLoader pseudo_loader_visitor(m_PseudoLoaderExt);
				rr.getNode()->accept(pseudo_loader_visitor);
			}

			const int lod_level = ExtractLODLevelFromFileName(filename);

			if (lod_level >= 0)
				ApplyNodeMaskToObjects(0x1 | Register.Scene.Shadow.ReceivesShadowTraversalMask, rr.getNode());

			if (m_TerrainStateSet && lod_level >= 0)
			{
				ApplyTerrainStateSet(m_TerrainStateSet, rr.getNode());
			}

			VPBInjectionLOD* injector = GetTargetLevel(lod_level);
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

					if (m_TerrainStateSet)
					{
						veg_node->setStateSet(m_TerrainStateSet);
					}
				}
			}
			return rr;
		}
	};
}