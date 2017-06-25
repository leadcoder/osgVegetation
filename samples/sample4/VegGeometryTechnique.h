#pragma once

#include <osgTerrain/Terrain>
#include <osgTerrain/TerrainTile>
#include <osgTerrain/GeometryTechnique>
//#include <osgTerrain/DisplacementMappingTechnique>
#include <osgTerrain/Layer>
#include <osgFX/MultiTextureControl>
#include <osg/PositionAttitudeTransform>
#include <osg/PatchParameter>
#include <osgDB/FileUtils>
#include <osg/AlphaFunc>
#include <osg/BlendFunc>
#include <osg/Material>
#include <osg/Texture2DArray>
#include <osg/Multisample>

#include "VegetationUtils.h"



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

class VegetationData
{
public:
	VegetationData()
	{
		std::vector<std::string> tex_names;
		tex_names.push_back("billboards/grass0.png");
		tex_names.push_back("billboards/fir01_bb.png");
		m_TexArray = osgVegetation::Utils::loadTextureArray(tex_names);
	}

	~VegetationData()
	{
	}
	osg::ref_ptr<osg::Texture2DArray> m_TexArray;
private:
};

class VegGeometryTechnique : public osgTerrain::GeometryTechnique
{
private:
	osg::Geode *_vegGeode;

	VegetationData* _vegData;
public:

	VegGeometryTechnique() :osgTerrain::GeometryTechnique() , 
		_vegData(NULL),
		_vegGeode(NULL)
	{

	}
	VegGeometryTechnique(const VegGeometryTechnique& gt, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY) :
		osgTerrain::GeometryTechnique(gt, copyop), _vegGeode(NULL), _vegData(NULL)
	{

	}
	META_Object(, VegGeometryTechnique);
	virtual void init(int dirtyMask, bool assumeMultiThreaded)
	{
		// Modify the data 

		// Call base class implementation to init the tile correctly: 
		osgTerrain::GeometryTechnique::init(dirtyMask, assumeMultiThreaded);
	}

