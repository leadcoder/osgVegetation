#include "BRTGeometryShader.h"
#include "VegetationUtils.h"
#include <osg/AlphaFunc>
#include <osg/BlendFunc>
#include <osg/Geode>
#include <osg/Material>
#include <osg/ShapeDrawable>
#include <osg/Texture2DArray>
#include <osg/Multisample>
#include <osg/Version>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgDB/FileUtils>

namespace osgVegetation
{

	bool readFile(const char* fName, std::string& s)
	{
		std::string foundFile = osgDB::findDataFile(fName);
		if (foundFile.empty()) return false;

		osgDB::ifstream is;//(fName);
		is.open(foundFile.c_str());
		if (is.fail())
		{
			std::cerr << "Could not open " << fName << " for reading.\n";
			return false;
		}
		char ch = is.get();
		while (!is.eof())
		{
			s += ch;
			ch = is.get();
		}
		is.close();
		return true;
	}

	std::string replaceString(const std::string & srceString, std::string fromString, std::string toString)
	{
		if (fromString == toString) return srceString;
		
		std::string destString;

		std::string::size_type fromLength = fromString.length();
		std::string::size_type srceLength = srceString.length();

		for (std::string::size_type pos = 0; pos < srceLength; )
		{
			std::string::size_type end = srceString.find(fromString, pos);

			if (end == std::string::npos)
				end = srceLength;

			destString.append(srceString, pos, end - pos);

			if (end == srceLength)
				break;

			destString.append(toString);
			pos = end + fromLength;
		}

		return destString;
	}

	BRTGeometryShader::BRTGeometryShader(BillboardData &data, const EnvironmentSettings &env_settings) : m_PPL(false)
	{
		if (!(data.Type == BT_ROTATED_QUAD || data.Type == BT_CROSS_QUADS || data.Type == BT_GRASS))
			OSGV_EXCEPT(std::string("BRTGeometryShader::BRTGeometryShader - Unsupported billboard type").c_str());

		m_StateSet = _createStateSet(data, env_settings);
	}

