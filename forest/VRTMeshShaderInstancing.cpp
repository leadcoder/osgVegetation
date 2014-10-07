#include "VRTMeshShaderInstancing.h"
#include <osg/AlphaFunc>
#include <osg/Billboard>
#include <osg/BlendFunc>
#include <osg/Depth>
#include <osg/Geode>
#include <osg/Material>
#include <osg/LOD>
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
	struct StaticBoundingBox : public osg::Drawable::ComputeBoundingBoxCallback
	{
		osg::BoundingBox _bbox;
		StaticBoundingBox( const osg::BoundingBox& bbox ) : _bbox(bbox) { }
		osg::BoundingBox computeBound(const osg::Drawable&) const { return _bbox; }
	};

	class ConvertToDrawInstanced : public osg::NodeVisitor
	{
	public:
		/**
		* Create the visitor that will convert primitive sets to draw
		* <num> instances.
		*/
		ConvertToDrawInstanced(unsigned numInstances, const osg::BoundingBox& bbox, bool optimize) :
			_numInstances(numInstances),
			_optimize(optimize)
		{
			setTraversalMode( TRAVERSE_ALL_CHILDREN );
			setNodeMaskOverride( ~0 );
			_staticBBoxCallback = new StaticBoundingBox(bbox);
			_bb = bbox;

		}

		void apply( osg::Geode& geode )
		{
			for( unsigned d=0; d<geode.getNumDrawables(); ++d )
			{
				osg::Geometry* geom = geode.getDrawable(d)->asGeometry();
				if ( geom )
				{
					if ( _optimize )
					{
						// activate VBOs
						geom->setUseDisplayList( false );
						geom->setUseVertexBufferObjects( true );
					}

					 
					 
					geom->setComputeBoundingBoxCallback(  new StaticBoundingBox(_bb)); 
					geom->dirtyBound();

					// convert to use DrawInstanced
					for( unsigned p=0; p<geom->getNumPrimitiveSets(); ++p )
					{
						osg::PrimitiveSet* ps = geom->getPrimitiveSet(p);
						ps->setNumInstances( _numInstances );
						_primitiveSets.push_back( ps );
					}
					_geometries.push_back(geom);
				}
			}

			traverse(geode);
		}
		void apply(osg::LOD& lod)
		{
			// find the highest LOD:
			int   minIndex = 0;
			float minRange = FLT_MAX;
			for(unsigned i=0; i<lod.getNumRanges(); ++i)
			{
				if ( lod.getRangeList()[i].first < minRange )
				{
					minRange = lod.getRangeList()[i].first;
					minIndex = i;
				}
			}

			//remove all but the highest:
			osg::ref_ptr<osg::Node> highestLOD = lod.getChild( minIndex );
			lod.removeChildren( 0, lod.getNumChildren() );

			//add it back with a full range.
			lod.addChild( highestLOD.get(), 0.0f, FLT_MAX );

			traverse(lod);
		}
		std::vector<osg::Geometry*> _geometries;
	protected:
		unsigned _numInstances;
		bool     _optimize;
		osg::ref_ptr<osg::Drawable::ComputeBoundingBoxCallback> _staticBBoxCallback;
		osg::BoundingBox _bb;
		std::list<osg::PrimitiveSet*> _primitiveSets;
		

	};

	void VRTMeshShaderInstancing::createStateSet(VegetationLayerVector &layers) 
	{
		//Load textures
		const osg::ref_ptr<osgDB::ReaderWriter::Options> options = new osgDB::ReaderWriter::Options(); 
		options->setOptionString("dds_flip");
		m_MeshNode = osgDB::readNodeFile(layers[0].MeshName);
		//osg::StateSet *dstate = node->getOrCreateStateSet();
		osg::StateSet *dstate = new osg::StateSet;
		osg::Texture2D *tex = new osg::Texture2D;
		tex->setWrap( osg::Texture2D::WRAP_S, osg::Texture2D::CLAMP );
		tex->setWrap( osg::Texture2D::WRAP_T, osg::Texture2D::CLAMP );
		tex->setImage(osgDB::readImageFile("Images/tree0.rgba"));
		dstate->setTextureAttributeAndModes(0, tex, osg::StateAttribute::ON );


//			dstate->setTextureAttribute(0, new osg::TexEnv );
		{
			osg::Program* program = new osg::Program;
			dstate->setAttribute(program);

			char vertexShaderSource[] =
				"#version 440 compatibility\n"
				"#extension GL_ARB_uniform_buffer_object : enable\n"
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
				"   vec4 data     = texelFetch(dataBuffer, instanceAddress + 2);\n"
				"   mat4 mvpMatrix = gl_ModelViewProjectionMatrix *\n"
				"        mat4( data.x, 0.0, 0.0, 0.0,\n"
				"              0.0, data.x, 0.0, 0.0,\n"
				"              0.0, 0.0, data.y, 0.0,\n"
				"              position.x, position.y, position.z, 1.0);\n"
				"   gl_Position = mvpMatrix * vec4(VertexPosition,1.0) ;\n"
				"   TexCoord = VertexTexCoord.xy;\n"
				"}\n";

			char fragmentShaderSource[] =
				"#version 440 core\n"
				"uniform sampler2D baseTexture; \n"
				"in vec2 TexCoord;\n"
				"in vec4 Color;\n"
				"layout(location = 0, index = 0) out vec4 FragData0;\n"
				"void main(void) \n"
				"{\n"
				"    vec4 finalColor = texture2D( baseTexture, TexCoord); \n"
				"    FragData0 = Color*finalColor;\n"
				"}\n";

			osg::Shader* vertex_shader = new osg::Shader(osg::Shader::VERTEX, vertexShaderSource);
			program->addShader(vertex_shader);

			osg::Shader* fragment_shader = new osg::Shader(osg::Shader::FRAGMENT, fragmentShaderSource);
			program->addShader(fragment_shader);

			osg::Uniform* baseTextureSampler = new osg::Uniform("baseTexture",0);
			dstate->addUniform(baseTextureSampler);
		
			m_StateSet = dstate; 

			
		}
	}
	static int count = 0;
	osg::Node* VRTMeshShaderInstancing::create(Cell* cell)
	{
		osg::Node* textureBufferGraph = createRec(cell);
		//if(textureBufferGraph)
		//	textureBufferGraph->setStateSet(m_StateSet);
		//textureBufferGraph->setStateSet(stateset);
		return textureBufferGraph;
	}

	osg::Node* VRTMeshShaderInstancing::createRec(Cell* cell)
	{
		bool needGroup = !(cell->_cells.empty());
		bool needTrees = !(cell->_trees.empty());

		osg::Node* geode = 0;
		osg::Group* group = 0;

		if (needTrees)
		{
			geode = (osg::Node*) m_MeshNode->clone( osg::CopyOp::DEEP_COPY_NODES | osg::CopyOp::DEEP_COPY_DRAWABLES | osg::CopyOp::DEEP_COPY_PRIMITIVES);

			ConvertToDrawInstanced cdi(cell->_trees.size(), cell->_bb, true);
			geode->accept( cdi );
			
			/*ConvertToDrawInstanced cdi(cell->_trees.size(), cell->_bb, true);
			m_MeshNode->accept( cdi );
			geode = new osg::Geode(); 
	
			for(size_t i = 0; i < cdi._geometries.size(); i++)
			{
				osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry(*cdi._geometries[i], osg::CopyOp::DEEP_COPY_ALL);
				geode->addDrawable(geometry);
			}*/
			count += cell->_trees.size();
			

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
				ptr[2] = osg::Vec4f(tree.Width, tree.Height, tree.TextureUnit, 1.0);
			}
			osg::ref_ptr<osg::TextureBuffer> tbo = new osg::TextureBuffer;
			tbo->setImage( treeParamsImage.get() );
			tbo->setInternalFormat(GL_RGBA32F_ARB);
			geode->getOrCreateStateSet()->setTextureAttribute(1, tbo.get(),osg::StateAttribute::ON);
			geode->setInitialBound( cell->_bb );
			osg::Uniform* dataBufferSampler = new osg::Uniform("dataBuffer",1);
			geode->getOrCreateStateSet()->addUniform(dataBufferSampler);
		}

		if (needGroup)
		{
			group = new osg::Group;
			for(Cell::CellList::iterator itr=cell->_cells.begin();
				itr!=cell->_cells.end();
				++itr)
			{
				osg::Node* ret_node = createRec(itr->get());
				if(ret_node)
					group->addChild(ret_node);
			}

			if (geode) group->addChild(geode);

		}
		if (group) return group;
		else return geode;
	}
}