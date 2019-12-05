#include <osg/AlphaFunc>
#include <osg/BlendFunc>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Math>
#include <osg/MatrixTransform>
#include <osg/Projection>
#include <osg/ShapeDrawable>
#include <osg/StateSet>
#include <osg/Texture2D>
#include <osg/Image>
#include <osg/Camera>
#include <osg/VertexProgram>
#include <osg/CullFace>
#include <osg/FragmentProgram>
#include <osg/ComputeBoundsVisitor>
#include <osg/Texture2DArray>
#include <osg/Multisample>
#include <osgUtil/SmoothingVisitor>
#include <osgUtil/Optimizer>
#include <osgDB/WriteFile>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <osgViewer/Viewer>
#include <iostream>
#include <sstream>

struct Model
{
	osg::Uniform* m_NormalUniform;
	osg::Uniform* m_RadiusUniform;
	osg::ref_ptr <osg::MatrixTransform> m_Transform;

	Model(osg::ref_ptr<osg::Node> mesh_node)
	{
		prepareMesh(mesh_node);
		m_Transform = new osg::MatrixTransform;
		m_Transform->setMatrix(osg::Matrix::scale(1, 1, 1)*osg::Matrixd::rotate(osg::DegreesToRadians(-90.0), 1, 0, 0));
		m_Transform->addChild(mesh_node);
		osg::ComputeBoundsVisitor  cbv;
		osg::BoundingBox &bb(cbv.getBoundingBox());
		m_Transform->accept(cbv);
		float minx = bb._min.x();
		float maxx = bb._max.x();
		float miny = bb._min.y();
		float maxy = bb._max.y();
		float minz = bb._min.z();
		float maxz = bb._max.z();
		m_NormalUniform->set(maxy);
		m_RadiusUniform->set(sqrt(maxx * maxx + maxz * maxz));
	}

	osg::BoundingBox SetupSide(double degrees)
	{
		m_Transform->setMatrix(osg::Matrixd::rotate(osg::DegreesToRadians(-degrees), 0, 0, 1) * osg::Matrixd::rotate(osg::DegreesToRadians(-90.0), 1, 0, 0));
		return GetBounds();
	}

	osg::BoundingBox SetupTop(double z_trans = -10)
	{
		m_Transform->setMatrix(osg::Matrix::scale(1, 1, 1)*osg::Matrixd::rotate(osg::DegreesToRadians(0.0), 0, 0, 1)*osg::Matrixd::rotate(osg::DegreesToRadians(0.0), 1, 0, 0) *osg::Matrix::translate(0, 0, z_trans));
		return GetBounds();
	}

	osg::BoundingBox GetBounds()
	{
		osg::ComputeBoundsVisitor cbv;
		//osg::BoundingBox &bb(cbv.getBoundingBox());
		m_Transform->accept(cbv);
		return cbv.getBoundingBox();
	}



