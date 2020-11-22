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
			IntensityVariation(0.2f),
			Scale(1.0f),
			ScaleVariation(0),
			DiffuseIntensity(1.0f)
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
			Enable(true),
			CastShadow(true),
			ReceiveShadow(true),
			BackFaceCulling(false),
			DynamicLODMaxDistanceRatio(3.0f)
		{

		}
		bool Enable;
		float Density;
		bool CastShadow;
		bool ReceiveShadow;
		bool BackFaceCulling;
		float DynamicLODMaxDistanceRatio;
		PassFilter Filter;
		std::vector<MeshTypeConfig> MeshTypes;
	private:
	};
}
