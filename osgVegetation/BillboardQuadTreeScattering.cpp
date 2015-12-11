#include "BillboardQuadTreeScattering.h"
#include <osg/MatrixTransform>
#include <osg/StateSet>
#include <osg/ComputeBoundsVisitor>
#include <osg/PagedLOD>
#include <osg/ProxyNode>
#include <osgDB/WriteFile>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <sstream>
#include <stdexcept>
#include "BRTGeometryShader.h"
#include "BRTShaderInstancing.h"
#include "VegetationUtils.h"
#include "ITerrainQuery.h"

namespace osgVegetation
{
	BillboardQuadTreeScattering::BillboardQuadTreeScattering(ITerrainQuery* tq, const EnvironmentSettings &env_settings) :
			m_BRT(NULL),
			m_TerrainQuery(tq),
			m_UsePagedLOD(false),
			m_FilenamePrefix("quadtree_"),
			m_EnvironmentSettings(env_settings)
	{

	}

	void BillboardQuadTreeScattering::_populateVegetationTile(const BillboardLayer& layer,const  osg::BoundingBoxd& bb,BillboardVegetationObjectVector& instances, osg::BoundingBoxd& out_bb) const
	{
		osg::Vec3d origin = bb._min; 
		osg::Vec3d size = bb._max - bb._min; 
		double min_z = FLT_MAX;
		double max_z = -FLT_MAX;
		unsigned int num_objects_to_create = size.x()*size.y()*layer.Density;
		instances.reserve(instances.size()+num_objects_to_create);
		out_bb = bb;
		//std::cout << "pos:" << origin.x() << "size: " << size.x();
		for(unsigned int i=0;i<num_objects_to_create;++i)
		{
			double rand_x = Utils::random(origin.x(), origin.x() + size.x());
			double rand_y = Utils::random(origin.y(), origin.y() + size.y());
			osg::Vec3d pos(rand_x, rand_y,0);
			osg::Vec3d inter;
			osg::Vec4 terrain_color;
			osg::Vec4 coverage_color;
			float rand_int = Utils::random(layer.ColorIntensity.x(),layer.ColorIntensity.y());
			osg::Vec3d offset_pos = pos + m_Offset;
			if(m_InitBB.contains(pos))
			{
				std::string material_name;
				if(m_TerrainQuery->getTerrainData(offset_pos, terrain_color, material_name, coverage_color, inter))
				{
					if(layer.hasCoverage(material_name))
					{
						BillboardObject* veg_obj = new BillboardObject;
						float tree_scale = Utils::random(layer.Scale.x() ,layer.Scale.y());
						veg_obj->Width = Utils::random(layer.Width.x(), layer.Width.y())*tree_scale;
						veg_obj->Height = Utils::random(layer.Height.x(), layer.Height.y())*tree_scale;
						veg_obj->TextureIndex = layer._TextureIndex;
						veg_obj->Position = inter - m_Offset;
						if(layer.UseTerrainIntensity)
						{
							float terrain_intensity = (terrain_color.r() + terrain_color.g() + terrain_color.b())/3.0;
							terrain_color.set(terrain_intensity,terrain_intensity,terrain_intensity,terrain_color.a());
						}
						//generate static color data
						veg_obj->Color = terrain_color*(layer.TerrainColorRatio*rand_int);
						veg_obj->Color += osg::Vec4(1,1,1,1)*(rand_int * (1.0 - layer.TerrainColorRatio));
						veg_obj->Color.set(veg_obj->Color.r(), veg_obj->Color.g(), veg_obj->Color.b(), 1.0);
						instances.push_back(veg_obj);

						if (veg_obj->Position.z() > max_z)
							max_z = veg_obj->Position.z();
						if (veg_obj->Position.z() < min_z)
							min_z = veg_obj->Position.z();
					}
				}
			}
		}
		
		if (instances.size() > 0)
		{
			if(max_z > out_bb._max.z())
				out_bb._max.z() = max_z;
			if (min_z > out_bb._min.z())
				out_bb._min.z() = min_z;
		}
	}

