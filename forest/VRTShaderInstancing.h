#pragma once
#include <osg/StateSet>
#include <osg/Geometry>
#include <osg/BoundingBox>
#include <math.h>
#include "VegetationRenderingTech.h"

namespace osgVegetation
{
	class VRTShaderInstancing :  public BillboardVegetationRenderingTech
	{
	public:
		VRTShaderInstancing(bool true_billboards, bool use_simple_normals);
		virtual ~VRTShaderInstancing();
		osg::Node* create(const BillboardVegetationObjectVector &trees, const osg::BoundingBox &bb);
		osg::StateSet* createStateSet(BillboardVegetationLayerVector &layers);
		void setAlphaRefValue(float value) {m_AlphaRefValue = value;}
		void setAlphaBlend(bool value) {m_AlphaBlend = value;}
		void setTerrainNormal(bool value) {m_TerrainNormal = value;}
	protected:
		osg::Geometry* createOrthogonalQuadsWithNormals( const osg::Vec3& pos, float w, float h);
		osg::Geometry* createSingleQuadsWithNormals( const osg::Vec3& pos, float w, float h);
		osg::StateSet* m_StateSet;
		bool m_TrueBillboards;
		bool m_PPL;
		bool m_TerrainNormal;
		float m_AlphaRefValue;
		bool  m_AlphaBlend;

	};
}