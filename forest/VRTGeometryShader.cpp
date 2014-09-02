#include "VRTGeometryShader.h"
#include <osg/AlphaFunc>
#include <osg/Billboard>
#include <osg/BlendFunc>
#include <osg/Depth>
#include <osg/Geode>
#include <osg/Material>
#include <osg/Math>
#include <osg/MatrixTransform>
#include <osg/PolygonOffset>
#include <osg/Projection>
#include <osg/ShapeDrawable>
#include <osg/Texture2D>
#include <osg/TextureBuffer>
#include <osg/LOD>
#include <osg/Image>
#include <osg/Texture2DArray>
#include <osgDB/ReadFile>
#include "VegetationCell.h"
#include "VegetationLayer.h"


namespace osgVegetation
{

	osg::Program* VRTGeometryShader::createGeometryShader() const
	{
		static const char* vertSource = {
			"#version 120\n"
			"#extension GL_EXT_geometry_shader4 : enable\n"
			"varying vec2 texcoord;\n"
			"void main(void)\n"
			"{\n"
			"    gl_Position = gl_Vertex;\n"
			"    texcoord = gl_MultiTexCoord0.st;\n"
			"}\n"
		};

		static const char* geomSource = {
			"#version 120\n"
			"#extension GL_EXT_geometry_shader4 : enable\n"
			"varying vec2 texcoord;\n"
			"varying float intensity; \n"
			"varying float red_intensity; \n"
			"varying float veg_type; \n"
			"void main(void)\n"
			"{\n"
			"    vec4 v = gl_PositionIn[0];\n"
			"    vec4 info = gl_PositionIn[1];\n"
			"    vec4 info2 = gl_PositionIn[2];\n"
			"    intensity = info.y;\n"
			"    red_intensity = info.z;\n"
			"    veg_type = info2.y;\n"
			"\n"
			"    float h = info.x;\n"
			"    float w = info2.x;\n"
			"    vec4 e;\n"
			"    e = v + vec4(-w,0.0,0.0,0.0);  gl_Position = gl_ModelViewProjectionMatrix * e; texcoord = vec2(0.0,0.0); EmitVertex();\n"
			"    e = v + vec4(w,0.0,0.0,0.0);  gl_Position = gl_ModelViewProjectionMatrix * e;  texcoord = vec2(1.0,0.0); EmitVertex();\n"
			"    e = v + vec4(-w,0.0,h,0.0);  gl_Position = gl_ModelViewProjectionMatrix * e;  texcoord = vec2(0.0,1.0); EmitVertex();\n"
			"    e = v + vec4(w,0.0,h,0.0);  gl_Position = gl_ModelViewProjectionMatrix * e;  texcoord = vec2(1.0,1.0); EmitVertex();\n"
			"    EndPrimitive();\n"
			"    e = v + vec4(0.0,-w,0.0,0.0);  gl_Position = gl_ModelViewProjectionMatrix * e; texcoord = vec2(0.0,0.0); EmitVertex();\n"
			"    e = v + vec4(0.0,w,0.0,0.0);  gl_Position = gl_ModelViewProjectionMatrix * e;  texcoord = vec2(1.0,0.0); EmitVertex();\n"
			"    e = v + vec4(0.0,-w,h,0.0);  gl_Position = gl_ModelViewProjectionMatrix * e;  texcoord = vec2(0.0,1.0); EmitVertex();\n"
			"    e = v + vec4(0.0,w,h,0.0);  gl_Position = gl_ModelViewProjectionMatrix * e;  texcoord = vec2(1.0,1.0); EmitVertex();\n"
			"    EndPrimitive();\n"
			"}\n"
		};


		static const char* fragSource = {
			"#version 120\n"
			"#extension GL_EXT_gpu_shader4 : enable\n"
			"#extension GL_EXT_texture_array : enable\n"
			"uniform sampler2DArray baseTexture; \n"
			"varying vec2 texcoord; \n"
			"varying float intensity; \n"
			"varying float red_intensity; \n"
			"varying float veg_type; \n"
			"\n"
			"void main(void) \n"
			"{ \n"
			"   vec4 finalColor = texture2DArray( baseTexture, vec3(texcoord, veg_type)); \n"
			"   vec4 color = finalColor * intensity;\n"
			"   color.w = finalColor.w;\n"
			"   color.x *= red_intensity;\n"
			"   float depth = gl_FragCoord.z / gl_FragCoord.w;"
			"   color.w = color.w * clamp(1 - ((depth-50)/10), 0.0, 1.0);"
			"   gl_FragColor = color;\n"
			"}\n"
		};

		osg::Program* pgm = new osg::Program;
		pgm->setName( "osgshader2 demo" );

		pgm->addShader( new osg::Shader( osg::Shader::VERTEX,   vertSource ) );
		pgm->addShader( new osg::Shader( osg::Shader::FRAGMENT, fragSource ) );

		pgm->addShader( new osg::Shader( osg::Shader::GEOMETRY, geomSource ) );
		pgm->setParameter( GL_GEOMETRY_VERTICES_OUT_EXT, 8 );
		pgm->setParameter( GL_GEOMETRY_INPUT_TYPE_EXT, GL_LINES );
		pgm->setParameter( GL_GEOMETRY_OUTPUT_TYPE_EXT, GL_TRIANGLE_STRIP);

		return pgm;

	}
	void VRTGeometryShader::createStateSet(VegetationLayerVector &layers)
	{
		//Load textures
		const osg::ref_ptr<osgDB::ReaderWriter::Options> options = new osgDB::ReaderWriter::Options(); 
		options->setOptionString("dds_flip");
		std::map<std::string, osg::Image*> image_map;
		std::map<std::string, int> index_map;
		int num_textures = 0;
		for(size_t i = 0; i < layers.size();i++)
		{
			if(image_map.find(layers[i].TextureName) == image_map.end() )
			{
				image_map[layers[i].TextureName] = osgDB::readImageFile(layers[i].TextureName,options);
				index_map[layers[i].TextureName] = num_textures;
				layers[i].TextureUnit = num_textures;
				num_textures++;
			}
			else
				layers[i].TextureUnit = index_map[layers[i].TextureName];

		}
		osg::Texture2DArray* tex = new osg::Texture2DArray;
		tex->setTextureSize(512, 512, num_textures);
		tex->setUseHardwareMipMapGeneration(true);   

		for(size_t i = 0; i < layers.size();i++)
		{
			tex->setImage(index_map[layers[i].TextureName], image_map[layers[i].TextureName]);
		}

		m_StateSet = new osg::StateSet;
		m_StateSet->setTextureAttribute(0, tex,	osg::StateAttribute::ON);
		m_StateSet->setAttributeAndModes( new osg::BlendFunc, osg::StateAttribute::ON );
		osg::AlphaFunc* alphaFunc = new osg::AlphaFunc;
		alphaFunc->setFunction(osg::AlphaFunc::GEQUAL,0.05f);
		m_StateSet->setAttributeAndModes( alphaFunc, osg::StateAttribute::ON );
		m_StateSet->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
		m_StateSet->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
		osg::Uniform* baseTextureSampler = new osg::Uniform(osg::Uniform::SAMPLER_2D_ARRAY, "baseTexture", num_textures);
		m_StateSet->addUniform(baseTextureSampler);
		m_StateSet->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
		m_StateSet->setAttribute( createGeometryShader() );
	}

