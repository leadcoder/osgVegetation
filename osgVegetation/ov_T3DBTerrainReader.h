#pragma once
#include "ov_Utils.h"
#include "ov_TerrainHelper.h"
#include "ov_LayerGenerator.h"
#include "ov_TerrainLODGenerator.h"
#include <osg/PagedLOD>
#include <osgDB/WriteFile>
#include <osgDB/FileNameUtils>


#define OV_T3DB_EXT "ov_t3db"
namespace osgVegetation
{
	class T3DBTerrainReaderConfig
	{
	public:
		T3DBTerrainReaderConfig(): TerrainCastShadow(false), 
			TerrainReceiveShadow(true),
			ObjectsCastShadow(true),
			ObjectsReceiveShadow(false) {}
		T3DBTerrainReaderConfig(const std::vector<TerrainLODGeneratorConfig>& lods) : TerrainCastShadow(false),
			TerrainReceiveShadow(true),
			ObjectsCastShadow(true),
			ObjectsReceiveShadow(false),
			TerrainLODs(lods)
		{}
		bool TerrainCastShadow;
		bool TerrainReceiveShadow;
		bool ObjectsCastShadow;
		bool ObjectsReceiveShadow;
		osg::ref_ptr<osg::StateSet> TerrainStateSet;
		std::vector<TerrainLODGeneratorConfig> TerrainLODs;
		static T3DBTerrainReaderConfig ReadXML(osgDB::XmlNode* vpb_node)
		{
			T3DBTerrainReaderConfig config;
			config.TerrainStateSet = loadTerrainStateSet(vpb_node);

			QueryBoolAttribute(vpb_node, "TerrainCastShadow", config.TerrainCastShadow);
			QueryBoolAttribute(vpb_node, "TerrainReceiveShadow", config.TerrainReceiveShadow);
			QueryBoolAttribute(vpb_node, "ObjectsCastShadow", config.ObjectsCastShadow);
			QueryBoolAttribute(vpb_node, "ObjectsReceiveShadow", config.ObjectsReceiveShadow);

			for (unsigned int i = 0; i < vpb_node->children.size(); ++i)
			{
				if (vpb_node->children[i]->name == "LODGenerator")
				{
					TerrainLODGeneratorConfig lod_config = TerrainLODGeneratorConfig::ReadXML(vpb_node->children[i].get());
					config.TerrainLODs.push_back(lod_config);
				}
			}
			return config;
		}
	};

	class T3DBTerrainReader : public osgDB::ReaderWriter
	{
	private:
		mutable std::vector<TerrainLODGenerator> m_Levels;
		osg::ref_ptr<osg::StateSet> m_TerrainStateSet;
		bool m_TerrainCastShadow;
		bool m_TerrainReceiveShadow;
		     
		bool m_ObjectsCastShadow;
		bool m_ObjectsReceiveShadow;
	public:
		T3DBTerrainReader(const T3DBTerrainReaderConfig&config) :
			m_TerrainCastShadow(config.TerrainCastShadow),
			m_TerrainReceiveShadow(config.TerrainReceiveShadow),
			m_ObjectsCastShadow(config.ObjectsCastShadow),
			m_ObjectsReceiveShadow(config.ObjectsReceiveShadow),
			m_TerrainStateSet(config.TerrainStateSet)
		{
			supportsExtension(OV_T3DB_EXT, "T3DBTerrainReader");
		
			for (size_t i = 0; i < config.TerrainLODs.size(); i++)
			{
				m_Levels.push_back(TerrainLODGenerator(config.TerrainLODs[i]));
			}
		}

		virtual const char* className() const
		{
			// Return a description of this class
			return "PagedLod File Reader";
		}

		virtual bool acceptsExtension(const std::string& extension) const
		{
			return osgDB::equalCaseInsensitive(extension, OV_T3DB_EXT);
		}
	
		virtual ReadResult readNode(const std::string& file,
			const osgDB::Options* options) const
		{

			if (!acceptsExtension(osgDB::getFileExtension(file)))
				return ReadResult::FILE_NOT_HANDLED;

			//const std::string ext = osgDB::getFileExtension(file);
			//remove pseudo extension
			const std::string filename = osgDB::getNameLessExtension(file);
			osgDB::ReaderWriter::ReadResult rr = osgDB::readNodeFile(filename, options);

			if (!rr.getNode())
				return rr;

			if (!rr.validNode())
				return rr;

			TerrainHelper::AddExtToPLOD(rr.getNode(), OV_T3DB_EXT);
			
			const int lod_level = GetLODLevelFromFileName(filename);

			//if (lod_level >= 0)
			{
				CollectGeodes collection;
				rr.getNode()->accept(collection);

				if(!m_TerrainCastShadow)
					ApplyNodeMask(~Register.CastsShadowTraversalMask, collection.TerrainTiles);
				if (!m_ObjectsCastShadow)
					ApplyNodeMask(~Register.CastsShadowTraversalMask, collection.Objects);

				if (!m_ObjectsReceiveShadow)
				{
					for (size_t i = 0; i < collection.Objects.size(); i++)
					{
						collection.Objects[i]->getOrCreateStateSet()->addUniform(new osg::Uniform("ov_receive_shadow", m_ObjectsReceiveShadow));
					}
				}

				if (!m_TerrainReceiveShadow)
				{
					for (size_t i = 0; i < collection.TerrainTiles.size(); i++)
					{
						collection.TerrainTiles[i]->getOrCreateStateSet()->addUniform(new osg::Uniform("ov_receive_shadow", m_ObjectsReceiveShadow));
					}
				}
			
				if (m_TerrainStateSet)
				{
					ApplyTerrainStateSet(m_TerrainStateSet, collection.TerrainTiles);
				}

				TerrainLODGenerator* injector = GetTargetLevel(lod_level);
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

		static osg::ref_ptr<T3DBTerrainReader> ReadXML(osgDB::XmlNode* node)
		{
			T3DBTerrainReaderConfig config = T3DBTerrainReaderConfig::ReadXML(node);
			return new T3DBTerrainReader(config);
		}

	private:

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

		TerrainLODGenerator* GetTargetLevel(int level) const
		{
			for (size_t i = 0; i < m_Levels.size(); i++)
			{
				if (m_Levels[i].TargetLevel == level)
					return &m_Levels[i];
			}
			return NULL;
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

		static void ApplyTerrainStateSet(osg::ref_ptr <osg::StateSet> state_set, const std::vector<osg::Geode*>& geodes)
		{
			for (size_t i = 0; i < geodes.size(); i++)
			{
				ApplyTerrainStateSet(state_set, geodes[i]);
			}
		}

		static void ApplyNodeMask(unsigned int node_mask, const std::vector<osg::Geode*>& nodes)
		{
			for (size_t i = 0; i < nodes.size(); i++)
			{
				nodes[i]->setNodeMask(nodes[i]->getNodeMask() & node_mask);
			}
		}

		static osg::Group* CreateTerrainGeometry(osg::Node* root_node, const std::vector<osg::Geode*>& geodes)
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
		}; //end CollectGeodes
	};
}