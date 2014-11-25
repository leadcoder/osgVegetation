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
	class VegetationTerrainQuery;

	class VegetationScattering : public osg::Referenced
	{
	public:
		VegetationScattering(osg::Node* terrain, double view_dist);
		osg::Node* create(BillboardVegetationLayerVector &layers);
		BillboardVegetationObjectVector generateVegetation(BillboardVegetationLayerVector &layers, const osg::BoundingBox &box);
	private:
		double m_PatchTargetSize;
		double m_ViewDistance;
		BillboardVegetationRenderingTech* m_VRT;
		osg::Node* m_Terrain;
		//osgUtil::IntersectionVisitor m_IntersectionVisitor;
		typedef std::map<std::string,osg::ref_ptr<osg::Image> > MaterialCacheMap; 
		MaterialCacheMap m_MaterialCache;
		VegetationTerrainQuery* m_TerrainQuery;
		
		float random(float min,float max) { return min + (max-min)*(float)rand()/(float)RAND_MAX; }
		int random(int min,int max) { return min + (int)(((float)(max-min)*(float)rand()/(float)RAND_MAX) + 0.5f); }

		std::string createFileName(unsigned int lv,	unsigned int x, unsigned int y);
	//	osg::Node* createPatch(BillboardVegetationLayerVector &layers, BillboardVegetationObjectVector &trees, const osg::BoundingBox &box);
		osg::Geode* createTerrain(const osg::Vec3& origin, const osg::Vec3& size);
		void populateVegetationLayer(const BillboardVegetationLayer& layer,const osg::BoundingBox &box, BillboardVegetationObjectVector& object_list);
		osg::Node* createLODRec(int ld, BillboardVegetationLayerVector &layers, BillboardVegetationObjectVector trees, const osg::BoundingBox &box ,int x, int y);
		//osg::Node* createPagedLODRec(int ld, osg::Node* terrain, VegetationLayerVector &layers, VegetationObjectVector &trees, float current_size, float target_patch_size, float final_patch_size, osg::Vec3 center,int x, int y);
	};
}