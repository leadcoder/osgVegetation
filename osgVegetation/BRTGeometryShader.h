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
		BRTGeometryShader(BillboardData &data, bool use_fog, osg::Fog::Mode fog_mode);
		osg::Node* create(const BillboardVegetationObjectVector &trees, const osg::BoundingBox &bb);
		osg::StateSet* getStateSet() const {return m_StateSet;}
	protected:
		osg::StateSet* _createStateSet(BillboardData &data);
		osg::Program* _createShaders(BillboardData &data) const;
		osg::StateSet* m_StateSet;

		bool m_TrueBillboards;
		bool m_PPL;
		/**
			Enable fog in shaders
		*/
		bool m_UseFog;
		/**
			Fog mode
		*/
		osg::Fog::Mode m_FogMode;
	};
}