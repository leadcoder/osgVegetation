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
}