#pragma once
#include "ov_Common.h"
#include <osg/Fog>

namespace osgVegetation
{
	enum FogModeEnum
	{
		FM_LINEAR = osg::Fog::LINEAR,
		FM_EXP = osg::Fog::EXP,
		FM_EXP2 = osg::Fog::EXP2,
		FM_DISABLED = 4
	};

	class FogSettings
	{
	public:
		FogSettings() : Mode(FM_DISABLED) {}
		FogModeEnum Mode;

		static std::string ModeToString(FogModeEnum mode)
		{
			std::string fog_mode_str;
			switch (mode)
			{
			case FM_DISABLED:
				fog_mode_str = "FM_DISABLED";
				break;
			case FM_LINEAR:
				fog_mode_str = "FM_LINEAR";
				break;
			case FM_EXP:
				fog_mode_str = "FM_EXP";
				break;
			case FM_EXP2:
				fog_mode_str = "FM_EXP2";
				break;

			}
			return fog_mode_str;
		}

		static FogModeEnum StringToMode(const std::string &mode_str)
		{
			if (mode_str == "FM_DISABLED")
				return FM_DISABLED;
			if (mode_str == "FM_LINEAR")
				return FM_LINEAR;
			if (mode_str == "FM_EXP")
				return FM_EXP;
			if (mode_str == "FM_EXP2")
				return FM_EXP2;
			return FM_DISABLED;
		}

		void Apply(osg::StateSet* state_set)
		{
			if (Mode != FM_DISABLED)
			{
				const std::string fog_mode_str = ModeToString(Mode);
				state_set->setDefine(fog_mode_str);
			}
		}
	};
}