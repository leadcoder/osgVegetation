#pragma once
#include "Common.h"
#include <osg/StateSet>
#include <osg/Geometry>
#include "IBillboardRenderingTech.h"
#include "BillboardData.h"
#include "EnvironmentSettings.h"

namespace osgVegetation
{
	/**
		IBillboardRenderingTech implementation that use a geometry shader based technique to generate vegetation quads.
	*/
	class BRTGeometryShader :  public IBillboardRenderingTech
	{
	public:
		BRTGeometryShader(BillboardData &data, const EnvironmentSettings &env_settings);

		//IBillboardRenderingTech
		osg::Node* create(const BillboardVegetationObjectVector &trees, const osg::BoundingBoxd &bb);
		osg::StateSet* getStateSet() const {return m_StateSet;}
	protected:
		osg::StateSet* _createStateSet(BillboardData &data, const EnvironmentSettings &env_settings);
		osg::Program* _createShaders(BillboardData &data, const EnvironmentSettings &env_settings) const;
		osg::StateSet* m_StateSet;
		bool m_PPL;
	};
}
