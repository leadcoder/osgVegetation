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
		Class used for vegetation generation. Vegetation is stored in quad tree 
		structure based on the osg::LOD node. The start tile is based on the terrain size and then recursivly 
		divided until the vegetation view distance is reached. The user can also let LOD nodes above the final hold
		vegetation data (with decreased density). This feature enable terrain far away (from camera) to hold 
		low density vegetation saving performance. See the BillboardData structure for more information about LOD settings.
	*/

	class osgvExport QuadTreeScattering : public osg::Referenced
	{
	public:
		/**
		@param tq Pointer to TerrainQuery classed used during the scattering step.
		*/
		QuadTreeScattering(ITerrainQuery* tq);
		/**
			Generate vegetation data by providing billboard data
			@param bb Generation area
			@param data Billboard layers and settings
			@param paged_lod_path Optional path to save PagedLOD nodes, if provided PagedLOD are used instead of regular LOD nodes
		*/
		osg::Node* generate(const osg::BoundingBox &bb, BillboardData &data, const std::string &paged_lod_path = "", const std::string &filename_prefix = "");
	private:
		//double m_TileTargetSize;
		double m_ViewDistance;

		//LOD Data
		float m_DensityLODRatio;
		float m_ScaleLODRatio;

		int m_FinalLOD;
		int m_StartLOD;

		//progress data
		int m_CurrentTile;
		int m_NumberOfTiles;
		int m_DensityLODs;

		//Area boudning box
		osg::BoundingBox m_InitBB;

		IBillboardRenderingTech* m_VRT;
		osg::Vec3 m_Offset;
		typedef std::map<std::string,osg::ref_ptr<osg::Image> > MaterialCacheMap; 
		MaterialCacheMap m_MaterialCache;
		ITerrainQuery* m_TerrainQuery;
		bool m_UsePagedLOD;
		std::string m_SavePath;
		std::string m_FilenamePrefix;

		//Helpers
		std::string _createFileName(unsigned int lv,	unsigned int x, unsigned int y);
		osg::Geode* _createTerrain(const osg::Vec3& origin, const osg::Vec3& size);
		void _populateVegetationLayer(const BillboardLayer& layer,const osg::BoundingBox &box, BillboardVegetationObjectVector& object_list, double lod_density, double lod_scale);
		BillboardVegetationObjectVector _generateVegetation(BillboardLayerVector &layers, const osg::BoundingBox &box, double lod_density, double lod_scale);
		osg::Node* _createLODRec(int ld, BillboardLayerVector &layers, BillboardVegetationObjectVector trees, const osg::BoundingBox &box ,int x, int y);
	};
}