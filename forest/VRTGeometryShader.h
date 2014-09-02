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
		osg::Node* create(Cell* cell);
		void createStateSet(VegetationLayerVector &layers);

	protected:
		osg::Program* createGeometryShader() const;
		osg::StateSet* m_StateSet; 
		
	};
}