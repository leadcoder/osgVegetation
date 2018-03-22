#pragma once

#include "ov_Common.h"
#include "ov_Terrain.h"
#include <osg/Program>
#include <osg/Texture2D>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>

namespace osgVegetation
{
	void PrepareTerrainForDetailMapping(osg::Node* terrain, const Terrain& tdm)
	{
		osg::Program* program = new osg::Program;
		terrain->getOrCreateStateSet()->setAttribute(program);
		terrain->getOrCreateStateSet()->addUniform(new osg::Uniform("ov_color_texture", 0));
		terrain->getOrCreateStateSet()->addUniform(new osg::Uniform("ov_land_cover_texture", 1));
		//terrain->getOrCreateStateSet()->addUniform(new osg::Uniform("ov_detail_texture1", 3));

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
			if(i < 4)
				scale[i] = tdm.DetailLayers[i].Scale;
		}
		
		terrain->getOrCreateStateSet()->addUniform(new osg::Uniform("ov_detail_scale", scale));

		program->addShader(osg::Shader::readShaderFile(osg::Shader::VERTEX, osgDB::findDataFile(tdm.VertexShader)));
		program->addShader(osg::Shader::readShaderFile(osg::Shader::FRAGMENT, osgDB::findDataFile(tdm.FragmentShader)));
	}

	//Setup terrain for simple detail mapping. Note: Sample data path must be added!
	void PrepareTerrainForDetailMapping(osg::Node* terrain)
	{
		Terrain tdm;
		tdm.VertexShader = "ov_terrain_detail_vertex.glsl";
		tdm.FragmentShader = "ov_terrain_detail_fragment.glsl";
		tdm.DetailLayers.push_back(DetailLayer(std::string("terrain/detail/detail_grass_mossy.dds"),10));
		tdm.DetailLayers.push_back(DetailLayer(std::string("terrain/detail/detail_dirt.dds"), 10));
		tdm.DetailLayers.push_back(DetailLayer(std::string("terrain/detail/detail_grass_mossy.dds"), 10));
		tdm.DetailLayers.push_back(DetailLayer(std::string("terrain/detail/detail_dirt.dds"), 10));
		PrepareTerrainForDetailMapping(terrain,tdm);
	}
	
}