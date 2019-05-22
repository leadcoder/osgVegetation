#pragma once
#include "ov_Common.h"
#include "ov_TerrainShadingStateSet.h"
#include <osg/Vec2>
#include <osg/Program>
#include <osg/Texture2D>
#include <osg/Texture2DArray>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>

#include <vector>

namespace osgVegetation
{

	class DetailLayer
	{
	public:
		DetailLayer(const std::string &texture, float scale = 1) : DetailTexture(texture), Scale(scale) {}
		std::string DetailTexture;
		float Scale;
	};

	class TerrainSplatShadingConfig : public TerrainShadingStateSetConfig
	{
	public:
		TerrainSplatShadingConfig() : TerrainShadingStateSetConfig(), MaxDistance(1000) , ColorModulateRatio(0), DetailTextureUnit(-1) {}
		std::vector<DetailLayer> DetailLayers;
		int DetailTextureUnit;
		TextureConfig SplatTexture;
		float MaxDistance;
		float ColorModulateRatio;
	};

	class TerrainSplatShadingStateSet : public TerrainShadingStateSet
	{
	public:
		TerrainSplatShadingStateSet(const TerrainSplatShadingConfig& config) : TerrainShadingStateSet(config)
		{
			_SetupSplatMapping(config);
		}

	private:

		void _SetSplatTexture(TextureConfig config)
		{
			_ApplyTextureConfig(config);
			if (config.TexUnit >= 0)
			{
				addUniform(new osg::Uniform("ov_splat_texture", config.TexUnit));
				//setDefine("OV_TERRAIN_SPLAT_TEXTURE");
			}
		}

		void _SetupSplatMapping(const TerrainSplatShadingConfig& config)
		{
			const bool has_detail = config.DetailLayers.size() > 0;
			if (has_detail && config.DetailTextureUnit > 0)
			{
				setDefine("OV_TERRAIN_SPLAT_MAPPING");
				_SetSplatTexture(config.SplatTexture);
				osg::Vec4 scale(1, 1, 1, 1);
				osg::ref_ptr<osg::Texture2DArray> tex = _CreateTextureArray(config.DetailLayers);
				setTextureAttributeAndModes(config.DetailTextureUnit, tex, osg::StateAttribute::ON);
				addUniform(new osg::Uniform("ov_splat_max_distance", config.MaxDistance));
				addUniform(new osg::Uniform("ov_splat_color_modulate_ratio", config.ColorModulateRatio));
				
				addUniform(new osg::Uniform("ov_splat_detail_texture", config.DetailTextureUnit));
				int num_detail_textures = config.DetailLayers.size();
				addUniform(new osg::Uniform("ov_splat_num_detail_textures", num_detail_textures));
				for (size_t i = 0; i < config.DetailLayers.size(); i++)
				{
					if (i < 4)
						scale[i] = config.DetailLayers[i].Scale;
				}
				addUniform(new osg::Uniform("ov_splat_detail_scale", scale));
			}
		}

		osg::ref_ptr<osg::Texture2DArray> _CreateTextureArray(const std::vector<DetailLayer> &detail_layers)
		{
			const osg::ref_ptr<osgDB::ReaderWriter::Options> options = new osgDB::ReaderWriter::Options();
			osg::ref_ptr<osg::Texture2DArray> tex = new osg::Texture2DArray();

			tex->setWrap(osg::Texture::WRAP_S, osg::Texture::REPEAT);
			tex->setWrap(osg::Texture::WRAP_T, osg::Texture::REPEAT);
			tex->setUseHardwareMipMapGeneration(true);
			tex->setFilter(osg::Texture2DArray::MIN_FILTER, osg::Texture2DArray::LINEAR_MIPMAP_LINEAR);
			tex->setMaxAnisotropy(4.0f);

			for (size_t i = 0; i < detail_layers.size(); i++)
			{
				const std::string texture_name = detail_layers[i].DetailTexture;
				//options->setOptionString("dds_flip");
				//osg::Image* image = osgDB::readImageFile(texture_name, options);
				osg::Image* image = osgDB::readImageFile(texture_name);
				if (image == NULL)
					OSGV_EXCEPT(std::string("TerrainShading::_CreateTextureArray - Failed to load texture:" + texture_name).c_str());
				tex->setImage(i, image);
			}
			return tex;
		}
	};

	class TerrainSplatShadingEffect : public osg::Group
	{
	public:
		TerrainSplatShadingEffect(const TerrainSplatShadingConfig& config)
		{
			osg::ref_ptr<osg::StateSet> state_set = new TerrainSplatShadingStateSet(config);
			setStateSet(state_set);
		}
	};
}