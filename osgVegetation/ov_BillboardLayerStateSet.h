#pragma once
#include "ov_Common.h"
#include <osg/Texture2DArray>
#include <osg/Multisample>
#include <osg/Program>
#include <osg/AlphaFunc>
#include <osg/BlendFunc>
#include <osg/Texture2D>
#include <osg/CullFace>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osg/PatchParameter>
#include "ov_Utils.h"
#include "ov_BillboardLayerConfig.h"
#include "ov_Register.h"

namespace osgVegetation
{
	class BillboardLayerStateSet : public osg::StateSet
	{
	public:
		BillboardLayerStateSet(const BillboardLayerConfig& data)
		{
			setDefine("OV_TERRAIN_TESSELLATION");

			const int billboard_tex_unit = Register.TexUnits.CreateOrGetUnit(OV_BILLBOARD_TEXTURE_ID);

			//apply filters
			data.Filter.Apply(this);

			addUniform(new osg::Uniform("ov_billboard_texture", billboard_tex_unit));
			addUniform(new osg::Uniform("ov_billboard_max_distance", data.MaxDistance));

			const double target_instance_area = 1.0 / data.Density;
			const float target_tri_side_lenght = static_cast<float>(GetEquilateralTriangleSideLengthFromArea(target_instance_area));

			addUniform(new osg::Uniform("ov_billboard_density", target_tri_side_lenght));
			addUniform(new osg::Uniform("ov_billboard_color_impact", data.ColorImpact));

			const int num_billboards = static_cast<int>(data.Billboards.size());
			osg::Uniform* numBillboards = new osg::Uniform("ov_num_billboards", num_billboards);
			addUniform(numBillboards);

			const int MAX_BILLBOARDS = 10;
			osg::Uniform *billboardUniform = new osg::Uniform(osg::Uniform::FLOAT_VEC4, "ov_billboard_data", MAX_BILLBOARDS);
			for (unsigned int i = 0; i < MAX_BILLBOARDS; ++i)
			{
				//Set default values
				billboardUniform->setElement(i, osg::Vec4f(1.0f, 1.0f, 1.0f, 1.0f));
			}

			for (unsigned int i = 0; i < data.Billboards.size(); ++i)
			{
				osg::Vec2 size = data.Billboards[i].Size;
				billboardUniform->setElement(i, osg::Vec4f(size.x(), size.y(), data.Billboards[i].Intensity, data.Billboards[i].Probability));
			}

			addUniform(billboardUniform);

			//need this for shadow pass
			osg::AlphaFunc* alphaFunc = new osg::AlphaFunc;
			alphaFunc->setFunction(osg::AlphaFunc::GEQUAL, data.AlphaRejectValue);
			setAttributeAndModes(alphaFunc, osg::StateAttribute::ON);

			if (false)
			{
				setAttributeAndModes(new osg::BlendFunc, osg::StateAttribute::ON);
			}
			else
			{
				setMode(GL_SAMPLE_ALPHA_TO_COVERAGE_ARB, 1);
				//stateset->setAttributeAndModes(new osg::BlendFunc, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
				//stateset->setAttributeAndModes(new osg::BlendFunc(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO), osg::StateAttribute::OVERRIDE);
			}
			//stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
			setTextureAttributeAndModes(billboard_tex_unit, _CreateTextureArray(data.Billboards), osg::StateAttribute::ON);
#if 0 //debug
			program->addShader(osg::Shader::readShaderFile(osg::Shader::VERTEX, osgDB::findDataFile("ov_billboard_vertex.glsl")));
			program->addShader(osg::Shader::readShaderFile(osg::Shader::TESSCONTROL, osgDB::findDataFile("ov_billboard_tess_ctrl.glsl")));
			program->addShader(osg::Shader::readShaderFile(osg::Shader::TESSEVALUATION, osgDB::findDataFile("ov_terrain_tess_eval.glsl")));
			program->addShader(osg::Shader::readShaderFile(osg::Shader::FRAGMENT, osgDB::findDataFile("ov_terrain_fragment.glsl")));
#else	

			osg::Program* program = new osg::Program();
			program->addShader(osg::Shader::readShaderFile(osg::Shader::VERTEX, osgDB::findDataFile("ov_terrain_vertex.glsl")));
			program->addShader(osg::Shader::readShaderFile(osg::Shader::VERTEX, osgDB::findDataFile("ov_terrain_elevation.glsl")));
			program->addShader(osg::Shader::readShaderFile(osg::Shader::VERTEX, osgDB::findDataFile("ov_common_vertex.glsl")));
			program->addShader(osg::Shader::readShaderFile(osg::Shader::TESSCONTROL, osgDB::findDataFile("ov_billboard_tess_ctrl.glsl")));
			program->addShader(osg::Shader::readShaderFile(osg::Shader::TESSEVALUATION, osgDB::findDataFile("ov_billboard_tess_eval.glsl")));
			program->addShader(osg::Shader::readShaderFile(osg::Shader::GEOMETRY, osgDB::findDataFile("ov_common_vertex.glsl")));
			program->addShader(osg::Shader::readShaderFile(osg::Shader::GEOMETRY, osgDB::findDataFile("ov_terrain_elevation.glsl")));
			program->addShader(osg::Shader::readShaderFile(osg::Shader::GEOMETRY, osgDB::findDataFile("ov_terrain_color.glsl")));
			program->addShader(osg::Shader::readShaderFile(osg::Shader::GEOMETRY, osgDB::findDataFile("ov_terrain_pass_filter.glsl")));
			program->addShader(osg::Shader::readShaderFile(osg::Shader::GEOMETRY, osgDB::findDataFile("ov_billboard_geometry.glsl")));
			program->addShader(osg::Shader::readShaderFile(osg::Shader::FRAGMENT, osgDB::findDataFile("ov_common_fragment.glsl")));
			program->addShader(osg::Shader::readShaderFile(osg::Shader::FRAGMENT, osgDB::findDataFile("ov_billboard_fragment.glsl")));

			setAttribute(program, osg::StateAttribute::PROTECTED | osg::StateAttribute::ON);

			//program->setParameter(GL_GEOMETRY_OUTPUT_TYPE_EXT, GL_TRIANGLE_STRIP);

			std::string blt_type;
			if (data.Type == BillboardLayerConfig::BLT_ROTATED_QUAD)
			{
				//program->setParameter(GL_GEOMETRY_VERTICES_OUT_EXT, 4);
				blt_type = "BLT_ROTATED_QUAD";
			}
			else if (data.Type == BillboardLayerConfig::BLT_CROSS_QUADS)
			{
				//program->setParameter(GL_GEOMETRY_VERTICES_OUT_EXT, 8);
				blt_type = "BLT_CROSS_QUADS";
			}
			else if (data.Type == BillboardLayerConfig::BLT_GRASS)
			{
				//program->setParameter(GL_GEOMETRY_VERTICES_OUT_EXT, 16);
				blt_type = "BLT_GRASS";
			}

			osg::StateSet::DefineList& defineList = getDefineList();
			defineList[blt_type].second = (osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

#endif
			setAttribute(new osg::PatchParameter(3));
			setAttributeAndModes(new osg::CullFace(), osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
		}

		BillboardLayerStateSet(const BillboardLayerStateSet& rhs, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY) : osg::StateSet(rhs, copyop)
		{

		}
		virtual Object* cloneType() const { return new osg::StateSet(); }
		virtual Object* clone(const osg::CopyOp& copyop) const { return new BillboardLayerStateSet(*this, copyop); }
private:

		osg::ref_ptr<osg::Texture2DArray> _CreateTextureArray(const std::vector<BillboardLayerConfig::Billboard> &textrues)
		{
			//Load textures
			const osg::ref_ptr<osgDB::ReaderWriter::Options> options = new osgDB::ReaderWriter::Options();
			//options->setOptionString("dds_flip");
			osg::ref_ptr<osg::Texture2DArray> tex = new osg::Texture2DArray;

			for (size_t i = 0; i < textrues.size(); i++)
			{
				const std::string texture_name = textrues[i].TextureName;
				options->setOptionString("dds_flip");
				osg::Image* image = osgDB::readImageFile(texture_name, options);
				if (image == NULL)
					OSGV_EXCEPT(std::string("BillboardLayerConfig::CreateTextureArray - Failed to load texture:" + texture_name).c_str());
				if (i == 0) // first image decide array size
				{
					tex->setTextureSize(image->s(), image->t(), textrues.size());
					tex->setUseHardwareMipMapGeneration(true);
				}
				tex->setImage(i, image);
			}
			return tex;
		}
	};

	class BillboardLayerEffect : public osg::Group
	{
	public:
		BillboardLayerEffect(const BillboardLayerConfig &config)
		{
			setStateSet(new BillboardLayerStateSet(config));
			int node_mask = 0x1;
			if (config.CastShadow)
				node_mask |= Register.Scene.Shadow.CastsShadowTraversalMask;
			if (config.ReceiveShadow)
				node_mask |= Register.Scene.Shadow.ReceivesShadowTraversalMask;
			setNodeMask(node_mask);
		}
		BillboardLayerEffect(const BillboardLayerEffect& rhs, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY) : osg::Group(rhs, copyop)
		{

		}
		virtual Object* cloneType() const { return new osg::Group(); }
		virtual Object* clone(const osg::CopyOp& copyop) const { return new BillboardLayerEffect(*this, copyop); }
	};

#if 0	

	class BillboardEffectFactory
	{
	public:
		static osg::ref_ptr <osg::Group> Create(const BillboardNodeGeneratorConfig &config, osg::ref_ptr <osg::Node> terrain_node)
		{
			osg::ref_ptr<osg::Group> layers = new osg::Group();
			for (size_t i = 0; i < config.Layers.size(); i++)
			{
				//int billboard_tex_unit = config.BillboardTexUnit >= 0 ? config.BillboardTexUnit : BillboardNodeGenerator::_GetFirstFreeTexUnit(config.TerrainTextureUnits);
				osg::ref_ptr<osg::Group> layer_node = new BillboardLayerEffect(config.Layers[i], config.BillboardTexUnit);
				layer_node->addChild(terrain_node);

				//Disable shadow casting for grass, TODO make this optional
				if (config.Layers[i].Type == BillboardLayerConfig::BLT_GRASS)
					layer_node->setNodeMask(config.ReceivesShadowTraversalMask);
				else
					layer_node->setNodeMask(config.ReceivesShadowTraversalMask | config.CastShadowTraversalMask);

				layers->addChild(layer_node);
			}
			return layers;
		}
	};
#endif
}
