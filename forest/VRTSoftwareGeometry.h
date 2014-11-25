#pragma once
#include <osg/StateSet>
#include <osg/Geometry>
#include <math.h>
#include "VegetationRenderingTech.h"

namespace osgVegetation
{
	class Cell;

	class VRTSoftwareGeometry : public VegetationRenderingTech
	{
	public:
		VRTSoftwareGeometry() {}
		osg::Node* create(Cell* cell);
		void createStateSet(BillboardVegetationLayerVector &layers);
	protected:
		osg::Node* createTransformGraph2(Cell* cell);
		osg::ref_ptr<osg::Node> m_VegMesh;
	};
}