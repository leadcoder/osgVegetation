#pragma once

#include "ov_Common.h"
#include "ov_Terrain.h"
#include <osg/Program>
#include <osg/Texture2D>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>

namespace osgVegetation
{
	class TerrainShading
	{
	public:
		TerrainShading(const TerrainShadingConfiguration& config)
		{
			m_StateSet = new osg::StateSet();
			_SetupTerrainTextures(m_StateSet, config.TextureUnits);
			if (config.UseTessellation)
				m_StateSet->setDefine("OV_TERRAIN_TESSELLATION");

			osg::Program* program = new osg::Program;
			//m_StateSet->setAttribute(program);
			m_StateSet->setAttribute(program, osg::StateAttribute::PROTECTED | osg::StateAttribute::ON);

			program->addShader(osg::Shader::readShaderFile(osg::Shader::VERTEX, osgDB::findDataFile("ov_terrain_vertex.glsl")));
			program->addShader(osg::Shader::readShaderFile(osg::Shader::VERTEX, osgDB::findDataFile("ov_common_vertex.glsl")));
			if (config.UseTessellation)
			{
				program->addShader(osg::Shader::readShaderFile(osg::Shader::TESSCONTROL, osgDB::findDataFile("ov_terrain_tess_ctrl.glsl")));
				program->addShader(osg::Shader::readShaderFile(osg::Shader::TESSEVALUATION, osgDB::findDataFile("ov_common_vertex.glsl")));
				program->addShader(osg::Shader::readShaderFile(osg::Shader::TESSEVALUATION, osgDB::findDataFile("ov_terrain_tess_eval.glsl")));
			}
			//else
			//	program->addShader(osg::Shader::readShaderFile(osg::Shader::VERTEX, osgDB::findDataFile("ov_common_vertex.glsl")));

			program->addShader(osg::Shader::readShaderFile(osg::Shader::FRAGMENT, osgDB::findDataFile("ov_common_fragment.glsl")));
			program->addShader(osg::Shader::readShaderFile(osg::Shader::FRAGMENT, osgDB::findDataFile("ov_terrain_detail_texturing.glsl")));
			program->addShader(osg::Shader::readShaderFile(osg::Shader::FRAGMENT, osgDB::findDataFile("ov_terrain_fragment.glsl")));
		
			const bool has_detail = config.DetailLayers.size() > 0;
			if (has_detail && config.TextureUnits.SplatTextureUnit > 0 && config.TextureUnits.DetailTextureUnit > 0)
			{
				m_StateSet->addUniform(new osg::Uniform("ov_land_cover_texture", config.TextureUnits.SplatTextureUnit));
				m_StateSet->setDefine("OV_TERRAIN_DETAIL_TEXTURING");
				osg::Vec4 scale(1, 1, 1, 1);
				int detail_tex_unit_index = config.TextureUnits.DetailTextureUnit;
				for (size_t i = 0; i < config.DetailLayers.size(); i++)
				{
					osg::Texture2D *dtex = new osg::Texture2D;
					dtex->setWrap(osg::Texture2D::WRAP_S, osg::Texture2D::REPEAT);
					dtex->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::REPEAT);
					dtex->setImage(osgDB::readRefImageFile(config.DetailLayers[i].DetailTexture));
					m_StateSet->setTextureAttributeAndModes(detail_tex_unit_index, dtex, osg::StateAttribute::ON);
					std::stringstream ss_tex;
					ss_tex << "ov_detail_texture" << i;
					m_StateSet->addUniform(new osg::Uniform(ss_tex.str().c_str(), detail_tex_unit_index));
					std::stringstream ss_scale;
					detail_tex_unit_index++;
					if (i < 4)
						scale[i] = config.DetailLayers[i].Scale;
				}
				m_StateSet->addUniform(new osg::Uniform("ov_detail_scale", scale));
			}
		}

