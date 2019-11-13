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

		class InjectionVisitor : public osg::NodeVisitor
		{
		public:
			VPBInjectionLOD* m_Injector;
			osg::Group* m_VegetationRoot;

			InjectionVisitor(VPBInjectionLOD* injector) : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
				m_Injector(injector),
				m_VegetationRoot(new osg::Group){}
		
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
							osg::ref_ptr<osg::Group> veg_node = m_Injector->CreateVegetationNode(terrain_tile->getChild(0));
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
					osg::ref_ptr<osg::Group> veg_node = m_Injector->CreateVegetationNode(geode_copy);
					new_matrix->addChild(veg_node);
					m_VegetationRoot->addChild(new_matrix);
				}
			}
		};

		//Vistor that apply sufix to PagedLOD file list, 
		//ie we want all files to be catched by pseudo loader
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
			return lod_level;
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
				//TODO: if plod inject group node above
				if (root_node && plod == NULL)
				{
					InjectionVisitor injector_visitor(injector);
					root_node->accept(injector_visitor);
					root_node->addChild(injector_visitor.m_VegetationRoot);
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
}