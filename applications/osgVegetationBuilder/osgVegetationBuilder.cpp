#include <osg/Geometry>
#include <osg/ProxyNode>
#include <osgDB/WriteFile>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osg/ComputeBoundsVisitor>
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
/*	std::string opt_env= "OSG_OPTIMIZER=COMBINE_ADJACENT_LODS SHARE_DUPLICATE_STATE MERGE_GEOMETRY MAKE_FAST_GEOMETRY CHECK_GEOMETRY OPTIMIZE_TEXTURE_SETTINGS STATIC_OBJECT_DETECTION";
#ifdef WIN32
	_putenv(opt_env.c_str());
#else
	char * writable = new char[opt_env.size() + 1];
	std::copy(opt_env.begin(), opt_env.end(), writable);
	writable[opt_env.size()] = '\0'; // don't forget the terminating 0
	putenv(writable);
	delete[] writable;
#endif
*/
	arguments.getApplicationUsage()->addCommandLineOption("--vegetation_config <filename>","Configuration file");
	arguments.getApplicationUsage()->addCommandLineOption("--environment_config <filename>", "Environment config file");
	arguments.getApplicationUsage()->addCommandLineOption("--terrain_query_config <filename>", "Terrain query config file");
	
	arguments.getApplicationUsage()->addCommandLineOption("--out","out file");
	arguments.getApplicationUsage()->addCommandLineOption("--terrain","Terrain file");
	arguments.getApplicationUsage()->addCommandLineOption("--seed_value","Seed value");

	arguments.getApplicationUsage()->addCommandLineOption("--bounding_box <x.min x-max y-min y-max>","Optional bounding box");
	arguments.getApplicationUsage()->addCommandLineOption("--paged_lod","Optional save paged LOD database");
	arguments.getApplicationUsage()->addCommandLineOption("--save_terrain","Optional inject terrain in database");

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

	osg::BoundingBoxd bounding_box;
	bool useBBox = false;

	double xmin = 0, xmax = 0, ymin = 0, ymax = 0;// , zmin = -1, zmax = 1;

	if(arguments.read("--bounding_box",xmin,ymin,xmax,ymax))
	{
		useBBox = true;
	}

	bool pagedLOD = false;
	if(arguments.read("--paged_lod"))
	{
		pagedLOD = true;
	}

	bool save_terrain = false;
	if(arguments.read("--save_terrain"))
	{
		save_terrain = true;
	}

	std::string out_file;
	if(!arguments.read("--out", out_file))
	{
		std::cerr << "No out file specified\n";
		return 0;
	}

	int seed_value  = 0;
	if(arguments.read("--seed_value", seed_value))
	{
		std::cout << "Using seed" << seed_value << "\n";
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

		osg::ComputeBoundsVisitor  cbv;
		terrain->accept(cbv);
		bounding_box = osg::BoundingBoxd(cbv.getBoundingBox()._min, cbv.getBoundingBox()._max);
		if(useBBox)
		{
			bounding_box._min.set(xmin,ymin,bounding_box._min.z());
			bounding_box._max.set(xmax,ymax,bounding_box._max.z());
		}
	}

	std::string config_file;
	if(!arguments.read("--vegetation_config",config_file))
	{
		std::cerr << "No config provided\n";
		return 0;
	}

	std::string env_filename;
	if (!arguments.read("--environment_config", env_filename))
	{
		std::cerr << "No environment config provided\n";
		return 0;
	}

	std::string tq_filename;
	if(!arguments.read("--terrain_query_config",tq_filename))
	{
		std::cerr << "No terrain query config provided\n";
		return 0;
	}

	osgVegetation::Serializer serializer;
	try
	{
		std::vector<osgVegetation::BillboardData> bb_vector = serializer.loadBillboardData(config_file);

		const std::string config_path = osgDB::getFilePath(config_file);
		osgDB::Registry::instance()->getDataFilePathList().push_back(config_path); 

		osg::ref_ptr<osgVegetation::ITerrainQuery> tq = serializer.loadTerrainQuery(terrain, tq_filename);
		osgVegetation::EnvironmentSettings env_settings;
		if(env_filename != "")
			env_settings = serializer.loadEnvironmentSettings(env_filename);
		osgVegetation::BillboardQuadTreeScattering scattering(tq, env_settings);
		std::cout << "Using bounding box:" << bounding_box.xMin() << " " << bounding_box.yMin() << " "<< bounding_box.xMax() << " " << bounding_box.yMax() << "\n";
		std::cout << "Start Scattering...\n";

		srand(seed_value); //reset random numbers, TODO: support layer seed

		osg::Node* bb_node = scattering.generate(bounding_box, bb_vector, out_file, pagedLOD);
		group->addChild(bb_node);
		
		if(save_terrain)
		{
			osg::ProxyNode* pn = dynamic_cast<osg::ProxyNode*>(bb_node);
			if(pn)
			{
				pn->setFileName(pn->getNumFileNames(),terrain_file );
			}
			else
			{
				osg::Group* veg_group = dynamic_cast<osg::Group*>(bb_node);
				if(veg_group)
					veg_group->addChild(terrain);
			}
		}
		osgDB::writeNodeFile(*bb_node, out_file);
		osgDB::writeNodeFile(*bb_node, out_file + ".osg");
	}

	catch(std::exception& e)
	{
		std::cerr << e.what();
		return 0;
	}
	return 0;
}
