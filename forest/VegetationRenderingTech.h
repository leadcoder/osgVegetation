#pragma once
#include <osg/StateSet>
#include <osg/Geometry>
#include <math.h>
#include "VegetationLayer.h"
namespace osgVegetation
{
	class Cell;
	class VegetationRenderingTech : public osg::Referenced
	{
	public:
		VegetationRenderingTech(){}
		virtual ~VegetationRenderingTech(){}
		virtual osg::Node* create(Cell* cell)= 0;
		virtual void createStateSet(VegetationLayerVector &layers) = 0;
	protected:
		osg::Geometry* createSprite( float w, float h, osg::Vec4ub color );
		osg::Geometry* createOrthogonalQuads( const osg::Vec3& pos, float w, float h, osg::Vec4ub color );
		osg::Geometry* createOrthogonalQuadsNoColor( const osg::Vec3& pos, float w, float h );
		float random(float min,float max) { return min + (max-min)*(float)rand()/(float)RAND_MAX; }
		int random(int min,int max) { return min + (int)(((float)(max-min)*(float)rand()/(float)RAND_MAX) + 0.5f); }
	};
}