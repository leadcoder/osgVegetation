
#include "impostor_generator.h"
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
#include <osg/Multisample>

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


int main(int argc, char **argv)
{
	osg::ArgumentParser arguments(&argc, argv);

	arguments.getApplicationUsage()->setApplicationName(arguments.getApplicationName());
	arguments.getApplicationUsage()->setDescription(arguments.getApplicationName() + " is a utility for creating billboards from 3D models.");
	arguments.getApplicationUsage()->addCommandLineOption("--model <filename>", "The input model.");

	// if user request help write it out to cout.
	if (arguments.read("-h") || arguments.read("--help"))
	{
		arguments.getApplicationUsage()->write(std::cout);
		return 1;
	}

	std::string filename;
	if (!arguments.read("--model", filename))
	{
		std::cout << "You need to provide a model";
		return 0;
	}

	const std::string out_path = osgDB::getFilePath(filename) + "/";

	int textureWidth = 512;
	int textureHeight = 512;

	arguments.read("--tw", textureWidth);
	arguments.read("--th", textureHeight);


	osg::DisplaySettings::instance()->setNumMultiSamples(4);

	osgViewer::Viewer viewer(arguments);

#if 0
	viewer.addEventHandler(new osgViewer::StatsHandler);

	viewer.addEventHandler(new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()));

	viewer.setSceneData(model.m_Transform);

	viewer.realize();



	if (!viewer.getCameraManipulator() && viewer.getCamera()->getAllowEventFocus())
	{
		viewer.setCameraManipulator(new osgGA::TrackballManipulator());
	}

	while (!viewer.done())
	{
		viewer.frame(0.1);
	}
	return 0;
#endif

	viewer.addEventHandler(new osgViewer::StatsHandler);
	viewer.addEventHandler(new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()));
	viewer.setUpViewInWindow(100, 100, 800, 600);
	viewer.realize();
	
	ImpostorGenerator generator(viewer, textureWidth, textureHeight);
	//osg::ref_ptr<osg::Node> mesh = osgDB::readNodeFile(filename);
	//Model model(mesh);
	//ImposterModel im = generator.GenerateImposter(model);
	
	const std::string out_model_name = osgDB::getStrippedName(filename);
	const std::string out_file = out_path + out_model_name + "_bb.osg";

	osg::ref_ptr<osg::Node> impostor = generator.GenerateImpostor(filename, out_file, ".tga");

	
	if (!viewer.getCameraManipulator() && viewer.getCamera()->getAllowEventFocus())
	{
		viewer.setCameraManipulator(new osgGA::TrackballManipulator());
	}
	osg::Group* sceneGraph = new osg::Group();
	osg::LOD* lod = new osg::LOD();
	float sdist = 20;

	osg::ref_ptr <osg::Node> lod0 = osgDB::readNodeFile(filename);
	lod0->getOrCreateStateSet()->setAttributeAndModes(new osg::CullFace(), osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
	lod0->getOrCreateStateSet()->setAttributeAndModes(new osg::BlendFunc, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
	osg::AlphaFunc* alphaFunc = new osg::AlphaFunc;
	alphaFunc->setFunction(osg::AlphaFunc::GEQUAL, 0.01);
	lod0->getOrCreateStateSet()->setAttributeAndModes(alphaFunc, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
	lod0->getOrCreateStateSet()->setMode(GL_SAMPLE_ALPHA_TO_COVERAGE_ARB, 1.0);// osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
	
	lod->addChild(lod0, 0, sdist);
	lod->addChild(impostor, sdist, 20000);
	sceneGraph->addChild(lod);

	viewer.setSceneData(sceneGraph);
	if (!arguments.read("--view"))
	{
		//	return 0;
	}
	while (!viewer.done())
	{
		viewer.frame(0.1);
	}

	return 0;
	
}
