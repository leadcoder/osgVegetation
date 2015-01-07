#pragma once
#include "Common.h"
#include <osg/BoundingBox>
#include <osg/Referenced>
#include <osg/vec4>
#include <osg/vec3>
#include <osg/vec2>
#include <osg/Vec4ub>
#include <osg/Node>
#include <osg/Fog>
#include <osg/ref_ptr>

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
		QuadTreeScattering(ITerrainQuery* tq, bool use_fog = false, osg::Fog::Mode fog_mode = osg::Fog::LINEAR);
		/**
			Generate vegetation data by providing billboard data
			@param bb Generation area
			@param data Billboard layers and settings
			@param paged_lod_path Optional path to save PagedLOD nodes, if provided PagedLOD are used instead of regular LOD nodes
		*/
		osg::Node* generate(const osg::BoundingBox &bb, BillboardData &data, const std::string &paged_lod_path = "", const std::string &filename_prefix = "");
	private:
		int m_FinalLOD;
		int m_StartLOD;

		//progress data
		int m_CurrentTile;
		int m_NumberOfTiles;
		
		//Area bounding box
		osg::BoundingBox m_InitBB;

		IBillboardRenderingTech* m_VRT;
		osg::Vec3 m_Offset;
		ITerrainQuery* m_TerrainQuery;
		bool m_UsePagedLOD;
		std::string m_SavePath;
		std::string m_FilenamePrefix;
		bool m_UseFog;
		osg::Fog::Mode m_FogMode;

		//Helpers
		std::string _createFileName(unsigned int lv,	unsigned int x, unsigned int y);
		void _populateVegetationLayer(const BillboardLayer& layer,const osg::BoundingBox &box, BillboardVegetationObjectVector& instances);
		//BillboardVegetationObjectVector _generateVegetation(BillboardData &data, const osg::BoundingBox &box);
		osg::Node* _createLODRec(int ld, BillboardData &data, BillboardVegetationObjectVector trees, const osg::BoundingBox &box ,int x, int y);
	};
}