		/*osg::ref_ptr<osg::Group> Create(osg::ref_ptr<osg::Node> terrain_geometry)
		{
			osg::ref_ptr<osg::Group> terrain_layer = new osg::Group();
			terrain_layer->setStateSet(m_StateSet);
			//add geometry
			terrain_layer->addChild(terrain_geometry);
			return terrain_layer;
		}*/
		osg::ref_ptr<osg::StateSet> GetStateSet() const 
		{
			return m_StateSet;
		}

		void  Apply(osg::ref_ptr<osg::StateSet> state_set) const
		{
			state_set->merge(*m_StateSet);
		}
	private:
		void _SetupTerrainTextures(osg::ref_ptr<osg::StateSet> state_set, const TerrainTextureUnitSettings &config)
		{
			if (config.ColorTextureUnit > -1)
			{
				state_set->addUniform(new osg::Uniform("ov_color_texture", config.ColorTextureUnit));
				state_set->setDefine("OV_TERRAIN_COLOR_TEXTURE");
			}

			if (config.SplatTextureUnit > -1)
			{
				state_set->addUniform(new osg::Uniform("ov_land_cover_texture", config.SplatTextureUnit));
				state_set->setDefine("OV_TERRAIN_SPLAT_TEXTURE");
			}

			if (config.ElevationTextureUnit > -1)
			{
				state_set->addUniform(new osg::Uniform("ov_elevation_texture", config.ElevationTextureUnit));
				state_set->setDefine("OV_TERRAIN_ELEVATION_TEXTURE");
			}
		}

		osg::ref_ptr<osg::StateSet> m_StateSet;
	};

	void PrepareTerrainForTesselation(osg::ref_ptr<osg::Node> terrain)
	{
		osg::Program* program = new osg::Program;
		terrain->getOrCreateStateSet()->setAttribute(program);

		program->addShader(osg::Shader::readShaderFile(osg::Shader::VERTEX, osgDB::findDataFile("ov_terrain_vertex.glsl")));
		
		program->addShader(osg::Shader::readShaderFile(osg::Shader::TESSCONTROL, osgDB::findDataFile("ov_terrain_tess_ctrl.glsl")));
		program->addShader(osg::Shader::readShaderFile(osg::Shader::TESSEVALUATION, osgDB::findDataFile("ov_terrain_tess_eval.glsl")));
		program->addShader(osg::Shader::readShaderFile(osg::Shader::TESSEVALUATION, osgDB::findDataFile("ov_common_vertex.glsl")));
		program->addShader(osg::Shader::readShaderFile(osg::Shader::FRAGMENT, osgDB::findDataFile("ov_common_fragment.glsl")));
		program->addShader(osg::Shader::readShaderFile(osg::Shader::FRAGMENT, osgDB::findDataFile("ov_terrain_fragment.glsl")));
		
		terrain->getOrCreateStateSet()->addUniform(new osg::Uniform("ov_color_texture", 0));
	}

	void ApplyTerrainShading(osg::Node* terrain, const TerrainShadingConfiguration& tdm)
	{
		TerrainShading terrain_shading(tdm);
		terrain_shading.Apply(terrain->getOrCreateStateSet());

#if 0
		osg::Program* program = new osg::Program;
		terrain->getOrCreateStateSet()->setAttribute(program, osg::StateAttribute::PROTECTED | osg::StateAttribute::ON);
		terrain->getOrCreateStateSet()->addUniform(new osg::Uniform("ov_color_texture", 0));
		terrain->getOrCreateStateSet()->addUniform(new osg::Uniform("ov_land_cover_texture", 1));

		int tex_unit_index = 2;

		osg::Vec4 scale(1, 1, 1, 1);
		for (size_t i = 0; i < tdm.DetailLayers.size(); i++)
		{
			osg::Texture2D *dtex = new osg::Texture2D;
			dtex->setWrap(osg::Texture2D::WRAP_S, osg::Texture2D::REPEAT);
			dtex->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::REPEAT);
			dtex->setImage(osgDB::readRefImageFile(tdm.DetailLayers[i].DetailTexture));
			terrain->getOrCreateStateSet()->setTextureAttributeAndModes(tex_unit_index, dtex, osg::StateAttribute::ON);
			std::stringstream ss_tex;
			ss_tex << "ov_detail_texture" << i;
			terrain->getOrCreateStateSet()->addUniform(new osg::Uniform(ss_tex.str().c_str(), tex_unit_index));
			std::stringstream ss_scale;
			tex_unit_index++;
			if (i < 4)
				scale[i] = tdm.DetailLayers[i].Scale;
		}

		terrain->getOrCreateStateSet()->addUniform(new osg::Uniform("ov_detail_scale", scale));

		program->addShader(osg::Shader::readShaderFile(osg::Shader::VERTEX, osgDB::findDataFile("ov_common_vertex.glsl")));
		program->addShader(osg::Shader::readShaderFile(osg::Shader::VERTEX, osgDB::findDataFile("ov_terrain_vertex.glsl")));
		//program->addShader(osg::Shader::readShaderFile(osg::Shader::VERTEX, osgDB::findDataFile(tdm.VertexShader)));

		program->addShader(osg::Shader::readShaderFile(osg::Shader::FRAGMENT, osgDB::findDataFile("ov_common_fragment.glsl")));
		program->addShader(osg::Shader::readShaderFile(osg::Shader::FRAGMENT, osgDB::findDataFile("ov_terrain_detail_texturing.glsl")));
		//program->addShader(osg::Shader::readShaderFile(osg::Shader::FRAGMENT, osgDB::findDataFile(tdm.FragmentShader)));
		program->addShader(osg::Shader::readShaderFile(osg::Shader::FRAGMENT, osgDB::findDataFile("ov_terrain_fragment.glsl")));
