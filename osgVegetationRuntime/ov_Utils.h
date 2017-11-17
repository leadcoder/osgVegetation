#pragma once
#include <osg/Program>
#include <osg/Texture2D>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>

namespace osgVegetation
{
	//Setup terrain for simple detail mapping. Note: Sample data path must be added!
	void PrepareTerrainForDetailMapping(osg::Node* terrain)
	{
		osg::Program* program = new osg::Program;
		terrain->getOrCreateStateSet()->setAttribute(program);
		terrain->getOrCreateStateSet()->addUniform(new osg::Uniform("ov_color_texture", 0));
		terrain->getOrCreateStateSet()->addUniform(new osg::Uniform("ov_land_cover_texture", 1));
		terrain->getOrCreateStateSet()->addUniform(new osg::Uniform("ov_detail_texture0", 2));
		terrain->getOrCreateStateSet()->addUniform(new osg::Uniform("ov_detail_texture1", 3));

		osg::Texture2D *d0_tex = new osg::Texture2D;
		d0_tex->setWrap(osg::Texture2D::WRAP_S, osg::Texture2D::REPEAT);
		d0_tex->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::REPEAT);
		d0_tex->setImage(osgDB::readRefImageFile("terrain/detail/detail_grass_mossy.dds"));
		terrain->getOrCreateStateSet()->setTextureAttributeAndModes(2, d0_tex, osg::StateAttribute::ON);

		osg::Texture2D *d1_tex = new osg::Texture2D;
		d1_tex->setWrap(osg::Texture2D::WRAP_S, osg::Texture2D::REPEAT);
		d1_tex->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::REPEAT);
		d1_tex->setImage(osgDB::readRefImageFile("terrain/detail/detail_dirt.dds"));
		terrain->getOrCreateStateSet()->setTextureAttributeAndModes(3, d1_tex, osg::StateAttribute::ON);
		program->addShader(osg::Shader::readShaderFile(osg::Shader::FRAGMENT, osgDB::findDataFile("ov_terrain_detail_fragment.glsl")));
	}
}