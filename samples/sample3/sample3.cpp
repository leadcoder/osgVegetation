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
#include <osg/TextureBuffer>
#include <osg/Image>
#include <osg/TexEnv>
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
#include "BRTShaderInstancing2.h"
#include "TerrainQuery.h"

int main( int argc, char **argv )
{
	// use an ArgumentParser object to manage the program arguments.
	osg::ArgumentParser arguments(&argc,argv);

	// construct the viewer.
	osgViewer::Viewer viewer(arguments);
	
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
	
	osgDB::Registry::instance()->getDataFilePathList().push_back("../data");
	osgDB::Registry::instance()->getDataFilePathList().push_back("./data"); //hack to be able to runt from GCC out dir

	//setup optimization variables
	std::string opt_env= "OSG_OPTIMIZER=COMBINE_ADJACENT_LODS SHARE_DUPLICATE_STATE MERGE_GEOMETRY MAKE_FAST_GEOMETRY CHECK_GEOMETRY OPTIMIZE_TEXTURE_SETTINGS STATIC_OBJECT_DETECTION";
#ifdef WIN32
	_putenv(opt_env.c_str());
#else
	char * writable = new char[opt_env.size() + 1];
	std::copy(opt_env.begin(), opt_env.end(), writable);
	writable[opt_env.size()] = '\0'; // don't forget the terminating 0
	putenv(writable);
	delete[] writable;
#endif

	//grass data
	osgVegetation::BillboardLayer  grass_l0("billboards/grass0.png", 40);

	grass_l0.Density = 0.1;
	grass_l0.Height.set(0.8,0.8);
	grass_l0.Width.set(1.0,1.0);
	grass_l0.Scale.set(0.8,0.9);
	grass_l0.ColorIntensity.set(3.0,3.0);
	grass_l0.TerrainColorRatio = 1.0;
	grass_l0.UseTerrainIntensity = false;

	osgVegetation::BillboardLayerVector grass_layers;

	grass_layers.push_back(grass_l0);

	osgVegetation::BillboardData grass_data(grass_layers, true,0.2,true);
	grass_data.CastShadows = false;
	//grass_data.UseFog = false;
	grass_data.Type = osgVegetation::BT_ROTATED_QUAD;
	grass_data.ReceiveShadows = false;

	osg::BoundingBox bb;
   bb._min.set(-50,-50,0);
   bb._max.set(50,50,3);

   //osg::Image* image = osgDB::readImageFile("C:/dev/OSGLab/osgVegetation/repo/samples/data/SP27GTIF.TIF.gdal");
   //Create texture

  
   // Assign the texture to the image we read from file: 
   
   osg::HeightField *grid = osgDB::readHeightFieldFile("C:/dev/OSGLab/osgVegetation/repo/samples/data/SP27GTIF.TIF.gdal"); 
   grid->setOrigin(osg::Vec3(0.0, 0.0, 0.0)); 
   double radius = 6371 * 1000;
   float xsize = (radius * osg::PI * grid->getXInterval()) / 180.0; 
   float ysize = (radius * osg::PI * grid->getYInterval()) / 180.0; 
   float xint = grid->getXInterval();
   float yint = grid->getYInterval();

 //  grid->setXInterval(xsize); 
   //grid->setYInterval(ysize);


   osg::Image* image = new osg::Image();

   //image->allocateImage(grid->getNumColumns(), grid->getNumRows(), 1, GL_LUMINANCE, GL_FLOAT);
   image->allocateImage(512, 512, 1, GL_LUMINANCE, GL_FLOAT);
   //image->setInternalTextureFormat( GL_LUMINANCE32F_ARB );
   image->setInternalTextureFormat( GL_LUMINANCE_FLOAT32_ATI  );
   
   //osg::Vec4 color2 = image->getColor(osg::Vec3(0.6,0.6,0.0));
   //float* data = reinterpret_cast<float*>(image->data());
   for(int i =0;i< 512;i++)
   {
	   for(int j =0; j<   512 ;j++)
	   {
		   float h = 10;
		   // data[1*(j + i*grid->getNumColumns())] = h;
		   *((float*)(image->data(j,i))) =h;

	   }
   }
   /* for(int i =0;i< grid->getNumRows();i++)
   {
	   for(int j =0; j<   grid->getNumColumns() ;j++)
	   {
		   float h = grid->getHeight(j,i);
		  // data[1*(j + i*grid->getNumColumns())] = h;
		   *((float*)(image->data(j,i))) =h;
		   
	   }
   }*/
  // image->dirty();
   
   //int w = image->s(), h = image->t(), nw, nh; 
   //nw = image->computeNearestPowerOfTwo(w); 
   //nh = image->computeNearestPowerOfTwo(h); 
   //unsigned int bits = image->getPixelSizeInBits();
  // image->scaleImage(nw, nh, image->r()); 
   //image->dirty();

   osg::ref_ptr<osg::Texture2D> tex = new osg::Texture2D;
   tex->setImage(image);
   tex->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::NEAREST);
   tex->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::NEAREST);
   tex->setResizeNonPowerOfTwoHint(false);

   osg::Geode* pGeode = new osg::Geode();
   pGeode->addDrawable( new osg::ShapeDrawable(grid) );

	   
   
	osgVegetation::BRTShaderInstancing2* test = new osgVegetation::BRTShaderInstancing2(grass_data);
	osg::Node* grass_node  = test->create(tex,grass_data, 40, bb);

	//Create root node
	osg::Group* group = new osg::Group;
	group->addChild(grass_node);
	//group->addChild(pGeode);

	  //load terrain
	osg::ref_ptr<osg::Node> terrain = osgDB::readNodeFile("C:/dev/gass/samples/common/data/sceneries/osg_demo/terrain.3ds");

	group->addChild(terrain);

	viewer.setSceneData(group);

	return viewer.run();
}
