#pragma once
#include "Common.h"
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
#include "IBillboardRenderingTech.h"
#include "BillboardLayer.h"
#include "BillboardData.h"

namespace osgVegetation
{
	class TerrainQuery;


	class osgvExport QuadTreeScattering : public osg::Referenced
	{
	public:
		QuadTreeScattering(osg::Node* terrain);
		osg::Node* create(BillboardData &layers);
	private:
		double m_PatchTargetSize;
		double m_ViewDistance;
		double m_MinPatchSize;
		int m_FinalLOD;
		IBillboardRenderingTech* m_VRT;
		osg::Node* m_Terrain;
		typedef std::map<std::string,osg::ref_ptr<osg::Image> > MaterialCacheMap; 
		MaterialCacheMap m_MaterialCache;
		TerrainQuery* m_TerrainQuery;
		std::string createFileName(unsigned int lv,	unsigned int x, unsigned int y);
		osg::Geode* createTerrain(const osg::Vec3& origin, const osg::Vec3& size);
		void populateVegetationLayer(const BillboardLayer& layer,const osg::BoundingBox &box, BillboardVegetationObjectVector& object_list,double density_scale);
		BillboardVegetationObjectVector generateVegetation(BillboardLayerVector &layers, const osg::BoundingBox &box,double density_scale);
		osg::Node* createLODRec(int ld, BillboardLayerVector &layers, BillboardVegetationObjectVector trees, const osg::BoundingBox &box ,int x, int y);
		//osg::Node* createPagedLODRec(int ld, osg::Node* terrain, VegetationLayerVector &layers, VegetationObjectVector &trees, float current_size, float target_patch_size, float final_patch_size, osg::Vec3 center,int x, int y);
	};
}