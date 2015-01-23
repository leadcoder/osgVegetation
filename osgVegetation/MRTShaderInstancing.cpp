#include "MRTShaderInstancing.h"
#include <osg/AlphaFunc>
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

					//geom->setComputeBoundingBoxCallback(  new StaticBoundingBox(_bb));
					//geom->setComputeBoundingBoxCallback(NULL);
					geom->setInitialBound(_bb);
					//geom->dirtyBound();

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

	MRTShaderInstancing::MRTShaderInstancing(MeshData &data)
	{
		m_StateSet = _createStateSet(data);
	}

	osg::StateSet* MRTShaderInstancing::_createStateSet(MeshData &data)
	{
		//Load mesh data
		const osg::ref_ptr<osgDB::ReaderWriter::Options> options = new osgDB::ReaderWriter::Options();
		options->setOptionString("dds_flip");
		for(size_t i = 0; i < data.Layers.size(); i++)
		{
			for(size_t j = 0; j < data.Layers[i].MeshLODs.size(); j++)
			{
				const std::string mesh_name = data.Layers[i].MeshLODs[j].MeshName;
				osg::ref_ptr<osg::Node> mesh = osgDB::readNodeFile(mesh_name);
				if(!mesh.valid())
					OSGV_EXCEPT(std::string("MRTShaderInstancing::_createStateSet - Failed to load mesh:" + mesh_name).c_str());
				m_MeshNodeMap[mesh_name] = mesh;
			}
		}
		osg::StateSet *dstate = new osg::StateSet;
		osg::Texture2D *tex = new osg::Texture2D;
		tex->setWrap( osg::Texture2D::WRAP_S, osg::Texture2D::CLAMP );
		tex->setWrap( osg::Texture2D::WRAP_T, osg::Texture2D::CLAMP );
		tex->setImage(osgDB::readImageFile("Images/tree0.rgba"));
		dstate->setTextureAttributeAndModes(0, tex, osg::StateAttribute::ON );
		{
			osg::Program* program = new osg::Program;
			dstate->setAttribute(program);

			char vertexShaderSource[] =
				"#extension GL_ARB_uniform_buffer_object : enable\n"
				"uniform samplerBuffer dataBuffer;\n"
				"varying vec2 TexCoord;\n"
				"varying vec4 Color;\n"
				"void main()\n"
				"{\n"
				"   int instanceAddress = gl_InstanceID * 4;\n"
				"   vec4 v1 = texelFetch(dataBuffer, instanceAddress);\n"
				"   vec4 v2 = texelFetch(dataBuffer, instanceAddress + 1);\n"
				"   vec4 v3 = texelFetch(dataBuffer, instanceAddress + 2);\n"
				"   vec4 v4 = texelFetch(dataBuffer, instanceAddress + 3);\n"
				"   mat4 mvpMatrix = gl_ModelViewProjectionMatrix* \n"
				"        mat4( v1.x, v1.y, v1.z, 0.0,\n"
				"              v2.x, v2.y, v2.z, 0.0,\n"
				"              v3.x, v3.y, v3.z, 0.0,\n"
				"              v4.x, v4.y, v4.z, 1.0);\n"
				"   Color = vec4(v1.w, v2.w, v3.w, v4.w);\n"
				"   gl_Position = mvpMatrix * vec4(gl_Vertex.xyz,1.0) ;\n"
				"   vec3 normal = normalize(gl_NormalMatrix * gl_Normal);\n"
				"   vec3 lightDir = normalize(gl_LightSource[0].position.xyz);\n"
				"   float NdotL = max(dot(normal, lightDir), 0.0);\n"
				"   Color.xyz = NdotL*Color.xyz*gl_LightSource[0].diffuse.xyz*gl_FrontMaterial.diffuse.xyz + gl_LightSource[0].ambient.xyz*Color.xyz*gl_FrontMaterial.ambient.xyz;\n"
				"   TexCoord = gl_MultiTexCoord0.st;\n"
				"}\n";

			char fragmentShaderSource[] =
				"uniform sampler2D baseTexture; \n"
				"varying  vec2 TexCoord;\n"
				"varying  vec4 Color;\n"
				"void main(void) \n"
				"{\n"
				"    vec4 finalColor = texture2D( baseTexture, TexCoord); \n"
				"    gl_FragColor = Color*finalColor;\n"
				"}\n";

			osg::Shader* vertex_shader = new osg::Shader(osg::Shader::VERTEX, vertexShaderSource);
			program->addShader(vertex_shader);

			osg::Shader* fragment_shader = new osg::Shader(osg::Shader::FRAGMENT, fragmentShaderSource);
			program->addShader(fragment_shader);

			osg::Uniform* baseTextureSampler = new osg::Uniform("baseTexture",0);
			dstate->addUniform(baseTextureSampler);


			return dstate;
		}
		return NULL;
	}

	osg::Node* MRTShaderInstancing::create(const MeshVegetationObjectVector &trees, const std::string &mesh_name, const osg::BoundingBox &bb)
	{
		osg::Node* geode = 0;
		osg::Group* group = 0;

		if(trees.size() > 0)
		{
			geode = (osg::Node*) m_MeshNodeMap[mesh_name]->clone( osg::CopyOp::DEEP_COPY_NODES | osg::CopyOp::DEEP_COPY_DRAWABLES | osg::CopyOp::DEEP_COPY_PRIMITIVES);
			ConvertToDrawInstanced cdi(trees.size(), bb, true);
			geode->accept( cdi );

			osg::ref_ptr<osg::Image> treeParamsImage = new osg::Image;
			treeParamsImage->allocateImage( 4*trees.size(), 1, 1, GL_RGBA, GL_FLOAT );
			unsigned int i=0;
			for(MeshVegetationObjectVector::const_iterator itr= trees.begin();
				itr!= trees.end();
				++itr,++i)
			{
				//generate matrix

				osg::Vec4f* ptr = (osg::Vec4f*)treeParamsImage->data(4*i);
				MeshObject& tree = **itr;

				osg::Matrixd trans_mat;
				trans_mat.identity();
				trans_mat.makeTranslate(tree.Position);
				trans_mat =  osg::Matrixd::rotate(tree.Rotation) * osg::Matrixd::scale(tree.Width, tree.Width, tree.Height)* trans_mat;
				double* m = trans_mat.ptr();

				ptr[0] = osg::Vec4f(m[0],m[1],m[2],tree.Color.r());
				ptr[1] = osg::Vec4f(m[4],m[5],m[6],tree.Color.g());
				ptr[2] = osg::Vec4f(m[8],m[9],m[10],tree.Color.b());
				ptr[3] = osg::Vec4f(m[12],m[13],m[14],1.0);
			}
			osg::ref_ptr<osg::TextureBuffer> tbo = new osg::TextureBuffer;
			tbo->setImage( treeParamsImage.get() );
			tbo->setInternalFormat(GL_RGBA32F_ARB);
			geode->getOrCreateStateSet()->setTextureAttribute(1, tbo.get(),osg::StateAttribute::ON);
			geode->setInitialBound( bb );
			osg::Uniform* dataBufferSampler = new osg::Uniform("dataBuffer",1);
			geode->getOrCreateStateSet()->addUniform(dataBufferSampler);
		}
		return geode;
	}

}
