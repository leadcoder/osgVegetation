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

namespace osgVegetation
{
	struct VegetationLayer
	{
		std::string TextureName;
		osg::Vec2 Height;
		osg::Vec2 Width;
		double Density;
		//TextureUnit
		int Type;
		osg::Vec4 MaterialColor;
	};
	typedef std::vector<VegetationLayer> VegetationLayerVector;

	class VegetationScattering : public osg::Referenced
	{
	public:
		VegetationScattering();
		osg::Node* create(osg::Node* terrain, const VegetationLayerVector &layers);
	private:
		double m_PatchTargetSize;
		double m_ViewDistance;

		osgUtil::IntersectionVisitor m_IntersectionVisitor;

		typedef std::map<std::string,osg::ref_ptr<osg::Image> > MaterialCacheMap; 

		MaterialCacheMap m_MaterialCache;
		typedef std::vector< osg::ref_ptr<VegetationObject> > VegetationObjectVector;
		
		float random(float min,float max) { return min + (max-min)*(float)rand()/(float)RAND_MAX; }
		int random(int min,int max) { return min + (int)(((float)(max-min)*(float)rand()/(float)RAND_MAX) + 0.5f); }

		std::string createFileName(unsigned int lv,	unsigned int x, unsigned int y);
		osg::Node* createPatch(osg::Node* terrain, const VegetationLayerVector &layers, osg::Vec3 origin, osg::Vec3 size, osg::StateSet *dstate);
		osg::Geode* createTerrain(const osg::Vec3& origin, const osg::Vec3& size);

		void populateVegetation(osg::Node* terrain,const VegetationLayer& layer, const osg::Vec3& origin, const osg::Vec3& size, VegetationObjectVector& object_list);

		//void createLODRec(osg::Node* terrain,  const VegetationLayerVector &layers, osg::StateSet *dstate, float target_patch_size, osg::LOD* node, float current_size);
		osg::Node* createLODRec(int ld, osg::Node* terrain, const VegetationLayerVector &layers,osg::StateSet *dstate, float current_size, float target_patch_size, osg::Vec3 center,int x, int y);

		//void createPatch(osg::Node* terrain, osg::StateSet *dstate, float patch_size, osg::LOD* node, float current_size);
		osg::Node* createPagedLODRec(int ld, osg::Node* terrain, const VegetationLayerVector &layers,osg::StateSet *dstate, float current_size, float target_patch_size, osg::Vec3 center,int x, int y);
	};
}