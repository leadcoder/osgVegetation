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

	XBFVegetationData(float max_dist = 1000, int density = 2, int lod_level = -1) : MaxDistance(max_dist),
		Density(density),
		TargetLODLevel(lod_level)
	{

	}

	~XBFVegetationData()
	{

	}

	int Density;
	float MaxDistance;
	int TargetLODLevel;
	std::vector<XBFLOD> MeshLODs;
private:
};
