#pragma once
#include "ov_Common.h"
#include <osg/Vec2>
#include <osg/Program>
#include <osg/Texture2D>
#include <osg/Texture2DArray>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <vector>

namespace osgVegetation
{
	class TextureConfig
	{
	public:
		TextureConfig(int tex_unit = -1) : TexUnit(tex_unit) {}
		TextureConfig(std::string filename, int tex_unit) : File(filename), TexUnit(tex_unit) {}
		TextureConfig(osg::ref_ptr<osg::Texture2D> texture, int tex_unit) : Texture(texture), TexUnit(tex_unit) {}
		int TexUnit;
		std::string File;
		osg::ref_ptr<osg::Texture2D> Texture;
	};

	class TerrainStateSetConfig
	{
	public:
		TextureConfig ColorTexture;
		TextureConfig SplatTexture;
		TextureConfig ElevationTexture;
	};

	class DetailLayer
	{
	public:
		DetailLayer(const std::string &texture, float scale = 1) : DetailTexture(texture), Scale(scale) {}
		std::string DetailTexture;
		float Scale;
	};

	class TerrainShadingConfiguration : public TerrainStateSetConfig
	{
	public:
		TerrainShadingConfiguration() : DetailTextureUnit(-1),
			UseTessellation(false)
		{
			
		}
		std::vector<DetailLayer> DetailLayers;
		int DetailTextureUnit;
		TextureConfig NoiseTexture;
		bool UseTessellation;
	};

	class TerrainStateSet : public osg::StateSet
	{
	public:
		TerrainStateSet(const TerrainStateSetConfig& config)
		{
			SetColorTexture(config.ColorTexture);
			SetElevationTexture(config.ElevationTexture);
			SetSplatTexture(config.SplatTexture);
		}

		void _ApplyTextureConfig(TextureConfig config)
		{
			if (config.File != "")
			{
				osg::ref_ptr<osg::Image> color_image = osgDB::readRefImageFile(config.File);
				if (color_image)
				{
					osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
					texture->setImage(color_image);
					setTextureAttributeAndModes(config.TexUnit, texture, osg::StateAttribute::ON);
				}
			}
			else if(config.Texture)
			{
				setTextureAttributeAndModes(config.TexUnit, config.Texture, osg::StateAttribute::ON);
			}
		}

		void SetColorTexture(TextureConfig config)
		{
			_ApplyTextureConfig(config);
			if (config.TexUnit >= 0)
			{
				addUniform(new osg::Uniform("ov_color_texture", config.TexUnit));
				setDefine("OV_TERRAIN_COLOR_TEXTURE");
			}
		}

		void SetElevationTexture(TextureConfig config)
		{
			_ApplyTextureConfig(config);
			if (config.TexUnit >= 0)
			{
				addUniform(new osg::Uniform("ov_elevation_texture", config.TexUnit));
				setDefine("OV_TERRAIN_ELEVATION_TEXTURE");
			}
		}

		void SetSplatTexture(TextureConfig config)
		{
			_ApplyTextureConfig(config);
			if (config.TexUnit >= 0)
			{
				addUniform(new osg::Uniform("ov_splat_texture", config.TexUnit));
				setDefine("OV_TERRAIN_SPLAT_TEXTURE");
			}
		}
	};

	class TerrainShadingStateSet : public TerrainStateSet
	{
	public:
		TerrainShadingStateSet(const TerrainShadingConfiguration& config) : TerrainStateSet(config)
		{
			if (config.UseTessellation)
				setDefine("OV_TERRAIN_TESSELLATION");

			_SetNoiseTexture(config.NoiseTexture);
			_SetupDetailTextures(config);

			osg::Program* program = _CreateProgram(config);
			setAttribute(program, osg::StateAttribute::PROTECTED | osg::StateAttribute::ON);
		}

