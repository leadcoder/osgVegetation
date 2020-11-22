#pragma once
#include "ov_Common.h"
#include "ov_TerrainShadingStateSet.h"
#include "ov_XMLUtils.h"
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
		TerrainSplatShadingConfig() : TerrainShadingStateSetConfig(), MaxDistance(1000) , SplatTexture(Register.TexUnits.GetUnit(OV_TERRAIN_SPLAT_TEXTURE_ID)),
			ColorModulateRatio(0), 
			DetailTextureUnit(Register.TexUnits.GetUnit(OV_TERRAIN_DETAIL_TEXTURE_ID)) {}
		std::vector<DetailLayer> DetailLayers;
		int DetailTextureUnit;
		TextureConfig SplatTexture;
		float MaxDistance;
		float ColorModulateRatio;

		static TerrainSplatShadingConfig ReadXML(osgDB::XmlNode* xmlNode)
		{
			TerrainSplatShadingConfig splat_config;
			QueryFloatAttribute(xmlNode, "MaxDistance", splat_config.MaxDistance);
			QueryFloatAttribute(xmlNode, "ColorModulateRatio", splat_config.ColorModulateRatio);
			QueryStringAttribute(xmlNode, "ColorTexture", splat_config.ColorTexture.File);
			QueryIntAttribute(xmlNode, "ColorTextureUnit", splat_config.ColorTexture.TexUnit);
			QueryStringAttribute(xmlNode, "SplatTexture", splat_config.SplatTexture.File);
			QueryIntAttribute(xmlNode, "SplatTextureUnit", splat_config.SplatTexture.TexUnit);
			QueryStringAttribute(xmlNode, "NoiseTexture", splat_config.NoiseTexture.File);
			QueryIntAttribute(xmlNode, "NoiseTextureUnit", splat_config.NoiseTexture.TexUnit);
			osgDB::XmlNode* dlsNode = getFirstNodeByName(xmlNode, "DetailLayers");
			if (dlsNode)
				splat_config.DetailLayers = _ReadDetailLayers(dlsNode);
			return splat_config;
		}
	private:
		static inline DetailLayer _ReadDetailLayer(osgDB::XmlNode* xmlNode)
		{
			std::string texture;
			if (!QueryStringAttribute(xmlNode, "Texture", texture))
				throw std::runtime_error(std::string("Serializer::loadDetailLayer - Failed to find attribute: Texture").c_str());

			DetailLayer layer(texture);
			QueryFloatAttribute(xmlNode, "Scale", layer.Scale);
			return layer;
		}

		static inline std::vector<DetailLayer> _ReadDetailLayers(osgDB::XmlNode* xmlNode)
		{
			std::vector<osgVegetation::DetailLayer> layers;
			for (unsigned int i = 0; i < xmlNode->children.size(); ++i)
			{
				if (xmlNode->children[i]->name == "DetailLayer")
				{
					DetailLayer dl = _ReadDetailLayer(xmlNode->children[i].get());
					layers.push_back(dl);
				}
			}
			return layers;
		}
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
			//auto generate id
			if (config.TexUnit < 0 && config.Texture || config.File != "")
			{
				config.TexUnit = Register.TexUnits.CreateOrGetUnit(OV_TERRAIN_SPLAT_TEXTURE_ID);
			}

			_ApplyTextureConfig(config);
			if (config.TexUnit >= 0)
			{
				addUniform(new osg::Uniform("ov_splat_texture", config.TexUnit));
			}
		}

		void _SetupSplatMapping(const TerrainSplatShadingConfig& config)
		{
			const bool has_detail = config.DetailLayers.size() > 0;
			if (has_detail)
			{
				int d_unit = config.DetailTextureUnit < 0 ? Register.TexUnits.CreateOrGetUnit(OV_TERRAIN_DETAIL_TEXTURE_ID) : config.DetailTextureUnit;
				setDefine("OV_TERRAIN_SPLAT_MAPPING");
				_SetSplatTexture(config.SplatTexture);
				osg::Vec4 scale(1, 1, 1, 1);
				osg::ref_ptr<osg::Texture2DArray> tex = _CreateTextureArray(config.DetailLayers);
				setTextureAttributeAndModes(d_unit, tex, osg::StateAttribute::ON);
				addUniform(new osg::Uniform("ov_splat_max_distance", config.MaxDistance));
				addUniform(new osg::Uniform("ov_splat_color_modulate_ratio", config.ColorModulateRatio));
				
				addUniform(new osg::Uniform("ov_splat_detail_texture", d_unit));
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
			int tex_size = 1;
			for (size_t i = 0; i < detail_layers.size(); i++)
			{
				const std::string texture_name = detail_layers[i].DetailTexture;
				osg::Image* image = osgDB::readImageFile(texture_name);
				if (image == NULL)
					OSGV_EXCEPT(std::string("TerrainShading::_CreateTextureArray - Failed to load texture:" + texture_name).c_str());
				
				if (i == 0) // first image decide texture dim
				{
					tex_size = image->s();
					tex->setTextureSize(tex_size, tex_size, detail_layers.size());
				}
				
				if(image->s() != tex_size || image->t() != tex_size)
				{
					image->scaleImage(tex_size, tex_size, 1);
				}
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