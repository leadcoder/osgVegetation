#pragma once
#include "Common.h"
#include <osg/Vec4>
#include <map>
#include <vector>
#include "CoverageColor.h"
#include <sstream>

namespace osgVegetation
{
	class CoverageData
	{
	public:
		struct CoverageMaterial
		{
			CoverageMaterial(const std::string &name, const CoverageColor& color, CoverageColor tolerance = CoverageColor(0,0,0,0)):Name(name),
				Tolerance(tolerance)
			{
				Colors.push_back(color);
			}
			std::string Name;
			std::vector<CoverageColor> Colors;
			CoverageColor Tolerance;

			bool hasColor(const CoverageColor& color) const
			{
				//std::cout << "color:" << color.x() << " color2:" << Colors[0].x() << "tol:" << Tolerance.z();

				for(size_t i = 0 ; i < Colors.size(); i++)
				{
					if (fabs(Colors[i].x() - color.x()) <= Tolerance.x() &&
							fabs(Colors[i].y() - color.y()) <= Tolerance.y() &&
							fabs(Colors[i].z() - color.z()) <= Tolerance.z())
								return true;
					/* 
					if(Colors[i] == color)
						return true;
					*/
				}
				return false;
			}
		};

		std::vector<CoverageMaterial> CoverageMaterials;

		CoverageMaterial getCoverageMaterial(const std::string &name)
		{
			for(size_t i = 0; i < CoverageMaterials.size(); i++)
			{
				if(CoverageMaterials[i].Name == name)
				{
					return CoverageMaterials[i];
				}
			}
			OSGV_EXCEPT(std::string("CoverageMaterial::getCoverageMaterial - Failed to find material:" + name).c_str());
		}

		std::string getCoverageMaterialName(const CoverageColor& color)
		{
			for(size_t i = 0; i < CoverageMaterials.size(); i++)
			{
				if(CoverageMaterials[i].hasColor(color))
				{
					return CoverageMaterials[i].Name;
				}
			}
			//cast exception?
			return "";
		}
	};
}