	void applyColorLayers(BufferData& buffer)
	{

		
		///////////////////////////////////////////////////////////////////
		// vertex shader using just Vec4 coefficients
		char vertexShaderSource2[] =
			"uniform sampler2D terrainTexture;\n"
			"uniform vec3 terrainOrigin;\n"
			"uniform vec3 terrainScaleDown;\n"
			"\n"
			"varying vec2 texcoord;\n"
			"\n"
			"void main(void)\n"
			"{\n"
			"    texcoord = gl_Vertex.xy - terrainOrigin.xy;\n"
			"    texcoord.x *= terrainScaleDown.x;\n"
			"    texcoord.y *= terrainScaleDown.y;\n"
			"\n"
			"    vec4 position;\n"
			"    position.x = gl_Vertex.x;\n"
			"    position.y = gl_Vertex.y;\n"
			"    position.z = texture2D(terrainTexture, texcoord).r;\n"
			"    position.w = 1.0;\n"
			" \n"
			"    gl_Position     = gl_ModelViewProjectionMatrix * position;\n"
			"   gl_FrontColor = vec4(1.0,1.0,1.0,1.0);\n"
			"}\n";


		char _vertexShaderSource[] =
			//"#version 140\n"
			//"in vec4 osg_Vertex;\n"
			//	"in vec4 osg_MultiTexCoord0;\n"
			"varying vec4 vPosition;\n"
			"varying vec2 vTexcoord;\n"
			//"uniform sampler2D terrainTexture;\n"
			"uniform vec3 terrainOrigin;\n"
			"uniform vec3 terrainScaleDown;\n"
			//"uniform mat4 osg_ProjectionMatrix;\n"
			//"uniform mat4 osg_ModelViewMatrix;\n"
			"uniform mat4 osg_ModelViewProjectionMatrix; \n"
			"\n"
			"void main(){\n"
			"    vTexcoord = gl_MultiTexCoord0.xy;\n"
			"\n"
			"    vec4 position;\n"
			"    position.x = gl_Vertex.x;\n"
			"    position.y = gl_Vertex.y;\n"
			"    position.z = gl_Vertex.z;\n"
			//"    position.z = texture2D(terrainTexture, vTexcoord).r;\n"
			"    position.w = gl_Vertex.w;\n"
			" \n"
			//"   gl_Position     = osg_ModelViewProjectionMatrix * position;\n"
			"   gl_Position     = position;\n"
			"   vPosition = position;\n"
			"   gl_FrontColor = vec4(1.0,1.0,1.0,1.0);\n"
			"}\n";


		//////////////////////////////////////////////////////////////////
		// fragment shader
		//
		char _fragmentShaderSource[] =
			"#version 120\n"
			"#extension GL_EXT_gpu_shader4 : enable\n"
			"#extension GL_EXT_texture_array : enable\n"
			"uniform sampler2DArray vegTexture; \n"
			"varying vec2 otexcoord;\n"
			"varying vec4 color;\n"
			"\n"
			"void main(void) \n"
			"{\n"
			"    gl_FragColor = color*texture2DArray(vegTexture, vec3(otexcoord, 0)); \n"
			//"    gl_FragColor = texture2D( vegTexture, otexcoord); \n"
			"}\n";

		char _geomShaderSource[] =
			"#version 120 \n"
			"#extension GL_ARB_geometry_shader4 : enable\n"
			"\n"
			"in vec2 texcoord[];\n"
			"varying vec2 otexcoord;\n"
			"uniform mat4 osg_ModelViewProjectionMatrix; \n"
			"void main(void) \n"
			"{\n"
			"vec4 pos = gl_PositionIn[0] + gl_PositionIn[1] + gl_PositionIn[2];\n"
			"pos *= 1.0/3.0;\n"
			"vec4 camera_pos = gl_ModelViewMatrixInverse[3];\n"
			"vec4 e;\n"
			"e.w = pos.w;\n"
			"vec3 dir = camera_pos.xyz - pos.xyz;\n"
			//we are only interested in xy-plane direction
			"dir.z = 0;\n"
			"dir = normalize(dir);\n"
			"vec3 up = vec3(0.0, 0.0, 1.0);//Up direction in OSG\n"
			"vec3 left = vec3(-dir.y, dir.x, 0);\n"
			"left = normalize(left);\n"
			"left.x *= 0.5;\n"
			"left.y *= 0.5;\n"
			"e.xyz = pos.xyz + left;   gl_Position = osg_ModelViewProjectionMatrix* e; otexcoord = vec2(0.0, 0.0);  EmitVertex();\n"
			"e.xyz = pos.xyz - left;   gl_Position = osg_ModelViewProjectionMatrix * e; otexcoord = vec2(1.0, 0.0);  EmitVertex();\n"
			"e.xyz = pos.xyz + left + up;  gl_Position = osg_ModelViewProjectionMatrix * e; otexcoord = vec2(0.0, 1.0);  EmitVertex();\n"
			"e.xyz = pos.xyz - left + up;  gl_Position = osg_ModelViewProjectionMatrix * e; otexcoord = vec2(1.0, 1.0);  EmitVertex();\n"
			"   EndPrimitive();\n"
			"}\n";
		/*

		"   for (int i = 0; i < gl_VerticesIn; i++)\n"
		"   {\n"
		"	   gl_Position = osg_ModelViewProjectionMatrix*gl_PositionIn[i];\n"
		"      otexcoord = texcoord[i];\n"
		"	   EmitVertex();\n"
		"   }\n"
		"   EndPrimitive();\n"
		"}\n";*/

		static const char* _tessControlSource = {
			"#version 400\n"
			"layout(vertices = 4) out;\n"
			"in vec4 vPosition[];\n"
			"in vec2 vTexcoord[];\n"
			"out vec4 tcPosition[];\n"
			"out vec2 tcTexcoord[];\n"
			"uniform mat4 osg_ModelViewMatrix; \n"
			//"uniform float TessLevelInner;\n"
			//"uniform float TessLevelOuter;\n"
			"#define ID gl_InvocationID\n"
			"void main(){\n"
			"    tcPosition[ID] = vPosition[ID];\n"
			"    tcTexcoord[ID] = vTexcoord[ID];\n"
			"    if (ID == 0) {\n"
			"        float level = 1;\n"
			"        vec4 cs_position = osg_ModelViewMatrix * vPosition[ID];\n"
			"		 float dist = -cs_position.z;\n"
			"        if(dist < 150) {level = 32;   }\n"
			//"        else if(dist < 100) {level = 20; }\n"
			//"        else if(dist < 200) {level = 8;}\n"
			"        else { level = 0;}\n"
			"        gl_TessLevelInner[0] = level;\n"
			"        gl_TessLevelInner[1] = level;\n"
			"        gl_TessLevelOuter[0] = level;\n"
			"        gl_TessLevelOuter[1] = level;\n"
			"        gl_TessLevelOuter[2] = level;\n"
			"        gl_TessLevelOuter[3] = level;\n"
			"    }\n"
			"}\n"
		};

		static const char* _tessEvalSource = {
			"#version 400\n"
			"layout(quads, equal_spacing, ccw) in;\n"
			"in vec4 tcPosition[];\n"
			"in vec2 tcTexcoord[];\n"
			"in float distScale[];\n"
			"out vec2 texcoord;\n"
			//"uniform sampler2D terrainTexture;\n"
			//"out vec3 tePatchDistance;\n"
			//"uniform mat4 osg_ProjectionMatrix;\n"
			//"uniform mat4 osg_ModelViewMatrix;\n"
			"uniform mat4 osg_ModelViewProjectionMatrix; \n"
			"void main(){\n"
			//"    vec3 p0 = gl_TessCoord.x * tcPosition[0];\n"
			//"    vec3 p1 = gl_TessCoord.y * tcPosition[1];\n"
			//"    vec3 p2 = gl_TessCoord.z * tcPosition[2];\n"
			"float u = gl_TessCoord.x;\n"
			"float v = gl_TessCoord.y;\n"
			"vec3 a = mix(tcPosition[1].xyz, tcPosition[0].xyz, u);\n"
			"vec3 b = mix(tcPosition[2].xyz, tcPosition[3].xyz, u);\n"
			"vec3 position = mix(a, b, v);\n"
			"vec2 a2 = mix(tcTexcoord[1], tcTexcoord[0], u);\n"
			"vec2 b2 = mix(tcTexcoord[2], tcTexcoord[3], u);\n"
			//"texcoord = position.xy;\n"
			"texcoord = mix(a2, b2, v);\n"

			//"position.z = texture2D(terrainTexture, texcoord).r;\n"
			//"    tePatchDistance = gl_TessCoord;\n"
			//"    tePosition = normalize(p0 + p1 + p2);\n"
			//"    gl_Position = osg_ModelViewProjectionMatrix * vec4(position,1);\n"
			"    gl_Position = vec4(position,1);\n"
			"}\n"
		};

		if (!_vegData)
			_vegData = new VegetationData();
		osgTerrain::GeometryTechnique::applyColorLayers(buffer);

		osg::StateSet* stateset = _vegGeode->getOrCreateStateSet();
		//osgTerrain::Layer* colorLayer = _terrainTile->getColorLayer(0);
		osg::Texture* texture = dynamic_cast<osg::Texture*>(buffer._geode->getOrCreateStateSet()->getTextureAttribute(0, osg::StateAttribute::TEXTURE));
		stateset->setTextureAttributeAndModes(0, texture, osg::StateAttribute::ON);
		stateset->setTextureAttributeAndModes(1, _vegData->m_TexArray, osg::StateAttribute::ON);

		osg::AlphaFunc* alphaFunc = new osg::AlphaFunc;
		alphaFunc->setFunction(osg::AlphaFunc::GEQUAL, 0.9);
		stateset->setAttributeAndModes(alphaFunc, osg::StateAttribute::ON);

		//if (data.UseAlphaBlend)
		{
			stateset->setAttributeAndModes(new osg::BlendFunc, osg::StateAttribute::ON);
			stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
		}

		//if (data.UseMultiSample)
		{
		//	stateset->setMode(GL_SAMPLE_ALPHA_TO_COVERAGE_ARB, 1);
		//	stateset->setAttributeAndModes(new osg::BlendFunc(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO), osg::StateAttribute::OVERRIDE);
		//	stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
		}

		stateset->setMode(GL_LIGHTING, osg::StateAttribute::ON);
		

		osg::Program* program = new osg::Program;
		stateset->setAttribute(program);

		osg::Uniform* baseTextureSampler = new osg::Uniform("baseTexture", 0);
		stateset->addUniform(baseTextureSampler);

		osg::Uniform* vegTextureSampler = new osg::Uniform("vegTexture", 1);
		stateset->addUniform(vegTextureSampler);


		std::string  vertexSource, tessCtrlSource, tessEvalSource, geometrySource, fragmentSource;
		/*readFile("shaders/veg_vertex.glsl", vertexSource);
		readFile("shaders/veg_tess_ctrl.glsl", tessCtrlSource);
		readFile("shaders/veg_tess_eval.glsl", tessEvalSource);
		readFile("shaders/veg_geometry.glsl", geometrySource);
		readFile("shaders/veg_fragment.glsl", fragmentSource);

		program->addShader(new osg::Shader(osg::Shader::GEOMETRY, geometrySource));
		program->setParameter(GL_GEOMETRY_VERTICES_OUT_EXT, 4);
		program->setParameter(GL_GEOMETRY_OUTPUT_TYPE_EXT, GL_TRIANGLE_STRIP);
		*/
		readFile("shaders/terrain_vertex.glsl", vertexSource);
		readFile("shaders/terrain_tess_ctrl.glsl", tessCtrlSource);
		readFile("shaders/terrain_tess_eval.glsl", tessEvalSource);
		readFile("shaders/terrain_fragment.glsl", fragmentSource);

		program->addShader(new osg::Shader(osg::Shader::VERTEX, vertexSource));
		program->addShader(new osg::Shader(osg::Shader::TESSCONTROL, tessCtrlSource));
		program->addShader(new osg::Shader(osg::Shader::TESSEVALUATION, tessEvalSource));
		program->addShader(new osg::Shader(osg::Shader::FRAGMENT, fragmentSource));

		

		//program->setParameter(GL_GEOMETRY_INPUT_TYPE_EXT, GL_PATCHES);
		
	}

