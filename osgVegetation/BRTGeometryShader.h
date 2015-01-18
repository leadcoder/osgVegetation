#pragma once
#include <osg/StateSet>
#include <osg/Geometry>
#include <math.h>
#include "IBillboardRenderingTech.h"
#include "BillboardData.h"

namespace osgVegetation
{
	/**
		IBillboardRenderingTech implementation that use a geometry shader based technique to generate vegetation quads.
	*/
	class BRTGeometryShader :  public IBillboardRenderingTech
	{
	public:
		BRTGeometryShader(BillboardData &data);

		//IBillboardRenderingTech
		osg::Node* create(double view_dist, const BillboardVegetationObjectVector &trees, const osg::BoundingBox &bb);
		osg::StateSet* getStateSet() const {return m_StateSet;}
	protected:
		osg::StateSet* _createStateSet(BillboardData &data);
		osg::Program* _createShaders(BillboardData &data) const;
		osg::StateSet* m_StateSet;
		bool m_PPL;
	};
}