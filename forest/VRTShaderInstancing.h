#pragma once
#include <osg/StateSet>
#include <osg/Geometry>
#include <osg/BoundingBox>
#include <math.h>
#include "VegetationRenderingTech.h"

namespace osgVegetation
{
	class VRTShaderInstancing :  public BillboardVegetationRenderingTech
	{
	public:
		VRTShaderInstancing() {}
		//osg::Node* create(Cell* cell);
		osg::Node* create(const BillboardVegetationObjectVector &trees, const osg::BoundingBox &bb);
		osg::StateSet* createStateSet(BillboardVegetationLayerVector &layers);
	protected:
		osg::Geometry* createOrthogonalQuadsNoColor( const osg::Vec3& pos, float w, float h);
		//osg::Node* createRec(Cell* cell, osg::Geometry* templateGeometry);
		osg::StateSet* m_StateSet; 
	};
}