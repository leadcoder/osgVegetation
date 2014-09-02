#pragma once
#include <osg/BoundingBox>
#include <osg/Referenced>
#include <osg/vec4>
#include <osg/vec3>
#include <osg/vec2>
#include <osg/Vec4ub>
#include <osg/Node>
#include <osg/LOD>
#include <osg/ref_ptr>
#include <osgUtil/IntersectionVisitor>
#include <vector>
#include "VegetationCell.h"
#include "VegetationRenderingTech.h"
#include "VegetationLayer.h"

namespace osgVegetation
{
	class VegetationScattering : public osg::Referenced
	{
	public:
		VegetationScattering(VegetationRenderingTech* vrt, double patch_size);
		osg::Node* create(osg::Node* terrain, VegetationLayerVector &layers);
	private:
		double m_PatchTargetSize;
		double m_ViewDistance;
		VegetationRenderingTech* m_VRT;
		osgUtil::IntersectionVisitor m_IntersectionVisitor;

		typedef std::map<std::string,osg::ref_ptr<osg::Image> > MaterialCacheMap; 

		MaterialCacheMap m_MaterialCache;
		typedef std::vector< osg::ref_ptr<VegetationObject> > VegetationObjectVector;
		
		float random(float min,float max) { return min + (max-min)*(float)rand()/(float)RAND_MAX; }
		int random(int min,int max) { return min + (int)(((float)(max-min)*(float)rand()/(float)RAND_MAX) + 0.5f); }

		std::string createFileName(unsigned int lv,	unsigned int x, unsigned int y);
		osg::Node* createPatch(osg::Node* terrain, VegetationLayerVector &layers, osg::Vec3 origin, osg::Vec3 size);
		osg::Geode* createTerrain(const osg::Vec3& origin, const osg::Vec3& size);
		void populateVegetationLayer(osg::Node* terrain,const VegetationLayer& layer, const osg::Vec3& origin, const osg::Vec3& size, VegetationObjectVector& object_list);
		osg::Node* createLODRec(int ld, osg::Node* terrain, VegetationLayerVector &layers, float current_size, float target_patch_size, osg::Vec3 center,int x, int y);
		osg::Node* createPagedLODRec(int ld, osg::Node* terrain, VegetationLayerVector &layers, float current_size, float target_patch_size, osg::Vec3 center,int x, int y);
	};
}