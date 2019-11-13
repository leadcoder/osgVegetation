#pragma once
#include "ov_VPBVegetationInjection.h"

namespace osgVegetation
{
	class CustomVegetationInjection : public VPBVegetationInjection
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

		class CopyGeodesToGroup : public osg::NodeVisitor
		{
		private:
			osg::Group* m_RootNode;
		public:
			CopyGeodesToGroup(osg::Group* root) : m_RootNode(root),
				osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) {
			}

			void apply(osg::Geode& geode)
			{
				if (osg::Geode* geode_copy = dynamic_cast<osg::Geode*>(geode.clone(osg::CopyOp::DEEP_COPY_ALL)))
				{
					//reset node mask
					geode_copy->setNodeMask(~0);
					m_RootNode->addChild(geode_copy);
				}
			}
		};

		CustomVegetationInjection(const VPBVegetationInjectionConfig &config) : VPBVegetationInjection(config)
		{

		}

		static osg::Group* CreateTerrainGeometry(osg::Node* root_node)
		{
			//Terrain geometry is under plods
			std::vector<osg::PagedLOD*> plods = PLODTerrainHelper::GetPagedLODNodes(root_node);

			//We know that we have no transforms at this level, (MeshLayers don't respect terrain transformation)
			osg::Group* terrain_geometry = new osg::Group();
			for (size_t i = 0; i < plods.size(); i++)
			{
				CopyGeodesToGroup tt_visitor(terrain_geometry);
				plods[i]->accept(tt_visitor);
			}

			//Prepare for tesselation shaders, change PrimitiveSet draw mode to GL_PATCHES
			ConvertToPatchesVisitor cv;
			terrain_geometry->accept(cv);

			return terrain_geometry;
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