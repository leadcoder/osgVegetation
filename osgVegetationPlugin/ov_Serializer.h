#pragma once
#include <osg/Node>
#include <vector>
#include "ov_BillboardLayer.h"
#include "ov_Terrain.h"
class TiXmlElement;
namespace osgVegetation
{
	class XMLSerializer
	{
	public:
		XMLSerializer(){}
		virtual ~XMLSerializer(){}
		//static osg::ref_ptr<osg::Node> LoadTerain(const std::string &filename);
		static void GetTerrainData(const std::string &filename, Terrain &terrain);
	};
}
