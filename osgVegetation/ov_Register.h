#pragma once
#include "ov_Common.h"
#include "ov_TextureUnits.h"

namespace osgVegetation
{
	class GlobalRegister
	{
	public:
		GlobalRegister() : ReceivesShadowTraversalMask(0x2),
			CastsShadowTraversalMask(0x4) {}

		TextureUnits TexUnits;
		unsigned int ReceivesShadowTraversalMask;
		unsigned int CastsShadowTraversalMask;
	};

	extern GlobalRegister Register;
}