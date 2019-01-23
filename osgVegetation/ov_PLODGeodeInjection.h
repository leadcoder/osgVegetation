#pragma once
#include "ov_Terrain.h"
#include "ov_BillboardLayer.h"
//#include "ov_BillboardTile.h"

namespace osgVegetation
{
	class GeodePLODInjection : public osgDB::ReadFileCallback
	{
	public:

		GeodePLODInjection(const Terrain &data) : m_Terrain(data)
		{

		}

		class GeodeVisitor : public osg::NodeVisitor
		{
		public:
			std::vector<osg::Geode*> Geodes;
			GeodeVisitor() :
				osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) {
			}

			void apply(osg::Node& node)
			{
				osg::Geode* geode = dynamic_cast<osg::Geode*>(&node);
				if (geode)
				{
					Geodes.push_back(geode);
				}
				else
				{
					traverse(node);
				}
			}
		};

		class PLODVisitor : public osg::NodeVisitor
		{
		public:
			std::vector<osg::PagedLOD*> PLODVec;
			PLODVisitor() :
				osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) {
			}

			void apply(osg::PagedLOD& plod)
			{
				PLODVec.push_back(&plod);
			}
		};

		int extractLODLevelFromFileName(const std::string& filename)
		{
			std::string clean_filename = filename;
			std::size_t found = clean_filename.find_last_of("/\\");
			if (found != std::string::npos)
				clean_filename = clean_filename.substr(found + 1);

			found = clean_filename.find("_L");
			int x1, x2, y1, y2;
			int lod_level = -1;
			sscanf(clean_filename.c_str(), "tile_%dx%d_%d_%dx%d", &x1, &y1, &lod_level, &x2, &y2);
			return lod_level;
		}

		std::vector<osg::Geode*> getTerrainGeodes(osg::Node* node)
		{
			//Get all paged lods;
			PLODVisitor plod_vist;
			node->accept(plod_vist);
			GeodeVisitor gv;
			for (size_t i = 0; i < plod_vist.PLODVec.size(); i++)
			{
				plod_vist.PLODVec[i]->accept(gv);
			}
			return gv.Geodes;
		}

		virtual osgDB::ReaderWriter::ReadResult readNode(const std::string& filename, const osgDB::Options* options)
		{
			osgDB::ReaderWriter::ReadResult rr = ReadFileCallback::readNode(filename, options);
			std::vector<osg::Geode*> geodes = getTerrainGeodes(rr.getNode());

			for (size_t j = 0; j < geodes.size(); j++)
			{
				//osgVegetation::PrepareTerrainForDetailMapping(geodes[j], m_Terrain);
			}

			const int lod_level = extractLODLevelFromFileName(filename);

			for (size_t i = 0; i < m_Terrain.BillboardLayers.size(); i++)
			{
				if (lod_level == m_Terrain.BillboardLayers[i].LODLevel && rr.validNode())
				{
					osg::Group* group = dynamic_cast<osg::Group*>(rr.getNode());
					if (group)
					{
						osg::Group* veg_layer = new osg::Group();
						group->addChild(veg_layer);
						for (size_t j = 0; j < geodes.size(); j++)
						{
							//osg::Node* veg_node = BillboardLayerHelper::CreateVegNodeFromGeode(*geodes[j], m_Terrain.BillboardLayers[i]);
							//veg_layer->addChild(veg_node);
						}
					}
				}
			}
			return rr;
		}
	protected:
		virtual ~GeodePLODInjection() {}
		Terrain m_Terrain;
	};
}