	private:
		osg::Program* _CreateProgram(const TerrainShadingConfiguration& config)
		{
			osg::Program* program = new osg::Program;
			program->addShader(osg::Shader::readShaderFile(osg::Shader::VERTEX, osgDB::findDataFile("ov_terrain_vertex.glsl")));
			program->addShader(osg::Shader::readShaderFile(osg::Shader::VERTEX, osgDB::findDataFile("ov_terrain_elevation.glsl")));
			program->addShader(osg::Shader::readShaderFile(osg::Shader::VERTEX, osgDB::findDataFile("ov_common_vertex.glsl")));
			if (config.UseTessellation)
			{
				program->addShader(osg::Shader::readShaderFile(osg::Shader::TESSCONTROL, osgDB::findDataFile("ov_terrain_tess_ctrl.glsl")));
				program->addShader(osg::Shader::readShaderFile(osg::Shader::TESSEVALUATION, osgDB::findDataFile("ov_common_vertex.glsl")));
				program->addShader(osg::Shader::readShaderFile(osg::Shader::TESSEVALUATION, osgDB::findDataFile("ov_terrain_elevation.glsl")));
				program->addShader(osg::Shader::readShaderFile(osg::Shader::TESSEVALUATION, osgDB::findDataFile("ov_terrain_tess_eval.glsl")));
			}
			program->addShader(osg::Shader::readShaderFile(osg::Shader::FRAGMENT, osgDB::findDataFile("ov_common_fragment.glsl")));
			program->addShader(osg::Shader::readShaderFile(osg::Shader::FRAGMENT, osgDB::findDataFile("ov_terrain_color.glsl")));
			program->addShader(osg::Shader::readShaderFile(osg::Shader::FRAGMENT, osgDB::findDataFile("ov_terrain_fragment.glsl")));
			return program;
		}

		void _SetNoiseTexture(TextureConfig config)
		{
			_ApplyTextureConfig(config);
			if (config.TexUnit >= 0)
			{
				addUniform(new osg::Uniform("ov_noise_texture", config.TexUnit));
				setDefine("OV_NOISE_TEXTURE");
			}
		}

		void _SetupDetailTextures(const TerrainShadingConfiguration& config)
		{
			
			/*if (config.NoiseTexture != "" && config.NoiseTextureUnit > -1)
			{
				osg::ref_ptr<osg::Image> noise_image = osgDB::readRefImageFile(config.NoiseTexture);
				if (noise_image)
				{
					osg::Texture2D* noise_tex = new osg::Texture2D;
					noise_tex->setWrap(osg::Texture2D::WRAP_S, osg::Texture2D::REPEAT);
					noise_tex->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::REPEAT);
					noise_tex->setImage(noise_image);
					setTextureAttributeAndModes(config.NoiseTextureUnit, noise_tex, osg::StateAttribute::ON);
					addUniform(new osg::Uniform("ov_noise_texture", config.NoiseTextureUnit));
					setDefine("OV_NOISE_TEXTURE");
				}
			}*/

			const bool has_detail = config.DetailLayers.size() > 0;
			if (has_detail && config.DetailTextureUnit > 0)
			{
				setDefine("OV_TERRAIN_DETAIL_TEXTURING");
				osg::Vec4 scale(1, 1, 1, 1);
				osg::ref_ptr<osg::Texture2DArray> tex = _CreateTextureArray(config.DetailLayers);
				setTextureAttributeAndModes(config.DetailTextureUnit, tex, osg::StateAttribute::ON);
				addUniform(new osg::Uniform("ov_detail_texture", config.DetailTextureUnit));
				int num_detail_textures = config.DetailLayers.size();
				addUniform(new osg::Uniform("ov_num_detail_textures", num_detail_textures));
				for (size_t i = 0; i < config.DetailLayers.size(); i++)
				{
					if (i < 4)
						scale[i] = config.DetailLayers[i].Scale;
				}
				addUniform(new osg::Uniform("ov_detail_scale", scale));
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

	class TerrainShadingEffect : public osg::Group
	{
	public:
		TerrainShadingEffect(const TerrainShadingConfiguration& config)
		{
			osg::ref_ptr<osg::StateSet> state_set = new TerrainShadingStateSet(config);
			setStateSet(state_set);
		}
	};
}