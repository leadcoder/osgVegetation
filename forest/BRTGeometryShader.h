#pragma once
#include <osg/StateSet>
#include <osg/Geometry>
#include <math.h>
#include "IBillboardRenderingTech.h"

namespace osgVegetation
{
	class BRTGeometryShader :  public IBillboardRenderingTech
	{
	public:
		BRTGeometryShader() {}
		osg::StateSet* createStateSet(BillboardLayerVector &layers);
	protected:
		osg::Program* createGeometryShader() const;
		osg::StateSet* m_StateSet; 
		
	};
}