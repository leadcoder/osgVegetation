#pragma once
#include "ov_Utils.h"
#include "ov_TerrainHelper.h"
#include "ov_LayerGenerator.h"
#include "ov_TerrainLODGenerator.h"
#include "ov_XMLUtils.h"
#include "ov_XMLTerrainShadingReader.h"

#include <osg/PagedLOD>
#include <osgTerrain/Terrain>
#include <osgTerrain/TerrainTile>
#include <osgDB/WriteFile>
#include <osgDB/FileNameUtils>

#define OV_VPB_EXT "ov_vpb"
namespace osgVegetation
{
	class VPBTerrainReaderConfig
	{
	public:
		VPBTerrainReaderConfig() {}
		VPBTerrainReaderConfig(const std::vector<TerrainLODGeneratorConfig>& lods) : TerrainLODs(lods)
		{}
		bool ObjectsCastShadow;
		bool TerrainCastShadow;
		osg::ref_ptr<osg::StateSet> TerrainStateSet;
		std::vector<TerrainLODGeneratorConfig> TerrainLODs;

		static VPBTerrainReaderConfig ReadXML(osgDB::XmlNode* vpb_node)
		{
			VPBTerrainReaderConfig config;
			config.TerrainStateSet = loadTerrainStateSet(vpb_node);

			QueryBoolAttribute(vpb_node, "TerrainCastShadow", config.TerrainCastShadow);
			QueryBoolAttribute(vpb_node, "ObjectsCastShadow", config.ObjectsCastShadow);

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

	//Inject vegetation layers into VirtualPlanetBuilder (VPB) PLOD terrains,
	//ie terrain created with --PagedLOD, --POLYGONAL or --TERRAIN.
	//Both flat and geocentric terrains should work.
	class VPBTerrainReader : public osgDB::ReaderWriter
	{
	private:
		mutable std::vector<TerrainLODGenerator> m_Levels;
		osg::ref_ptr<osg::StateSet> m_TerrainStateSet;
		bool m_TerrainCastShadow;
		virtual ~VPBTerrainReader() {}
	public:
		VPBTerrainReader(const VPBTerrainReaderConfig& config) : m_TerrainCastShadow(config.TerrainCastShadow),
			m_TerrainStateSet(config.TerrainStateSet)
		{
			supportsExtension(OV_VPB_EXT, "VPBTerrainReader");
			for (size_t i = 0; i < config.TerrainLODs.size(); i++)
			{
				m_Levels.push_back(TerrainLODGeneratorConfig(config.TerrainLODs[i]));
			}
		}

		virtual const char* className() const
		{
			// Return a description of this class
			return "PagedLod File Reader";
		}

		virtual bool acceptsExtension(const std::string& extension) const
		{
			return osgDB::equalCaseInsensitive(extension, OV_VPB_EXT);
		}

		virtual ReadResult readNode(const std::string& file,
			const osgDB::Options* options) const
		{
			if (!acceptsExtension(osgDB::getFileExtension(file)))
				return ReadResult::FILE_NOT_HANDLED;

			const std::string ext = osgDB::getFileExtension(file);

			//remove pseudo extension
			const std::string filename = osgDB::getNameLessExtension(file);
			osgDB::ReaderWriter::ReadResult rr = osgDB::readNodeFile(filename, options);

			if (!rr.getNode())
				return rr;

			if (!rr.validNode())
				return rr;

			TerrainHelper::AddExtToPLOD(rr.getNode(), OV_VPB_EXT);
			
			//disable terrain self shadowing
			if (!m_TerrainCastShadow)
			{
				TerrainNodeMaskVisitor mask_visitor(~Register.CastsShadowTraversalMask);
				rr.getNode()->accept(mask_visitor);
			}

			if (m_TerrainStateSet)
				rr.getNode()->setStateSet(m_TerrainStateSet);

#ifdef OV_USE_TILE_ID_LOD_LEVEL
			const int lod_level = ttv.Tiles.size() > 0 ? ttv.Tiles[0]->getTileID().level - 1 : 0;
#else
			const int lod_level = ExtractLODLevelFromFileName(filename);
#endif
			TerrainLODGenerator* generator = GetTargetLevel(lod_level);
			if (generator)
			{
				osg::Group* root_node = dynamic_cast<osg::Group*>(rr.getNode());
				osg::PagedLOD* plod = dynamic_cast<osg::PagedLOD*>(root_node);
				//check that root node is a group-node, also check if PagedLOD (leaf nodes?), then not possible to attach
				//TODO: if plod inject group node above
				if (root_node && plod == NULL)
				{
					root_node->addChild(VPBGeneratorVisitor::CreateVegetationNode(generator, root_node));
				}
			}
			return rr;
		}

		static osg::ref_ptr<VPBTerrainReader> ReadXML(osgDB::XmlNode* node)
		{
			VPBTerrainReaderConfig config = VPBTerrainReaderConfig::ReadXML(node);
			return new VPBTerrainReader(config);
		}

	private:
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

		TerrainLODGenerator* GetTargetLevel(int level) const
		{
			for (size_t i = 0; i < m_Levels.size(); i++)
			{
				if (m_Levels[i].TargetLevel == level)
					return &m_Levels[i];
			}
			return NULL;
		}

		//Visitor used to find terrain data and feed that to the generator that will create 
		//the final vegetation data.
		class VPBGeneratorVisitor : public osg::NodeVisitor
		{
		private:
			TerrainLODGenerator* m_Generator;
			osg::ref_ptr <osg::Group> m_VegetationRoot;
		public:
			VPBGeneratorVisitor(TerrainLODGenerator* generator, osg::ref_ptr<osg::Group> veg_group) : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
				m_Generator(generator),
				m_VegetationRoot(veg_group) {}

			void apply(osg::Node& node)
			{
				osgTerrain::TerrainTile* tile = dynamic_cast<osgTerrain::TerrainTile*>(&node);
				if (tile)
				{
					if (osg::MatrixTransform* terrain_tile = dynamic_cast<osg::MatrixTransform*>(TerrainHelper::CreateTerrainNodeFromTerrainTile(tile)))
					{
						if (terrain_tile->getNumChildren() > 0)
						{
							//insert vegetation below matrix transform, (MeshLayer don't respect transformed terrain)
							osg::ref_ptr<osg::Group> veg_node = m_Generator->CreateVegetationNode(terrain_tile->getChild(0));
							terrain_tile->removeChild(unsigned int(0));
							terrain_tile->addChild(veg_node);
							m_VegetationRoot->addChild(terrain_tile);
						}
					}
				}
				else
				{
					traverse(node);
				}
			}

			//If we have geode, assmue polygon terrain for now..
			void apply(osg::Geode& geode)
			{
				if (osg::Geode* geode_copy = dynamic_cast<osg::Geode*>(geode.clone(osg::CopyOp::DEEP_COPY_ALL)))
				{
					//reset node mask
					geode_copy->setNodeMask(~0);

					ConvertToPatchesVisitor cv;
					geode_copy->accept(cv);

					osg::MatrixTransform* new_matrix = new osg::MatrixTransform();
					osg::NodePath nodePath = getNodePath();
					new_matrix->setMatrix(osg::computeLocalToWorld(nodePath));
					osg::ref_ptr<osg::Group> veg_node = m_Generator->CreateVegetationNode(geode_copy);
					new_matrix->addChild(veg_node);
					m_VegetationRoot->addChild(new_matrix);
				}
			}

			static osg::ref_ptr<osg::Group> CreateVegetationNode(TerrainLODGenerator* generator, osg::ref_ptr<osg::Node> terrain)
			{
				osg::ref_ptr<osg::Group> veg_group = new osg::Group();
				VPBGeneratorVisitor injector_visitor(generator, veg_group);
				terrain->accept(injector_visitor);
				return veg_group;
			}
		}; //end VPBGeneratorVisitor

		class TerrainNodeMaskVisitor : public osg::NodeVisitor
		{
		private:
			unsigned int m_NodeMask;

		public:
			TerrainNodeMaskVisitor(unsigned int node_mask) : m_NodeMask(node_mask),
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
		}; //end TerrainNodeMaskVisitor

		
	}; 
}