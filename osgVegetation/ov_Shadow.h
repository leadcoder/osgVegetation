#pragma once
#include "ov_Common.h"

namespace osgVegetation
{
	enum ShadowModeEnum
	{
		SM_DISABLED,
		SM_UNDEFINED,
		SM_LISPSM,
		SM_VDSM1, //one texture
		SM_VDSM2, //two textures
	};

	class ShadowSettings
	{
	public:
		ShadowSettings() : ReceivesShadowTraversalMask(0x2),
			CastsShadowTraversalMask(0x4), Mode(SM_DISABLED){}
		ShadowModeEnum Mode;
		unsigned int ReceivesShadowTraversalMask;
		unsigned int CastsShadowTraversalMask;

		static std::string ModeToString(ShadowModeEnum mode)
		{
			std::string shadow_mode_str;
			switch (mode)
			{
			case SM_DISABLED:
				shadow_mode_str = "SM_DISABLED";
				break;
			case SM_UNDEFINED:
				shadow_mode_str = "SM_UNDEFINED";
				break;
			case SM_LISPSM:
				shadow_mode_str = "SM_LISPSM";
				break;
			case SM_VDSM1:
				shadow_mode_str = "SM_VDSM1";
				break;
			case SM_VDSM2:
				shadow_mode_str = "SM_VDSM2";
				break;
			}
			return shadow_mode_str;
		}

		static ShadowModeEnum StringToMode(const std::string &mode_str)
		{
			if (mode_str == "SM_DISABLED")
				return SM_DISABLED;
			if (mode_str == "SM_UNDEFINED")
				return SM_UNDEFINED;
			if (mode_str == "SM_LISPSM")
				return SM_LISPSM;
			if (mode_str == "SM_VDSM1")
				return SM_VDSM1;
			if (mode_str == "SM_VDSM2")
				return SM_VDSM2;
			return SM_UNDEFINED;
		}

		void Apply(osg::StateSet* state_set)
		{
			if (Mode != SM_DISABLED && Mode != SM_UNDEFINED)
			{
				const std::string shadow_mode_str = ModeToString(Mode);
				state_set->setDefine(shadow_mode_str);
			}
		}
	};
}