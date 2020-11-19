#pragma once
#include "ov_Common.h"
#include "ov_TerrainStateSet.h"
#include <osg/Vec2>
#include <osg/Program>
#include <osg/Texture2D>
#include <osg/Texture2DArray>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <vector>

namespace osgVegetation
{
	class TerrainShadingStateSetConfig : public TerrainStateSetConfig
	{
	public:
		TerrainShadingStateSetConfig() : TerrainStateSetConfig(), UseTessellation(false), 
			NoiseTexture(Register.TexUnits.GetUnit(OV_TERRAIN_NOISE_TEXTURE_ID))
		{
			
		}
		TextureConfig NoiseTexture;
		bool UseTessellation;
	};

	class TerrainShadingStateSet : public TerrainStateSet
	{
	public:
		TerrainShadingStateSet(const TerrainShadingStateSetConfig& config) : TerrainStateSet(config)
		{
			if (config.UseTessellation)
				setDefine("OV_TERRAIN_TESSELLATION");

			_SetNoiseTexture(config.NoiseTexture);
			

			osg::Program* program = _CreateProgram(config);
			setAttribute(program, osg::StateAttribute::PROTECTED | osg::StateAttribute::ON);
		}

	private:
		osg::Program* _CreateProgram(const TerrainShadingStateSetConfig& config)
		{
			osg::Program* program = new osg::Program;
			program->addShader(osg::Shader::readShaderFile(osg::Shader::VERTEX, osgDB::findDataFile("ov_terrain_vertex.glsl")));
			program->addShader(osg::Shader::readShaderFile(osg::Shader::VERTEX, osgDB::findDataFile("ov_terrain_elevation.glsl")));
			program->addShader(osg::Shader::readShaderFile(osg::Shader::VERTEX, osgDB::findDataFile("ov_shadow_vertex.glsl")));
			if (config.UseTessellation)
			{
				program->addShader(osg::Shader::readShaderFile(osg::Shader::TESSCONTROL, osgDB::findDataFile("ov_terrain_tess_ctrl.glsl")));
				program->addShader(osg::Shader::readShaderFile(osg::Shader::TESSEVALUATION, osgDB::findDataFile("ov_shadow_vertex.glsl")));
				program->addShader(osg::Shader::readShaderFile(osg::Shader::TESSEVALUATION, osgDB::findDataFile("ov_terrain_elevation.glsl")));
				program->addShader(osg::Shader::readShaderFile(osg::Shader::TESSEVALUATION, osgDB::findDataFile("ov_terrain_tess_eval.glsl")));
			}
			program->addShader(osg::Shader::readShaderFile(osg::Shader::FRAGMENT, osgDB::findDataFile("ov_common_fragment.glsl")));
			program->addShader(osg::Shader::readShaderFile(osg::Shader::FRAGMENT, osgDB::findDataFile("ov_shadow_fragment.glsl")));
			program->addShader(osg::Shader::readShaderFile(osg::Shader::FRAGMENT, osgDB::findDataFile("ov_terrain_color.glsl")));
			program->addShader(osg::Shader::readShaderFile(osg::Shader::FRAGMENT, osgDB::findDataFile("ov_terrain_fragment.glsl")));
			return program;
		}

		void _SetNoiseTexture(TextureConfig config)
		{
			//auto generate id
			if (config.TexUnit < 0 && config.Texture || config.File != "")
			{
				config.TexUnit = Register.TexUnits.CreateOrGetUnit(OV_TERRAIN_NOISE_TEXTURE_ID);
			}

			_ApplyTextureConfig(config);
			if (config.TexUnit >= 0)
			{
				addUniform(new osg::Uniform("ov_noise_texture", config.TexUnit));
				setDefine("OV_NOISE_TEXTURE");
			}
		}
	};

	class TerrainShadingEffect : public osg::Group
	{
	public:
		TerrainShadingEffect(const TerrainShadingStateSetConfig& config)
		{
			osg::ref_ptr<osg::StateSet> state_set = new TerrainShadingStateSet(config);
			setStateSet(state_set);
		}
	};
}