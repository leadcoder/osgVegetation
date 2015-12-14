#pragma once
#include "Common.h"
#include <osg/StateSet>
#include <osg/Geometry>
#include <osg/BoundingBox>
#include <osg/Texture2D>
#include <math.h>
#include "IBillboardRenderingTech.h"
#include "BillboardData.h"

namespace osgVegetation
{
	/**
		IBillboardRenderingTech implementation that use a shader instancing based technique to generate vegetation billboars.
	*/
	class osgvExport BRTShaderInstancing2 
	{
	public:
		BRTShaderInstancing2(BillboardData &data);
		virtual ~BRTShaderInstancing2();
		
		//IBillboardRenderingTech
		osg::Node* create(osg::ref_ptr<osg::Texture2D> hm_tex,double view_dist, const BillboardVegetationObjectVector &trees, const osg::BoundingBoxd &bb);
		osg::StateSet* getStateSet() const {return m_StateSet;}
		osg::Node* create(osg::ref_ptr<osg::Texture2D> hm_tex, double view_dist, const osg::BoundingBoxd &bb);
		osg::Node* create(osg::ref_ptr<osg::Texture2D> hm_tex, BillboardData &data, double view_dist, const osg::BoundingBoxd &bb);

	protected:
		void _populateVegetationTile(const BillboardLayer& layer,const  osg::BoundingBoxd& bb,BillboardVegetationObjectVector& instances) const;

		osg::StateSet* _createStateSet(BillboardData &data);
		osg::Geometry* _createOrthogonalQuadsWithNormals( const osg::Vec3& pos, float w, float h);
		osg::Geometry* _createSingleQuadsWithNormals( const osg::Vec3& pos, float w, float h);
		osg::StateSet* m_StateSet;
		bool m_TrueBillboards;
		bool m_PPL;
	};
}