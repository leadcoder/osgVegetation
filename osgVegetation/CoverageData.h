#pragma once
#include "Common.h"
#include <osg/Vec4>
#include <string>
#include <map>
#include <vector>
#include "CoverageColor.h"

namespace osgVegetation
{
	class CoverageData
	{
	public:
		struct CoverageMaterial
		{
			CoverageMaterial(const std::string &name, const CoverageColor& color):Name(name)
			{
				Colors.push_back(color);
			}
			std::string Name;
			std::vector<CoverageColor> Colors;

			bool hasColor(const CoverageColor& color) const
			{
				for(size_t i = 0 ; i < Colors.size(); i++)
				{
					if(Colors[i] == color)
						return true;
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
