#pragma once
#include "ov_Common.h"
#include <osg/Texture2DArray>
#include <osg/Multisample>
#include <osg/Program>
#include <osg/AlphaFunc>
#include <osg/BlendFunc>
#include <osg/Texture2D>
#include <osg/Multisample>
#include <osg/CullFace>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osg/PatchParameter>
#include <osgDB/ReadFile>
#include "ov_Utils.h"
#include "ov_BillboardLayer.h"

namespace osgVegetation
{
	class BillboardNodeGenerator
	{
	public:
		BillboardNodeGenerator(const std::vector<BillboardLayer> &layers) : m_Config(layers),
			m_TerrainColorTexUnit(0),
			m_TerrainLandcoverTexUnit(1),
			m_BillboardTexUnit(2),
			m_ReceivesShadowTraversalMask(0x1)
		{
			for(size_t i = 0; i < layers.size(); i++)
				m_Layers.push_back(_CreateStateSet(layers[i]));
		}

		osg::ref_ptr<osg::Node> CreateNode(osg::Node* terrain) const
		{
			osg::ref_ptr<osg::Group> layers = new osg::Group();
			for (size_t i = 0; i < m_Layers.size(); i++)
			{
				osg::ref_ptr<osg::Group> layer_node = new osg::Group();
				layer_node->setStateSet(m_Layers[i]);
				layer_node->addChild(terrain);
				
				//Disable shadow casting for grass, TODO make this optional
				if (m_Config[i].Type == BillboardLayer::BLT_GRASS)
					layer_node->setNodeMask(m_ReceivesShadowTraversalMask);
				layers->addChild(layer_node);
			}
			return layers;
		}
	private:
		osg::StateSet* _CreateStateSet(const BillboardLayer& data)
		{
			osg::StateSet* stateset = new osg::StateSet();
			osg::Program* program = new osg::Program;
			//stateset->setAttribute(program);
			stateset->setAttribute(program, osg::StateAttribute::PROTECTED | osg::StateAttribute::ON);
			stateset->addUniform(new osg::Uniform("ov_color_texture", m_TerrainColorTexUnit));
			stateset->addUniform(new osg::Uniform("ov_billboard_texture", m_BillboardTexUnit));
			stateset->addUniform(new osg::Uniform("ov_billboard_max_distance", data.MaxDistance));

			const double target_instance_area = 1.0 / data.Density;
			const float target_tri_side_lenght = static_cast<float>(GetEquilateralTriangleSideLengthFromArea(target_instance_area));

			stateset->addUniform(new osg::Uniform("ov_billboard_density", target_tri_side_lenght));
			stateset->addUniform(new osg::Uniform("ov_billboard_color_threshold", data.ColorThreshold));
			stateset->addUniform(new osg::Uniform("ov_billboard_color_impact", data.ColorImpact));
			const bool use_land_cover = data.LandCoverID >= 0;
			if(use_land_cover)
			{
				stateset->addUniform(new osg::Uniform("ov_land_cover_texture", m_TerrainLandcoverTexUnit));
				stateset->addUniform(new osg::Uniform("ov_billboard_land_cover_id", data.LandCoverID));
			}

			int num_billboards = static_cast<int>(data.Billboards.size());
			osg::Uniform* numBillboards = new osg::Uniform("ov_num_billboards", num_billboards);
			stateset->addUniform(numBillboards);

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

			stateset->addUniform(billboardUniform);
			
			//need this for shadow pass
			osg::AlphaFunc* alphaFunc = new osg::AlphaFunc;
			alphaFunc->setFunction(osg::AlphaFunc::GEQUAL, data.AlphaRejectValue);
			stateset->setAttributeAndModes(alphaFunc, osg::StateAttribute::ON);

			if (false)
			{
				stateset->setAttributeAndModes(new osg::BlendFunc, osg::StateAttribute::ON);
			}
			else
			{
				stateset->setMode(GL_SAMPLE_ALPHA_TO_COVERAGE_ARB, 1);
				//stateset->setAttributeAndModes(new osg::BlendFunc, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
				//stateset->setAttributeAndModes(new osg::BlendFunc(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO), osg::StateAttribute::OVERRIDE);
			}
			//stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
			stateset->setTextureAttributeAndModes(m_BillboardTexUnit, _CreateTextureArray(data.Billboards), osg::StateAttribute::ON);
#if 0 //debug
			program->addShader(osg::Shader::readShaderFile(osg::Shader::VERTEX, osgDB::findDataFile("ov_billboard_vertex.glsl")));
			program->addShader(osg::Shader::readShaderFile(osg::Shader::TESSCONTROL, osgDB::findDataFile("ov_billboard_tess_ctrl.glsl")));
			program->addShader(osg::Shader::readShaderFile(osg::Shader::TESSEVALUATION, osgDB::findDataFile("ov_terrain_tess_eval.glsl")));
			program->addShader(osg::Shader::readShaderFile(osg::Shader::FRAGMENT, osgDB::findDataFile("ov_terrain_fragment.glsl")));
#else			
			program->addShader(osg::Shader::readShaderFile(osg::Shader::VERTEX, osgDB::findDataFile("ov_billboard_vertex.glsl")));
			program->addShader(osg::Shader::readShaderFile(osg::Shader::TESSCONTROL, osgDB::findDataFile("ov_billboard_tess_ctrl.glsl")));
			program->addShader(osg::Shader::readShaderFile(osg::Shader::TESSEVALUATION, osgDB::findDataFile("ov_billboard_tess_eval.glsl")));
			program->addShader(osg::Shader::readShaderFile(osg::Shader::GEOMETRY, osgDB::findDataFile("ov_common_vertex.glsl")));
			program->addShader(osg::Shader::readShaderFile(osg::Shader::GEOMETRY, osgDB::findDataFile("ov_billboard_geometry.glsl")));
			program->addShader(osg::Shader::readShaderFile(osg::Shader::FRAGMENT, osgDB::findDataFile("ov_common_fragment.glsl")));
			program->addShader(osg::Shader::readShaderFile(osg::Shader::FRAGMENT, osgDB::findDataFile("ov_billboard_fragment.glsl")));

			//program->setParameter(GL_GEOMETRY_OUTPUT_TYPE_EXT, GL_TRIANGLE_STRIP);

			std::string blt_type;
			if (data.Type == BillboardLayer::BLT_ROTATED_QUAD)
			{
				//program->setParameter(GL_GEOMETRY_VERTICES_OUT_EXT, 4);
				blt_type = "BLT_ROTATED_QUAD";
			}
			else if (data.Type == BillboardLayer::BLT_CROSS_QUADS)
			{
				//program->setParameter(GL_GEOMETRY_VERTICES_OUT_EXT, 8);
				blt_type = "BLT_CROSS_QUADS";
			}
			else if (data.Type == BillboardLayer::BLT_GRASS)
			{
				//program->setParameter(GL_GEOMETRY_VERTICES_OUT_EXT, 16);
				blt_type = "BLT_GRASS";
			}

			osg::StateSet::DefineList& defineList = stateset->getDefineList();
			defineList[blt_type].second = (osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
			if (use_land_cover)
				defineList["USE_LANDCOVER"].second = (osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
#endif
			stateset->setAttribute(new osg::PatchParameter(3));
			stateset->setAttributeAndModes(new osg::CullFace(), osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
			return stateset;
		}

		osg::ref_ptr<osg::Texture2DArray> _CreateTextureArray(const std::vector<BillboardLayer::Billboard> &textrues) const
		{
			//Load textures
			const osg::ref_ptr<osgDB::ReaderWriter::Options> options = new osgDB::ReaderWriter::Options();
			//options->setOptionString("dds_flip");
			osg::ref_ptr<osg::Texture2DArray> tex = new osg::Texture2DArray;

			for (size_t i = 0; i < textrues.size(); i++)
			{
				const std::string texture_name = textrues[i].TextureName;
				//const osg::ref_ptr<osgDB::ReaderWriter::Options> new_options = new osgDB::ReaderWriter::Options();
				options->setOptionString("dds_flip");
				osg::Image* image = osgDB::readImageFile(texture_name, options);
				if (image == NULL)
					OSGV_EXCEPT(std::string("BillboardLayer::CreateTextureArray - Failed to load texture:" + texture_name).c_str());
				if (i == 0) // first image decide array size
				{
					tex->setTextureSize(image->s(), image->t(), textrues.size());
					tex->setUseHardwareMipMapGeneration(true);
				}
				tex->setImage(i, image);
			}
			return tex;
		}

		std::vector<osg::ref_ptr<osg::StateSet> > m_Layers;
		std::vector<BillboardLayer> m_Config;

		int m_TerrainLandcoverTexUnit;
		int m_TerrainColorTexUnit;
		int m_BillboardTexUnit;
		int m_ReceivesShadowTraversalMask;
	};
}