	void  prepareMesh(osg::ref_ptr<osg::Node> mesh_node)
	{
		osg::ComputeBoundsVisitor  cbv2;
		osg::BoundingBox &bb2(cbv2.getBoundingBox());
		mesh_node->accept(cbv2);

		osg::Program* program = new osg::Program;
		mesh_node->getOrCreateStateSet()->setAttribute(program);
		mesh_node->getOrCreateStateSet()->addUniform(new osg::Uniform("ov_color_texture", 0));
		m_NormalUniform = new osg::Uniform("ov_normal_ouput", 0.0f);
		mesh_node->getOrCreateStateSet()->addUniform(m_NormalUniform);

		osg::Uniform* height_uniform = new osg::Uniform("ov_height", 10.0f);
		mesh_node->getOrCreateStateSet()->addUniform(height_uniform);

		m_RadiusUniform = new osg::Uniform("ov_radius", 10.0f);
		mesh_node->getOrCreateStateSet()->addUniform(m_RadiusUniform);

		mesh_node->getOrCreateStateSet()->setAttributeAndModes(new osg::CullFace(), osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
		mesh_node->getOrCreateStateSet()->setAttributeAndModes(new osg::BlendFunc, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
		osg::AlphaFunc* alphaFunc = new osg::AlphaFunc;
		alphaFunc->setFunction(osg::AlphaFunc::GEQUAL, 0.1);
		mesh_node->getOrCreateStateSet()->setAttributeAndModes(alphaFunc, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
		mesh_node->getOrCreateStateSet()->setMode(GL_SAMPLE_ALPHA_TO_COVERAGE_ARB, 1.0);
		std::stringstream vertexShaderSource;
		vertexShaderSource <<
			"uniform mat4 osg_ModelViewProjectionMatrix;\n"
			"uniform float ov_height;\n"
			"uniform float ov_radius;\n"
			"varying vec3 ov_normal;\n"
			"varying vec2 ov_tex_coord0;\n"
			"void main() {\n"
			"gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
			"//float sphere_dist = sqrt(ov_height*ov_height - gl_Vertex.z*gl_Vertex.z);\n"
			"//float norm_height = gl_Vertex.z / ov_height;\n"
			"//float sphere_dist = ov_radius * sqrt(1 - norm_height*norm_height);\n"
			"//ov_normal = normalize(vec3(gl_Vertex.xy,0));//gl_Normal;//normalize(gl_NormalMatrix * gl_Normal);\n"
			"ov_normal = gl_Normal;\n"
			"//ov_normal = normalize( vec3(0, 0, gl_Vertex.z) + (normalize(vec3(gl_Vertex.xy,0)) * sphere_dist));\n"
			"ov_tex_coord0 = gl_MultiTexCoord0.xy;\n"
			"} \n";

		std::stringstream fragmentShaderSource;
		fragmentShaderSource <<
			"#version 120\n"
			"uniform sampler2D ov_color_texture; \n"
			"uniform float ov_normal_ouput = 0; \n"
			"varying vec3 ov_normal; \n"
			"varying vec2 ov_tex_coord0; \n"
			"void main(void)\n"
			"{\n"
			"   vec4 base_color = texture2D(ov_color_texture, ov_tex_coord0.xy); base_color.w *= 2;\n"
			"   vec3 n = ov_normal;//gl_FrontFacing ? ov_normal : -ov_normal; \n"
			"   //n.x = -n.x; \n"
			"   //n.xyz = n.xzy; \n"
			"   //n.z = -n.z; \n"
			"   //ov_normal.y = -temp; \n"
			"   if(ov_normal_ouput > 0.5) \n"
			"      base_color.xyz = (normalize(n) + 1)*0.5; \n"
			"   gl_FragColor = base_color; \n"
			"}\n";

		program->addShader(new osg::Shader(osg::Shader::VERTEX, vertexShaderSource.str().c_str()));
		program->addShader(new osg::Shader(osg::Shader::FRAGMENT, fragmentShaderSource.str().c_str()));
		mesh_node->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
	}
};

struct TextureCamera
{
	osg::ref_ptr<osg::Camera> m_Camera;
	osg::ref_ptr<osg::Texture2D> m_Texture;
	osg::ref_ptr<osg::Image> m_Image;

	TextureCamera(int textureWidth, int textureHeight)
	{
		m_Camera = new osg::Camera();
		m_Camera->setReferenceFrame(osg::Camera::ABSOLUTE_RF);
		m_Camera->setRenderOrder(osg::Camera::PRE_RENDER);

		//Set the camera's viewport to the same size of the texture
		m_Camera->setViewport(0, 0, textureWidth, textureHeight);

		//osg::Vec4 clearColor(30.0/255.0, 41.0/255.0, 18.0/255.0, 0); //pine
		//osg::Vec4 clearColor(16.0/255.0, 20.0/255.0, 8.0/255.0, 0); //spruce
		osg::Vec4 clearColor(27.0 / 255.0, 44.0 / 255.0, 18.0 / 255.0, 0); //birch
		// Set the camera's clear color and masks
		m_Camera->setClearColor(clearColor);
		m_Camera->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		m_Camera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
		//Capture* capture_cb = new Capture("c:/temp/output2.tga");
		//m_Camera->setPostDrawCallback(capture_cb);

		m_Texture = new osg::Texture2D();
		m_Image = new osg::Image();

		//Create the texture image
		m_Image->allocateImage(textureWidth,
			textureHeight,
			1,   // 2D texture is 1 pixel deep
			GL_RGBA,
			GL_UNSIGNED_BYTE);
		m_Image->setInternalTextureFormat(GL_RGBA8);

		// Create the texture object and set the image
		//osg::ref_ptr<osg::Texture2D> texture2D = new osg::Texture2D();
		m_Texture->setImage(m_Image);

		// Attach the texture to the camera. You can also attach
		// the image to the camera instead. Attaching the image may
		// work better in the Windows Remote Desktop
		//camera->attach(osg::Camera::COLOR_BUFFER, texture2D.get());
		int multisampleSamples = 4;
		m_Camera->attach(osg::Camera::COLOR_BUFFER, m_Image.get(), multisampleSamples);
	}

	void UpdateView(const osg::BoundingBox &bb)
	{
		// Set the camera's projection matrix to reflect the
		// world of the geometries that will appear in the texture
		m_Camera->setProjectionMatrixAsOrtho2D(bb.xMin(), bb.xMax(), bb.yMin(), bb.yMax());

		//m_Camera->setProjectionMatrixAsOrtho(bb.xMin(), bb.xMax(), bb.yMin(), bb.yMax(), -10, 7);
		//m_Camera->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);
	}
};

struct ImpostorModel
{
	osg::ref_ptr<osg::Node> Geometry;
	osg::ref_ptr<osg::Image> Atlas;
};


struct ImpostorGenerator
{
	TextureCamera m_TexCamera;
	int m_TextureWidth;
	int m_TextureHeight;
	int m_NumSideViews;
	bool m_TopView;
	osgViewer::Viewer &m_Viewer;

	ImpostorGenerator(osgViewer::Viewer &viewer, int texture_width = 512, int texture_height = 512) : m_TexCamera(texture_width, texture_height),
		m_Viewer(viewer),
		m_TextureWidth(texture_width),
		m_TextureHeight(texture_height),
		m_NumSideViews(8),
		m_TopView(true)
	{
		//add camera
		viewer.setSceneData(m_TexCamera.m_Camera);
	}

	osg::ref_ptr<osg::Image> Render(Model &model, bool normal_map = false)
	{
		normal_map ? model.m_NormalUniform->set(1.0f) : model.m_NormalUniform->set(0.0f);
		m_Viewer.frame(0.1);
		m_Viewer.frame(0.1);
		osg::ref_ptr <osg::Image> out_image = static_cast<osg::Image*>(m_TexCamera.m_Image->clone(osg::CopyOp::DEEP_COPY_IMAGES));
		return out_image;
	}

	void RenderToAtlas(Model &model, osg::ref_ptr<osg::Image> atlas, int slot)
	{
		osg::ref_ptr<osg::Image> color_image = Render(model, false);
		color_image->scaleImage(m_TextureWidth, m_TextureHeight, 1);
		//osgDB::writeImageFile(*color_image, "d:/temp/treetest/temp.tga");
		osg::ref_ptr<osg::Image> normal_image = Render(model, true);
		normal_image->scaleImage(m_TextureWidth, m_TextureHeight, 1);

		atlas->copySubImage(slot * m_TextureWidth, m_TextureHeight, 0, color_image);
		atlas->copySubImage(slot * m_TextureWidth, 0, 0, normal_image);
	}


	osg::ref_ptr<osg::Node> GenerateImpostor(const std::string& model_filename, const std::string impostor_filename, std::string atlas_ext = ".tga")
	{
		osg::ref_ptr<osg::Node> mesh = osgDB::readNodeFile(model_filename);
		Model model(mesh);
		ImpostorModel im = GenerateImpostor(model);
		if (impostor_filename != "")
		{
			//save texture
			const std::string atlas_filename = osgDB::getFilePath(impostor_filename) + "/" + osgDB::getStrippedName(impostor_filename) + atlas_ext;
			const std::string atlas_filename_only = osgDB::getSimpleFileName(atlas_filename);
			im.Atlas->setFileName(atlas_filename_only);
			osgDB::writeImageFile(*im.Atlas, atlas_filename);
			//save model
			osgDB::writeNodeFile(*im.Geometry, impostor_filename);
		}
		return im.Geometry;
	}

	ImpostorModel GenerateImpostor(Model &model)
	{
		m_TexCamera.m_Camera->addChild(model.m_Transform);
		const size_t tot_num_images = m_TopView ? m_NumSideViews + 1 : m_NumSideViews;
		const double rotation_step = 360.0 / (double)(m_NumSideViews);
		ImpostorModel im;

		//Create the texture image
		im.Atlas = new osg::Image();
		im.Atlas->allocateImage(m_TextureWidth*tot_num_images,
			m_TextureHeight * 2,
			1,   // 2D texture is 1 pixel deep
			GL_RGBA,
			GL_UNSIGNED_BYTE);
		im.Atlas->setInternalTextureFormat(GL_RGBA8);

		int atlas_slot = 0;
		osg::ref_ptr <osg::Group> out_model = new osg::Group;
		for (size_t i = 0; i < m_NumSideViews; i++, atlas_slot++)
		{
			const double angle = static_cast<double>(i) * rotation_step;
			const osg::BoundingBox bb = model.SetupSide(angle);
			m_TexCamera.UpdateView(bb);
			RenderToAtlas(model, im.Atlas, i);
			out_model->addChild(CreateSideQuad(bb, angle, atlas_slot, tot_num_images));
		}

		//render top
		if (m_TopView)
		{
			const osg::BoundingBox bb = model.SetupTop();
			m_TexCamera.UpdateView(bb);
			RenderToAtlas(model, im.Atlas, atlas_slot);
			out_model->addChild(CreateTopQuad(bb, atlas_slot, tot_num_images));
		}
		//scale to power of two
		im.Atlas->scaleImage(m_TextureWidth * m_NumSideViews, m_TextureHeight * 2, 1);
		im.Geometry = out_model;
		SetupStateSet(im);
		m_TexCamera.m_Camera->removeChild(model.m_Transform);
		return im;
	}

	osg::ref_ptr <osg::Node> CreateSideQuad(const osg::BoundingBox &bb, float angle, int atlas_slot, int tot_num_images)
	{
		osg::ref_ptr <osg::MatrixTransform> transform = new osg::MatrixTransform;
		transform->setMatrix(osg::Matrixd::rotate(osg::DegreesToRadians(angle), 0, 0, 1));
		transform->addChild(CreateQuad(bb._min, bb._max, atlas_slot, tot_num_images));
		transform->setDataVariance(osg::Object::STATIC);
		return transform;
	}

	osg::ref_ptr <osg::Node> CreateTopQuad(const osg::BoundingBox &bb, int atlas_slot, int tot_num_images)
	{
		osg::ref_ptr <osg::MatrixTransform> transform = new osg::MatrixTransform;
		const osg::Vec3 top_min(bb._min.x(), bb._min.y(), 0);
		const osg::Vec3 top_max(bb._max.x(), bb._max.y(), 0);
		const double height = (bb._max.z() - bb._min.z())*0.5;
		const double scale = 1.0;
		transform->setMatrix(osg::Matrixd::rotate(osg::DegreesToRadians(-90.0), 1, 0, 0) * osg::Matrix::translate(0, 0, height));
		transform->addChild(CreateQuad(top_min*scale, top_max*scale, atlas_slot, tot_num_images));
		transform->setDataVariance(osg::Object::STATIC);
		return transform;
	}

	osg::ref_ptr<osg::Geode> CreateQuad(const osg::Vec3 &min_pos, const osg::Vec3 &max_pos, int atlas_slot, int max_slots)
	{
		osg::Vec3 size = max_pos - min_pos;
		// Create a geode to display t/he texture
		osg::ref_ptr<osg::Geode> geode = new osg::Geode();
		geode->setDataVariance(osg::Object::STATIC);
		// Create a geometry for the geode
		osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry();
		geode->addDrawable(geometry);
		geometry->setDataVariance(osg::Object::STATIC);

		// The coordinates can be different if we want to display the
		// texture at a location different from the coordinates of the
		// geometries inside the texture
		osg::ref_ptr<osg::Vec3Array> vertexArray = new osg::Vec3Array();
		osg::Vec3 p1 = osg::Vec3(min_pos.x(), 0, min_pos.y());
		osg::Vec3 p2 = osg::Vec3(max_pos.x(), 0, min_pos.y());
		osg::Vec3 p3 = osg::Vec3(max_pos.x(), 0, max_pos.y());
		osg::Vec3 p4 = osg::Vec3(min_pos.x(), 0, max_pos.y());

		//osg::Vec3  v1 = p2 - p1;
		//osg::Vec3  v2 = p3 - p1;
		//osg::Vec3 normal = v1 ^ v2;
		//normal.normalize();

		vertexArray->push_back(p1);
		vertexArray->push_back(p2);
		vertexArray->push_back(p3);
		vertexArray->push_back(p4);
		geometry->setVertexArray(vertexArray);


		osg::Vec3 normal(0, -1, 0);
		//osg::Vec3 n1(-1.0, -1.0, 0); n1.normalize();
		//osg::Vec3 n2(1.0, -1.0, 0); n2.normalize();
		osg::ref_ptr<osg::Vec3Array> normalArray = new osg::Vec3Array();
		normalArray->push_back(normal);
		normalArray->push_back(normal);
		normalArray->push_back(normal);
		normalArray->push_back(normal);
		//geometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
		geometry->setNormalArray(normalArray, osg::Array::BIND_PER_VERTEX);

		// The geometry color for the texture mapping should be white
		// (1.f, 1.f, 1.f, 1.f) so that color blending won't affect
		// the texture real color
		osg::ref_ptr<osg::Vec4Array> colorArray = new osg::Vec4Array();
		colorArray->push_back(osg::Vec4(1.f, 1.f, 1.f, 1.f));
		//geometry->setColorArray(colorArray);
		//geometry->setColorBinding(osg::Geometry::BIND_OVERALL);

		// We are using the entire texture by using the four corners
		// of the texture coordinates
		const float u_size = 1.0f / static_cast<float>(max_slots);
		const float u = static_cast<float>(atlas_slot);
		osg::ref_ptr<osg::Vec2Array> texCoordArray = new osg::Vec2Array();
		const float v_start = 0.5f;
		const float v_end = 1.0f;
		texCoordArray->push_back(osg::Vec2(u_size * u, v_start));
		texCoordArray->push_back(osg::Vec2(u_size * (u + 1.0f), v_start));
		texCoordArray->push_back(osg::Vec2(u_size * (u + 1.0f), v_end));
		texCoordArray->push_back(osg::Vec2(u_size * u, v_end));
		geometry->setTexCoordArray(0, texCoordArray);

		// We always create a square texture.
		geometry->addPrimitiveSet(new osg::DrawArrays(GL_TRIANGLE_FAN, 0, 4));

		//geode->getOrCreateStateSet()->setAttribute( new osg::AlphaFunc(osg::AlphaFunc::GEQUAL,0.1f) ,osg::StateAttribute::ON);

		// Make sure that we are using color blending
		// and the transparent bin, since the features may not
		// cover the entire texture and the empty
		// space should be transparent.
		//geode->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
		//osg::ref_ptr<osg::BlendFunc> blendFunc = new osg::BlendFunc;
		//blendFunc->setFunction(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		//geode->getOrCreateStateSet()->setAttributeAndModes(blendFunc);
		return geode;
	}

	void SetupStateSet(ImpostorModel im)
	{
		osg::ref_ptr<osg::Texture2D> atlas_texture = new osg::Texture2D();
		// Add the texture to the geode
		atlas_texture->setImage(im.Atlas);
		im.Geometry->getOrCreateStateSet()->setTextureAttributeAndModes(0, atlas_texture, osg::StateAttribute::ON);
		osg::AlphaFunc* alphaFunction = new osg::AlphaFunc;
		alphaFunction->setFunction(osg::AlphaFunc::GEQUAL, 0.05f);
		im.Geometry->getOrCreateStateSet()->setAttributeAndModes(alphaFunction, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
		osg::CullFace* cull = new osg::CullFace();
		cull->setMode(osg::CullFace::BACK);
		im.Geometry->getOrCreateStateSet()->setAttributeAndModes(cull, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
		im.Geometry->getOrCreateStateSet()->setMode(GL_SAMPLE_ALPHA_TO_COVERAGE_ARB, 1.0);// osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);


		osgUtil::Optimizer optimzer;
		optimzer.optimize(im.Geometry, osgUtil::Optimizer::FLATTEN_STATIC_TRANSFORMS);
		optimzer.optimize(im.Geometry, osgUtil::Optimizer::REMOVE_REDUNDANT_NODES);
		optimzer.optimize(im.Geometry, osgUtil::Optimizer::MERGE_GEODES);
		optimzer.optimize(im.Geometry, osgUtil::Optimizer::MERGE_GEOMETRY);
		//optimzer.optimize(out_group,osgUtil::Optimizer::FLATTEN_STATIC_TRANSFORMS_DUPLICATING_SHARED_SUBGRAPHS);
		//optimzer.optimize(out_group,osgUtil::Optimizer::VERTEX_PRETRANSFORM|osgUtil::Optimizer::VERTEX_POSTTRANSFORM);

		osg::Program* program = new osg::Program;
		im.Geometry->getOrCreateStateSet()->setAttribute(program);
		im.Geometry->getOrCreateStateSet()->addUniform(new osg::Uniform("ov_color_texture", 0));

		std::stringstream vertexShaderSource;
		vertexShaderSource <<
			"uniform mat4 osg_ModelViewProjectionMatrix;\n"
			"varying vec3 ov_normal;\n"
			"varying vec2 ov_tex_coord0;\n"
			"void main() {\n"
			"gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
			"ov_normal = gl_Normal;\n"
			"ov_tex_coord0 = gl_MultiTexCoord0.xy;\n"
			"} \n";

		std::stringstream fragmentShaderSource;
		fragmentShaderSource <<
			"#version 120\n"
			"uniform sampler2D ov_color_texture; \n"
			"varying vec3 ov_normal; \n"
			"varying vec2 ov_tex_coord0; \n"
			"float fastpow(in float x, in float y)\n"
			"{\n"
			"return x / (x + y - y * x);\n"
			"}\n"
			"void main(void)\n"
			"{\n"
			"    vec3 light_dir = normalize(gl_LightSource[0].position.xyz);\n"
			"    vec4 base_color = texture2D(ov_color_texture, ov_tex_coord0.xy); \n"
			"    vec4 coded_normal = texture2D(ov_color_texture, ov_tex_coord0.xy - vec2(0.0,0.5)); \n"
			"    vec3 normal = normalize(gl_NormalMatrix * normalize(coded_normal.xyz * 2.0 - 1));\n"
			"    vec3 plane_normal = gl_NormalMatrix * normalize(ov_normal);\n"
			"    if(ov_normal.z < 0.9) plane_normal.y = 0;\n"
			"    plane_normal = normalize(plane_normal);\n"
			"    float NdotL = max(dot(normal, light_dir), 0);\n"
			"    float NdotC = max(abs(dot(plane_normal, vec3(0,0,-1))), 0);\n"
			"    //if(NdotC < 0.92388) NdotC = 0.0; else NdotC = 1.0;\n"
			"    base_color.xyz = base_color.xyz*NdotL*gl_LightSource[0].diffuse.xyz + base_color.xyz*gl_FrontLightModelProduct.sceneColor.xyz; \n"
			"    base_color.w = mix(0.0, base_color.w, fastpow(NdotC,10)); \n"
			"    //base_color.w = mix(base_color.w, 0.0,1 - NdotC); \n"
			"    gl_FragColor = base_color; \n"
			"}\n";

		program->addShader(new osg::Shader(osg::Shader::VERTEX, vertexShaderSource.str().c_str()));
		program->addShader(new osg::Shader(osg::Shader::FRAGMENT, fragmentShaderSource.str().c_str()));
	}
};