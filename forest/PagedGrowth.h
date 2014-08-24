/****************************************************************************
*                                                                           *
* HiFiEngine                                                                *
* Copyright (C)2003 - 2005 Johan Hedstrom                                   *
* Email: hifiengine@gmail.com                                               *
* Web page: http://n00b.dyndns.org/HiFiEngine                               *
*                                                                           *
* HiFiEngine is only used with knowledge from the author. This software     *
* is not allowed to redistribute without permission from the author.        *
* For further license information, please turn to the product home page or  *
* contact author. Abuse against the HiFiEngine license is prohibited by law.*
*                                                                           *
*****************************************************************************/ 

#pragma once

#include <vector>
#include <string>
#include <map>
#include <list>
#include <osg/Vec2>
#include <osg/Vec3>
#include <osg/Matrix>
#include <osg/BoundingSphere>
#include <osg/BoundingBox>
#include "tinyxml.h"

namespace osgVegetation
{

	struct  GrowthMaterialType
	{	
		std::string GeometryName;
		float MinRadiusDistToEquals;
		float MinRadiusDistToOthers;
		float NormalScale;
		float Probability;
		std::string ScaleString;
		osg::Vec2 Scale;
		//IGeometry *Geometry;
		//IGeometry *HighLodGeometry;
	};

	struct  GrowthMaterial
	{
		std::vector<GrowthMaterialType> MaterialTypes;
		std::string Name;
		int Id;
	};

	struct GrowthPatchObject
	{
		GrowthMaterialType* MaterialType;
		osg::Vec3 Color;
		osg::Matrix Transform;
		float Dist;
		float Alpha;
		osg::BoundingSphere BoudingSphere;

	};

	typedef std::vector<GrowthPatchObject> GrowthPatchObjectVector;
	typedef std::map<std::string,GrowthPatchObjectVector> GrowthObjectMap;

	struct GrowthPatch
	{
		osg::BoundingBox BoundingBox;
		osg::BoundingSphere BoudingSphere;
		osg::Vec3 Pos;
		GrowthObjectMap ObjectMap;
	};

	class PagedGrowth
	{
	public:
		PagedGrowth(void);
		~PagedGrowth(void);
		bool Load(const std::string &wst_file);
		int Init();
		//void Render(ICameraNode* cam);
		void PushPatch(GrowthPatch* patch);
		void SetSamplesInPatch(int value){m_SamplesInPatch = value;}
		void SetVisGridSize(int value){m_VisGridSize = value;}
		typedef std::vector<GrowthPatchObject*> GrowthPatchObjectPointerVector;
	private:
		int LoadXML(TiXmlElement   *xmlElem);
		//void AddToPatch(float pos_x,float pos_z,GrowthPatch *patch);
		void AddToPatch(float pos_x,float pos_z,GrowthPatch *patch,GrowthMaterial* mat, GrowthMaterialType* gmat);
		void RenderUnderGrowth(GrowthPatchObjectPointerVector* pov);
		void RenderOverGrowth(GrowthPatchObjectPointerVector* pov);
		float FadeOut(float dist, float start_fade, float span);
		float FadeIn(float dist, float start_fade, float span);

		std::string m_MaterialMapFilename;
		std::string m_Type;
		int m_MaterialMapSideSize;
		float m_Viewdistance;
		int m_VisGridSize;
		std::vector<GrowthMaterial> m_Materials;
		std::map<std::string,int> m_NameToId;
		std::vector<GrowthPatch> m_PatchVector;

		//RawFile m_GroundCover;
		int m_NumPatches;
		int m_SamplesInPatch;
		float m_AlphaRefOverHigh;
		float m_AlphaRefOverLow;

		typedef std::map<std::string,GrowthPatchObjectPointerVector> GrowthRenderMap;
		GrowthRenderMap m_RenderMap;
		float m_InvTerrainSize;


		int m_SetupMeshCalls;
		int m_DrawMeshCalls;
		bool m_OverGrowth;
		bool m_Sort;
		float m_AlphaRef;
		bool m_CheckToSame;
		bool m_CheckToOthers;
	};
}