#pragma once
#include "ov_Common.h"
#include "ov_Register.h"
#include <osg/Texture2D>
#include <osg/Texture2DArray>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>

namespace osgVegetation
{
	class TextureConfig
	{
	public:
		TextureConfig(int tex_unit = -1) : TexUnit(tex_unit) {}
		TextureConfig(std::string filename, int tex_unit) : File(filename), TexUnit(tex_unit) {}
		TextureConfig(std::string filename, std::string texture_id) : File(filename), TexUnit(Register.TexUnits.CreateOrGetUnit(texture_id)) {}
		TextureConfig(osg::ref_ptr<osg::Texture2D> texture, int tex_unit) : Texture(texture), TexUnit(tex_unit) {}
		int TexUnit;
		std::string File;
		osg::ref_ptr<osg::Texture2D> Texture;
	};
}