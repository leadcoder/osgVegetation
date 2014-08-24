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
		osg::Node* create(Cell* cell, osg::StateSet* dstate);
		osg::Node* createRec(Cell* cell, osg::Geometry* templateGeometry);
	protected:
		
	};
}