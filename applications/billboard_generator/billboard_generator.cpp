/* OpenSceneGraph example, osgforest.
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
*  THE SOFTWARE.
*/

#include <osg/AlphaFunc>
#include <osg/Billboard>
#include <osg/BlendFunc>
#include <osg/Depth>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Material>
#include <osg/Math>
#include <osg/MatrixTransform>
#include <osg/PolygonOffset>
#include <osg/Projection>
#include <osg/ShapeDrawable>
#include <osg/StateSet>
#include <osg/Switch>
#include <osg/Texture2D>
#include <osg/Image>
#include <osg/TexEnv>
#include <osg/Camera>
#include <osg/VertexProgram>
#include <osg/CullFace>
#include <osg/FragmentProgram>
#include <osg/ComputeBoundsVisitor>
#include <osgDB/WriteFile>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <osg/Texture2DArray>

#include <osgUtil/LineSegmentIntersector>
#include <osgUtil/IntersectionVisitor>
#include <osgUtil/SmoothingVisitor>
#include <osgUtil/Optimizer>


#include <osgText/Text>

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include <osgGA/StateSetManipulator>
#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>
#include <osgGA/KeySwitchMatrixManipulator>
#include <osgGA/StateSetManipulator>
#include <osgGA/AnimationPathManipulator>
#include <osgGA/TerrainManipulator>
#include <osgGA/SphericalManipulator>

#include <iostream>
#include <sstream>
// for the grid data..
osg::ref_ptr<osg::Texture2D> texture2D = new osg::Texture2D();
osg::ref_ptr<osg::Camera> camera = new osg::Camera();
osg::ref_ptr<osg::Image> image = new osg::Image();


class Capture : public osg::Camera::DrawCallback { 
public: 
	Capture( const std::string& imagename ) : m_imageName(imagename) {} 
	void setFilename(const std::string  &filename) {m_imageName = filename;}
	virtual void operator () (osg::RenderInfo& renderInfo) const { 
		/*osg::ref_ptr<osg::Image> image = new osg::Image; 
		image->allocateImage(512,
		512,
		1,   // 2D texture is 1 pixel deep
		GL_RGBA,
		GL_UNSIGNED_BYTE);
		image->setInternalTextureFormat(GL_RGBA8);*/
		//camera->getOrCreateStateSet()->setTextureAttributeAndModes(0, texture2D, osg::StateAttribute::ON); 
		//glBindTexture(GL_TEXTURE_2D,texture2D->getTextureObject(renderInfo.getContextID())->_id);
		//image->readPixels(0,0,512,512,GL_RGBA,GL_UNSIGNED_BYTE);

		//renderInfo.getState()->applyAttribute(texture2D); 
		//osg::ref_ptr<osg::Image> image = new osg::Image;
		//image->readImageFromCurrentTexture(renderInfo.getContextID(),false,GL_UNSIGNED_BYTE); 

		//texture2D->apply( *( renderInfo.getState() ) ); 
		//image->readImageFromCurrentTexture(renderInfo.getContextID(), true); 
		osgDB::writeImageFile(*image, m_imageName); 
	} 

private: 
	std::string m_imageName; 
}; 

osg::ref_ptr<osg::Geode> CreateGeometry(const osg::Vec3 &min_pos, const osg::Vec3 &max_pos, osg::ref_ptr<osg::Texture2D> texture)
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
	vertexArray->push_back(osg::Vec3(min_pos.x(), 0, min_pos.y()));
	vertexArray->push_back(osg::Vec3(max_pos.x(), 0, min_pos.y()));
	vertexArray->push_back(osg::Vec3(max_pos.x(), 0, max_pos.y()));
	vertexArray->push_back(osg::Vec3(min_pos.x(), 0, max_pos.y()));
	geometry->setVertexArray(vertexArray);


	osg::Vec3 n1(-1.0, -1.0, 0);n1.normalize();
	osg::Vec3 n2(1.0, -1.0, 0);n2.normalize();
	osg::ref_ptr<osg::Vec3Array> normalArray = new osg::Vec3Array();
	normalArray->push_back(n1);
	normalArray->push_back(n2);
	normalArray->push_back(n2);
	normalArray->push_back(n1);
	geometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
	geometry->setNormalArray(normalArray);

	// The geometry color for the texture mapping should be white
	// (1.f, 1.f, 1.f, 1.f) so that color blending won't affect
	// the texture real color
	osg::ref_ptr<osg::Vec4Array> colorArray = new osg::Vec4Array();
	colorArray->push_back(osg::Vec4(1.f, 1.f, 1.f, 1.f));
	//geometry->setColorArray(colorArray);
	//geometry->setColorBinding(osg::Geometry::BIND_OVERALL);

	// We are using the entire texture by using the four corners
	// of the texture coordinates
	osg::ref_ptr<osg::Vec2Array> texCoordArray = new osg::Vec2Array();
	texCoordArray->push_back(osg::Vec2(0.f, 0.f));
	texCoordArray->push_back(osg::Vec2(1.f, 0.f));
	texCoordArray->push_back(osg::Vec2(1.f, 1.f));
	texCoordArray->push_back(osg::Vec2(0.f, 1.f));
	geometry->setTexCoordArray(0, texCoordArray);

	// We always create a square texture.
	geometry->addPrimitiveSet(new osg::DrawArrays(GL_TRIANGLE_FAN, 0, 4));

	// Add the texture to the geode
	geometry->getOrCreateStateSet()->setTextureAttributeAndModes(0, texture, osg::StateAttribute::ON);
	osg::AlphaFunc* alphaFunction = new osg::AlphaFunc;
	alphaFunction->setFunction(osg::AlphaFunc::GEQUAL,0.1f);
	geometry->getOrCreateStateSet()->setAttributeAndModes( alphaFunction, osg::StateAttribute::ON );
	osg::CullFace* cull = new osg::CullFace(); 
	cull->setMode(osg::CullFace::BACK); 
	geometry->getOrCreateStateSet()->setAttributeAndModes(cull, osg::StateAttribute::ON); 

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

