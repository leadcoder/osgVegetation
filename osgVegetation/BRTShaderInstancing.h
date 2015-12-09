#pragma once
#include "Common.h"
#include <osg/StateSet>
#include <osg/Geometry>
#include <osg/BoundingBox>
#include <math.h>
#include "IBillboardRenderingTech.h"
#include "BillboardData.h"
#include "EnvironmentSettings.h"

namespace osgVegetation
{
	/**
		IBillboardRenderingTech implementation that use a shader instancing based technique to generate vegetation billboars.
	*/
	class osgvExport BRTShaderInstancing :  public IBillboardRenderingTech
	{
	public:
		BRTShaderInstancing(BillboardData &data, const EnvironmentSettings &env_settings);
		virtual ~BRTShaderInstancing();
		
		//IBillboardRenderingTech
		osg::Node* create(double view_dist, const BillboardVegetationObjectVector &trees, const osg::BoundingBoxd &bb);
		osg::StateSet* getStateSet() const {return m_StateSet;}

	protected:
		osg::StateSet* _createStateSet(BillboardData &data, const EnvironmentSettings &env_settings);
		osg::Geometry* _createOrthogonalQuadsWithNormals( const osg::Vec3& pos, float w, float h);
		osg::Geometry* _createSingleQuadsWithNormals( const osg::Vec3& pos, float w, float h);
		osg::StateSet* m_StateSet;
		bool m_TrueBillboards;
		bool m_PPL;
	};
}