	void generateGeometry(BufferData& buffer, osgTerrain::Locator* masterLocator, const osg::Vec3d& centerModel)
	{
		//buffer._transform->addChild(osgDB::readRefNodeFile("dumptruck.osgt"));
		osgTerrain::GeometryTechnique::generateGeometry(buffer, masterLocator, centerModel);

		if (!_terrainTile) return;

		osgTerrain::HeightFieldLayer* layer = dynamic_cast<osgTerrain::HeightFieldLayer*>(_terrainTile->getElevationLayer());
		if (layer)
		{
			//_terrainTile->setNodeMask(0);
			osg::HeightField* hf = layer->getHeightField();
			if (hf)
			{
				
				unsigned int numColumns = hf->getNumColumns();
				unsigned int  numRows = hf->getNumRows();
				float columnCoordDelta = hf->getXInterval();
				float rowCoordDelta = hf->getYInterval();

				osg::Geometry* geometry = new osg::Geometry;

				osg::Vec3Array& v = *(new osg::Vec3Array(numColumns*numRows));
				osg::Vec2Array& t = *(new osg::Vec2Array(numColumns*numRows));
				osg::Vec4ubArray& color = *(new osg::Vec4ubArray(1));

				color[0].set(255, 255, 255, 255);


				float rowTexDelta = 1.0f / (float)(numRows - 1);
				float columnTexDelta = 1.0f / (float)(numColumns - 1);
				osg::Vec3 origin = hf->getOrigin();
				origin.x() = -(columnCoordDelta*(numColumns - 1)) / 2.0;
				origin.y() = -(rowCoordDelta*(numRows - 1)) / 2.0;
				//origin.z() += 1;

				osg::Vec3 pos(origin.x(), origin.y(), origin.z());
				osg::Vec2 tex(0.0f, 0.0f);
				int vi = 0;
				for (int r = 0; r < numRows; ++r)
				{
					pos.x() = origin.x();
					tex.x() = 0.0f;
					for (int c = 0; c < numColumns; ++c)
					{
						float h = hf->getHeight(c, r);
						v[vi].set(pos.x(), pos.y(), h);
						t[vi].set(tex.x(), tex.y());
						pos.x() += columnCoordDelta;
						tex.x() += columnTexDelta;
						++vi;
					}
					pos.y() += rowCoordDelta;
					tex.y() += rowTexDelta;
				}

				geometry->setVertexArray(&v);
				geometry->setTexCoordArray(0, &t);
				geometry->setColorArray(&color, osg::Array::BIND_OVERALL);

				osg::DrawElementsUShort& drawElements = *(new osg::DrawElementsUShort(GL_PATCHES, 2 * 3 * (numColumns*numRows)));
				geometry->addPrimitiveSet(&drawElements);
				int ei = 0;
				for (int r = 0; r < numRows - 1; ++r)
				{
					//osg::DrawElementsUShort& drawElements = *(new osg::DrawElementsUShort(GL_PATCHES, 4 * (numColumns)));
					//osg::DrawElementsUShort& drawElements = *(new osg::DrawElementsUShort(GL_QUADS, 4 * (numColumns)));
					//geometry->addPrimitiveSet(&drawElements);
					//int ei = 0;
					for (int c = 0; c < numColumns - 1; ++c)
					{
						/*drawElements[ei++] = (r + 1)*numColumns + c;
						drawElements[ei++] = (r)*numColumns + c;
						drawElements[ei++] = (r)*numColumns + c + 1;

						drawElements[ei++] = (r)*numColumns + c + 1;
						drawElements[ei++] = (r + 1)*numColumns + c + 1;
						drawElements[ei++] = (r + 1)*numColumns + c;*/
					
						
						
						drawElements[ei++] = (r)*numColumns + c;
						drawElements[ei++] = (r)*numColumns + c + 1;
						drawElements[ei++] = (r + 1)*numColumns + c + 1;

						drawElements[ei++] = (r + 1)*numColumns + c + 1;
						drawElements[ei++] = (r + 1)*numColumns + c;
						drawElements[ei++] = (r)*numColumns + c;
					}
				}

				/*osg::DrawElementsUShort& drawElements = *(new osg::DrawElementsUShort(GL_PATCHES, 4));
				geometry->addPrimitiveSet(&drawElements);
				int ei = 0;
				drawElements[ei++] = (numRows - 1)*numColumns;
				drawElements[ei++] = 0;
				drawElements[ei++] = 0 + numColumns - 1;
				drawElements[ei++] = (numRows - 1)*numColumns + numColumns - 1;
				*/
				osg::Vec3 size(numColumns*columnCoordDelta, numRows*rowCoordDelta, 100);
				//geometry->setInitialBound(osg::BoundingBox(origin, origin + size));
				_vegGeode = new osg::Geode();

				osg::StateSet* stateset = _vegGeode->getOrCreateStateSet();
				stateset->setAttribute(new osg::PatchParameter(3));

				_vegGeode->addDrawable(geometry);
				//buffer._transform->addChild(_vegGeode);

				osg::PagedLOD* plod = dynamic_cast<osg::PagedLOD*>(_terrainTile->getParent(0));

				osg::ref_ptr<osg::MatrixTransform> trans = dynamic_cast<osg::MatrixTransform*>(buffer._transform->clone(osg::CopyOp::SHALLOW_COPY));

				int nf = plod->getNumFileNames();
				if( nf > 0)
					std::cout << plod->getFileName(0);

				osg::Group* plod_group = dynamic_cast<osg::Group*>(plod->getParent(0));
				if(plod_group)
				plod_group->addChild(_vegGeode);

				//Add geode to top object

				//osg::StateSet* stateset = buffer._geode->getOrCreateStateSet();
				//stateset = dynamic_cast<osg::StateSet*>(stateset->clone(osg::CopyOp::sDEEP_COPY_ALL));
				
				//osgTerrain::Layer* colorLayer = _terrainTile->getColorLayer(0);
				//osg::Texture* texure = dynamic_cast<osg::Texture*>(buffer._geode->getOrCreateStateSet()->getTextureAttribute(0, osg::StateAttribute::TEXTURE));
				//stateset->setTextureAttributeAndModes(0, texure, osg::StateAttribute::ON);
				//geode->setStateSet(stateset);
				
			}
		}
	}
};

