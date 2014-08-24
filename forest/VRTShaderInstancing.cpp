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
#include "VegetationCell.h"


namespace osgVegetation
{
	osg::Node* VRTShaderInstancing::create(Cell* cell, osg::StateSet* dstate)
	{
		osg::StateSet* stateset = new osg::StateSet(*dstate, osg::CopyOp::DEEP_COPY_ALL);
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
		}

		osg::ref_ptr<osg::Geometry> templateGeometry = createOrthogonalQuadsNoColor(osg::Vec3(0.0f,0.0f,0.0f),1.0f,1.0f);
		templateGeometry->setUseVertexBufferObjects(true);
		templateGeometry->setUseDisplayList(false);
		osg::Node* textureBufferGraph = createRec(cell, templateGeometry.get());
		textureBufferGraph->setStateSet( stateset );
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

			osg::ref_ptr<osg::Image> treeParamsImage = new osg::Image;
			treeParamsImage->allocateImage( 3*cell->_trees.size(), 1, 1, GL_RGBA, GL_FLOAT );

			unsigned int i=0;
			for(TreeList::iterator itr=cell->_trees.begin();
				itr!=cell->_trees.end();
				++itr,++i)
			{
				osg::Vec4f* ptr = (osg::Vec4f*)treeParamsImage->data(3*i);
				Tree& tree = **itr;
				ptr[0] = osg::Vec4f(tree._position.x(),tree._position.y(),tree._position.z(),1.0);
				ptr[1] = osg::Vec4f((float)tree._color.r()/255.0f,(float)tree._color.g()/255.0f, (float)tree._color.b()/255.0f, 1.0);
				ptr[2] = osg::Vec4f(tree._width, tree._height, 1.0, 1.0);
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