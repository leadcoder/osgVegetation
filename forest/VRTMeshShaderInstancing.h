#pragma once
#include <osg/StateSet>
#include <osg/Geometry>
#include <math.h>
#include "VegetationRenderingTech.h"

namespace osgVegetation
{
	class VRTMeshShaderInstancing :  public MeshVegetationRenderingTech
	{
	public:
		VRTMeshShaderInstancing() {}
		//osg::Node* create(Cell* cell);
		osg::StateSet* createStateSet(MeshVegetationLayerVector &layers);
		//osg::Node* create(const MeshVegetationObjectVector &trees);
		osg::Node* create(const MeshVegetationObjectVector &trees, const std::string &mesh_name, const osg::BoundingBox &bb);
		osg::StateSet* m_StateSet ; 
	protected:
		osg::Node* createRec(Cell* cell);
		std::map<std::string, osg::ref_ptr<osg::Node>  > m_MeshNodeMap;
		std::vector<osg::Geometry*> m_Geometries;
	};
}