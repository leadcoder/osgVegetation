#include "VRTShaderInstancing.h"
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
#include <osg/Image>
#include <osg/Texture2DArray>
#include <osgDB/ReadFile>
#include "VegetationCell.h"
#include "VegetationLayer.h"


namespace osgVegetation
{

	void VRTShaderInstancing::createStateSet(VegetationLayerVector &layers) 
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
				//layers[i].Type = num_textures;
				num_textures++;
			}
			else
			{
				//layers[i].Type = index_map[layers[i].TextureName];
			}
		}
		osg::Texture2DArray* tex = new osg::Texture2DArray;
		tex->setTextureSize(512, 512, num_textures);
		tex->setUseHardwareMipMapGeneration(true);   

		for(size_t i = 0; i < layers.size();i++)
		{
			tex->setImage(index_map[layers[i].TextureName], image_map[layers[i].TextureName]);
		}

		osg::StateSet *dstate = new osg::StateSet;
		dstate->setTextureAttribute(0, tex,	osg::StateAttribute::ON);
		dstate->setAttributeAndModes( new osg::BlendFunc, osg::StateAttribute::ON );
		osg::AlphaFunc* alphaFunc = new osg::AlphaFunc;
		alphaFunc->setFunction(osg::AlphaFunc::GEQUAL,0.05f);
		dstate->setAttributeAndModes( alphaFunc, osg::StateAttribute::ON );
		dstate->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
		dstate->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
		osg::Uniform* baseTextureSampler = new osg::Uniform(osg::Uniform::SAMPLER_2D_ARRAY, "baseTexture", num_textures);
		dstate->addUniform(baseTextureSampler);
		dstate->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
