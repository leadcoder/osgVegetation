#pragma once
#include "Common.h"
#include "BillboardObject.h"
#include <osg/StateSet>
#include <osg/Geometry>
#include <osg/Node>

namespace osgVegetation
{
	class IBillboardRenderingTech : public osg::Referenced
	{
	public:
		IBillboardRenderingTech(){}
		virtual ~IBillboardRenderingTech(){}
		virtual osg::Node* create(const BillboardVegetationObjectVector &trees, const osg::BoundingBoxd &bb) = 0;
		virtual osg::StateSet* getStateSet() const = 0;
	};
}