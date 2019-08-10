#pragma once

#include "ov_PassFilter.h"
#include <string>
#include <vector>

namespace osgVegetation
{
	class MeshTypeConfig
	{
	public:

		class MeshLODConfig
		{
		public:
			MeshLODConfig() :
				Distance(0, 0, 0, 0),
				Type(0),
				Intensity(1.0) {}

			MeshLODConfig(const std::string& mesh, const osg::Vec4 &distance, unsigned int type = 0, float intensity = 1.0) : Mesh(mesh),
				Distance(distance),
				Type(type),
				Intensity(intensity) {}
			std::string Mesh;
			osg::Vec4 Distance;
			int Type;
			float Intensity;
		};

		MeshTypeConfig() : Probability(0)
		{

		}
		std::vector<MeshLODConfig> MeshLODs;
		float Probability;
	private:
	};

	class MeshLayerConfig
	{
	public:
		MeshLayerConfig(float density = 2) : Density(density)
		{

		}
		float Density;
		PassFilter Filter;
		std::vector<MeshTypeConfig> MeshTypes;
	private:
	};
}