	osg::Node* VRTGeometryShader::create(Cell* cell)
	{
		bool needGroup = !(cell->_cells.empty());
		bool needTrees = !(cell->_trees.empty());

		osg::Geode* geode = 0;
		osg::Group* group = 0;

		if (needTrees)
		{
			geode = new osg::Geode;
			geode->setStateSet(m_StateSet);

			osg::Geometry* geometry = new osg::Geometry;
			geode->addDrawable(geometry);

			osg::Vec3Array* v = new osg::Vec3Array;

			for(VegetationObjectList::iterator itr=cell->_trees.begin();
				itr!=cell->_trees.end();
				++itr)
			{
				VegetationObject& tree = **itr;
				v->push_back(tree.Position);
				v->push_back(osg::Vec3(tree.Height,(double)random(0.75f,1.15f),(double)random(1.0f,1.250f)));
				v->push_back(osg::Vec3(tree.Width,tree.TextureUnit,0));
			}
			geometry->setVertexArray( v );
			geometry->addPrimitiveSet( new osg::DrawArrays( GL_TRIANGLES, 0, v->size() ) );

			//osg::StateSet* sset = geode->getOrCreateStateSet();
			//sset->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
			//sset->setAttribute( createGeometryShader() );

			//osg::Uniform* baseTextureSampler = new osg::Uniform("baseTexture",0);
			//osg::Uniform* baseTextureSampler = new osg::Uniform(osg::Uniform::SAMPLER_2D_ARRAY, "baseTexture", 0);
			//sset->addUniform(baseTextureSampler);

		}

		if (needGroup)
		{
			group = new osg::Group;
			//group->setCenter(cell->_bb.center());
			for(Cell::CellList::iterator itr=cell->_cells.begin();
				itr!=cell->_cells.end();
				++itr)
			{
				group->addChild(create(itr->get()));
				
			}

			if (geode) group->addChild(geode);

		}
		if (group) return group;
		else return geode;
	}

	
}