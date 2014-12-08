#pragma once
#include <osg/StateSet>
#include <osg/Geometry>
#include <math.h>
#include "VegetationRenderingTech.h"

namespace osgVegetation
{
	class Cell;
	class VRTGeometryShader :  public BillboardVegetationRenderingTech
	{
	public:
		VRTGeometryShader() {}
		osg::Node* create(Cell* cell);
		osg::StateSet* createStateSet(BillboardVegetationLayerVector &layers);
	protected:
		osg::Program* createGeometryShader() const;
		osg::StateSet* m_StateSet; 
		
	};
}