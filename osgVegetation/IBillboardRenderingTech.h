#pragma once
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
		virtual osg::Node* create(const BillboardVegetationObjectVector &trees, const osg::BoundingBox &bb) = 0;
		virtual osg::StateSet* getStateSet() const = 0;
		virtual void setAlphaRefValue(float value) = 0;
		virtual void setAlphaBlend(bool value) = 0;
		virtual void setTerrainNormal(bool value) = 0;
		virtual void setReceivesShadows(bool value) = 0;
	protected:
	};
}