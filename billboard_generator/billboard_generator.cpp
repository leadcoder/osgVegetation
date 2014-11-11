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
#include <osg/FragmentProgram>
#include <osg/ComputeBoundsVisitor>
#include <osgDB/WriteFile>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osg/Texture2DArray>

#include <osgUtil/LineSegmentIntersector>
#include <osgUtil/IntersectionVisitor>
#include <osgUtil/SmoothingVisitor>

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

void CreateGeometry(const osg::Vec3 &min_pos, const osg::Vec3 &max_pos)
{
	osg::Vec3 size = max_pos - min_pos;
	// Create a geode to display t/he texture
	osg::ref_ptr<osg::Geode> geode = new osg::Geode();

	// Create a geometry for the geode
	osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry();
	geode->addDrawable(geometry);

	// The coordinates can be different if we want to display the
	// texture at a location different from the coordinates of the
	// geometries inside the texture
	osg::ref_ptr<osg::Vec3Array> vertexArray = new osg::Vec3Array();
	vertexArray->push_back(osg::Vec3(min_pos.x(), min_pos.y(), min_pos.z()));
	vertexArray->push_back(osg::Vec3(max_pos.x(), min_pos.y(), min_pos.z()));
	vertexArray->push_back(osg::Vec3(max_pos.x(), max_pos.y(), min_pos.z()));
	vertexArray->push_back(osg::Vec3(min_pos.x(), max_pos.y(), min_pos.z()));
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

}

int main( int argc, char **argv )
{

	//osg::Group* mesh_ = 
	osg::Node* mesh_node = osgDB::readNodeFile("C:/temp/exp_tress/birch/birch.obj");
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

	int textureWidth = 512;
	int textureHeight= 512;

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

	osg::Vec4 clearColor(1,1,1,0);
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

	osg::ArgumentParser arguments(&argc,argv);

	// construct the viewer.
	osgViewer::Viewer viewer(arguments);
	viewer.addEventHandler(new osgViewer::StatsHandler);

	//viewer.addEventHandler(new TechniqueEventHandler(ttm.get()));
	viewer.addEventHandler(new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()));

	viewer.setSceneData(sceneGraph);

	viewer.realize();

	viewer.frame(0.1);
	const size_t num_images = 8;
	const double rotation_step = 360.0/(double) num_images;
	//osgDB::writeImageFile(*image, "c:/temp/output.tga"); 
	for(size_t i =0 ; i < num_images; i++)
	{
		double angle = i*rotation_step;
		transform->setMatrix(osg::Matrix::scale(1,1,1)*osg::Matrixd::rotate(osg::DegreesToRadians(angle),0,0,1)*osg::Matrixd::rotate(osg::DegreesToRadians(-90.0),1,0,0));
		std::stringstream ss;
		ss << "c:/temp/billboard_" << i << ".tga";// << std::endl;
		capture_cb->setFilename(ss.str());
		//update twice to get correct rotation
		viewer.frame(0.1);
		viewer.frame(0.1);
	}


	/*transform->setMatrix(osg::Matrix::scale(1,1,1)*osg::Matrixd::rotate(osg::DegreesToRadians(90.0),0,0,1)*osg::Matrixd::rotate(osg::DegreesToRadians(-90.0),1,0,0));
	camera->setPostDrawCallback( new Capture("c:/temp/output2.tga") ); 
	viewer.frame(0.1);
	viewer.frame(0.1);
	transform->setMatrix(osg::Matrix::scale(1,1,1)*osg::Matrixd::rotate(osg::DegreesToRadians(45.0),0,0,1)*osg::Matrixd::rotate(osg::DegreesToRadians(-90.0),1,0,0));
	camera->setPostDrawCallback( new Capture("c:/temp/output3.tga") ); 
	viewer.frame(0.1);
	viewer.frame(0.1);
	//osgDB::writeImageFile(*image, "c:/test/output2.tga"); 
	camera->setPostDrawCallback(NULL);
	//save texture!
	*/
	//image->readImageFromCurrentTexture(0,false);
	//image->readPixels(0,0,textureWidth,textureHeight,GL_RGBA,GL_UNSIGNED_BYTE);
	//osgDB::writeImageFile(*image,"c:/temp/test.tga");

	if (!viewer.getCameraManipulator() && viewer.getCamera()->getAllowEventFocus())
	{
		viewer.setCameraManipulator(new osgGA::TrackballManipulator());
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
