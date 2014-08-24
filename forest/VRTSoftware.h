#pragma once
#include <osg/StateSet>
#include <osg/Geometry>
#include <math.h>
#include "VegetationRenderingTech.h"

namespace osgVegetation
{
	class Cell;

	enum VRTech
	{
		VRT_BILLBOARD,
		VRT_MATRIX,
		VRT_COPY
	};	

	class VRTSoftware : public VegetationRenderingTech
	{
	public:
		VRTSoftware(VRTech tech) : m_Tech(tech) {}
		osg::Node* create(Cell* cell,osg::StateSet* stateset);
	protected:
		osg::Node* createBillboardGraph(Cell* cell,osg::StateSet* stateset);
		osg::Node* createXGraph(Cell* cell,osg::StateSet* stateset);
		osg::Node* createTransformGraph(Cell* cell,osg::StateSet* stateset);
		VRTech m_Tech;
	};
}