//		dstate->setAttribute( createGeometryShader() );

		{
			osg::Program* program = new osg::Program;
			dstate->setAttribute(program);

			char vertexShaderSource[] =
				"#version 330 compatibility\n"
				"uniform samplerBuffer dataBuffer;\n"
				"layout(location = 0) in vec3 VertexPosition;\n"
				"layout(location = 8) in vec3 VertexTexCoord;\n"
				"out vec2 TexCoord;\n"
				"out vec4 Color;\n"
				"void main()\n"
				"{\n"
				"   int instanceAddress = gl_InstanceID * 3;\n"
				"   vec3 position = texelFetch(dataBuffer, instanceAddress).xyz;\n"
				"   Color         = texelFetch(dataBuffer, instanceAddress + 1);\n"
				"   vec2 size     = texelFetch(dataBuffer, instanceAddress + 2).xy;\n"
				"   mat4 mvpMatrix = gl_ModelViewProjectionMatrix *\n"
				"        mat4( size.x, 0.0, 0.0, 0.0,\n"
				"              0.0, size.x, 0.0, 0.0,\n"
				"              0.0, 0.0, size.y, 0.0,\n"
				"              position.x, position.y, position.z, 1.0);\n"
				"   gl_Position = mvpMatrix * vec4(VertexPosition,1.0) ;\n"
				"   TexCoord = VertexTexCoord.xy;\n"
				"}\n";

			char fragmentShaderSource[] =
				"#version 330 core\n"
				"uniform sampler2D baseTexture; \n"
				"in vec2 TexCoord;\n"
				"in vec4 Color;\n"
				"layout(location = 0, index = 0) out vec4 FragData0;\n"
				"void main(void) \n"
				"{\n"
				"    FragData0 = Color*texture(baseTexture, TexCoord);\n"
				"}\n";

			osg::Shader* vertex_shader = new osg::Shader(osg::Shader::VERTEX, vertexShaderSource);
			program->addShader(vertex_shader);

			osg::Shader* fragment_shader = new osg::Shader(osg::Shader::FRAGMENT, fragmentShaderSource);
			program->addShader(fragment_shader);

			osg::Uniform* baseTextureSampler = new osg::Uniform("baseTexture",0);
			dstate->addUniform(baseTextureSampler);

			osg::Uniform* dataBufferSampler = new osg::Uniform("dataBuffer",1);
			dstate->addUniform(dataBufferSampler);
			m_StateSet = dstate; 
		}
	}

	osg::Node* VRTShaderInstancing::create(Cell* cell)
	{
		/*osg::StateSet* stateset = new osg::StateSet(*dstate, osg::CopyOp::DEEP_COPY_ALL);
		{
			osg::Program* program = new osg::Program;
			stateset->setAttribute(program);

			char vertexShaderSource[] =
				"#version 330 compatibility\n"
				"uniform samplerBuffer dataBuffer;\n"
				"layout(location = 0) in vec3 VertexPosition;\n"
				"layout(location = 8) in vec3 VertexTexCoord;\n"
				"out vec2 TexCoord;\n"
				"out vec4 Color;\n"
				"void main()\n"
				"{\n"
				"   int instanceAddress = gl_InstanceID * 3;\n"
				"   vec3 position = texelFetch(dataBuffer, instanceAddress).xyz;\n"
				"   Color         = texelFetch(dataBuffer, instanceAddress + 1);\n"
				"   vec2 size     = texelFetch(dataBuffer, instanceAddress + 2).xy;\n"
				"   mat4 mvpMatrix = gl_ModelViewProjectionMatrix *\n"
				"        mat4( size.x, 0.0, 0.0, 0.0,\n"
				"              0.0, size.x, 0.0, 0.0,\n"
				"              0.0, 0.0, size.y, 0.0,\n"
				"              position.x, position.y, position.z, 1.0);\n"
				"   gl_Position = mvpMatrix * vec4(VertexPosition,1.0) ;\n"
				"   TexCoord = VertexTexCoord.xy;\n"
				"}\n";

			char fragmentShaderSource[] =
				"#version 330 core\n"
				"uniform sampler2D baseTexture; \n"
				"in vec2 TexCoord;\n"
				"in vec4 Color;\n"
				"layout(location = 0, index = 0) out vec4 FragData0;\n"
				"void main(void) \n"
				"{\n"
				"    FragData0 = Color*texture(baseTexture, TexCoord);\n"
				"}\n";

			osg::Shader* vertex_shader = new osg::Shader(osg::Shader::VERTEX, vertexShaderSource);
			program->addShader(vertex_shader);

			osg::Shader* fragment_shader = new osg::Shader(osg::Shader::FRAGMENT, fragmentShaderSource);
			program->addShader(fragment_shader);

			osg::Uniform* baseTextureSampler = new osg::Uniform("baseTexture",0);
			stateset->addUniform(baseTextureSampler);

			osg::Uniform* dataBufferSampler = new osg::Uniform("dataBuffer",1);
			stateset->addUniform(dataBufferSampler);
		}*/
		osg::ref_ptr<osg::Geometry> templateGeometry = createOrthogonalQuadsNoColor(osg::Vec3(0.0f,0.0f,0.0f),1.0f,1.0f);
		templateGeometry->setUseVertexBufferObjects(true);
		templateGeometry->setUseDisplayList(false);
		osg::Node* textureBufferGraph = createRec(cell, templateGeometry.get());
		//textureBufferGraph->setStateSet( stateset );
		return textureBufferGraph;
	}

	osg::Node* VRTShaderInstancing::createRec(Cell* cell, osg::Geometry* templateGeometry)
	{
		bool needGroup = !(cell->_cells.empty());
		bool needTrees = !(cell->_trees.empty());

		osg::Geode* geode = 0;
		osg::Group* group = 0;

		if (needTrees)
		{
			osg::Geometry* geometry = (osg::Geometry*)templateGeometry->clone( osg::CopyOp::DEEP_COPY_PRIMITIVES );
			osg::DrawArrays* primSet = dynamic_cast<osg::DrawArrays*>( geometry->getPrimitiveSet(0) );
			primSet->setNumInstances( cell->_trees.size() );
			geode = new osg::Geode;
			geode->addDrawable(geometry);

			geometry->setStateSet(m_StateSet);

			osg::ref_ptr<osg::Image> treeParamsImage = new osg::Image;
			treeParamsImage->allocateImage( 3*cell->_trees.size(), 1, 1, GL_RGBA, GL_FLOAT );

			unsigned int i=0;
			for(VegetationObjectList::iterator itr=cell->_trees.begin();
				itr!=cell->_trees.end();
				++itr,++i)
			{
				osg::Vec4f* ptr = (osg::Vec4f*)treeParamsImage->data(3*i);
				VegetationObject& tree = **itr;
				ptr[0] = osg::Vec4f(tree.Position.x(),tree.Position.y(),tree.Position.z(),1.0);
				ptr[1] = osg::Vec4f((float)tree.Color.r()/255.0f,(float)tree.Color.g()/255.0f, (float)tree.Color.b()/255.0f, 1.0);
				ptr[2] = osg::Vec4f(tree.Width, tree.Height, 1.0, 1.0);
			}
			osg::ref_ptr<osg::TextureBuffer> tbo = new osg::TextureBuffer;
			tbo->setImage( treeParamsImage.get() );
			tbo->setInternalFormat(GL_RGBA32F_ARB);
			geometry->getOrCreateStateSet()->setTextureAttribute(1, tbo.get());
			geometry->setInitialBound( cell->_bb );
		}

		if (needGroup)
		{
			group = new osg::Group;
			for(Cell::CellList::iterator itr=cell->_cells.begin();
				itr!=cell->_cells.end();
				++itr)
			{
				group->addChild(createRec(itr->get(),templateGeometry));
			}

			if (geode) group->addChild(geode);

		}
		if (group) return group;
		else return geode;
	}
}