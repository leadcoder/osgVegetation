#pragma once
#include <osg/StateSet>
#include <osg/Geometry>
#include <osg/Node>
#include <math.h>
#include "MeshObject.h"
#include "MeshLayer.h"

namespace osgVegetation
{
	class IMeshRenderingTech : public osg::Referenced
	{
	public:
		IMeshRenderingTech(){}
		virtual ~IMeshRenderingTech(){}
		virtual osg::Node* create(const MeshVegetationObjectVector &trees, const std::string &mesh_name, const osg::BoundingBox &bb) = 0;
		virtual osg::StateSet* createStateSet(MeshLayerVector &layers) = 0;
	protected:
		/*osg::Geometry* createSprite( float w, float h, osg::Vec4ub color );
		osg::Geometry* createOrthogonalQuads( const osg::Vec3& pos, float w, float h, osg::Vec4ub color );
		osg::Geometry* createOrthogonalQuadsNoColor( const osg::Vec3& pos, float w, float h );
		float random(float min,float max) { return min + (max-min)*(float)rand()/(float)RAND_MAX; }
		int random(int min,int max) { return min + (int)(((float)(max-min)*(float)rand()/(float)RAND_MAX) + 0.5f); */
	};
}