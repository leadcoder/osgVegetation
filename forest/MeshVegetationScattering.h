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
#include "MeshVegetationObject.h"

namespace osgVegetation
{
	class MeshVegetationScattering : public osg::Referenced
	{
	public:
		MeshVegetationScattering(osg::Node* terrain, double view_dist);
		osg::Node* create(MeshVegetationLayerVector &layers);
	private:
		int m_StartLOD;
		double m_ViewDistance;
		MeshVegetationRenderingTech* m_VRT;
		osgUtil::IntersectionVisitor m_IntersectionVisitor;
		typedef std::map<std::string,osg::ref_ptr<osg::Image> > MaterialCacheMap; 
		MaterialCacheMap m_MaterialCache;
		osg::Node* m_Terrain; 

		float random(float min,float max) { return min + (max-min)*(float)rand()/(float)RAND_MAX; }
		int random(int min,int max) { return min + (int)(((float)(max-min)*(float)rand()/(float)RAND_MAX) + 0.5f); }

		std::string createFileName(unsigned int lv,	unsigned int x, unsigned int y);
		MeshVegetationMap generateVegetation( MeshVegetationLayerVector &layers, osg::Vec3 origin, osg::Vec3 size);
		osg::Geode* createTerrain(const osg::Vec3& origin, const osg::Vec3& size);
		void populateVegetationLayer(const MeshVegetationLayer& layer, const osg::Vec3& origin, const osg::Vec3& size, MeshVegetationObjectVector& object_list);
		osg::Node* createLODRec(int ld, MeshVegetationLayerVector &layers, MeshVegetationMap trees, float current_size, osg::Vec3 center,int x, int y);
		//osg::Node* createPagedLODRec(int ld, osg::Node* terrain, VegetationLayerVector &layers, VegetationObjectVector &trees, float current_size, float target_patch_size, float final_patch_size, osg::Vec3 center,int x, int y);
	};
}