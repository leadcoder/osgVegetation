#pragma once
#include <osg/StateSet>
#include <osg/Geometry>
#include <math.h>
namespace osgVegetation
{
	class Cell;

	class VegetationRenderingTech
	{
	public:
		virtual osg::Node* create(Cell* cell,osg::StateSet* stateset) = 0;
	protected:
		osg::Geometry* createSprite( float w, float h, osg::Vec4ub color );
		osg::Geometry* createOrthogonalQuads( const osg::Vec3& pos, float w, float h, osg::Vec4ub color );
		osg::Geometry* createOrthogonalQuadsNoColor( const osg::Vec3& pos, float w, float h );
		float random(float min,float max) { return min + (max-min)*(float)rand()/(float)RAND_MAX; }
		int random(int min,int max) { return min + (int)(((float)(max-min)*(float)rand()/(float)RAND_MAX) + 0.5f); }
	};
}