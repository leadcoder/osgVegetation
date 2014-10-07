#pragma once
#include <osg/StateSet>
#include <osg/Geometry>
#include <math.h>
#include "VegetationRenderingTech.h"

namespace osgVegetation
{
	class VRTMeshShaderInstancing :  public VegetationRenderingTech
	{
	public:
		VRTMeshShaderInstancing() {}
		osg::Node* create(Cell* cell);
		void createStateSet(VegetationLayerVector &layers);
		osg::StateSet* m_StateSet; 
	protected:
		osg::Node* createRec(Cell* cell);
		osg::ref_ptr<osg::Node> m_MeshNode;
		//osg::ref_ptr<osg::Node> m_Geode;
		std::vector<osg::Geometry*> m_Geometries;
	};
}