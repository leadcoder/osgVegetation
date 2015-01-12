#pragma once
#include <osg/StateSet>
#include <osg/Geometry>
#include <math.h>
#include "IBillboardRenderingTech.h"
#include "BillboardData.h"

namespace osgVegetation
{
	/*

	*/
	class BRTGeometryShader :  public IBillboardRenderingTech
	{
	public:
		BRTGeometryShader(BillboardData &data);
		osg::Node* create(double view_dist, const BillboardVegetationObjectVector &trees, const osg::BoundingBox &bb);
		osg::StateSet* getStateSet() const {return m_StateSet;}
	protected:
		osg::StateSet* _createStateSet(BillboardData &data);
		osg::Program* _createShaders(BillboardData &data) const;
		osg::StateSet* m_StateSet;

		bool m_TrueBillboards;
		bool m_PPL;
	};
}