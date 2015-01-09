#include <osg/Geode>
#include <osg/Geometry>
#include <osg/MatrixTransform>
#include <osg/StateSet>
#include <osgDB/WriteFile>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osg/ComputeBoundsVisitor>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>

#include <iostream>
#include <sstream>

#include "BillboardQuadTreeScattering.h"
#include "MeshQuadTreeScattering.h"
#include "Serializer.h"

#include "TerrainQuery.h"

int main( int argc, char **argv )
{
	// use an ArgumentParser object to manage the program arguments.
	osg::ArgumentParser arguments(&argc,argv);
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
	/////////////////////////////////////////////////////
	arguments.getApplicationUsage()->addCommandLineOption("--config <filename>","Configuration file");

	unsigned int helpType = 0;
	if ((helpType = arguments.readHelpType()))
	{
		arguments.getApplicationUsage()->write(std::cout, helpType);
		return 1;
	}

	// report any errors if they have occurred when parsing the program arguments.
	if (arguments.errors())
	{
		arguments.writeErrorMessages(std::cout);
		return 1;
	}

	osg::BoundingBox bounding_box;
	bool useBBox = false;
	double xmin=0,xmax=0,ymin=0,ymax=0,zmin=-1,zmax=1;
	
	if(arguments.read("--bounding_box",xmin,ymin,xmax,ymax))
	{
		useBBox = true;
	}

	//Load terrain
	osg::ref_ptr<osg::Group> group = new osg::Group;
	osg::Node* terrain = NULL;
	std::string terrain_file;
	if(arguments.read("--terrain",terrain_file))
	{
		terrain = osgDB::readNodeFile(terrain_file);
		if(!terrain)
		{
			std::cerr << "Failed to load terrain: " + terrain_file + "\n";
			return 0;
		}

		//add terrain path 
		const std::string terrain_path = osgDB::getFilePath(terrain_file);
		osgDB::Registry::instance()->getDataFilePathList().push_back(terrain_path);  


		group->addChild(terrain);
		osg::ComputeBoundsVisitor  cbv;
		osg::BoundingBox &bb(cbv.getBoundingBox());
		terrain->accept(cbv);
		bounding_box = bb;
		if(useBBox)
		{
			bounding_box._min.set(xmin,ymin,bounding_box._min.z());
			bounding_box._min.set(xmax,ymax,bounding_box._max.z());
		}
	}

	std::string config_file;
	if(!arguments.read("--vegetation_config",config_file))
	{
		std::cerr << "no config provided\n";
		return 0;
	}

	osgVegetation::Serializer serializer;
	try
	{
		osgVegetation::BillboardData bb_data = serializer.loadBillboardData(config_file);
		osgVegetation::TerrainQuery tq(terrain);
		osgVegetation::BillboardQuadTreeScattering scattering(&tq);
		osg::Node* bb_node = scattering.generate(bounding_box,bb_data);
		
		group->addChild(bb_node);
		osgDB::writeNodeFile(*group,"c:/temp/tree_veg.ive");
	}

	catch(std::exception& e)
	{
		std::cerr << e.what();
		return 0;
	}
	return 0;
}