	osg::Program* BRTGeometryShader::_createShaders(BillboardData &data, const EnvironmentSettings &env_settings) const
	{
#ifdef GENERATE_SHADER
		std::stringstream vertexSource;
		vertexSource <<
			"#version 120\n"
			//"#extension GL_EXT_geometry_shader4 : enable\n"
			"varying vec2 TexCoord;\n"
			"void main(void)\n"
			"{\n"
			"    gl_Position = gl_Vertex;\n"
			//"    Normal = normalize(gl_NormalMatrix * gl_Normal);\n";
			//"    TexCoord = gl_MultiTexCoord0.st;\n"
			"}\n";

		std::stringstream geomSource;
		//static const char* geomSource = {
		geomSource << "#version 120\n"
			"#extension GL_EXT_geometry_shader4 : enable\n"
			"uniform float TileRadius; \n";
		if (env_settings.ShadowMode != SM_DISABLED && data.ReceiveShadows)
		{
			if (env_settings.ShadowMode == SM_LISPSM)
			{
				geomSource << "uniform int shadowTextureUnit; \n";
			}
			else if (env_settings.ShadowMode == SM_VDSM1)
			{
				geomSource << "uniform int shadowTextureUnit0; \n";
			}
			else if (env_settings.ShadowMode == SM_VDSM2)
			{
				geomSource <<
					"uniform int shadowTextureUnit0; \n"
					"uniform int shadowTextureUnit1; \n";
			}
		}
		geomSource <<
			"varying vec2 TexCoord;\n"
			"varying vec3 Normal;\n"
			"varying vec3 Color; \n"
			"varying float TextureIndex; \n";
		if (env_settings.ShadowMode != SM_DISABLED && data.ReceiveShadows)
		{
			geomSource << "void DynamicShadow(vec4 ecPosition )                               \n"
				"{                                                                      \n"
				"    ecPosition = gl_ModelViewMatrix * ecPosition;						\n"
				"    // generate coords for shadow mapping                              \n";
			if (env_settings.ShadowMode == SM_LISPSM)
			{
				geomSource <<
					"    gl_TexCoord[shadowTextureUnit].s = dot( ecPosition, gl_EyePlaneS[shadowTextureUnit] );             \n"
					"    gl_TexCoord[shadowTextureUnit].t = dot( ecPosition, gl_EyePlaneT[shadowTextureUnit] );             \n"
					"    gl_TexCoord[shadowTextureUnit].p = dot( ecPosition, gl_EyePlaneR[shadowTextureUnit] );             \n"
					"    gl_TexCoord[shadowTextureUnit].q = dot( ecPosition, gl_EyePlaneQ[shadowTextureUnit] );             \n"
					"} \n";
			}
			else if (env_settings.ShadowMode == SM_VDSM1)
			{
				geomSource <<
					"    gl_TexCoord[shadowTextureUnit0].s = dot( ecPosition, gl_EyePlaneS[shadowTextureUnit0] );             \n"
					"    gl_TexCoord[shadowTextureUnit0].t = dot( ecPosition, gl_EyePlaneT[shadowTextureUnit0] );             \n"
					"    gl_TexCoord[shadowTextureUnit0].p = dot( ecPosition, gl_EyePlaneR[shadowTextureUnit0] );             \n"
					"    gl_TexCoord[shadowTextureUnit0].q = dot( ecPosition, gl_EyePlaneQ[shadowTextureUnit0] );             \n"
					"} \n";
			}
			else if (env_settings.ShadowMode == SM_VDSM2)
			{
				geomSource <<
					"    gl_TexCoord[shadowTextureUnit0].s = dot( ecPosition, gl_EyePlaneS[shadowTextureUnit0] );             \n"
					"    gl_TexCoord[shadowTextureUnit0].t = dot( ecPosition, gl_EyePlaneT[shadowTextureUnit0] );             \n"
					"    gl_TexCoord[shadowTextureUnit0].p = dot( ecPosition, gl_EyePlaneR[shadowTextureUnit0] );             \n"
					"    gl_TexCoord[shadowTextureUnit0].q = dot( ecPosition, gl_EyePlaneQ[shadowTextureUnit0] );             \n"
					"    gl_TexCoord[shadowTextureUnit1].s = dot( ecPosition, gl_EyePlaneS[shadowTextureUnit1] );             \n"
					"    gl_TexCoord[shadowTextureUnit1].t = dot( ecPosition, gl_EyePlaneT[shadowTextureUnit1] );             \n"
					"    gl_TexCoord[shadowTextureUnit1].p = dot( ecPosition, gl_EyePlaneR[shadowTextureUnit1] );             \n"
					"    gl_TexCoord[shadowTextureUnit1].q = dot( ecPosition, gl_EyePlaneQ[shadowTextureUnit1] );             \n"
					"} \n";
			}
		}
		geomSource <<
			"void main(void)\n"
			"{\n"
			"    vec4 pos = gl_PositionIn[0];\n"
			"    vec4 info = gl_PositionIn[1];\n"
			"    vec4 info2 = gl_PositionIn[2];\n"
			"    TextureIndex = info.z;\n"
			"    Color = info2.xyz;\n"
			"    vec2 scale = info.xy;\n"
			"    scale.x *= 0.5;\n"
			"    vec4 camera_pos = gl_ModelViewMatrixInverse[3];\n";

		if (!data.CastShadows) //shadow casting and vertex fading don't mix well
		{
			geomSource <<
				"    float distance = length(camera_pos.xyz - pos.xyz);\n"
				"    scale = scale*clamp((1.0 - (distance-TileRadius))/(TileRadius*0.2),0.0,1.0);\n";
		}
		geomSource <<
			"    vec4 e;\n"
			"    e.w = pos.w;\n";
		if (data.Type == BT_ROTATED_QUAD)
		{
			geomSource <<
				"    vec3 dir = camera_pos.xyz - pos.xyz;\n"
				"    //we are only interested in xy-plane direction"
				"    dir.z = 0;\n"
				"    dir = normalize(dir);\n"
				"    vec3 up   = vec3(0.0, 0.0, 1.0*scale.y);//Up direction in OSG\n"
				//"    vec3 left = cross(dir,up); //Generate billboard base vector\n"
				"    vec3 left = vec3(-dir.y,dir.x, 0);\n"
				"    left = normalize(left);\n"
				"    left.xy *= scale.xx;\n";
			if (data.TerrainNormal)
			{
				geomSource << "vec3 n1 = vec3(0.0, 1.0, 0.0);vec3 n2=n1;vec3 n3=n1;vec3 n4=n1;\n";
			}
			else
			{
				geomSource <<
					"    float n_offset = 1.0;\n"
					"    vec3 n1 = vec3( n_offset,0.0,1.0);\n"
					"    vec3 n2 = vec3(-n_offset,0.0,1.0);\n"
					"    vec3 n3 = n1;\n"
					"    vec3 n4 = n2;\n";
			}

			if (env_settings.ShadowMode != SM_DISABLED && data.ReceiveShadows)
			{

				geomSource <<
					"    e.xyz =  pos.xyz + left;       gl_Position = gl_ModelViewProjectionMatrix * e; TexCoord = vec2(0.0,0.0); DynamicShadow(e); Normal = n1; EmitVertex();\n"
					"    e.xyz =  pos.xyz - left;       gl_Position = gl_ModelViewProjectionMatrix * e; TexCoord = vec2(1.0,0.0); DynamicShadow(e); Normal = n2; EmitVertex();\n"
					"    e.xyz =  pos.xyz + left + up;  gl_Position = gl_ModelViewProjectionMatrix * e; TexCoord = vec2(0.0,1.0); DynamicShadow(e); Normal = n3; EmitVertex();\n"
					"    e.xyz =  pos.xyz - left + up;  gl_Position = gl_ModelViewProjectionMatrix * e; TexCoord = vec2(1.0,1.0); DynamicShadow(e); Normal = n4; EmitVertex();\n"
					"    EndPrimitive();\n"
					"}\n";
			}
			else
			{
				geomSource <<
					"    e.xyz =  pos.xyz + left;       gl_Position = gl_ModelViewProjectionMatrix * e; TexCoord = vec2(0.0,0.0); Normal = n1; EmitVertex();\n"
					"    e.xyz =  pos.xyz - left;       gl_Position = gl_ModelViewProjectionMatrix * e; TexCoord = vec2(1.0,0.0); Normal = n2; EmitVertex();\n"
					"    e.xyz =  pos.xyz + left + up;  gl_Position = gl_ModelViewProjectionMatrix * e; TexCoord = vec2(0.0,1.0); Normal = n3; EmitVertex();\n"
					"    e.xyz =  pos.xyz - left + up;  gl_Position = gl_ModelViewProjectionMatrix * e; TexCoord = vec2(1.0,1.0); Normal = n4; EmitVertex();\n"
					"    EndPrimitive();\n"
					"}\n";
			}
		}
		else
		{
			geomSource <<
				"    float rand_rad = mod(pos.x, 2*3.14);\n"
				"    float sw = scale.x*sin(rand_rad);\n"
				"    float cw = scale.x*cos(rand_rad);\n"
				"    float h = scale.y;\n";

			if (data.TerrainNormal)
			{
				geomSource << "   vec3 n = normalize(gl_NormalMatrix * vec3(0.0,0.0,1.0));\n";
			}
			else
			{
				geomSource << "   vec3 n = vec3(0.0,0.0,1.0);\n";
			}

			if (env_settings.ShadowMode != SM_DISABLED &&  data.ReceiveShadows)
			{
				geomSource <<
					"    e = pos + vec4(-sw,-cw,0.0,0.0);  gl_Position = gl_ModelViewProjectionMatrix * e; DynamicShadow(e); TexCoord = vec2(0.0,0.0); Normal = n; EmitVertex();\n"
					"    e = pos + vec4( sw, cw,0.0,0.0);  gl_Position = gl_ModelViewProjectionMatrix * e; DynamicShadow(e); TexCoord = vec2(1.0,0.0); Normal = n; EmitVertex();\n"
					"    e = pos + vec4(-sw,-cw,h,0.0);    gl_Position = gl_ModelViewProjectionMatrix * e; DynamicShadow(e); TexCoord = vec2(0.0,1.0); Normal = n; EmitVertex();\n"
					"    e = pos + vec4( sw, cw,h,0.0);    gl_Position = gl_ModelViewProjectionMatrix * e; DynamicShadow(e); TexCoord = vec2(1.0,1.0); Normal = n; EmitVertex();\n"
					"    EndPrimitive();\n"
					"    e = pos + vec4(-cw, sw,0.0,0.0);  gl_Position = gl_ModelViewProjectionMatrix * e; DynamicShadow(e); TexCoord = vec2(0.0,0.0); Normal = n; EmitVertex();\n"
					"    e = pos + vec4( cw,-sw,0.0,0.0);  gl_Position = gl_ModelViewProjectionMatrix * e; DynamicShadow(e); TexCoord = vec2(1.0,0.0); Normal = n; EmitVertex();\n"
					"    e = pos + vec4(-cw, sw,h,0.0);    gl_Position = gl_ModelViewProjectionMatrix * e; DynamicShadow(e); TexCoord = vec2(0.0,1.0); Normal = n; EmitVertex();\n"
					"    e = pos + vec4( cw,-sw,h,0.0);    gl_Position = gl_ModelViewProjectionMatrix * e; DynamicShadow(e); TexCoord = vec2(1.0,1.0); Normal = n; EmitVertex();\n"
					"    EndPrimitive();\n"
					"}\n";
			}
			else
			{
				geomSource <<
					"    e = pos + vec4(-sw,-cw,0.0,0.0);  gl_Position = gl_ModelViewProjectionMatrix * e; TexCoord = vec2(0.0,0.0); Normal = n; EmitVertex();\n"
					"    e = pos + vec4( sw, cw,0.0,0.0);  gl_Position = gl_ModelViewProjectionMatrix * e; TexCoord = vec2(1.0,0.0); Normal = n; EmitVertex();\n"
					"    e = pos + vec4(-sw,-cw,h,0.0);    gl_Position = gl_ModelViewProjectionMatrix * e; TexCoord = vec2(0.0,1.0); Normal = n; EmitVertex();\n"
					"    e = pos + vec4( sw, cw,h,0.0);    gl_Position = gl_ModelViewProjectionMatrix * e; TexCoord = vec2(1.0,1.0); Normal = n; EmitVertex();\n"
					"    EndPrimitive();\n"
					"    e = pos + vec4(-cw, sw,0.0,0.0);  gl_Position = gl_ModelViewProjectionMatrix * e; TexCoord = vec2(0.0,0.0); Normal = n; EmitVertex();\n"
					"    e = pos + vec4( cw,-sw,0.0,0.0);  gl_Position = gl_ModelViewProjectionMatrix * e; TexCoord = vec2(1.0,0.0); Normal = n; EmitVertex();\n"
					"    e = pos + vec4(-cw, sw,h,0.0);    gl_Position = gl_ModelViewProjectionMatrix * e; TexCoord = vec2(0.0,1.0); Normal = n; EmitVertex();\n"
					"    e = pos + vec4( cw,-sw,h,0.0);    gl_Position = gl_ModelViewProjectionMatrix * e; TexCoord = vec2(1.0,1.0); Normal = n; EmitVertex();\n"
					"    EndPrimitive();\n"
					"}\n";
			}
		}

		std::stringstream fragSource;
		fragSource << "#version 120\n"
			"#pragma import_defines ( TEST_DEF )\n"
			"#extension GL_EXT_gpu_shader4 : enable\n"
			"#extension GL_EXT_texture_array : enable\n"
			"uniform sampler2DArray baseTexture; \n";
		if (env_settings.ShadowMode != SM_DISABLED && data.ReceiveShadows)
		{
			if (env_settings.ShadowMode == SM_LISPSM)
			{
				fragSource <<
					"uniform sampler2DShadow shadowTexture; \n"
					"uniform int shadowTextureUnit; \n";
			}
			else if (env_settings.ShadowMode == SM_VDSM1)
			{
				fragSource <<
					"uniform sampler2DShadow shadowTexture0; \n"
					"uniform int shadowTextureUnit0; \n";
			}
			else if (env_settings.ShadowMode == SM_VDSM2)
			{
				fragSource <<
					"uniform sampler2DShadow shadowTexture0; \n"
					"uniform int shadowTextureUnit0; \n"
					"uniform sampler2DShadow shadowTexture1; \n"
					"uniform int shadowTextureUnit1; \n";
			}
		}
		fragSource <<
			"uniform float TileRadius; \n"
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
			"   vec3 normal = normalize(Normal);\n"
			//"   if (gl_FrontFacing) normal = -normal;\n"
			"   //add diffuse lighting \n"
			"   float NdotL = max(dot(normal, lightDir), 0);\n";
		if (data.ReceiveShadows)
		{
			if (env_settings.ShadowMode == SM_LISPSM)
			{
				fragSource <<
					"   float shadow = shadow2DProj( shadowTexture, gl_TexCoord[shadowTextureUnit] ).r;   \n"
					"   NdotL *= shadow; \n";
			}
			else if (env_settings.ShadowMode == SM_VDSM1)
			{
				fragSource <<
					"   float shadow0 = shadow2DProj( shadowTexture0, gl_TexCoord[shadowTextureUnit0] ).r;   \n"
					"   NdotL *= shadow0; \n";
			}
			else if (env_settings.ShadowMode == SM_VDSM2)
			{
				fragSource <<
					"   float shadow0 = shadow2DProj( shadowTexture0, gl_TexCoord[shadowTextureUnit0] ).r;   \n"
					"   float shadow1 = shadow2DProj( shadowTexture1, gl_TexCoord[shadowTextureUnit1] ).r;   \n"
					"   NdotL *= shadow0*shadow1; \n";
			}
		}
		fragSource <<
			"   outColor.xyz *= (NdotL * gl_LightSource[0].diffuse.xyz + gl_LightSource[0].ambient.xyz);\n"
			"   //outColor.w = outColor.w * clamp(1.0 - ((depth-TileRadius)/(TileRadius*0.1)), 0.0, 1.0);\n";
		if (env_settings.UseFog)
		{
			switch (env_settings.FogMode)
			{
			case osg::Fog::LINEAR:
				// Linear fog
				fragSource << "   float fogFactor = (gl_Fog.end - depth) * gl_Fog.scale;\n";
				break;
			case osg::Fog::EXP:
				// Exp fog
				fragSource << "   float fogFactor = exp(-gl_Fog.density * depth);\n";
				break;
			case osg::Fog::EXP2:
				// Exp fog
				fragSource << "   float fogFactor = exp(-pow((gl_Fog.density * depth), 2.0));\n";
				break;
			}
			fragSource <<
				"   fogFactor = clamp(fogFactor, 0.0, 1.0);\n"
				"   outColor.xyz = mix(gl_Fog.color.xyz, outColor.xyz, fogFactor);\n";
		}
		fragSource <<
			"#ifdef TEST_DEF\n"
			"			outColor.x = 1;\n"
			"#endif\n"
			"   gl_FragColor = outColor;\n"
			"}\n";
		osg::Shader* vertex_shader = new osg::Shader(osg::Shader::VERTEX, vertexSource.str());
		osg::Shader* fragment_shader = new osg::Shader(osg::Shader::FRAGMENT, fragSource.str());
		osg::Shader* geometry_shader = new osg::Shader(osg::Shader::GEOMETRY, geomSource.str());

#else
		std::stringstream ss;
		if (data.Type == BT_ROTATED_QUAD)
			ss << "#define BT_ROTATED_QUAD\n";
		else if (data.Type == BT_GRASS)
			ss << "#define BT_GRASS\n";
		if (env_settings.ShadowMode == SM_LISPSM)
			ss << "#define SM_LISPSM\n";
		else if (env_settings.ShadowMode == SM_VDSM1)
			ss << "#define SM_LISPSM\n";
		else if (env_settings.ShadowMode == SM_VDSM2)
			ss << "#define SM_LISPSM\n";
		if (env_settings.FogMode == osg::Fog::LINEAR)
			ss << "#define FM_LINEAR\n";
		else if (env_settings.FogMode == osg::Fog::EXP)
			ss << "#define FM_EXP\n";
		else if (env_settings.FogMode == osg::Fog::EXP2)
			ss << "#define FM_EXP2\n";

		if (data.CastShadows)
			ss << "#define CAST_SHADOW\n";
		if (data.TerrainNormal)
			ss << "#define TERRAIN_NORMAL\n";

		std::string  vertexSource, fragmentSource, geometrySource;
		readFile("shaders/brt_vertex.glsl", vertexSource);
		readFile("shaders/brt_fragment.glsl", fragmentSource);
		readFile("shaders/brt_geometry.glsl", geometrySource);

		vertexSource = replaceString(vertexSource,"#pragma osgveg",ss.str());
		fragmentSource = replaceString(fragmentSource, "#pragma osgveg", ss.str());
		geometrySource = replaceString(geometrySource, "#pragma osgveg", ss.str());
	
		osg::Shader* fragment_shader = new osg::Shader(osg::Shader::FRAGMENT, fragmentSource);
		osg::Shader* vertex_shader = new osg::Shader(osg::Shader::VERTEX, vertexSource);
		osg::Shader* geometry_shader = new osg::Shader(osg::Shader::GEOMETRY, geometrySource);

#endif

		osg::Program* pgm = new osg::Program;
		pgm->setName("GeometryShaderBillboards");

		pgm->addShader(vertex_shader);
		pgm->addShader(fragment_shader);
		pgm->addShader(geometry_shader);

		//const std::string btr_vertex_file("vegetation.glsl");
		//osgDB::writeShaderFile(*vertex_shader, "vertex.glsl");
		//osgDB::writeShaderFile(*geometry_shader, "geometry.glsl");

		if (data.Type == BT_ROTATED_QUAD)
			pgm->setParameter(GL_GEOMETRY_VERTICES_OUT_EXT, 4);
		else if (data.Type == BT_CROSS_QUADS)
			pgm->setParameter(GL_GEOMETRY_VERTICES_OUT_EXT, 8);
		else if (data.Type == BT_GRASS)
			pgm->setParameter(GL_GEOMETRY_VERTICES_OUT_EXT, 16);

		pgm->setParameter(GL_GEOMETRY_INPUT_TYPE_EXT, GL_TRIANGLES);
		pgm->setParameter(GL_GEOMETRY_OUTPUT_TYPE_EXT, GL_TRIANGLE_STRIP);

		return pgm;

	}

