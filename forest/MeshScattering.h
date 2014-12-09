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
#include "IMeshRenderingTech.h"
#include "MeshLayer.h"
//#include "MeshData.h"
#include "MeshObject.h"

namespace osgSim {class DatabaseCacheReadCallback;}

namespace osgVegetation
{
	class TerrainQuery;
	class MeshScattering : public osg::Referenced
	{
	public:
		MeshScattering(osg::Node* terrain, double view_dist);
		osg::Node* create(MeshLayerVector &layers);
	private:

		int m_StartLOD;
		double m_ViewDistance;
		IMeshRenderingTech* m_MRT;
		osgUtil::IntersectionVisitor m_IntersectionVisitor;
		typedef std::map<std::string,osg::ref_ptr<osg::Image> > MaterialCacheMap; 
		MaterialCacheMap m_MaterialCache;
		osg::Node* m_Terrain; 
		//osgSim::DatabaseCacheReadCallback* m_Cache;
		TerrainQuery* m_TerrainQuery;

		float random(float min,float max) { return min + (max-min)*(float)rand()/(float)RAND_MAX; }
		int random(int min,int max) { return min + (int)(((float)(max-min)*(float)rand()/(float)RAND_MAX) + 0.5f); }

		std::string createFileName(unsigned int lv,	unsigned int x, unsigned int y);
		MeshVegetationMap generateVegetation( MeshLayerVector &layers, osg::BoundingBox &box);
		osg::Geode* createTerrain(osg::BoundingBox &box);
		void populateVegetationLayer(const MeshLayer& layer, osg::BoundingBox &box , MeshVegetationObjectVector& object_list);
		osg::Node* createLODRec(int ld, MeshLayerVector &layers, MeshVegetationMap trees, osg::BoundingBox& boundingBox,int x, int y);
		//osg::Node* createPagedLODRec(int ld, osg::Node* terrain, VegetationLayerVector &layers, VegetationObjectVector &trees, float current_size, float target_patch_size, float final_patch_size, osg::Vec3 center,int x, int y);
	};
}