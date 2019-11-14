#pragma once
#include "ov_VPBVegetationInjection.h"

namespace osgVegetation
{
	class CustomVegetationInjection : public VPBVegetationInjection
	{
	public:
		class CollectGeodes : public osg::NodeVisitor
		{
		public:
			std::vector<osg::Geode*> Objects;
			std::vector<osg::Geode*> TerrainTiles;
			CollectGeodes() : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) {}

			bool HasPLODParent(osg::Node* node)
			{
				if (node->getNumParents() > 0)
				{
					if (osg::PagedLOD* plod = dynamic_cast<osg::PagedLOD*>(node->getParent(0)))
					{
						return true;
					}
					else
					{
						return HasPLODParent(node->getParent(0));
					}
				}
				return false;
			}

			void apply(osg::Geode& node)
			{
				if (HasPLODParent(&node))
					TerrainTiles.push_back(&node);
				else
					Objects.push_back(&node);
			}
		};

		CustomVegetationInjection(const VPBVegetationInjectionConfig &config) : VPBVegetationInjection(config)
		{

		}

		static void ApplyTerrainStateSet(osg::ref_ptr <osg::StateSet> state_set, osg::Geode* geode)
		{
			//First check if parent is PagedLOD, if so we need to inject a group node between plod and geode that
			//can hold the terrain stateset. 
			//TODO: Test to change the geode copy (when we create the terrain for the vegetation)
			//to not include shaders avoid this extra node.
			if (osg::PagedLOD* plod = dynamic_cast<osg::PagedLOD*>(geode->getParent(0)))
			{
				osg::Group* group = new osg::Group();
				group->getOrCreateStateSet()->merge(*state_set);
				group->addChild(geode);
				//group->setNodeMask(~Register.Scene.Shadow.CastsShadowTraversalMask);
				//inject terrain stateset node
				plod->replaceChild(geode, group);
			}
			else if (osg::Group* group = dynamic_cast<osg::Group*>(geode->getParent(0)))
			{
				group->getOrCreateStateSet()->merge(*state_set);
				//group->setNodeMask(~Register.Scene.Shadow.CastsShadowTraversalMask);
			}
		}

		static void ApplyTerrainStateSet(osg::ref_ptr <osg::StateSet> state_set, const std::vector<osg::Geode*> &geodes)
		{
			for (size_t i = 0; i < geodes.size(); i++)
			{
				ApplyTerrainStateSet(state_set,geodes[i]);
			}
		}

		static void ApplyNodeMask(unsigned int node_mask, const std::vector<osg::Geode*> &nodes)
		{
			for (size_t i = 0; i < nodes.size(); i++)
			{
				nodes[i]->setNodeMask(nodes[i]->getNodeMask() & node_mask);
			}
		}

		static osg::Group* CreateTerrainGeometry(osg::Node* root_node, const std::vector<osg::Geode*> &geodes)
		{
			osg::Group* terrain_geometry = new osg::Group();
			for (size_t i = 0; i < geodes.size(); i++)
			{
				if (osg::Geode* geode_copy = dynamic_cast<osg::Geode*>(geodes[i]->clone(osg::CopyOp::DEEP_COPY_ALL)))
				{
					//reset node mask
					geode_copy->setNodeMask(~0);
					terrain_geometry->addChild(geode_copy);
				}
			}
			//Prepare for tesselation shaders, change PrimitiveSet draw mode to GL_PATCHES
			ConvertToPatchesVisitor cv;
			terrain_geometry->accept(cv);
			return terrain_geometry;
		}
	
		static int GetLODLevelFromFileName(const std::string& filename)
		{
			int lod_level = -1;
			std::string clean_filename = filename;
			std::size_t found = clean_filename.find_last_of("/\\");
			if (found != std::string::npos)
				clean_filename = clean_filename.substr(found + 1);
			int x1, x2, y1, y2;
			int lod = -1;
			int num_args = sscanf(clean_filename.c_str(), "tile_%dx%d_%d_%dx%d", &x1, &y1, &lod, &x2, &y2);
			if (num_args == 5)
				lod_level = lod;
			
			return lod_level;
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

			const int lod_level = GetLODLevelFromFileName(filename);

			//if (lod_level >= 0)
			{
				CollectGeodes collection;
				rr.getNode()->accept(collection);

				if(!m_TerrainCastShadow)
					ApplyNodeMask(~Register.Scene.Shadow.CastsShadowTraversalMask, collection.TerrainTiles);
				if (!m_ObjectsCastShadow)
					ApplyNodeMask(~Register.Scene.Shadow.CastsShadowTraversalMask, collection.Objects);
			
				if (m_TerrainStateSet)
				{
					ApplyTerrainStateSet(m_TerrainStateSet, collection.TerrainTiles);
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
						osg::Group* terrain_geometry = CreateTerrainGeometry(root_node, collection.TerrainTiles);
						osg::ref_ptr<osg::Node> veg_node = injector->CreateVegetationNode(terrain_geometry);
						root_node->addChild(veg_node);

						if (m_TerrainStateSet)
						{
							//We need to insert terrain stateate above vegetation to get 
							//hold of shaders, uniforms, defines etc. during vegetation rendering
							veg_node->setStateSet(m_TerrainStateSet);
						}
					}
				}
			}
			return rr;
		}
	};
}