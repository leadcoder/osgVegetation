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
		osg::Node* create(const BillboardVegetationObjectVector &trees, const osg::BoundingBox &bb);
		osg::StateSet* getStateSet() const {return m_StateSet;}
		
		void setAlphaRefValue(float value) {m_AlphaRefValue = value;}
		void setAlphaBlend(bool value) {m_AlphaBlend = value;}
		void setTerrainNormal(bool value) {m_TerrainNormal = value;}
		void setReceivesShadows(bool value) {m_ReceivesShadows = value;}
	protected:
		osg::StateSet* _createStateSet(BillboardLayerVector &layers);
		osg::Geometry* _createOrthogonalQuadsWithNormals( const osg::Vec3& pos, float w, float h);
		osg::Geometry* _createSingleQuadsWithNormals( const osg::Vec3& pos, float w, float h);

		osg::StateSet* m_StateSet;
		bool m_TrueBillboards;
		bool m_PPL;
		bool m_TerrainNormal;
		bool m_ReceivesShadows;
		float m_AlphaRefValue;
		bool  m_AlphaBlend;

	};
}