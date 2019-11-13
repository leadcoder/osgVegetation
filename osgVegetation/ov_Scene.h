#pragma once
#include "ov_Common.h"
#include "ov_Shadow.h"
#include "ov_Fog.h"

namespace osgVegetation
{
	class SceneConfig
	{
	public:
		SceneConfig()
		{

		}
		ShadowSettings Shadow;
		FogSettings Fog;

		void Apply(osg::StateSet* state_set)
		{
			Shadow.Apply(state_set);
			Fog.Apply(state_set);
		}
	};
}