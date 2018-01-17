#pragma once
#include <string>
#include <vector>

class XBFVegetationData
{
public:
	class XBFLOD
	{
	public:
		XBFLOD(const std::string& mesh, float max_dist = -1) : Mesh(mesh),
			MaxDist(max_dist) {}
		std::string Mesh;
		float MaxDist;
	};

	XBFVegetationData(float max_dist = 1000, int density = 2, int lod_level = -1, float fade_dist = 30) : MaxDistance(max_dist),
		Density(density),
		TargetLODLevel(lod_level),
		FadeDistance(fade_dist)
	{

	}

	~XBFVegetationData()
	{

	}

	int Density;
	float MaxDistance;
	float FadeDistance;
	int TargetLODLevel;
	std::vector<XBFLOD> MeshLODs;
private:
};
