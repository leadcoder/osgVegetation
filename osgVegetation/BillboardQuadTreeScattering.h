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
		Class used for billboard generation. Billboards are stored in quad tree 
		structure based on the osg::LOD node. The start tile is based on supplied bounding box and then recursivly 
		divided until the the shortest vegetation layer distance is reached. 
	*/

	class osgvExport BillboardQuadTreeScattering : public osg::Referenced
	{
	public:
		/**
		@param tq Pointer to TerrainQuery classed used during the scattering step.
		*/
		BillboardQuadTreeScattering(ITerrainQuery* tq);
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
		
		
		IBillboardRenderingTech* m_BRT;
		
		//Offset based on initial bounding box, used to avoid floating point precision problem 
		osg::Vec3 m_Offset; 
		
		ITerrainQuery* m_TerrainQuery;

		//paged LOD stuff
		bool m_UsePagedLOD;
		std::string m_SavePath;
		std::string m_FilenamePrefix;
	
		//Helpers
		std::string _createFileName(unsigned int lv,	unsigned int x, unsigned int y);
		void _populateVegetationLayer(const BillboardLayer& layer,const osg::BoundingBox &box, BillboardVegetationObjectVector& instances);
		osg::Node* _createLODRec(int ld, BillboardData &data, BillboardVegetationObjectVector trees, const osg::BoundingBox &box ,int x, int y);
	};
}