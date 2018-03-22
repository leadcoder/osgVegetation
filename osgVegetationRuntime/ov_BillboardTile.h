#pragma once

#include "ov_BillboardLayer.h"
#include <osg/PositionAttitudeTransform>
#include <osg/PatchParameter>
#include <osg/Program>
#include <osg/AlphaFunc>
#include <osg/BlendFunc>
#include <osg/Texture2D>
#include <osg/Multisample>
#include <osg/CullFace>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osgTerrain/TerrainTile>
#include <iostream>

namespace osgVegetation
{

	class BillboardLayerHelper
	{
	public:
		class ConvertToPatchesVisitor : public osg::NodeVisitor
		{
		public:

			ConvertToPatchesVisitor() :
				osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) {
			}

			void apply(osg::Node& node)
			{
				osg::Geometry* geom = dynamic_cast<osg::Geometry*>(&node);
				if (geom)
				{
					geom->setUseDisplayList(false);
					geom->getPrimitiveSet(0)->setMode(GL_PATCHES);
				}
				else
				{
					traverse(node);
				}
			}
		};


		static void PrepareVegLayer(osg::StateSet* stateset, BillboardLayer& data)
		{
			osg::Program* program = new osg::Program;
			//stateset->setAttribute(program);
			stateset->setAttribute(program, osg::StateAttribute::PROTECTED | osg::StateAttribute::ON);
			stateset->addUniform(new osg::Uniform("ov_color_texture", 0));
			stateset->addUniform(new osg::Uniform("ov_land_cover_texture", 1));

			stateset->addUniform(new osg::Uniform("ov_billboard_texture", 2));

			stateset->addUniform(new osg::Uniform("ov_billboard_max_distance", data.MaxDistance));
			stateset->addUniform(new osg::Uniform("ov_billboard_density", data.Density));
			stateset->addUniform(new osg::Uniform("ov_billboard_color_threshold", data.ColorThreshold));
			stateset->addUniform(new osg::Uniform("ov_billboard_color_impact", data.ColorImpact));
			stateset->addUniform(new osg::Uniform("ov_billboard_land_cover_id", data.LandCoverID));


			int num_billboards = static_cast<int>(data.Billboards.size());
			osg::Uniform* numBillboards = new osg::Uniform("ov_num_billboards", num_billboards);
			stateset->addUniform(numBillboards);

			const int MAX_BILLBOARDS = 10;
			osg::Uniform *billboardUniform = new osg::Uniform(osg::Uniform::FLOAT_VEC4, "ov_billboard_data", MAX_BILLBOARDS);
			for (unsigned int i = 0; i < MAX_BILLBOARDS; ++i)
			{
				//Setdefault values
				billboardUniform->setElement(i, osg::Vec4f(1.0f, 1.0f, 1.0f, 1.0f));
			}

			for (unsigned int i = 0; i < data.Billboards.size(); ++i)
			{
				osg::Vec2 size = data.Billboards[i].Size;
				billboardUniform->setElement(i, osg::Vec4f(size.x(), size.y(), data.Billboards[i].Intensity, data.Billboards[i].Probability));
			}

			stateset->addUniform(billboardUniform);
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
				stateset->setAttributeAndModes(new osg::BlendFunc(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO), osg::StateAttribute::OVERRIDE);
			}
			stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
			stateset->setTextureAttributeAndModes(2, data.GetOrCreateTexArray(), osg::StateAttribute::ON);
#if 0 //debug
			program->addShader(osg::Shader::readShaderFile(osg::Shader::VERTEX, osgDB::findDataFile("shaders/terrain_vertex.glsl")));
			program->addShader(osg::Shader::readShaderFile(osg::Shader::TESSCONTROL, osgDB::findDataFile("shaders/terrain_tess_ctrl.glsl")));
			program->addShader(osg::Shader::readShaderFile(osg::Shader::TESSEVALUATION, osgDB::findDataFile("shaders/terrain_tess_eval.glsl")));
			program->addShader(osg::Shader::readShaderFile(osg::Shader::FRAGMENT, osgDB::findDataFile("shaders/terrain_fragment.glsl")));
#else			
			program->addShader(osg::Shader::readShaderFile(osg::Shader::VERTEX, osgDB::findDataFile("ov_billboard_vertex.glsl")));
			program->addShader(osg::Shader::readShaderFile(osg::Shader::TESSCONTROL, osgDB::findDataFile("ov_billboard_tess_ctrl.glsl")));
			program->addShader(osg::Shader::readShaderFile(osg::Shader::TESSEVALUATION, osgDB::findDataFile("ov_billboard_tess_eval.glsl")));
			program->addShader(osg::Shader::readShaderFile(osg::Shader::GEOMETRY, osgDB::findDataFile("ov_billboard_geometry.glsl")));
			program->addShader(osg::Shader::readShaderFile(osg::Shader::FRAGMENT, osgDB::findDataFile("ov_billboard_fragment.glsl")));

