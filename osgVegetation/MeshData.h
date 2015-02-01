#pragma once
#include "Common.h"
#include "MeshLayer.h"
#include <osg/Referenced>
#include <osg/Vec4>
#include <osg/Vec3>
#include <osg/Vec2>
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
			The mesh layer collection
		*/
		MeshLayerVector Layers;
	};
}
