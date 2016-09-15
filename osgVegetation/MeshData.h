#pragma once
#include "Common.h"
#include "MeshLayer.h"

namespace osgVegetation
{
	/**
		Struct holding mesh collection and settings common
		for all mesh layers used by the scattering stage
	*/
	struct MeshData
	{
		MeshData() : ReceiveShadows(false),
			UseMultiSample(false)
		{

		}

		/*
			Should geometry receive shadows or not.
		*/
		bool ReceiveShadows;

		/**
		Enable multi sampling
		*/
		bool UseMultiSample;


		/**
			The mesh layer collection
		*/
		MeshLayerVector Layers;
	};
}
