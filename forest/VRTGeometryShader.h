#pragma once
#include <osg/StateSet>
#include <osg/Geometry>
#include <math.h>
#include "VegetationRenderingTech.h"

namespace osgVegetation
{
	class VRTGeometryShader :  public VegetationRenderingTech
	{
	public:
		VRTGeometryShader() {}
		osg::Node* create(Cell* cell, osg::StateSet* dstate);
	protected:
		osg::Program* createGeometryShader();
	};
}