			program->setParameter(GL_GEOMETRY_OUTPUT_TYPE_EXT, GL_TRIANGLE_STRIP);

			std::string blt_type;
			if (data.Type == BillboardLayer::BLT_ROTATED_QUAD)
			{
				program->setParameter(GL_GEOMETRY_VERTICES_OUT_EXT, 4);
				blt_type = "BLT_ROTATED_QUAD";
			}
			else if (data.Type == BillboardLayer::BLT_CROSS_QUADS)
			{
				program->setParameter(GL_GEOMETRY_VERTICES_OUT_EXT, 8);
				blt_type = "BLT_CROSS_QUADS";
			}
			else if (data.Type == BillboardLayer::BLT_GRASS)
			{
				program->setParameter(GL_GEOMETRY_VERTICES_OUT_EXT, 16);
				blt_type = "BLT_GRASS";
			}
#if OSG_VERSION_GREATER_OR_EQUAL( 3, 4, 0 ) 
			osg::StateSet::DefineList& defineList = stateset->getDefineList();
			defineList[blt_type].second = (osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
#else
			//TODO: add compiler warning 
#endif
#endif

			stateset->setAttribute(new osg::PatchParameter(3));

			stateset->setAttributeAndModes(new osg::CullFace(), osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
		}

		static void PrepareVegGeode(osg::Geode &veg_geode, BillboardLayer& data)
		{
			ConvertToPatchesVisitor cv;
			veg_geode.accept(cv);
			if(data.Type == BillboardLayer::BLT_GRASS)
				veg_geode.setNodeMask(0x1);
			PrepareVegLayer(veg_geode.getOrCreateStateSet(), data);
		}

		static osg::Geode* CreateVegGeode(osg::Geode &template_geode, BillboardLayer& data)
		{
			osg::Geode* veg_geode = dynamic_cast<osg::Geode*>(template_geode.clone(osg::CopyOp::DEEP_COPY_ALL));
			PrepareVegGeode(*veg_geode, data);
			return veg_geode;
		}
	};


	class BillboardTile : public osg::PositionAttitudeTransform
	{
	public:
		BillboardTile(BillboardLayer &layer, osgTerrain::TerrainTile* tile)
		{
			BillboardLayerHelper::PrepareVegLayer(getOrCreateStateSet(), layer);
			_CreateGeode(tile);
		}

		~BillboardTile()
		{

		}
	
		private:

		void _CreateGeode(osgTerrain::TerrainTile* tile)
		{
			osg::Node* node = NULL;
			osgTerrain::HeightFieldLayer* layer = dynamic_cast<osgTerrain::HeightFieldLayer*>(tile->getElevationLayer());
			if (layer)
			{
				osg::HeightField* hf = layer->getHeightField();
				if (hf)
				{
					osg::Geometry* hf_geom = _CreateGeometry(hf);

					//Add color texture
					osgTerrain::Layer* colorLayer = tile->getColorLayer(0);
					if (colorLayer)
					{
						osg::Image* image = colorLayer->getImage();
						osg::Texture2D* texture = new osg::Texture2D(image);
						hf_geom->getOrCreateStateSet()->setTextureAttributeAndModes(0, texture, osg::StateAttribute::ON);
					}

					//Add land cover texture
					osgTerrain::Layer* landCoverLayer = tile->getColorLayer(1);
					if (landCoverLayer)
					{
						osg::Image* image = landCoverLayer->getImage();
						osg::Texture2D* texture = new osg::Texture2D(image);
						hf_geom->getOrCreateStateSet()->setTextureAttributeAndModes(1, texture, osg::StateAttribute::ON);
					}

					osg::Geode* geode = new osg::Geode();
					geode->addDrawable(hf_geom);
					setPosition(hf->getOrigin());
					addChild(geode);
				}
			}
		}

