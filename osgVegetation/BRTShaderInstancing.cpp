#include "BRTShaderInstancing.h"
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
#include <osg/CullFace>
#include <osg/Image>
#include <osg/Texture2DArray>
#include <osgDB/ReadFile>
#include "BillboardLayer.h"


namespace osgVegetation
{
	BRTShaderInstancing::BRTShaderInstancing(BillboardData &data) : m_TrueBillboards(true),
		m_PPL(false),
		m_TerrainNormal(data.TerrainNormal),
		m_AlphaRefValue(data.AlphaRefValue),
		m_AlphaBlend(data.UseAlphaBlend)
	{
		m_StateSet = _createStateSet(data.Layers);
	}

	BRTShaderInstancing::~BRTShaderInstancing()
	{

	}

	osg::StateSet* BRTShaderInstancing::_createStateSet(BillboardLayerVector &layers) 
	{
		int tex_width = 0;
		int tex_height = 0;
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
				osg::Image* image = osgDB::readImageFile(layers[i].TextureName,options);
				if(image && tex_width == 0) // first image decide array size
				{
					tex_width = image->s();
					tex_height = image->t();
				}
				image_map[layers[i].TextureName] = image;
				index_map[layers[i].TextureName] = num_textures;
				layers[i]._TextureIndex = num_textures;
				num_textures++;
			}
			else
				layers[i]._TextureIndex = index_map[layers[i].TextureName];
		}
	
		osg::Texture2DArray* tex = new osg::Texture2DArray;
		tex->setTextureSize(tex_width, tex_height, num_textures);
		tex->setUseHardwareMipMapGeneration(true);   
		
		for(size_t i = 0; i < layers.size();i++)
		{
			tex->setImage(index_map[layers[i].TextureName], image_map[layers[i].TextureName]);
		}

		osg::StateSet *dstate = new osg::StateSet;
		dstate->setTextureAttribute(0, tex,	osg::StateAttribute::ON);
		
		osg::AlphaFunc* alphaFunc = new osg::AlphaFunc;
		alphaFunc->setFunction(osg::AlphaFunc::GEQUAL,m_AlphaRefValue);

		dstate->setAttributeAndModes( alphaFunc, osg::StateAttribute::ON );
		if(m_TrueBillboards)
			dstate->setAttributeAndModes(new osg::CullFace(),osg::StateAttribute::OFF);
		else
			dstate->setAttributeAndModes(new osg::CullFace(),osg::StateAttribute::ON);
			
		//dstate->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
		
		if(m_AlphaBlend)
		{
			dstate->setAttributeAndModes( new osg::BlendFunc, osg::StateAttribute::ON );
			dstate->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
		}
		osg::Uniform* baseTextureSampler = new osg::Uniform(osg::Uniform::SAMPLER_2D_ARRAY, "baseTexture", num_textures);
		dstate->addUniform(baseTextureSampler);

		dstate->setMode( GL_LIGHTING, osg::StateAttribute::ON );

		{
			osg::Program* program = new osg::Program;
			dstate->setAttribute(program);

			std::stringstream vertexShaderSource;
			vertexShaderSource << 
				"#version 430 compatibility\n"
				"#extension GL_ARB_uniform_buffer_object : enable\n"
				"uniform samplerBuffer dataBuffer;\n"
				"uniform float fadeInDist;\n"
				"out vec2 TexCoord;\n";
			if(m_PPL)
			{
				vertexShaderSource << 
				"out vec3 Normal;\n"
			    "out vec3 LightDir;\n";
			}
			vertexShaderSource << 
			    "out vec4 Color;\n"
				"out vec3 Ambient;\n"
				"out float veg_type; \n"
				"const vec3 LightPosition = vec3(0.0, 0.0, 4.0);\n" 
				"void main()\n"
				"{\n"
				"   vec3 normal;\n"
				"   int instanceAddress = gl_InstanceID * 3;\n"
				"   vec3 position = texelFetch(dataBuffer, instanceAddress).xyz;\n"
				"   Color         = texelFetch(dataBuffer, instanceAddress + 1);\n"
				"   vec4 data     = texelFetch(dataBuffer, instanceAddress + 2);\n";
				if(m_TrueBillboards)
				{
					vertexShaderSource << 
					"   mat4 modelView = gl_ModelViewMatrix * mat4( data.x, 0.0, 0.0, 0.0,\n"
					"              0.0, data.x, 0.0, 0.0,\n"
					"              0.0, 0.0, data.y, 0.0,\n"
					"              position.x, position.y, position.z, 1.0);\n"
					"   modelView[0][0] = data.x; modelView[0][1] = 0.0;modelView[0][2] = 0.0;\n"
					"   modelView[1][0] = 0;      modelView[1][1] = data.y;modelView[1][2] = 0.0;\n"
					"   vec4 prePos = modelView * vec4(gl_Vertex.xyz,1.0) ;\n"
					"   prePos.y = prePos.y - data.y*gl_Vertex.z * clamp(((-prePos.z-fadeInDist)/(fadeInDist*0.2)), 0.0, 1.0);\n"
					"   gl_Position = gl_ProjectionMatrix * prePos ;\n";
					
					if(m_TerrainNormal)
					{
						vertexShaderSource << 
							"   normal = normalize(gl_NormalMatrix * vec3(0,0,1));\n";
					}
					else
					{
						//skip standard normal transformation for billboards, 
						//we want normal in eye-space and we know how to handle this transformation by hand
						vertexShaderSource << 
							"   normal = normalize(vec3(gl_Normal.x,0,-gl_Normal.y));\n";
					}
				}
				else
				{
					vertexShaderSource << 
					"   mat4 mvpMatrix = gl_ModelViewProjectionMatrix * mat4( data.x, 0.0, 0.0, 0.0,\n"
					"              0.0, data.x, 0.0, 0.0,\n"
					"              0.0, 0.0, data.y, 0.0,\n"
					"              position.x, position.y, position.z, 1.0);\n"
					"   gl_Position = mvpMatrix * gl_Vertex;\n";
					//"   normal = normalize(gl_NormalMatrix * gl_Normal);\n";
					if(m_TerrainNormal)
					{
						vertexShaderSource << 
							"   normal = normalize(gl_NormalMatrix * vec3(0,0,1));\n";
					}
					else
					{
						//skip standard normal transformation for billboards, 
						//we want normal in eye-space and we know how to handle this transformation by hand
						vertexShaderSource << 
							"   normal = normalize(vec3(gl_Normal.x,0,-gl_Normal.y));\n";
					}
				}
				
				if(m_PPL)
				{
					vertexShaderSource << 
						"   Normal = normal;\n"
						"   LightDir = normalize(gl_LightSource[0].position.xyz);\n"
						"   Ambient  = gl_LightSource[0].ambient.xyz;\n";
				}
				else
				{
					vertexShaderSource <<
					"   vec3 lightDir = normalize(gl_LightSource[0].position.xyz);\n"
					"   float NdotL = max(dot(normal, lightDir), 0.0);\n"
					"   Color.xyz = NdotL*Color.xyz + gl_LightSource[0].ambient.xyz*Color.xyz;\n";
				}
				vertexShaderSource << 
				"   veg_type = data.z;\n"
				"   TexCoord = gl_MultiTexCoord0.st;\n"
				"}\n";

			std::stringstream fragmentShaderSource;
			fragmentShaderSource <<
				"#version 430 core\n"
				"#extension GL_EXT_gpu_shader4 : enable\n"
				"#extension GL_EXT_texture_array : enable\n"
				"uniform sampler2DArray baseTexture; \n"
				"uniform float fadeInDist;\n"
				"in float veg_type; \n"
				"in vec3 Ambient; \n"
				"in vec2 TexCoord;\n";
			if(m_PPL)
			{
				fragmentShaderSource << 
				"in vec3 Normal;\n"
				"in vec3 LightDir;\n";
			}	

			fragmentShaderSource <<
				"in vec4 Color;\n"
				"layout(location = 0, index = 0) out vec4 FragData0;\n"
				"void main(void) \n"
				"{\n"
				"    vec4 finalColor = texture2DArray( baseTexture, vec3(TexCoord, veg_type)); \n";
		    if(m_PPL)
			{
			fragmentShaderSource << 
				"   float NdotL = max(dot(normalize(Normal), LightDir), 0.0);\n"
				"   finalColor.xyz = NdotL*finalColor.xyz*Color.xyz + Ambient.xyz*finalColor.xyz*Color.xyz;\n";
			}
			else
			{
			fragmentShaderSource << 
				"   finalColor.xyz = finalColor.xyz * Color.xyz;\n";
			}

			fragmentShaderSource <<
				"    float depth = gl_FragCoord.z / gl_FragCoord.w;\n"
				"    finalColor.w = finalColor.w * clamp(1 - ((depth-fadeInDist)/(fadeInDist*0.2)), 0.0, 1.0);\n"
				"    FragData0 = finalColor;\n"
				"}\n";

			osg::Shader* vertex_shader = new osg::Shader(osg::Shader::VERTEX, vertexShaderSource.str());
			program->addShader(vertex_shader);

			osg::Shader* fragment_shader = new osg::Shader(osg::Shader::FRAGMENT, fragmentShaderSource.str());
			program->addShader(fragment_shader);
		}
		return dstate;
	}

	osg::Geometry* BRTShaderInstancing::_createSingleQuadsWithNormals( const osg::Vec3& pos, float w, float h)
	{
		// set up the coords
		osg::Vec3Array& v = *(new osg::Vec3Array(8));
		osg::Vec3Array& n = *(new osg::Vec3Array(8));
		osg::Vec2Array& t = *(new osg::Vec2Array(8));
		
		float sw = w*0.5f;
		v[0].set(pos.x()-sw,pos.y(),pos.z()+0.0f);
		v[1].set(pos.x()   ,pos.y(),pos.z()+0.0f);
		v[2].set(pos.x()   ,pos.y(),pos.z()+h);
		v[3].set(pos.x()-sw,pos.y(),pos.z()+h);

		v[4].set(pos.x()   ,pos.y(),pos.z()+0.0f);
		v[5].set(pos.x()+sw,pos.y(),pos.z()+0.0f);
		v[6].set(pos.x()+sw,pos.y(),pos.z()+h);
		v[7].set(pos.x()   ,pos.y(),pos.z()+h);
		
		double roundness = 1.0;

		n[0].set(-roundness,-1,0);
		n[1].set( 0,-1,0);
		n[2].set( 0,-1,0);
		n[3].set(-roundness,-1,0);

		n[4].set( 0,-1.0,0);
		n[5].set( roundness,-1,0);
		n[6].set( roundness,-1,0);
		n[7].set( 0,-1,0);
		
		t[0].set(0.0f,0.0f);
		t[1].set(0.5f,0.0f);
		t[2].set(0.5f,1.0f);
		t[3].set(0.0f,1.0f);
		
		t[4].set(0.5f,0.0f);
		t[5].set(1.0f,0.0f);
		t[6].set(1.0f,1.0f);
		t[7].set(0.5f,1.0f);
		
		osg::Geometry *geom = new osg::Geometry;

		geom->setVertexArray( &v );
		geom->setNormalArray(&n);
		geom->setTexCoordArray( 0, &t );
		geom->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
		geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS,0,8));

		return geom;
	}


	osg::Geometry* BRTShaderInstancing::_createOrthogonalQuadsWithNormals( const osg::Vec3& pos, float w, float h)
	{
		// set up the coords
		osg::Vec3Array& v = *(new osg::Vec3Array(16));
		osg::Vec3Array& n = *(new osg::Vec3Array(16));
		osg::Vec2Array& t = *(new osg::Vec2Array(16));
		
		float sw = w*0.5f;
		v[0].set(pos.x()-sw,pos.y(),pos.z()+0.0f);
		v[1].set(pos.x()+sw,pos.y(),pos.z()+0.0f);
		v[2].set(pos.x()+sw,pos.y(),pos.z()+h);
		v[3].set(pos.x()-sw,pos.y(),pos.z()+h);

		v[4].set(pos.x()-sw,pos.y(),pos.z()+h);
		v[5].set(pos.x()+sw,pos.y(),pos.z()+h);
		v[6].set(pos.x()+sw,pos.y(),pos.z()+0.0f);
		v[7].set(pos.x()-sw,pos.y(),pos.z()+0.0f);
	
		v[8].set(pos.x(),pos.y()+sw,pos.z()+0.0f);
		v[9].set(pos.x(),pos.y()-sw,pos.z()+0.0f);
		v[10].set(pos.x(),pos.y()-sw,pos.z()+h);
		v[11].set(pos.x(),pos.y()+sw,pos.z()+h);
		
		v[12].set(pos.x(),pos.y()+sw,pos.z()+h);
		v[13].set(pos.x(),pos.y()-sw,pos.z()+h);
		v[14].set(pos.x(),pos.y()-sw,pos.z()+0.0f);
		v[15].set(pos.x(),pos.y()+sw,pos.z()+0.0f);


		//osg::Vec3 n1(0,-1,0);
		//osg::Vec3 n2(0,1,0);

		//osg::Vec3 n3(-1,0,0);
		//osg::Vec3 n4(1,0,0);


		double roundness = 1.0;

		n[0].set(-roundness,-1,0);
		n[1].set( roundness,-1,0);
		n[2].set( roundness,-1,0);
		n[3].set(-roundness,-1,0);

		n[4].set(-roundness,-1,0);
		n[5].set( roundness,-1,0);
		n[6].set( roundness,-1,0);
		n[7].set(-roundness,-1,0);


		n[8].set(-roundness,-1,0);
		n[9].set( roundness,-1,0);
		n[10].set( roundness,-1,0);
		n[11].set(-roundness,-1,0);


		n[12].set(-roundness,-1,0);
		n[13].set( roundness,-1,0);
		n[14].set( roundness,-1,0);
		n[15].set(-roundness,-1,0);


		/*n[0].set(-roundness,-1,0);
		n[1].set( roundness,-1,0);
		n[2].set( roundness,-1,0);
		n[3].set(-roundness,-1,0);

		n[4].set(-roundness,1,0);
		n[5].set( roundness,1,0);
		n[6].set( roundness,1,0);
		n[7].set(-roundness,1,0);

		n[8].set( -1, roundness,0);
		n[9].set( -1,-roundness,0);
		n[10].set(-1,-roundness,0);
		n[11].set(-1, roundness,0);

		n[12].set(1, roundness,0);
		n[13].set(1,-roundness,0);
		n[14].set(1,-roundness,0);
		n[15].set(1, roundness,0);*/

		t[0].set(0.0f,0.0f);
		t[1].set(1.0f,0.0f);
		t[2].set(1.0f,1.0f);
		t[3].set(0.0f,1.0f);


		t[4].set(0.0f,1.0f);
		t[5].set(1.0f,1.0f);
		t[6].set(1.0f,0.0f);
		t[7].set(0.0f,0.0f);
	
		t[8].set(0.0f,0.0f);
		t[9].set(1.0f,0.0f);
		t[10].set(1.0f,1.0f);
		t[11].set(0.0f,1.0f);

		t[12].set(0.0f,1.0f);
		t[13].set(1.0f,1.0f);
		t[14].set(1.0f,0.0f);
		t[15].set(0.0f,0.0f);
	
		osg::Geometry *geom = new osg::Geometry;

		geom->setVertexArray( &v );
		geom->setNormalArray(&n);
		geom->setTexCoordArray( 0, &t );
		geom->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
		geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS,0,16));

		return geom;
	}

	osg::Node* BRTShaderInstancing::create(const BillboardVegetationObjectVector &trees, const osg::BoundingBox &bb)
	{
		osg::Geode* geode = 0;
		osg::Group* group = 0;
		if(trees.size() > 0)
		{
			osg::ref_ptr<osg::Geometry> templateGeometry; 
			if(m_TrueBillboards)
				templateGeometry = _createSingleQuadsWithNormals(osg::Vec3(0.0f,0.0f,0.0f),1.0f,1.0f);
			else
				templateGeometry = _createOrthogonalQuadsWithNormals(osg::Vec3(0.0f,0.0f,0.0f),1.0f,1.0f);

			templateGeometry->setUseVertexBufferObjects(true);
			templateGeometry->setUseDisplayList(false);
			osg::Geometry* geometry = (osg::Geometry*)templateGeometry->clone( osg::CopyOp::DEEP_COPY_PRIMITIVES );
			geometry->setUseDisplayList(false);
			osg::DrawArrays* primSet = dynamic_cast<osg::DrawArrays*>( geometry->getPrimitiveSet(0) );
			primSet->setNumInstances( trees.size() );
			geode = new osg::Geode;
			geode->addDrawable(geometry);
			unsigned int i=0;
			osg::ref_ptr<osg::Image> treeParamsImage = new osg::Image;
			treeParamsImage->allocateImage( 3*trees.size(), 1, 1, GL_RGBA, GL_FLOAT );
			for(BillboardVegetationObjectVector::const_iterator itr= trees.begin();
				itr!= trees.end();
				++itr,++i)
			{
				osg::Vec4f* ptr = (osg::Vec4f*)treeParamsImage->data(3*i);
				BillboardObject& tree = **itr;
				ptr[0] = osg::Vec4f(tree.Position.x(),tree.Position.y(),tree.Position.z(),1.0);
				ptr[1] = osg::Vec4f((float)tree.Color.r(),(float)tree.Color.g(), (float)tree.Color.b(), 1.0);
				ptr[2] = osg::Vec4f(tree.Width, tree.Height, tree.TextureIndex, 1.0);
			}

			osg::ref_ptr<osg::TextureBuffer> tbo = new osg::TextureBuffer;
			tbo->setImage( treeParamsImage.get() );
			tbo->setInternalFormat(GL_RGBA32F_ARB);
			geometry->getOrCreateStateSet()->setTextureAttribute(1, tbo.get(),osg::StateAttribute::ON);
			geometry->setInitialBound( bb );
			osg::Uniform* dataBufferSampler = new osg::Uniform("dataBuffer",1);
			geometry->getOrCreateStateSet()->addUniform(dataBufferSampler);

			osg::Uniform* fadeInDist = new osg::Uniform(osg::Uniform::FLOAT, "fadeInDist");
			double bb_size = (bb._max.x() - bb._min.x());
			float radius = sqrt(bb_size*bb_size);

			fadeInDist->set(radius*2.0f);
			//fadeInDist->setDataVariance(osg::Object::DYNAMIC);
			geometry->getOrCreateStateSet()->addUniform(fadeInDist);
			//geode->setStateSet((osg::StateSet*) m_StateSet->clone(osg::CopyOp::DEEP_COPY_STATESETS));
		}
		return geode;
	}
}