	std::string BillboardQuadTreeScattering::_createFileName( unsigned int lv,	unsigned int x, unsigned int y ) const
	{
		std::stringstream sstream;
		sstream << m_FilenamePrefix << lv << "_X" << x << "_Y" << y << "." << m_SaveExt;
		return sstream.str();
	}

	osg::Node* BillboardQuadTreeScattering::_createLODRec(int ld, BillboardData &data, BillboardVegetationObjectVector instances, const osg::BoundingBoxd &bb,int x, int y)
	{
		if(ld < 6) //only show progress above lod 6, we don't want to spam the log
			std::cout << "Progress:" << (int)(100.0f*((float) m_CurrentTile/(float) m_NumberOfTiles)) <<  "% Tile:" << m_CurrentTile << " of:" << m_NumberOfTiles << std::endl;
		m_CurrentTile++;

		
		osg::ref_ptr<osg::Group> children_group = new osg::Group;

		//mesh_group is returned as raw pointer
		osg::Group* mesh_group = new osg::Group;


		BillboardVegetationObjectVector tile_instances;
		//double max_tile_size = 0;
		osg::BoundingBoxd tile_bb = bb;
		tile_bb._min.z() = FLT_MAX;
		tile_bb._max.z() = -FLT_MAX;

		for(size_t i = 0; i < data.Layers.size(); i++)
		{
			if(ld == data.Layers[i]._QTLevel)
			{
				 _populateVegetationTile(data.Layers[i], bb, tile_instances, tile_bb);
				 //save view max view distance for this tile level
				 //if(data.Layers[i].MinTileSize > max_tile_size)
				//	 max_tile_size = data.Layers[i].MinTileSize;
			}
		}
	
		const double bb_size = (bb._max.x() - bb._min.x());
		double tile_radius = bb.radius();
		double tile_cutoff = tile_radius*2.0f;
		osg::Vec3d tile_center = bb.center();
		double tile_min_z = bb._min.z();
		double tile_max_z = bb._max.z();

		if(tile_instances.size() > 0)
		{
			//we have geometry in this tile, update radius etc.
			tile_radius = tile_bb.radius();
			tile_cutoff = tile_radius*2.0f;
			tile_center = tile_bb.center();
			tile_min_z = tile_bb._min.z();
			tile_max_z = tile_bb._max.z();
			//expand view distance to cutoff?
			//max_tile_size = std::max(max_tile_size, tile_cutoff);
			osg::Node* tile_geometry = m_BRT->create(tile_instances, tile_bb);

			mesh_group->addChild(tile_geometry);
		}

		//split bounding box into four new children
		bool final_lod = (ld == m_FinalLOD);
		if(!final_lod)
		{
			double sx = (bb._max.x() - bb._min.x())*0.5;
			double sy = (bb._max.x() - bb._min.x())*0.5;

			osg::BoundingBoxd b1(bb._min, osg::Vec3(bb._min.x() + sx,  bb._min.y() + sy  ,tile_max_z));
			osg::BoundingBoxd b2(osg::Vec3(bb._min.x() + sx , bb._min.y()       , tile_min_z),
								 osg::Vec3(bb._max.x(),       bb._min.y() + sy  , tile_max_z));

			osg::BoundingBoxd b3(osg::Vec3(bb._min.x() + sx,  bb._min.y() + sy   , tile_min_z),
				osg::Vec3(bb._max.x(),       bb._max.y()		, tile_max_z));

			osg::BoundingBoxd b4(osg::Vec3(bb._min.x(),		 bb._min.y() + sy  , tile_min_z),
				osg::Vec3(bb._min.x() + sx,  bb._max.y()		, tile_max_z));

			//first check that we are inside initial bounding box
			if(b1.intersects(m_InitBB))	children_group->addChild( _createLODRec(ld+1,data,instances,b1, x*2,   y*2));
			if(b2.intersects(m_InitBB))	children_group->addChild( _createLODRec(ld+1,data,instances,b2, x*2,   y*2+1));
			if(b3.intersects(m_InitBB)) children_group->addChild( _createLODRec(ld+1,data,instances,b3, x*2+1, y*2+1));
			if(b4.intersects(m_InitBB)) children_group->addChild( _createLODRec(ld+1,data,instances,b4, x*2+1, y*2));

			if(m_UsePagedLOD)
			{
				osg::PagedLOD* plod = new osg::PagedLOD;
				plod->setCenterMode( osg::PagedLOD::USER_DEFINED_CENTER );
				
				plod->setCenter(tile_center);
				plod->setRadius(tile_radius);
				
				int c_index = 0;
				if(mesh_group->getNumChildren() > 0)
				{
					plod->addChild(mesh_group);// , 0, FLT_MAX );
					c_index++;
				}
				const std::string filename = _createFileName(ld, x,y);
				plod->setFileName( c_index, filename );
				
				if(data.TilePixelSize > 0)
				{
					plod->setRangeMode(osg::LOD::PIXEL_SIZE_ON_SCREEN);
					plod->setRange( 0, data.TilePixelSize, FLT_MAX);
					if(c_index > 0)
						plod->setRange( 1, data.TilePixelSize, FLT_MAX );
				}
				else
				{
					plod->setRange(0, data.TilePixelSize, FLT_MAX);
					if (c_index > 0)
					{
						plod->setRange(0, 0, FLT_MAX);
						plod->setRange(1, 0, tile_cutoff);
					}
					else
						plod->setRange(0, 0, tile_cutoff);
				}

				osgDB::writeNodeFile( *children_group, m_SavePath + filename );

				
				return plod;
			}
			else
			{
				osg::LOD* plod = new osg::LOD;
				plod->setCenterMode(osg::PagedLOD::USER_DEFINED_CENTER);
				plod->setCenter( tile_center);
				plod->setRadius(tile_radius);
				plod->addChild(mesh_group, 0, FLT_MAX );
				plod->addChild(children_group, 0.0f, tile_cutoff );

				if(data.TilePixelSize > 0) //override
				{
					plod->setRangeMode(osg::LOD::PIXEL_SIZE_ON_SCREEN);
					plod->setRange( 0, data.TilePixelSize, FLT_MAX);
					plod->setRange( 1, data.TilePixelSize, FLT_MAX );
				}
				return plod;
			}
		}
		else
			return mesh_group;
	}

