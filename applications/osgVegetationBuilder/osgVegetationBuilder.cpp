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

	//osgDB::Registry::instance()->getDataFilePathList().push_back("f:/temp/detail_mapping/Grid0/tiles");
	//osgDB::Registry::instance()->getDataFilePathList().push_back("f:/temp/detail_mapping/Grid0/material_textures");  
	//osgDB::Registry::instance()->getDataFilePathList().push_back("f:/temp/detail_mapping/Grid0/color_textures");  


	/////////////////////////////////////////////////////
	arguments.getApplicationUsage()->addCommandLineOption("--vegetation_config <filename>","Configuration file");
	arguments.getApplicationUsage()->addCommandLineOption("--out","out file");
	arguments.getApplicationUsage()->addCommandLineOption("--terrain","Terrain file");

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

	double xmin=0,xmax=0,ymin=0,ymax=0,zmin=-1,zmax=1;

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

		if(save_terrain)
		{
			group->addChild(terrain);
		}

		osg::ComputeBoundsVisitor  cbv;
		
		terrain->accept(cbv);
		//osg::BoundingBox bb(cbv.getBoundingBox());
		bounding_box = osg::BoundingBoxd(cbv.getBoundingBox());
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

		//if(material_suffix != "")
		//	tq->setMaterialTextureSuffix(material_suffix);
		
		osgVegetation::BillboardQuadTreeScattering scattering(tq);
		std::cout << "Using bounding box:" << bounding_box.xMin() << " " << bounding_box.yMin() << " "<< bounding_box.xMax() << " " << bounding_box.yMax() << "\n";
		std::cout << "Start Scattering...\n";

		srand(0); //reset random numbers, TODO: support layer seed
		for(size_t i=0; i < bb_vector.size();i++)
		{
			std::stringstream ss;
			ss << "bb_" << i << "_";
			osg::Node* bb_node = scattering.generate(bounding_box,bb_vector[i], out_file, pagedLOD, ss.str());
			group->addChild(bb_node);

		
		}
		osgDB::ReaderWriter::Options *options = new osgDB::ReaderWriter::Options();
		options->setOptionString(std::string("OutputTextureFiles OutputShaderFiles"));
		osgDB::writeNodeFile(*group, out_file,options);
	}

	catch(std::exception& e)
	{
		std::cerr << e.what();
		return 0;
	}
	return 0;
}
