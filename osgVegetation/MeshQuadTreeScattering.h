#pragma once
#include "Common.h"
#include <osg/BoundingBox>
#include <osg/Referenced>
#include <osg/vec4>
#include <osg/vec3>
#include <osg/vec2>
#include <osg/Vec4ub>
#include <osg/Node>
#include <osg/ref_ptr>
#include <vector>
#include "IMeshRenderingTech.h"
#include "MeshLayer.h"
#include "MeshData.h"

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

	class osgvExport MeshQuadTreeScattering : public osg::Referenced
	{
	public:
		/**
		@param tq Pointer to TerrainQuery classed used during the scattering step.
		*/
		MeshQuadTreeScattering(ITerrainQuery* tq);
		/**
			Generate vegetation data by providing mesh data
			@param bb Generation area
			@param data Mesh layers and settings
			@param out_put_file Filename if you want to save data base, if using paged LODs this also will decide format and path for all Paged LOD files 
			@param use_paged_lod Use PagedLOD instead of regular LOD nodes (can only be true if out_put_file is defined, otherwise we have no path)
			@param filename_prefix Added to all files (only relevant if out_put_file is defined)
			*/
		osg::Node* generate(const osg::BoundingBox &bb, MeshData &data, const std::string &output_file = "", bool use_paged_lod = false, const std::string &filename_prefix = "");
	private:
		int m_FinalLOD;
		
		//progress data
		int m_CurrentTile;
		int m_NumberOfTiles;
		
		//Area bounding box
		osg::BoundingBox m_InitBB;

		IMeshRenderingTech* m_MRT;
		osg::Vec3 m_Offset;
		ITerrainQuery* m_TerrainQuery;
		
		bool m_UsePagedLOD;

		//Output stuff
		std::string m_SavePath;
		std::string m_FilenamePrefix;
		std::string m_SaveExt;

		//Helpers
		std::string _createFileName(unsigned int lv, unsigned int x, unsigned int y);
		void _populateVegetationLayer(MeshLayer& layer,const osg::BoundingBox &box);
		osg::Node* _createLODRec(int ld, MeshData &data, MeshVegetationObjectVector trees, const osg::BoundingBox &box ,int x, int y);
	};
}