	bool BillboardSortPredicate(const BillboardLayer &lhs, const BillboardLayer &rhs)
	{
		return lhs.MinTileSize > rhs.MinTileSize;
	}

	osg::Node* BillboardQuadTreeScattering::generate(const osg::BoundingBoxd &bounding_box,std::vector<osgVegetation::BillboardData> &data, const std::string &output_file, bool use_paged_lod)
	{
		if(output_file != "")
		{
			m_UsePagedLOD = use_paged_lod;
			m_SavePath = osgDB::getFilePath(output_file);
			m_SavePath += "/";
			m_SaveExt = osgDB::getFileExtension(output_file);
		}
		else if(m_UsePagedLOD)
		{
			OSGV_EXCEPT(std::string("BillboardQuadTreeScattering::generate - paged lod requested but no output file supplied").c_str());
		}

		osg::Node *node = NULL;

		//use proxy file for top node
		if(m_UsePagedLOD)
		{
			osg::ProxyNode* pn = new osg::ProxyNode();
			node  = pn;
			for(size_t i=0; i < data.size();i++)
			{
				std::stringstream ss;
				ss << "billboard_layer" << i;
				osg::Node* bb_node = generate(bounding_box, data[i], output_file, use_paged_lod, ss.str());
				if(bb_node)
				{
					//save osg files that can be used for editing
					const std::string file_name = ss.str() + ".osg";
					osgDB::ReaderWriter::Options *options = new osgDB::ReaderWriter::Options();
					options->setOptionString(std::string("OutputShaderFiles"));
					osgDB::writeNodeFile(*bb_node, m_SavePath + file_name,options);
					pn->setFileName(i, file_name);
					
					/*const std::string file_name = ss.str() + ".ive";
					osgDB::ReaderWriter::Options *options = new osgDB::ReaderWriter::Options();
					options->setOptionString(std::string("OutputShaderFiles"));
					osgDB::writeNodeFile(*bb_node, m_SavePath + file_name, options);
					pn->setFileName(i, file_name);*/
				}
			}

			if(output_file != "") //save proxy node
			{
				osgDB::writeNodeFile(*pn, output_file + ".osg");
			}
		}
		else
		{
			osg::Group* group = new osg::Group();
			node = group;
			for(size_t i=0; i < data.size();i++)
			{
				std::stringstream ss;
				ss << "billboard_layer" << i;
				osg::Node* bb_node = generate(bounding_box, data[i], output_file, use_paged_lod, ss.str());
				if(bb_node)
				{
					group->addChild(bb_node);
				}
			}
		}
		return node;
	}

