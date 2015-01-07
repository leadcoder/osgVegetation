#include "BRTGeometryShader.h"
#include "VegetationUtils.h"
#include "BillboardLayer.h"
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
#include <osg/Image>
#include <osg/Texture2DArray>
#include <osgDB/ReadFile>

namespace osgVegetation
{

	BRTGeometryShader::BRTGeometryShader(BillboardData &data,  bool use_fog, osg::Fog::Mode fog_mode) : m_PPL(false),
		m_UseFog(use_fog),
		m_FogMode(fog_mode),
		m_TrueBillboards(true)
	{
		m_StateSet = _createStateSet(data);
	}

	osg::Program* BRTGeometryShader::_createShaders(BillboardData &data) const
	{
		std::stringstream vertexSource;

		vertexSource <<
			"#version 120\n"
			//"#extension GL_EXT_geometry_shader4 : enable\n"
			"varying vec2 TexCoord;\n"
			//"varying vec3 Normal;\n"
			"void main(void)\n"
			"{\n"
			"    gl_Position = gl_Vertex;\n"
			//"    Normal = normalize(gl_NormalMatrix * gl_Normal);\n";
			//"    TexCoord = gl_MultiTexCoord0.st;\n"
			"}\n";

		std::stringstream geomSource;
		//static const char* geomSource = {
		geomSource <<	"#version 120\n"
			"#extension GL_EXT_geometry_shader4 : enable\n"
			"uniform float FadeInDist; \n"
			"varying vec2 TexCoord;\n"
			"varying vec3 Normal;\n"
			"varying vec3 Color; \n"
			"varying float TextureIndex; \n"
			"void DynamicShadow(vec4 ecPosition )                               \n"
			"{                                                                      \n";
		if(data.ReceiveShadows)
		{
			geomSource <<
				"	 ecPosition = gl_ModelViewMatrix * ecPosition;						\n"
				"    // generate coords for shadow mapping                              \n"
				"    gl_TexCoord[2].s = dot( ecPosition, gl_EyePlaneS[2] );             \n"
				"    gl_TexCoord[2].t = dot( ecPosition, gl_EyePlaneT[2] );             \n"
				"    gl_TexCoord[2].p = dot( ecPosition, gl_EyePlaneR[2] );             \n"
				"    gl_TexCoord[2].q = dot( ecPosition, gl_EyePlaneQ[2] );             \n"
				"    gl_TexCoord[3].s = dot( ecPosition, gl_EyePlaneS[3] );             \n"
				"    gl_TexCoord[3].t = dot( ecPosition, gl_EyePlaneT[3] );             \n"
				"    gl_TexCoord[3].p = dot( ecPosition, gl_EyePlaneR[3] );             \n"
				"    gl_TexCoord[3].q = dot( ecPosition, gl_EyePlaneQ[3] );             \n";
		}
		geomSource <<
			"} \n" 

			"void main(void)\n"
			"{\n"
			"    vec4 position = gl_PositionIn[0];\n"
			"    vec4 info = gl_PositionIn[1];\n"
			"    vec4 info2 = gl_PositionIn[2];\n"
			"    TextureIndex = info.z;\n"
			"    Color = info2.xyz;\n"
			"    vec2 scale = info.xy;\n"
			"    scale.x *= 0.5;\n"
			"    float distance = length((gl_ModelViewMatrix * vec4(position.x, position.y, position.z, 1.0)).xyz);\n";
		if(!data.ReceiveShadows) //shadows and vertex fading don't mix well
		{
			geomSource << "	 scale = scale*clamp((1.0 - (distance-FadeInDist))/(FadeInDist*0.2),0.0,1.0);\n";
		}
		geomSource << 
			"    vec4 e;\n";
		if(m_TrueBillboards)
		{
			geomSource <<
				"  mat4 modelView = gl_ModelViewMatrix * mat4( scale.x, 0.0, 0.0, 0.0,\n"
				"              0.0, scale.x, 0.0, 0.0,\n"
				"              0.0, 0.0, scale.y, 0.0,\n"
				"              position.x, position.y, position.z, 1.0);\n"
				"  modelView[0][0] = scale.x; modelView[0][1] = 0.0;     modelView[0][2] = 0.0;\n"
				"  modelView[1][0] = 0;       modelView[1][1] = scale.y; modelView[1][2] = 0.0;\n";
			if(data.TerrainNormal)
			{
				geomSource <<
					"    e = vec4(-1.0,0.0,0.0,1.0);  gl_Position = gl_ProjectionMatrix * (modelView * e); TexCoord = vec2(0.0,0.0); Normal = vec3(0.0,1.0,0.0); EmitVertex();\n"
					"    e = vec4(1.0,0.0,0.0,1.0);   gl_Position = gl_ProjectionMatrix * (modelView * e); TexCoord = vec2(1.0,0.0); Normal = vec3(0.0,1.0,0.0); EmitVertex();\n"
					"    e = vec4(-1.0,0.0,1.0,1.0);  gl_Position = gl_ProjectionMatrix * (modelView * e); TexCoord = vec2(0.0,1.0); Normal = vec3(0.0,1.0,0.0); EmitVertex();\n"
					"    e = vec4(1.0,0.0,1.0,1.0);   gl_Position = gl_ProjectionMatrix * (modelView * e); TexCoord = vec2(1.0,1.0); Normal = vec3(0.0,1.0,0.0); EmitVertex();\n";
			}
			else
			{
				geomSource <<
					"	 float roundness = 1.0;\n"
					"    e = vec4(-1.0,0.0,0.0,1.0);  gl_Position = gl_ProjectionMatrix * (modelView * e); TexCoord = vec2(0.0,0.0); Normal = normalize(vec3(-roundness,0.0,1.0)); EmitVertex();\n"
					"    e = vec4(1.0,0.0,0.0,1.0);   gl_Position = gl_ProjectionMatrix * (modelView * e); TexCoord = vec2(1.0,0.0); Normal = normalize(vec3( roundness,0.0,1.0)); EmitVertex();\n"
					"    e = vec4(-1.0,0.0,1.0,1.0);  gl_Position = gl_ProjectionMatrix * (modelView * e); TexCoord = vec2(0.0,1.0); Normal = normalize(vec3(-roundness,0.0,1.0)); EmitVertex();\n"
					"    e = vec4(1.0,0.0,1.0,1.0);   gl_Position = gl_ProjectionMatrix * (modelView * e); TexCoord = vec2(1.0,1.0); Normal = normalize(vec3( roundness,0.0,1.0)); EmitVertex();\n";
			}
			geomSource <<
				"    EndPrimitive();\n"
				"}\n";
		}
		else
		{
			geomSource <<
				"	 float rand_rad = mod(position.x, 3.14);\n"
				"    float sw = scale.x*sin(rand_rad);\n"
				"    float cw = scale.x*cos(rand_rad);\n"
				"    float h = scale.y;\n";
			if(data.TerrainNormal)
			{
				geomSource <<
					"    e = position + vec4(-sw,-cw,0.0,0.0);  gl_Position = gl_ModelViewProjectionMatrix * e; DynamicShadow(e); TexCoord = vec2(0.0,0.0); Normal = normalize(gl_NormalMatrix * vec3(0.0,0.0,1.0)); EmitVertex();\n"
					"    e = position + vec4( sw, cw,0.0,0.0);  gl_Position = gl_ModelViewProjectionMatrix * e; DynamicShadow(e); TexCoord = vec2(1.0,0.0); Normal = normalize(gl_NormalMatrix * vec3(0.0,0.0,1.0)); EmitVertex();\n"
					"    e = position + vec4(-sw,-cw,h,0.0);    gl_Position = gl_ModelViewProjectionMatrix * e; DynamicShadow(e); TexCoord = vec2(0.0,1.0); Normal = normalize(gl_NormalMatrix * vec3(0.0,0.0,1.0)); EmitVertex();\n"
					"    e = position + vec4( sw, cw,h,0.0);    gl_Position = gl_ModelViewProjectionMatrix * e; DynamicShadow(e); TexCoord = vec2(1.0,1.0); Normal = normalize(gl_NormalMatrix * vec3(0.0,0.0,1.0)); EmitVertex();\n"
					"    EndPrimitive();\n"
					"    e = position + vec4(-cw, sw,0.0,0.0);  gl_Position = gl_ModelViewProjectionMatrix * e; DynamicShadow(e); TexCoord = vec2(0.0,0.0); Normal = normalize(gl_NormalMatrix * vec3(0.0,0.0,1.0)); EmitVertex();\n"
					"    e = position + vec4( cw,-sw,0.0,0.0);  gl_Position = gl_ModelViewProjectionMatrix * e; DynamicShadow(e); TexCoord = vec2(1.0,0.0); Normal = normalize(gl_NormalMatrix * vec3(0.0,0.0,1.0)); EmitVertex();\n"
					"    e = position + vec4(-cw, sw,h,0.0);    gl_Position = gl_ModelViewProjectionMatrix * e; DynamicShadow(e); TexCoord = vec2(0.0,1.0); Normal = normalize(gl_NormalMatrix * vec3(0.0,0.0,1.0)); EmitVertex();\n"
					"    e = position + vec4( cw,-sw,h,0.0);    gl_Position = gl_ModelViewProjectionMatrix * e; DynamicShadow(e); TexCoord = vec2(1.0,1.0); Normal = normalize(gl_NormalMatrix * vec3(0.0,0.0,1.0)); EmitVertex();\n"
					"    EndPrimitive();\n";
			}
			else
			{
				geomSource <<
					"	 float roundness = 0.0;\n"
					"    e = position + vec4(-sw,-cw,0.0,0.0);  gl_Position = gl_ModelViewProjectionMatrix * e; DynamicShadow(e); TexCoord = vec2(0.0,0.0); Normal = normalize(vec3(-roundness,0.0,1.0)); EmitVertex();\n"
					"    e = position + vec4( sw, cw,0.0,0.0);  gl_Position = gl_ModelViewProjectionMatrix * e; DynamicShadow(e); TexCoord = vec2(1.0,0.0); Normal = normalize(vec3( roundness,0.0,1.0)); EmitVertex();\n"
					"    e = position + vec4(-sw,-cw,h,0.0);    gl_Position = gl_ModelViewProjectionMatrix * e; DynamicShadow(e); TexCoord = vec2(0.0,1.0); Normal = normalize(vec3(-roundness,0.0,1.0)); EmitVertex();\n"
					"    e = position + vec4( sw, cw,h,0.0);    gl_Position = gl_ModelViewProjectionMatrix * e; DynamicShadow(e); TexCoord = vec2(1.0,1.0); Normal = normalize(vec3( roundness,0.0,1.0)); EmitVertex();\n"
					"    EndPrimitive();\n"
					"    e = position + vec4(-cw, sw,0.0,0.0);  gl_Position = gl_ModelViewProjectionMatrix * e; DynamicShadow(e); TexCoord = vec2(0.0,0.0); Normal = normalize(vec3(-roundness,0.0,1.0)); EmitVertex();\n"
					"    e = position + vec4( cw,-sw,0.0,0.0);  gl_Position = gl_ModelViewProjectionMatrix * e; DynamicShadow(e); TexCoord = vec2(1.0,0.0); Normal = normalize(vec3( roundness,0.0,1.0)); EmitVertex();\n"
					"    e = position + vec4(-cw, sw,h,0.0);    gl_Position = gl_ModelViewProjectionMatrix * e; DynamicShadow(e); TexCoord = vec2(0.0,1.0); Normal = normalize(vec3(-roundness,0.0,1.0)); EmitVertex();\n"
					"    e = position + vec4( cw,-sw,h,0.0);    gl_Position = gl_ModelViewProjectionMatrix * e; DynamicShadow(e); TexCoord = vec2(1.0,1.0); Normal = normalize(vec3( roundness,0.0,1.0)); EmitVertex();\n"
					"    EndPrimitive();\n";

					/*"    e = position + vec4(-sw,-cw,0.0,0.0);  gl_Position = gl_ModelViewProjectionMatrix * e; DynamicShadow(e); TexCoord = vec2(0.0,0.0); Normal = normalize(gl_NormalMatrix * vec3(cw,-sw,0.0)); EmitVertex();\n"
					"    e = position + vec4( sw, cw,0.0,0.0);  gl_Position = gl_ModelViewProjectionMatrix * e; DynamicShadow(e); TexCoord = vec2(1.0,0.0); Normal = normalize(gl_NormalMatrix * vec3(cw,-sw,0.0)); EmitVertex();\n"
					"    e = position + vec4(-sw,-cw,h,0.0);    gl_Position = gl_ModelViewProjectionMatrix * e; DynamicShadow(e); TexCoord = vec2(0.0,1.0); Normal = normalize(gl_NormalMatrix * vec3(cw,-sw,0.0)); EmitVertex();\n"
					"    e = position + vec4( sw, cw,h,0.0);    gl_Position = gl_ModelViewProjectionMatrix * e; DynamicShadow(e); TexCoord = vec2(1.0,1.0); Normal = normalize(gl_NormalMatrix * vec3(cw,-sw,0.0)); EmitVertex();\n"
					"    EndPrimitive();\n"
					"    e = position + vec4(-cw, sw,0.0,0.0);  gl_Position = gl_ModelViewProjectionMatrix * e; DynamicShadow(e); TexCoord = vec2(0.0,0.0); Normal = normalize(gl_NormalMatrix * vec3(-sw,-cw,0.0)); EmitVertex();\n"
					"    e = position + vec4( cw,-sw,0.0,0.0);  gl_Position = gl_ModelViewProjectionMatrix * e; DynamicShadow(e); TexCoord = vec2(1.0,0.0); Normal = normalize(gl_NormalMatrix * vec3(-sw,-cw,0.0)); EmitVertex();\n"
					"    e = position + vec4(-cw, sw,h,0.0);    gl_Position = gl_ModelViewProjectionMatrix * e; DynamicShadow(e); TexCoord = vec2(0.0,1.0); Normal = normalize(gl_NormalMatrix * vec3(-sw,-cw,0.0)); EmitVertex();\n"
					"    e = position + vec4( cw,-sw,h,0.0);    gl_Position = gl_ModelViewProjectionMatrix * e; DynamicShadow(e); TexCoord = vec2(1.0,1.0); Normal = normalize(gl_NormalMatrix * vec3(-sw,-cw,0.0)); EmitVertex();\n"
					"    EndPrimitive();\n";*/
			}
			geomSource << "}\n";
		}

		std::stringstream fragSource;
		fragSource <<	"#version 120\n"
			"#extension GL_EXT_gpu_shader4 : enable\n"
			"#extension GL_EXT_texture_array : enable\n"
			"uniform sampler2DArray baseTexture; \n"
			"uniform sampler2DShadow shadowTexture0; \n"
			"uniform int shadowTextureUnit0; \n"
			"uniform sampler2DShadow shadowTexture1; \n"
			"uniform int shadowTextureUnit1; \n"
			"uniform float FadeInDist; \n"
			"varying vec2 TexCoord; \n"
			"varying vec3 Normal; \n"
			"varying vec3 Color; \n"
			"varying float TextureIndex; \n"
			"\n"
			"void main(void) \n"
			"{ \n"
			"   vec4 outColor = texture2DArray( baseTexture, vec3(TexCoord, TextureIndex)); \n"
			"   outColor.xyz *= Color; \n"
			"   float depth = gl_FragCoord.z / gl_FragCoord.w;\n"
			"   vec3 lightDir = normalize(gl_LightSource[0].position.xyz);\n"
			"   vec3 normal = -Normal;\n"
			"   if (gl_FrontFacing) normal = -normal;\n"
			"	//add diffuse lighting \n"
			"   float NdotL = max(dot(normal, lightDir), 0);\n";
		if(data.ReceiveShadows)
		{
			fragSource <<
				"  float shadow0 = shadow2DProj( shadowTexture0, gl_TexCoord[shadowTextureUnit0] ).r;   \n"
				"  float shadow1 = shadow2DProj( shadowTexture1, gl_TexCoord[shadowTextureUnit1] ).r;   \n"
				"  NdotL *= shadow0*shadow1; \n";
		}
		fragSource <<
			"   outColor.xyz *= (NdotL * gl_LightSource[0].diffuse.xyz + gl_LightSource[0].ambient.xyz);\n"
			"   outColor.w = outColor.w * clamp(1.0 - ((depth-FadeInDist)/(FadeInDist*0.1)), 0.0, 1.0);\n";
		if(m_UseFog)
		{
			switch(m_FogMode)
			{
			case osg::Fog::LINEAR:
				// Linear fog
				fragSource << "float fogFactor = (gl_Fog.end - depth) * gl_Fog.scale;\n";
				break;
			case osg::Fog::EXP:
				// Exp fog
				fragSource << "float fogFactor = exp(-gl_Fog.density * depth);\n";
				break;
			case osg::Fog::EXP2:
				// Exp fog
				fragSource << "float fogFactor = exp(-pow((gl_Fog.density * depth), 2.0));\n";
				break;
			}
			fragSource <<
				"fogFactor = clamp(fogFactor, 0.0, 1.0);\n"
				"outColor.xyz = mix(gl_Fog.color.xyz, outColor.xyz, fogFactor);\n";
		}
		fragSource <<
			"   gl_FragColor = outColor;\n"
			"}\n";

		osg::Program* pgm = new osg::Program;
		pgm->setName( "GeometryShaderBillboards" );

		pgm->addShader( new osg::Shader( osg::Shader::VERTEX,   vertexSource.str() ) );
		pgm->addShader( new osg::Shader( osg::Shader::FRAGMENT, fragSource.str() ) );
		pgm->addShader( new osg::Shader( osg::Shader::GEOMETRY, geomSource.str() ) );

		if(m_TrueBillboards)
			pgm->setParameter( GL_GEOMETRY_VERTICES_OUT_EXT, 4 );
		else
			pgm->setParameter( GL_GEOMETRY_VERTICES_OUT_EXT, 8 );

		pgm->setParameter( GL_GEOMETRY_INPUT_TYPE_EXT, GL_TRIANGLES );
		pgm->setParameter( GL_GEOMETRY_OUTPUT_TYPE_EXT, GL_TRIANGLE_STRIP);

		return pgm;

	}
	osg::StateSet* BRTGeometryShader::_createStateSet(BillboardData &data)
	{
		//Load textures
		const osg::ref_ptr<osgDB::ReaderWriter::Options> options = new osgDB::ReaderWriter::Options(); 
		options->setOptionString("dds_flip");
		std::map<std::string, osg::Image*> image_map;
		std::map<std::string, int> index_map;
		int num_textures = 0;
		for(size_t i = 0; i < data.Layers.size();i++)
		{
			if(image_map.find(data.Layers[i].TextureName) == image_map.end() )
			{
				image_map[data.Layers[i].TextureName] = osgDB::readImageFile(data.Layers[i].TextureName,options);
				index_map[data.Layers[i].TextureName] = num_textures;
				data.Layers[i]._TextureIndex = num_textures;
				num_textures++;
			}
			else
				data.Layers[i]._TextureIndex = index_map[data.Layers[i].TextureName];

		}
		osg::Texture2DArray* tex = new osg::Texture2DArray;
		tex->setTextureSize(512, 512, num_textures);
		tex->setUseHardwareMipMapGeneration(true);   

		for(size_t i = 0; i < data.Layers.size();i++)
		{
			tex->setImage(index_map[data.Layers[i].TextureName], image_map[data.Layers[i].TextureName]);
		}

		m_StateSet = new osg::StateSet;
		m_StateSet->setTextureAttribute(0, tex,	osg::StateAttribute::ON);
		osg::AlphaFunc* alphaFunc = new osg::AlphaFunc;
		alphaFunc->setFunction(osg::AlphaFunc::GEQUAL,data.AlphaRefValue);
		m_StateSet->setAttributeAndModes( alphaFunc, osg::StateAttribute::ON );

		if(data.UseAlphaBlend)
		{
			m_StateSet->setAttributeAndModes( new osg::BlendFunc, osg::StateAttribute::ON );
			m_StateSet->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
		}

		m_StateSet->setMode( GL_LIGHTING, osg::StateAttribute::ON);

		osg::Uniform* baseTextureSampler = new osg::Uniform(osg::Uniform::SAMPLER_2D_ARRAY, "baseTexture", num_textures);
		m_StateSet->addUniform(baseTextureSampler);
		m_StateSet->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
		m_StateSet->setAttribute( _createShaders(data) );
		return m_StateSet;
	}