int main( int argc, char **argv )
{
	osg::ArgumentParser arguments(&argc,argv);

	arguments.getApplicationUsage()->setApplicationName(arguments.getApplicationName());
	arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is a utility for creating billboards from 3D models.");
	arguments.getApplicationUsage()->addCommandLineOption("--model <filename>","The input model.");
	

	// if user request help write it out to cout.
	if (arguments.read("-h") || arguments.read("--help"))
	{
		arguments.getApplicationUsage()->write(std::cout);
		return 1;
	}

	std::string filename;
	if (!arguments.read("--model",filename))
	{
		std::cout << "You need to provide a model";
		return 0;
	}
	std::string out_path = osgDB::getFilePath(filename) + "/";


	int textureWidth = 512;
	int textureHeight= 512;

	if (arguments.read("--tw",textureWidth))
	{
		
	}

	if (arguments.read("--th",textureHeight))
	{

	}

		
	osgViewer::Viewer viewer(arguments);
	//osg::Group* mesh_ = 
	osg::Node* mesh_node = osgDB::readNodeFile(filename);//"C:/dev/dependencies/osg/OpenSceneGraph-Data-3.0.0/trees/birch.osg");
 	//osg::Node* mesh_node = osgDB::readNodeFile("C:/temp/exp_tress/birch/birch.obj");
//	  osg::Node* mesh_node = osgDB::readNodeFile("C:/temp/exp_tress/pine01/pine01_no_alpha.osg");
	mesh_node->getOrCreateStateSet()->setMode( GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE );
	osg::MatrixTransform* transform = new osg::MatrixTransform;
	transform->setMatrix(osg::Matrix::scale(1,1,1)*osg::Matrixd::rotate(osg::DegreesToRadians(-90.0),1,0,0));
	transform->addChild(mesh_node);

	osg::ComputeBoundsVisitor  cbv;
	osg::BoundingBox &bb(cbv.getBoundingBox());
	transform->accept(cbv);

	float minx = bb._min.x();
	float maxx = bb._max.x();
	float miny = bb._min.y();
	float maxy = bb._max.y();

	float minz = bb._min.z();
	float maxz = bb._max.z();

	

	// Create a render-to-texture camera with an absolute
	// reference frame and pre-render
	//osg::ref_ptr<osg::Camera> camera = new osg::Camera();
	camera->setReferenceFrame(osg::Camera::ABSOLUTE_RF);
	camera->setRenderOrder(osg::Camera::PRE_RENDER);

	// Set the camera's projection matrix to reflect the
	// world of the geometries that will appear in the texture
	camera->setProjectionMatrixAsOrtho2D(minx, maxx, miny, maxy);

	//Set the camera's viewport to the same size of the texture
	camera->setViewport(0, 0, textureWidth, textureHeight);

	//osg::Vec4 clearColor(30.0/255.0, 41.0/255.0, 18.0/255.0, 0); //pine
	//osg::Vec4 clearColor(16.0/255.0, 20.0/255.0, 8.0/255.0, 0); //spruce
	osg::Vec4 clearColor(27.0/255.0, 44.0/255.0, 18.0/255.0, 0); //birch
	// Set the camera's clear color and masks
	camera->setClearColor(clearColor);
	camera->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	camera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT); 
	Capture* capture_cb = new Capture("c:/temp/output2.tga");
	camera->setPostDrawCallback( capture_cb); 

	//Create the texture image
	image->allocateImage(textureWidth,
		textureHeight,
		1,   // 2D texture is 1 pixel deep
		GL_RGBA,
		GL_UNSIGNED_BYTE);
	image->setInternalTextureFormat(GL_RGBA8);

	// Create the texture object and set the image
	//osg::ref_ptr<osg::Texture2D> texture2D = new osg::Texture2D();
	texture2D->setImage(image);

	// Attach the texture to the camera. You can also attach
	// the image to the camera instead. Attaching the image may
	// work better in the Windows Remote Desktop
	//camera->attach(osg::Camera::COLOR_BUFFER, texture2D.get());
	camera->attach(osg::Camera::COLOR_BUFFER, image.get());


	// Add the geometries that will appear in the texture to the camera
	camera->addChild(transform);

	// Create a geode to display the texture
	osg::ref_ptr<osg::Geode> geode = new osg::Geode();

	// Create a geometry for the geode
	osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry();
	geode->addDrawable(geometry);

	// The coordinates can be different if we want to display the
	// texture at a location different from the coordinates of the
	// geometries inside the texture
	osg::ref_ptr<osg::Vec3Array> vertexArray = new osg::Vec3Array();
	vertexArray->push_back(osg::Vec3(minx, miny, minz));
	vertexArray->push_back(osg::Vec3(maxx, miny, minz));
	vertexArray->push_back(osg::Vec3(maxx, maxy, minz));
	vertexArray->push_back(osg::Vec3(minx, maxy, minz));
	geometry->setVertexArray(vertexArray);

	// The geometry color for the texture mapping should be white
	// (1.f, 1.f, 1.f, 1.f) so that color blending won't affect
	// the texture real color
	osg::ref_ptr<osg::Vec4Array> colorArray = new osg::Vec4Array();
	colorArray->push_back(osg::Vec4(1.f, 1.f, 1.f, 1.f));
	geometry->setColorArray(colorArray);
	geometry->setColorBinding(osg::Geometry::BIND_OVERALL);

	// We are using the entire texture by using the four corners
	// of the texture coordinates
	osg::ref_ptr<osg::Vec2Array> texCoordArray = new osg::Vec2Array();
	texCoordArray->push_back(osg::Vec2(0.f, 0.f));
	texCoordArray->push_back(osg::Vec2(1.f, 0.f));
	texCoordArray->push_back(osg::Vec2(1.f, 1.f));
	texCoordArray->push_back(osg::Vec2(0.f, 1.f));
	geometry->setTexCoordArray(0, texCoordArray);

	// We always create a square texture.
	geometry->addPrimitiveSet(new osg::DrawArrays(GL_TRIANGLE_FAN, 0, 4));

	// Add the texture to the geode
	geode->getOrCreateStateSet()->setTextureAttributeAndModes(0, texture2D, osg::StateAttribute::ON);

	// Make sure that we are using color blending
	// and the transparent bin, since the features may not
	// cover the entire texture and the empty
	// space should be transparent.
	geode->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
	osg::ref_ptr<osg::BlendFunc> blendFunc = new osg::BlendFunc;
	blendFunc->setFunction(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	geode->getOrCreateStateSet()->setAttributeAndModes(blendFunc);

	// Add both the rtt camera and the geode to the scene graph
	osg::Group *sceneGraph = new osg::Group ();
	sceneGraph->addChild(camera);
	sceneGraph->addChild(geode);

	
	viewer.addEventHandler(new osgViewer::StatsHandler);

	viewer.addEventHandler(new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()));

	viewer.setSceneData(sceneGraph);

	viewer.realize();

	//viewer.frame(0.1);
	//viewer.frame(0.1);
	const size_t num_images = 4;
	const double rotation_step = 360.0/(double) num_images;
	
	std::string billboard_prefix = osgDB::getStrippedName(filename);
	osg::Group* out_group = new osg::Group();
	for(size_t i =0 ; i < num_images; i++)
	{
		osg::MatrixTransform* out_transform = new osg::MatrixTransform;
		double angle = i*rotation_step;
		transform->setMatrix(osg::Matrix::scale(1,1,1)*osg::Matrixd::rotate(osg::DegreesToRadians(angle),0,0,1)*osg::Matrixd::rotate(osg::DegreesToRadians(-90.0),1,0,0));
		std::stringstream ss;
		ss << out_path << billboard_prefix << "_bb_" << i << ".tga";// << std::endl;
		capture_cb->setFilename(ss.str());
		//update twice to get correct rotation
		viewer.frame(0.1);
		viewer.frame(0.1);
		osg::ref_ptr<osg::Texture2D> out_texture = new osg::Texture2D();
		osg::Image* out_image = static_cast<osg::Image*>(image->clone(osg::CopyOp::DEEP_COPY_IMAGES));
		out_texture->setImage(out_image);
		out_image->setFileName(osgDB::getSimpleFileName(ss.str()));
		out_transform->setMatrix(osg::Matrix::scale(1,1,1)*osg::Matrixd::rotate(osg::DegreesToRadians(angle),0,0,1));
		out_transform->addChild(CreateGeometry(bb._min , bb._max, out_texture));
		out_transform->setDataVariance(osg::Object::STATIC);
		out_group->addChild(out_transform);
	}

	if (!viewer.getCameraManipulator() && viewer.getCamera()->getAllowEventFocus())
	{
		viewer.setCameraManipulator(new osgGA::TrackballManipulator());
	}
	osgUtil::Optimizer optimzer;
	optimzer.optimize(out_group,osgUtil::Optimizer::FLATTEN_STATIC_TRANSFORMS);
	optimzer.optimize(out_group,osgUtil::Optimizer::REMOVE_REDUNDANT_NODES);
	optimzer.optimize(out_group,osgUtil::Optimizer::MERGE_GEODES);
	optimzer.optimize(out_group,osgUtil::Optimizer::MERGE_GEOMETRY);
	//optimzer.optimize(out_group,osgUtil::Optimizer::TEXTURE_ATLAS_BUILDER);
	//optimzer.optimize(out_group,osgUtil::Optimizer::FLATTEN_STATIC_TRANSFORMS_DUPLICATING_SHARED_SUBGRAPHS);
	//optimzer.optimize(out_group,osgUtil::Optimizer::VERTEX_PRETRANSFORM|osgUtil::Optimizer::VERTEX_POSTTRANSFORM);
	//optimzer.optimize(out_group,osgUtil::Optimizer::MERGE_GEOMETRY);
	//optimzer.optimize(out_group,osgUtil::Optimizer::TEXTURE_ATLAS_BUILDER);

	sceneGraph->removeChild(camera);
	sceneGraph->removeChild(geode);
	sceneGraph->addChild(out_group);

	//generate output model
	std::string out_file = osgDB::getStrippedName(filename);
	out_file = out_path + out_file + "_bb.osg";
	osgDB::writeNodeFile(*out_group,out_file);

	if (!arguments.read("--view"))
	{
	//	return 0;
	}
	while(!viewer.done())
	{
		viewer.frame(0.1);
	}

	return 0;
	// use an ArgumentParser object to manage the program arguments.
	/*	osg::ArgumentParser arguments(&argc,argv);

	// construct the viewer.
	osgViewer::Viewer viewer(arguments);

	//unsigned int numTreesToCreate = 2000;
	//arguments.read("--trees",numTreesToCreate);
	//unsigned int maxNumTreesPerCell = sqrtf(static_cast<float>(numTreesToCreate));
	//arguments.read("--trees-per-cell",maxNumTreesPerCell);
	//osg::ref_ptr<osgVegetation::ForestTechniqueManager> ttm = new osgVegetation::ForestTechniqueManager;

	// add the stats handler
	viewer.addEventHandler(new osgViewer::StatsHandler);

	//viewer.addEventHandler(new TechniqueEventHandler(ttm.get()));
	viewer.addEventHandler(new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()));


	osg::ref_ptr<osgGA::KeySwitchMatrixManipulator> keyswitchManipulator = new osgGA::KeySwitchMatrixManipulator;

	keyswitchManipulator->addMatrixManipulator( '1', "Trackball", new osgGA::TrackballManipulator() );
	keyswitchManipulator->addMatrixManipulator( '2', "Flight", new osgGA::FlightManipulator() );
	keyswitchManipulator->addMatrixManipulator( '3', "Drive", new osgGA::DriveManipulator() );
	keyswitchManipulator->addMatrixManipulator( '4', "Terrain", new osgGA::TerrainManipulator() );
	keyswitchManipulator->addMatrixManipulator( '5', "Orbit", new osgGA::OrbitManipulator() );
	keyswitchManipulator->addMatrixManipulator( '6', "FirstPerson", new osgGA::FirstPersonManipulator() );
	keyswitchManipulator->addMatrixManipulator( '7', "Spherical", new osgGA::SphericalManipulator() );
	viewer.setCameraManipulator( keyswitchManipulator.get() );

	viewer.setSceneData(osgDB::readNodeFile("C:/temp/exp_tress/birch/birch.obj"));

	return viewer.run();*/
}
