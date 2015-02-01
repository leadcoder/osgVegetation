#pragma once
#include "Common.h"
#include "CoverageColor.h"
#include "MeshObject.h"
#include <osg/Referenced>
#include <osg/Vec4>
#include <osg/Vec3>
#include <osg/Vec2>
#include <osg/Vec4ub>
#include <osg/ref_ptr>

namespace osgVegetation
{
	struct MeshLOD
	{
		MeshLOD(const std::string &mesh, double max_dist):MeshName(mesh),
			MaxDistance(max_dist){}
		/**
			Max Distance this Mesh is visible at
		*/
		double MaxDistance;

		/**
			Mesh file for this LOD
		*/
		std::string MeshName;

		/*
			Internal data holding the quad tree level where this mesh should be injected
		*/
		int _StartQTLevel;

	};
	typedef std::vector<MeshLOD> MeshLODVector;

	/*
		Mesh layer holding Mesh LOD vector and
	*/
	struct MeshLayer
	{
		MeshLayer(const MeshLODVector &mesh_lods) : MeshLODs(mesh_lods)
		{

		}
		double Density;

		MeshLODVector MeshLODs;

		/**
			Color intensity interval
		*/
		osg::Vec2 ColorIntensity;

		/**
			Percentage to of ground texture color/intensity to use for model coloring
		*/
		double TerrainColorRatio;

		/**
			Only use intensity from ground texture (i.e (r+g+b)/3.0) when generating model color.
			This is false by the fault which means that each color
			channel from the ground texture is respected.
		*/
		bool UseTerrainIntensity;

		/**
			Height interval (x=min, y=max) for all models populated in this layers
		*/
		osg::Vec2 Height;

		/**
			Width interval (x=min, y=max) for all models populated in this layers
		*/
		osg::Vec2 Width;

		/**
			Overall scale interval (x=min, y=max) for all models populated in this layers (will effect both height and width)
		*/
		osg::Vec2 Scale;

		/**
			Coverage materials that control scattering for this layer
		*/
		std::vector<std::string> CoverageMaterials;

		/**
			Helper function to check is this layer hold coverage material
		*/
		bool hasCoverage(const std::string& name) const
		{
			for(size_t i = 0 ; i < CoverageMaterials.size(); i++)
			{
				if(CoverageMaterials[i] == name)
					return true;
			}
			return false;
		}

		/*
			Internal data used during scattering
		*/
		MeshVegetationObjectVector _Instances;
	};

	typedef std::vector<MeshLayer> MeshLayerVector;
}
