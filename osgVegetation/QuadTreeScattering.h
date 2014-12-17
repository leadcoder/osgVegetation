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
	class ITerrainQuery;

	/**
		Class used for vegetation scattering. Vegetation is stored in quad tree 
		structure based on the osg::LOD node. The start tile is based on the terrain size and then recursivly 
		divided until the vegetation view distance is reached. The user can also let LOD nodes above the final hold
		vegetation data (with decreased density). This feature enable terrain far away (from camera) to hold 
		low density vegetation saving performance.
	*/
	class osgvExport QuadTreeScattering : public osg::Referenced
	{
	public:
		QuadTreeScattering(osg::Node* terrain, ITerrainQuery* tq);
		osg::Node* create(BillboardData &layers);
	private:
		double m_TileTargetSize;
		double m_ViewDistance;
		double m_MinTileSize;

		int m_FinalLOD;

		//progress data
		int m_CurrentTile;
		int m_NumberOfTiles;

		osg::BoundingBox m_InitBB;
		IBillboardRenderingTech* m_VRT;
		osg::Node* m_Terrain;
		osg::Vec3 m_Offset;
		typedef std::map<std::string,osg::ref_ptr<osg::Image> > MaterialCacheMap; 
		MaterialCacheMap m_MaterialCache;
		ITerrainQuery* m_TerrainQuery;
		std::string createFileName(unsigned int lv,	unsigned int x, unsigned int y);
		osg::Geode* createTerrain(const osg::Vec3& origin, const osg::Vec3& size);
		void populateVegetationLayer(const BillboardLayer& layer,const osg::BoundingBox &box, BillboardVegetationObjectVector& object_list,double density_scale);
		BillboardVegetationObjectVector generateVegetation(BillboardLayerVector &layers, const osg::BoundingBox &box,double density_scale);
		osg::Node* createLODRec(int ld, BillboardLayerVector &layers, BillboardVegetationObjectVector trees, const osg::BoundingBox &box ,int x, int y);
		//osg::Node* createPagedLODRec(int ld, osg::Node* terrain, VegetationLayerVector &layers, VegetationObjectVector &trees, float current_size, float target_patch_size, float final_patch_size, osg::Vec3 center,int x, int y);
	};
}