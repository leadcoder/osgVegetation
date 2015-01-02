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
		osg::Node* create(const BillboardVegetationObjectVector &trees, const osg::BoundingBox &bb);
		
		osg::StateSet* getStateSet() const {return m_StateSet;}
		void setAlphaRefValue(float value) {m_AlphaRefValue = value;}
		void setAlphaBlend(bool value) {m_AlphaBlend = value;}
		void setTerrainNormal(bool value) {m_TerrainNormal = value;}
		void setReceivesShadows(bool value) {m_ReceivesShadows = value;}
	protected:
		osg::StateSet* _createStateSet(BillboardData &data);
		osg::Program* _createShaders() const;
		
		osg::StateSet* m_StateSet;
		bool m_TrueBillboards;
		bool m_PPL;
		bool m_TerrainNormal;
		bool m_ReceivesShadows;
		float m_AlphaRefValue;
		bool  m_AlphaBlend;
	};
}