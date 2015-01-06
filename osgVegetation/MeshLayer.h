#pragma once
#include "MaterialColor.h"
#include "MeshObject.h"
#include <osg/Referenced>
#include <osg/vec4>
#include <osg/vec3>
#include <osg/vec2>
#include <osg/Vec4ub>
#include <osg/ref_ptr>

namespace osgVegetation
{
	struct MeshLOD
	{
		MeshLOD(const std::string &mesh, double max_dist):MeshName(mesh),
			MaxDistance(max_dist){}
		double MaxDistance;
		std::string MeshName;
		int _StartQTLODLevel;
		
	};
	typedef std::vector<MeshLOD> MeshLODVector;
	

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
			Percentage to of ground texture color to use for billboard coloring
		*/
		double MixInColorRatio;
		/**
			Only use intensity from ground texture when color billboard
		*/
		bool MixInIntensity;
		
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
	
		std::vector<MaterialColor> Materials;

		bool hasMaterial(const MaterialColor& mat) const
		{
			for(size_t i = 0 ; i < Materials.size(); i++)
			{
				if(Materials[i] == mat)
					return true;
			}
			return false;
		}

		MeshVegetationObjectVector _Instances;
	};

	typedef std::vector<MeshLayer> MeshLayerVector;
}