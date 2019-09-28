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
		ShadowSettings() : ReceivesShadowTraversalMask(0x1),
			CastsShadowTraversalMask(0x2), Mode(SM_DISABLED){}
		ShadowModeEnum Mode;
		int ReceivesShadowTraversalMask;
		int CastsShadowTraversalMask;
	};
}