#endif
	}

	//Setup terrain for simple detail mapping. Note: Sample data path must be added!
	/*void PrepareTerrainForDetailMapping(osg::Node* terrain)
	{
		TerrainShadingConfiguration tsc;
		//tdm.VertexShader = "ov_terrain_detail_vertex.glsl";
		//tdm.FragmentShader = "ov_terrain_detail_fragment.glsl";
		tsc.DetailLayers.push_back(DetailLayer(std::string("terrain/detail/detail_grass_mossy.dds"),0.1));
		tsc.DetailLayers.push_back(DetailLayer(std::string("terrain/detail/detail_dirt.dds"), 0.1));
		tsc.DetailLayers.push_back(DetailLayer(std::string("terrain/detail/detail_grass_mossy.dds"), 0.1));
		tsc.DetailLayers.push_back(DetailLayer(std::string("terrain/detail/detail_dirt.dds"), 0.1));
		ApplyTerrainShading(terrain, tsc);
	}*/

	double GetEquilateralTriangleSideLengthFromArea(double area)
	{
		return  sqrt(area / (sqrt(3.0) / 4.0));
	}
	

	class ConvertToPatchesVisitor : public osg::NodeVisitor
	{
	public:

		ConvertToPatchesVisitor() :
			osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) {
		}

		void apply(osg::Node& node)
		{
			osg::Geometry* geometry = dynamic_cast<osg::Geometry*>(&node);
			if (geometry)
			{
				geometry->setUseDisplayList(false);
				for (unsigned int i = 0; i < geometry->getNumPrimitiveSets() ; i++)
				{
					geometry->getPrimitiveSet(i)->setMode(GL_PATCHES);
				}
			}
			else
			{
				traverse(node);
			}
		}
	};

	void ConvertToPatches(osg::Node* node)
	{
		ConvertToPatchesVisitor visitor;
		node->accept(visitor);
	}

	osg::Geometry* CreateGeometryFromHeightField(osg::HeightField* hf)
	{
		unsigned int numColumns = hf->getNumColumns();
		unsigned int numRows = hf->getNumRows();
		float columnCoordDelta = hf->getXInterval();
		float rowCoordDelta = hf->getYInterval();

		osg::Geometry* geometry = new osg::Geometry;

		osg::Vec3Array& v = *(new osg::Vec3Array(numColumns*numRows));
		osg::Vec2Array& t = *(new osg::Vec2Array(numColumns*numRows));
		osg::Vec4ubArray& color = *(new osg::Vec4ubArray(1));
		color[0].set(255, 255, 255, 255);
		float rowTexDelta = 1.0f / (float)(numRows - 1);
		float columnTexDelta = 1.0f / (float)(numColumns - 1);
		osg::Vec3 local_origin(0, 0, 0);

		osg::Vec3 pos(local_origin.x(), local_origin.y(), local_origin.z());
		osg::Vec2 tex(0.0f, 0.0f);
		int vi = 0;
		for (unsigned int r = 0; r < numRows; ++r)
		{
			pos.x() = local_origin.x();
			tex.x() = 0.0f;
			for (unsigned int c = 0; c < numColumns; ++c)
			{
				float h = hf->getHeight(c, r);
				v[vi].set(pos.x(), pos.y(), h);
				t[vi].set(tex.x(), tex.y());
				pos.x() += columnCoordDelta;
				tex.x() += columnTexDelta;
				++vi;
			}
			pos.y() += rowCoordDelta;
			tex.y() += rowTexDelta;
		}

		geometry->setVertexArray(&v);
		geometry->setTexCoordArray(0, &t);
		geometry->setColorArray(&color, osg::Array::BIND_OVERALL);

		//osg::DrawElementsUShort& drawElements = *(new osg::DrawElementsUShort(GL_PATCHES, 2 * 3 * (numColumns*numRows)));
		osg::DrawElementsUShort& drawElements = *(new osg::DrawElementsUShort(GL_TRIANGLES, 2 * 3 * (numColumns*numRows)));
		geometry->addPrimitiveSet(&drawElements);
		int ei = 0;
		for (unsigned int r = 0; r < numRows - 1; ++r)
		{
			for (unsigned int c = 0; c < numColumns - 1; ++c)
			{
				// Try to imitate how GeometryTechnique::generateGeometry optimize 
				// which way to put the diagonal by choosing to
				// place it between the two corners that have the least curvature
				// relative to each other.
				// Due to how normals are calculated we don't get exact match...fix this by using same normal calulations

				osg::Vec3 n00 = hf->getNormal(c, r);
				osg::Vec3 n01 = hf->getNormal(c, r + 1);
				osg::Vec3 n10 = hf->getNormal(c + 1, r);
				osg::Vec3 n11 = hf->getNormal(c + 1, r + 1);
				float dot_00_11 = n00 * n11;
				float dot_01_10 = n01 * n10;
				if (dot_00_11 > dot_01_10)
				{
					drawElements[ei++] = (r)*numColumns + c;
					drawElements[ei++] = (r)*numColumns + c + 1;
					drawElements[ei++] = (r + 1)*numColumns + c + 1;

					drawElements[ei++] = (r + 1)*numColumns + c + 1;
					drawElements[ei++] = (r + 1)*numColumns + c;
					drawElements[ei++] = (r)*numColumns + c;
				}
				else
				{
					drawElements[ei++] = (r)*numColumns + c;
					drawElements[ei++] = (r)*numColumns + c + 1;
					drawElements[ei++] = (r + 1)*numColumns + c;

					drawElements[ei++] = (r)*numColumns + c + 1;
					drawElements[ei++] = (r + 1)*numColumns + c + 1;
					drawElements[ei++] = (r + 1)*numColumns + c;
				}
			}
		}
		geometry->setUseDisplayList(false);
		return geometry;
	}
}