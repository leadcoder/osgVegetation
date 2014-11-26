#pragma once
#include <osg/StateSet>
#include <osg/Geometry>
#include <osg/Node>
#include <math.h>
#include "VegetationLayer.h"
#include "MeshVegetationObject.h"
#include "BillboardVegetationObject.h"

namespace osgVegetation
{
	class VegetationRenderingTech : public osg::Referenced
	{
	public:
		VegetationRenderingTech(){}
		virtual ~VegetationRenderingTech(){}
	protected:
		osg::Geometry* createSprite( float w, float h, osg::Vec4ub color );
		osg::Geometry* createOrthogonalQuads( const osg::Vec3& pos, float w, float h, osg::Vec4ub color );
		osg::Geometry* createOrthogonalQuadsNoColor( const osg::Vec3& pos, float w, float h );
		float random(float min,float max) { return min + (max-min)*(float)rand()/(float)RAND_MAX; }
		int random(int min,int max) { return min + (int)(((float)(max-min)*(float)rand()/(float)RAND_MAX) + 0.5f); }
	};

	class Cell;
	class BillboardVegetationRenderingTech : public osg::Referenced
	{
	public:
		BillboardVegetationRenderingTech(){}
		virtual ~BillboardVegetationRenderingTech(){}
		//virtual osg::Node* create(const BillboardVegetationObjectVector &trees, const osg::BoundingBox &bb) = 0;
		virtual osg::Node* create(const BillboardVegetationObjectVector &trees, const osg::BoundingBox &bb) = 0;
		//virtual osg::Node* create(Cell* cell)= 0;
		virtual osg::StateSet* createStateSet(BillboardVegetationLayerVector &layers) = 0;
	protected:
		osg::Geometry* createSprite( float w, float h, osg::Vec4ub color );
		osg::Geometry* createOrthogonalQuads( const osg::Vec3& pos, float w, float h, osg::Vec4ub color );
		osg::Geometry* createOrthogonalQuadsNoColor( const osg::Vec3& pos, float w, float h );
		float random(float min,float max) { return min + (max-min)*(float)rand()/(float)RAND_MAX; }
		int random(int min,int max) { return min + (int)(((float)(max-min)*(float)rand()/(float)RAND_MAX) + 0.5f); }
	};


	class MeshVegetationRenderingTech : public osg::Referenced
	{
	public:
		MeshVegetationRenderingTech(){}
		virtual ~MeshVegetationRenderingTech(){}
		virtual osg::Node* create(const MeshVegetationObjectVector &trees, const std::string &mesh_name, const osg::BoundingBox &bb) = 0;
		virtual osg::StateSet* createStateSet(MeshVegetationLayerVector &layers) = 0;
	protected:
		osg::Geometry* createSprite( float w, float h, osg::Vec4ub color );
		osg::Geometry* createOrthogonalQuads( const osg::Vec3& pos, float w, float h, osg::Vec4ub color );
		osg::Geometry* createOrthogonalQuadsNoColor( const osg::Vec3& pos, float w, float h );
		float random(float min,float max) { return min + (max-min)*(float)rand()/(float)RAND_MAX; }
		int random(int min,int max) { return min + (int)(((float)(max-min)*(float)rand()/(float)RAND_MAX) + 0.5f); }
	};
}