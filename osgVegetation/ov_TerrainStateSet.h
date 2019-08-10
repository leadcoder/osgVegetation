#pragma once
#include "ov_Common.h"
#include "ov_TextureConfig.h"
#include <osg/Vec2>
#include <osg/Program>
#include <osg/Texture2D>
#include <osg/Texture2DArray>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <vector>

namespace osgVegetation
{
	class TerrainStateSetConfig
	{
	public:
		TextureConfig ColorTexture;
		TextureConfig NormalTexture;
		TextureConfig ElevationTexture;
	};

	class TerrainStateSet : public osg::StateSet
	{
	public:
		TerrainStateSet(const TerrainStateSetConfig& config)
		{
			SetColorTexture(config.ColorTexture);
			SetElevationTexture(config.ElevationTexture);
			SetNormalTexture(config.NormalTexture);
		}

		void _ApplyTextureConfig(TextureConfig config)
		{
			if (config.TexUnit >= 0)
			{
				if (config.File != "")
				{
					osg::ref_ptr<osg::Image> color_image = osgDB::readRefImageFile(config.File);
					if (color_image)
					{
						osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
						texture->setImage(color_image);
						texture->setWrap(osg::Texture::WRAP_S, osg::Texture::REPEAT);
						texture->setWrap(osg::Texture::WRAP_T, osg::Texture::REPEAT);
						setTextureAttributeAndModes(config.TexUnit, texture, osg::StateAttribute::ON);
					}
				}
				else if (config.Texture)
				{
					setTextureAttributeAndModes(config.TexUnit, config.Texture, osg::StateAttribute::ON);
				}
			}
		}

		void SetColorTexture(TextureConfig config)
		{
			if (config.TexUnit >= 0)
			{
				_ApplyTextureConfig(config);
				addUniform(new osg::Uniform("ov_color_texture", config.TexUnit));
				setDefine("OV_TERRAIN_COLOR_TEXTURE");
			}
		}

		void SetElevationTexture(TextureConfig config)
		{
			if (config.TexUnit >= 0)
			{
				_ApplyTextureConfig(config);
				addUniform(new osg::Uniform("ov_elevation_texture", config.TexUnit));
				setDefine("OV_TERRAIN_ELEVATION_TEXTURE");
			}
		}

		void SetNormalTexture(TextureConfig config)
		{
			if (config.TexUnit >= 0)
			{
				_ApplyTextureConfig(config);
				addUniform(new osg::Uniform("ov_normal_texture", config.TexUnit));
				setDefine("OV_TERRAIN_NORMAL_TEXTURE");
			}
		}
	};
}