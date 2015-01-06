#pragma once
#include "MeshLayer.h"
#include <osg/Referenced>
#include <osg/vec4>
#include <osg/vec3>
#include <osg/vec2>
#include <osg/Vec4ub>
#include <osg/ref_ptr>

namespace osgVegetation
{
	/**
		Struct holding mesh collection and settings common 
		for all mesh layers used by the scattering stage
	*/
	struct MeshData
	{
		MeshData() : ReceiveShadows(false)
		{

		}

		/*
			Should geometry receive shadows or not. 
		*/
		bool ReceiveShadows;
		
		/**
			The billboard collection
		*/
		MeshLayerVector Layers;
	};
}