	osg::Node* BillboardQuadTreeScattering::generate(const osg::BoundingBoxd &boudning_box, BillboardData &data, const std::string &output_file, bool use_paged_lod, const std::string &filename_prefix)
	{
		m_FilenamePrefix = filename_prefix;
		//remove any previous render technique
		delete m_BRT;

		if(data.Technique == BRT_SHADER_INSTANCING)
			m_BRT = new BRTShaderInstancing(data, m_EnvironmentSettings);
		else if (data.Technique == BRT_GEOMETRY_SHADER)
			m_BRT = new BRTGeometryShader(data, m_EnvironmentSettings);
		else
			OSGV_EXCEPT(std::string("BillboardQuadTreeScattering::generate - unkown rendering tech").c_str());

		//get max bb side, we want square area for to begin quad tree splitting
		double max_bb_size = std::max(boudning_box._max.x() - boudning_box._min.x(),
			boudning_box._max.y() - boudning_box._min.y());

		//Offset vegetation by using new origin at boudning_box._min
		m_Offset = boudning_box._min;

		//Create initial bounding box at new origin
		m_InitBB._min.set(0,0,0);
		m_InitBB._max = boudning_box._max - boudning_box._min;

		//add offset matrix
		osg::MatrixTransform* transform = new osg::MatrixTransform;
		transform->setMatrix(osg::Matrix::translate(m_Offset));

		//reset
		m_FinalLOD =0;
		m_NumberOfTiles = 1; //at least one LOD tile
		m_CurrentTile = 0;

		//sort by tile size
		std::sort(data.Layers.begin(), data.Layers.end(), BillboardSortPredicate);

		//Get max tile size
		for(size_t i = 0; i < data.Layers.size(); i++)
		{
			max_bb_size = std::max(max_bb_size, data.Layers[i].MinTileSize);
		}

		//set quad tree LOD level for each billboard layer
		for(size_t i = 0; i < data.Layers.size(); i++)
		{
			double temp_size  = max_bb_size;
			int ld = 0;
			while(temp_size > data.Layers[i].MinTileSize)
			{
				ld++;
				temp_size *= 0.5;
			}
			data.Layers[i]._QTLevel = ld;
			if(m_FinalLOD < ld)
				m_FinalLOD = ld;
		}

		//Create squared bounding box for top level quad tree tile
		osg::BoundingBoxd qt_bb;
		qt_bb._max.set(max_bb_size, max_bb_size, boudning_box._max.z() - boudning_box._min.z());
		qt_bb._min.set(0,0,0);

		//get total number of tiles to process, used for progress report
		int ld = 0;
		while(ld < m_FinalLOD)
		{
			int side_tile_count = 2 << ld;
			m_NumberOfTiles += side_tile_count*side_tile_count;
			ld++;
		}

		//Start recursive scattering process
		BillboardVegetationObjectVector instances;
		osg::Node* outnode = _createLODRec(0, data, instances, qt_bb,0,0);

		//Add state set to top node
		outnode->setStateSet((osg::StateSet*) m_BRT->getStateSet()->clone(osg::CopyOp::DEEP_COPY_STATESETS));
		transform->addChild(outnode);
		return transform;
	}
}
