#pragma once
#include "Common.h"
#include "BillboardLayer.h"
#include "BillboardObject.h"

#include <osg/StateSet>
#include <osg/Geometry>
#include <osg/Node>
#include <math.h>


namespace osgVegetation
{
	class IBillboardRenderingTech : public osg::Referenced
	{
	public:
		IBillboardRenderingTech(){}
		virtual ~IBillboardRenderingTech(){}
		virtual osg::Node* create(const BillboardVegetationObjectVector &trees, const osg::BoundingBoxd &bb) = 0;
		virtual osg::StateSet* getStateSet() const = 0;
	protected:
	};
}