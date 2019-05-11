#pragma once
#include "ov_Common.h"
#include <osg/StateSet>
#include <sstream>

namespace osgVegetation
{
	struct PassFilter
	{
		std::string SplatFilter;
		std::string ColorFilter;
		std::string NormalFilter;

		void Apply(osg::StateSet *state_set) const
		{
			if (SplatFilter != "")
			{
				state_set->setDefine("OV_SPLAT_FILTER(splat_color)", SplatFilter);
			}

			if (ColorFilter != "")
			{
				state_set->setDefine("OV_COLOR_FILTER(base_color)", ColorFilter);
			}
		}

		std::string static GenerateSplatFilter(osg::Vec4 threshold, std::string color_operator = "<")
		{
			std::stringstream ss;
			if (threshold.x() > 0)
			{
				ss << "if((splat_color.x) "  << color_operator  << threshold.x() << ") return false;";
			}

			if (threshold.y() > 0)
			{
				ss << "if((splat_color.y) "<< color_operator << threshold.y() << ") return false;";
			}

			if (threshold.z() > 0)
			{
				ss << "if((splat_color.z) " << color_operator << threshold.z() << ") return false;";
			}

			if (threshold.w() > 0)
			{
				ss << "if((splat_color.w) " << color_operator << threshold.w() << ") return false;";
			}
			return ss.str();
		}

		std::string static GenerateSplatFilter(int detail_layer, float reject_threshold = 0.0, float include_threshold = 0.0)
		{
			std::stringstream ss;
			if (detail_layer == 0)
			{
				ss << "if((splat_color.x) > " << reject_threshold << ") return false;";
				ss << "if((splat_color.y) > " << reject_threshold << ") return false;";
				ss << "if((splat_color.z) > " << reject_threshold << ") return false;";
				ss << "if((splat_color.w) > " << reject_threshold << ") return false;";
			}
			if (detail_layer == 1)
			{
				ss << "if((splat_color.x) < " << include_threshold << ") return false;";
				ss << "if((splat_color.y) > " << reject_threshold << ") return false;";
				ss << "if((splat_color.z) > " << reject_threshold << ") return false;";
				ss << "if((splat_color.w) > " << reject_threshold << ") return false;";
			}

			if (detail_layer == 2)
			{
				ss << "if((splat_color.y) < " << include_threshold << ") return false;";
				ss << "if((splat_color.z) > " << reject_threshold << ") return false;";
				ss << "if((splat_color.w) > " << reject_threshold << ") return false;";
			}

			if (detail_layer == 3)
			{
				ss << "if((splat_color.z) < " << include_threshold << ") return false;";
				ss << "if((splat_color.w) > " << reject_threshold << ") return false;";
			}

			if (detail_layer == 4)
			{
				ss << "if((splat_color.w) < " << include_threshold << ") return false;";
			}
			return ss.str();
		}
	};
}
