#pragma once
#include "Common.h"

#include <osg/PositionAttitudeTransform>
#include <osg/PatchParameter>
#include <osg/Program>
#include <osg/AlphaFunc>
#include <osg/BlendFunc>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>

#include <iostream>
#include "VegetationUtils.h"


namespace osgVegetation
{
	class Billboard
	{
	public:
		Billboard(const std::string &texture, const osg::Vec2f &size) : TextureName(texture),
			Size(size)
		{

		}

		~Billboard()
		{

		}
		osg::Vec2f Size;
		std::string TextureName;
	private:
	};

	class VegetationData
	{
	public:
		VegetationData(float max_dist = 150, float density = 8, int lod_level = -1) : MaxDistance(max_dist),
			Density(density), LODLevel(lod_level)
		{

			//m_TexArray = osgVegetation::Utils::loadTextureArray(tex_names);
		}

		~VegetationData()
		{

		}

		float Density;
		float MaxDistance;
		int LODLevel;
		std::vector<Billboard> Billboards;

		osg::ref_ptr<osg::Texture2DArray> _BillboardTextures;
		osg::ref_ptr<osg::Texture2DArray> GetOrCreateTexArray()
		{
			if (_BillboardTextures.valid())
				return _BillboardTextures;
			std::vector<std::string> tex_names;
			for (size_t i = 0; i < Billboards.size(); i++)
			{
				tex_names.push_back(Billboards[i].TextureName);
			}
			_BillboardTextures = osgVegetation::Utils::loadTextureArray(tex_names);
			return _BillboardTextures;
		}

		static void PrepareVegLayer(osg::StateSet* stateset, VegetationData& data)
		{
			osg::Program* program = new osg::Program;
			stateset->setAttribute(program);
			osg::Uniform* baseTextureSampler = new osg::Uniform("baseTexture", 0);
			stateset->addUniform(baseTextureSampler);
			osg::Uniform* vegTextureSampler = new osg::Uniform("vegTexture", 1);
			stateset->addUniform(vegTextureSampler);


			stateset->addUniform(new osg::Uniform("vegMaxDistance", data.MaxDistance));
			stateset->addUniform(new osg::Uniform("vegDensity", data.Density));

			int num_billboards = static_cast<int>(data.Billboards.size());
			osg::Uniform* numBillboards = new osg::Uniform("numBillboards", num_billboards);
			stateset->addUniform(numBillboards);

			const int MAX_BILLBOARDS = 10;
			osg::Uniform *billboardUniform = new osg::Uniform(osg::Uniform::FLOAT_VEC4, "billboardData", MAX_BILLBOARDS);
			for (unsigned int i = 0; i < MAX_BILLBOARDS; ++i)
			{
				//Setdefault values
				billboardUniform->setElement(i, osg::Vec4f(1.0f, 1.0f, 1.0f, 1.0f));
			}

			for (unsigned int i = 0; i < data.Billboards.size(); ++i)
			{
				osg::Vec2 size = data.Billboards[i].Size;
				billboardUniform->setElement(i, osg::Vec4f(size.x(), size.y(), 1.0f, 1.0f));
			}

			stateset->addUniform(billboardUniform);
			osg::AlphaFunc* alphaFunc = new osg::AlphaFunc;
			alphaFunc->setFunction(osg::AlphaFunc::GEQUAL, 0.9);
			stateset->setAttributeAndModes(alphaFunc, osg::StateAttribute::ON);

			//if (data.UseAlphaBlend)
			{
				stateset->setAttributeAndModes(new osg::BlendFunc, osg::StateAttribute::ON);
				stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
			}

			stateset->setTextureAttributeAndModes(1, data.GetOrCreateTexArray(), osg::StateAttribute::ON);
#if 0 //debug
			program->addShader(osg::Shader::readShaderFile(osg::Shader::VERTEX, osgDB::findDataFile("shaders/terrain_vertex.glsl")));
			program->addShader(osg::Shader::readShaderFile(osg::Shader::TESSCONTROL, osgDB::findDataFile("shaders/terrain_tess_ctrl.glsl")));
			program->addShader(osg::Shader::readShaderFile(osg::Shader::TESSEVALUATION, osgDB::findDataFile("shaders/terrain_tess_eval.glsl")));
			program->addShader(osg::Shader::readShaderFile(osg::Shader::FRAGMENT, osgDB::findDataFile("shaders/terrain_fragment.glsl")));
#else			
			program->addShader(osg::Shader::readShaderFile(osg::Shader::VERTEX, osgDB::findDataFile("shaders/veg_vertex.glsl")));
			program->addShader(osg::Shader::readShaderFile(osg::Shader::TESSCONTROL, osgDB::findDataFile("shaders/veg_tess_ctrl.glsl")));
			program->addShader(osg::Shader::readShaderFile(osg::Shader::TESSEVALUATION, osgDB::findDataFile("shaders/veg_tess_eval.glsl")));
			program->addShader(osg::Shader::readShaderFile(osg::Shader::GEOMETRY, osgDB::findDataFile("shaders/veg_geometry.glsl")));
			program->addShader(osg::Shader::readShaderFile(osg::Shader::FRAGMENT, osgDB::findDataFile("shaders/veg_fragment.glsl")));
			program->setParameter(GL_GEOMETRY_VERTICES_OUT_EXT, 4);
			program->setParameter(GL_GEOMETRY_OUTPUT_TYPE_EXT, GL_TRIANGLE_STRIP);
#endif			
			stateset->setAttribute(new osg::PatchParameter(3));
		}
	private:
	};

	class GeomConvert
	{
	public:
		GeomConvert() {}

		static osg::Geode* ToTriPatches(osg::HeightField* hf)
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

			osg::DrawElementsUShort& drawElements = *(new osg::DrawElementsUShort(GL_PATCHES, 2 * 3 * (numColumns*numRows)));
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
			osg::Geode* geode = new osg::Geode();
			geode->addDrawable(geometry);
			return geode;
		}

		static osg::Node* ToTriPatchGeometry(osgTerrain::TerrainTile* tile)
		{
			osg::Node* node = NULL;
			osgTerrain::HeightFieldLayer* layer = dynamic_cast<osgTerrain::HeightFieldLayer*>(tile->getElevationLayer());
			if (layer)
			{
				osg::HeightField* hf = layer->getHeightField();
				if (hf)
				{
					osg::Geode* hf_geom = osgVegetation::GeomConvert::ToTriPatches(hf);
				
					//Add color texture
					osgTerrain::Layer* colorLayer = tile->getColorLayer(0);
					if (colorLayer)
					{
						osg::Image* image = colorLayer->getImage();
						osg::Texture2D* texture = new osg::Texture2D(image);
						hf_geom->getOrCreateStateSet()->setTextureAttributeAndModes(0, texture, osg::StateAttribute::ON);
					}
					osg::PositionAttitudeTransform* pat = new osg::PositionAttitudeTransform();
					pat->setPosition(hf->getOrigin());
					pat->addChild(hf_geom);
					node = pat;
				}
			}
			return node;
		}

	private:
	};

	
}
