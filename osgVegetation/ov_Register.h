#pragma once
#include "ov_Common.h"
#include "ov_TextureUnits.h"
#include "ov_Scene.h"

namespace osgVegetation
{
	class GlobalRegister
	{
	public:
		TextureUnits TexUnits;
		SceneConfig Scene;
	};

	static GlobalRegister Register;
}