	osg::StateSet* BRTGeometryShader::_createStateSet(BillboardData &data, const EnvironmentSettings &env_settings)
	{
		osg::ref_ptr<osg::Texture2DArray> tex = Utils::loadTextureArray(data);

		m_StateSet = new osg::StateSet;
		m_StateSet->setTextureAttribute(0, tex, osg::StateAttribute::ON);
		osg::AlphaFunc* alphaFunc = new osg::AlphaFunc;
		alphaFunc->setFunction(osg::AlphaFunc::GEQUAL, data.AlphaRefValue);
		m_StateSet->setAttributeAndModes(alphaFunc, osg::StateAttribute::ON);

		if (data.UseAlphaBlend)
		{
			m_StateSet->setAttributeAndModes(new osg::BlendFunc, osg::StateAttribute::ON);
			m_StateSet->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
		}

		if (data.UseMultiSample)
		{
			m_StateSet->setMode(GL_SAMPLE_ALPHA_TO_COVERAGE_ARB, 1);
			m_StateSet->setAttributeAndModes(new osg::BlendFunc(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO), osg::StateAttribute::OVERRIDE);
			m_StateSet->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
		}

		m_StateSet->setMode(GL_LIGHTING, osg::StateAttribute::ON);
		const int num_textures = tex->getNumImages();
		osg::Uniform* baseTextureSampler = new osg::Uniform(osg::Uniform::SAMPLER_2D_ARRAY, "baseTexture", num_textures);
		m_StateSet->addUniform(baseTextureSampler);


		osg::Uniform* shadowTextureUnit = new osg::Uniform(osg::Uniform::INT, "shadowTextureUnit");
		shadowTextureUnit->set(env_settings.BaseShadowTextureUnit);
		m_StateSet->addUniform(shadowTextureUnit);

		m_StateSet->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
		osg::Program *program = _createShaders(data, env_settings);
		m_StateSet->setAttribute(program);

		//Protect to avoid problems with LIPSSM shadows
		m_StateSet->setAttribute(program, osg::StateAttribute::PROTECTED | osg::StateAttribute::ON);
		m_StateSet->setDataVariance(osg::Object::DYNAMIC);

		

#ifdef TEST_DEFLIST
		osg::StateSet::DefineList& defineList = m_StateSet->getDefineList();
		if (data.Type == BT_ROTATED_QUAD)
			defineList["BT_ROTATED_QUAD"].second = (osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
		else if (data.Type == BT_GRASS)
			defineList["BT_GRASS"].second = (osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
		if (env_settings.ShadowMode == SM_LISPSM)
			defineList["SM_LISPSM"].second = (osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
		else if (env_settings.ShadowMode == SM_VDSM1)
			defineList["SM_VDSM1"].second = (osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
		else if (env_settings.ShadowMode == SM_VDSM2)
			defineList["SM_VDSM2"].second = (osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
		if (env_settings.FogMode == osg::Fog::LINEAR)
			defineList["FM_LINEAR"].second = (osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
		else if (env_settings.FogMode == osg::Fog::EXP)
			defineList["FM_EXP"].second = (osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
		else if (env_settings.FogMode == osg::Fog::EXP2)
			defineList["FM_EXP2"].second = (osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

		if (data.CastShadows)
			defineList["CAST_SHADOW"].second = (osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
		if (data.TerrainNormal)
			defineList["TERRAIN_NORMAL"].second = (osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
#endif
		return m_StateSet;
	}

	osg::Node* BRTGeometryShader::create(const BillboardVegetationObjectVector &objects, const osg::BoundingBoxd &bb)
	{
		osg::Geode* geode = new osg::Geode;

		osg::Geometry* geometry = new osg::Geometry;
		geode->addDrawable(geometry);
		osg::Vec3Array* v = new osg::Vec3Array;

		for (size_t i = 0; i < objects.size(); i++)
		{
			v->push_back(objects[i]->Position);
			v->push_back(osg::Vec3(objects[i]->Width, objects[i]->Height, objects[i]->TextureIndex));
			v->push_back(osg::Vec3(objects[i]->Color.r(), objects[i]->Color.g(), objects[i]->Color.b()));
		}
		geometry->setVertexArray(v);
		geometry->addPrimitiveSet(new osg::DrawArrays(GL_TRIANGLES, 0, v->size()));

		osg::Uniform* tile_rad_uniform = new osg::Uniform(osg::Uniform::FLOAT, "TileRadius");
		float radius = bb.radius();
		tile_rad_uniform->set(radius);
		geometry->getOrCreateStateSet()->addUniform(tile_rad_uniform);

		return geode;
		//osg::StateSet* sset = geode->getOrCreateStateSet();
		//sset->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
		//sset->setAttribute( createGeometryShader() );

		//osg::Uniform* baseTextureSampler = new osg::Uniform("baseTexture",0);
		//osg::Uniform* baseTextureSampler = new osg::Uniform(osg::Uniform::SAMPLER_2D_ARRAY, "baseTexture", 0);
		//sset->addUniform(baseTextureSampler);
	}
}
