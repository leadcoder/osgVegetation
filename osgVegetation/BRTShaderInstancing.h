#pragma once
#include "Common.h"
#include <osg/StateSet>
#include <osg/Geometry>
#include <osg/BoundingBox>
#include <math.h>
#include "IBillboardRenderingTech.h"
#include "BillboardData.h"

namespace osgVegetation
{
	class osgvExport BRTShaderInstancing :  public IBillboardRenderingTech
	{
	public:
		BRTShaderInstancing(BillboardData &data);
		virtual ~BRTShaderInstancing();
		osg::Node* create(double view_dist, const BillboardVegetationObjectVector &trees, const osg::BoundingBox &bb);
		osg::StateSet* getStateSet() const {return m_StateSet;}
	protected:
		osg::StateSet* _createStateSet(BillboardData &data);
		osg::Geometry* _createOrthogonalQuadsWithNormals( const osg::Vec3& pos, float w, float h);
		osg::Geometry* _createSingleQuadsWithNormals( const osg::Vec3& pos, float w, float h);
		osg::StateSet* m_StateSet;
		bool m_TrueBillboards;
		bool m_PPL;
	};
}