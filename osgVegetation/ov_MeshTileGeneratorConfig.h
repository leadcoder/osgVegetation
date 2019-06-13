#pragma once

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
			MeshLODConfig(const std::string& mesh, const osg::Vec4 &distance) : Mesh(mesh), Distance(distance) {}
			std::string Mesh;
			osg::Vec4 Distance;
		};

		MeshTypeConfig()
		{

		}
		std::vector<MeshLODConfig> MeshLODs;
	private:
	};

	class MeshLayerConfig
	{
	public:
		MeshLayerConfig(float density = 2, int lod_level = -1) : Density(density),
			TargetLODLevel(lod_level)
		{

		}
		float Density;
		int TargetLODLevel;
		std::vector<MeshTypeConfig> MeshTypes;
	private:
	};
}