		static osg::Geometry* _CreateGeometry(osg::HeightField* hf)
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
			return geometry;
		}
	};


	



	/*class BillboardTile2 : public osg::PositionAttitudeTransform
	{
	public:

		BillboardTile2(BillboardLayer &layer, osg::Geode* tile)
		{
			_PrepareVegLayer(getOrCreateStateSet(), layer);
			addChild(tile);
		}

		~BillboardTile2()
		{

		}
	//private:
		static void _PrepareVegLayer(osg::StateSet* stateset, BillboardLayer& data)
		{
			osg::Program* program = new osg::Program;
			stateset->setAttribute(program);
			stateset->addUniform(new osg::Uniform("ov_color_texture", 0));
			stateset->addUniform(new osg::Uniform("ov_land_cover_texture", 1));

			stateset->addUniform(new osg::Uniform("ov_billboard_texture", 2));

			stateset->addUniform(new osg::Uniform("ov_billboard_max_distance", data.MaxDistance));
			stateset->addUniform(new osg::Uniform("ov_billboard_density", data.Density));
			stateset->addUniform(new osg::Uniform("ov_billboard_color_threshold", data.ColorThreshold));
			stateset->addUniform(new osg::Uniform("ov_billboard_color_impact", data.ColorImpact));
			stateset->addUniform(new osg::Uniform("ov_billboard_land_cover_id", data.LandCoverID));


			int num_billboards = static_cast<int>(data.Billboards.size());
			osg::Uniform* numBillboards = new osg::Uniform("ov_num_billboards", num_billboards);
			stateset->addUniform(numBillboards);

			const int MAX_BILLBOARDS = 10;
			osg::Uniform *billboardUniform = new osg::Uniform(osg::Uniform::FLOAT_VEC4, "ov_billboard_data", MAX_BILLBOARDS);
			for (unsigned int i = 0; i < MAX_BILLBOARDS; ++i)
			{
				//Setdefault values
				billboardUniform->setElement(i, osg::Vec4f(1.0f, 1.0f, 1.0f, 1.0f));
			}

			for (unsigned int i = 0; i < data.Billboards.size(); ++i)
			{
				osg::Vec2 size = data.Billboards[i].Size;
				billboardUniform->setElement(i, osg::Vec4f(size.x(), size.y(), data.Billboards[i].Intensity, data.Billboards[i].Probability));
			}

			stateset->addUniform(billboardUniform);
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
				stateset->setAttributeAndModes(new osg::BlendFunc(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO), osg::StateAttribute::OVERRIDE);
			}
			stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
			stateset->setTextureAttributeAndModes(2, data.GetOrCreateTexArray(), osg::StateAttribute::ON);
#if 0 //debug
			program->addShader(osg::Shader::readShaderFile(osg::Shader::VERTEX, osgDB::findDataFile("shaders/terrain_vertex.glsl")));
			program->addShader(osg::Shader::readShaderFile(osg::Shader::TESSCONTROL, osgDB::findDataFile("shaders/terrain_tess_ctrl.glsl")));
			program->addShader(osg::Shader::readShaderFile(osg::Shader::TESSEVALUATION, osgDB::findDataFile("shaders/terrain_tess_eval.glsl")));
			program->addShader(osg::Shader::readShaderFile(osg::Shader::FRAGMENT, osgDB::findDataFile("shaders/terrain_fragment.glsl")));
#else			
			program->addShader(osg::Shader::readShaderFile(osg::Shader::VERTEX, osgDB::findDataFile("ov_billboard_vertex.glsl")));
			program->addShader(osg::Shader::readShaderFile(osg::Shader::TESSCONTROL, osgDB::findDataFile("ov_billboard_tess_ctrl.glsl")));
			program->addShader(osg::Shader::readShaderFile(osg::Shader::TESSEVALUATION, osgDB::findDataFile("ov_billboard_tess_eval.glsl")));
			program->addShader(osg::Shader::readShaderFile(osg::Shader::GEOMETRY, osgDB::findDataFile("ov_billboard_geometry.glsl")));
			program->addShader(osg::Shader::readShaderFile(osg::Shader::FRAGMENT, osgDB::findDataFile("ov_billboard_fragment.glsl")));

			program->setParameter(GL_GEOMETRY_OUTPUT_TYPE_EXT, GL_TRIANGLE_STRIP);

			std::string blt_type;
			if (data.Type == BillboardLayer::BLT_ROTATED_QUAD)
			{
				program->setParameter(GL_GEOMETRY_VERTICES_OUT_EXT, 4);
				blt_type = "BLT_ROTATED_QUAD";
			}
			else if (data.Type == BillboardLayer::BLT_CROSS_QUADS)
			{
				program->setParameter(GL_GEOMETRY_VERTICES_OUT_EXT, 8);
				blt_type = "BLT_CROSS_QUADS";
			}
			else if (data.Type == BillboardLayer::BLT_GRASS)
			{
				program->setParameter(GL_GEOMETRY_VERTICES_OUT_EXT, 16);
				blt_type = "BLT_GRASS";
			}
#if OSG_VERSION_GREATER_OR_EQUAL( 3, 4, 0 ) 
			osg::StateSet::DefineList& defineList = stateset->getDefineList();
			defineList[blt_type].second = (osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
#else
			//TODO: add compiler warning 
#endif
#endif

			stateset->setAttribute(new osg::PatchParameter(3));
			stateset->setAttributeAndModes(new osg::CullFace(), osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);

		}
	};*/
}
