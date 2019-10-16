#pragma once
#include "ov_ILayerConfig.h"
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

		MeshTypeConfig() : Probability(0), 
			IntensityVariation(0.2),
			Scale(1.0),
			ScaleVariation(0),
			DiffuseIntensity(1.0)
		{

		}
		std::vector<MeshLODConfig> MeshLODs;
		float Probability;
		float IntensityVariation;
		float Scale;
		float ScaleVariation;
		float DiffuseIntensity;
	private:
	};

	class MeshLayerConfig : public ILayerConfig
	{
	public:
		MeshLayerConfig(float density = 2) : Density(density),
			CastShadow(true),
			ReceiveShadow(true)
		{

		}
		float Density;
		bool CastShadow;
		bool ReceiveShadow;
		PassFilter Filter;
		std::vector<MeshTypeConfig> MeshTypes;
	private:
	};
}