	osg::Node* BRTGeometryShader::create(const BillboardVegetationObjectVector &objects, const osg::BoundingBox &bb)
	{
		osg::Geode* geode = new osg::Geode;

		osg::Geometry* geometry = new osg::Geometry;
		geode->addDrawable(geometry);
		osg::Vec3Array* v = new osg::Vec3Array;

		for(size_t i = 0; i < objects.size(); i++)
		{
			v->push_back(objects[i]->Position);
			v->push_back(osg::Vec3(objects[i]->Width,objects[i]->Height,objects[i]->TextureIndex));
			v->push_back(osg::Vec3(objects[i]->Color.r(), objects[i]->Color.g(),objects[i]->Color.b()));
		}
		geometry->setVertexArray( v );
		geometry->addPrimitiveSet( new osg::DrawArrays( GL_TRIANGLES, 0, v->size() ) );

		osg::Uniform* fadeInDist = new osg::Uniform(osg::Uniform::FLOAT, "FadeInDist");
		double bb_size = (bb._max.x() - bb._min.x());
		float radius = sqrt(bb_size*bb_size);
		fadeInDist->set(radius*2.0f);
		geometry->getOrCreateStateSet()->addUniform(fadeInDist);

		return geode;
		//osg::StateSet* sset = geode->getOrCreateStateSet();
		//sset->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
		//sset->setAttribute( createGeometryShader() );

		//osg::Uniform* baseTextureSampler = new osg::Uniform("baseTexture",0);
		//osg::Uniform* baseTextureSampler = new osg::Uniform(osg::Uniform::SAMPLER_2D_ARRAY, "baseTexture", 0);
		//sset->addUniform(baseTextureSampler);
	}



}