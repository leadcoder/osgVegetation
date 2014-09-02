#pragma once
#include <osg/StateSet>
#include <osg/Geometry>
#include <math.h>
#include "VegetationRenderingTech.h"

namespace osgVegetation
{
	class VRTShaderInstancing :  public VegetationRenderingTech
	{
	public:
		VRTShaderInstancing() {}
		osg::Node* create(Cell* cell);
		void createStateSet(VegetationLayerVector &layers);
	protected:
		osg::Node* createRec(Cell* cell, osg::Geometry* templateGeometry);
		osg::StateSet* m_StateSet; 
	};
}