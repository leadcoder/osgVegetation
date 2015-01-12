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
#include "BRTGeometryShader.h"
#include "BRTShaderInstancing.h"
#include "VegetationUtils.h"
#include "ITerrainQuery.h"

namespace osgVegetation
{
	BillboardQuadTreeScattering::BillboardQuadTreeScattering(ITerrainQuery* tq) : 
			m_BRT(NULL),
			m_TerrainQuery(tq),
			m_UsePagedLOD(false),
			m_FilenamePrefix("quadtree_")
	{

	}

	void BillboardQuadTreeScattering::_populateVegetationLayer(const BillboardLayer& layer,const  osg::BoundingBox& bb,BillboardVegetationObjectVector& instances) const
	{
		osg::Vec3 origin = bb._min; 
		osg::Vec3 size = bb._max - bb._min; 

		unsigned int num_objects_to_create = size.x()*size.y()*layer.Density;
		instances.reserve(instances.size()+num_objects_to_create);

		for(unsigned int i=0;i<num_objects_to_create;++i)
		{
			osg::Vec3 pos(Utils::random(origin.x(),origin.x()+size.x()),Utils::random(origin.y(),origin.y()+size.y()),0);
			osg::Vec3 inter;
			osg::Vec4 color;
			osg::Vec4 coverage_color;
			float rand_int = Utils::random(layer.ColorIntensity.x(),layer.ColorIntensity.y());
			osg::Vec3 offset_pos = pos + m_Offset;
			if(m_InitBB.contains(pos))
			{
				std::string material_name;
				if(m_TerrainQuery->getTerrainData(offset_pos, color, material_name, coverage_color, inter))
				{
					if(layer.hasCoverage(material_name))
					{
						BillboardObject* veg_obj = new BillboardObject;
						float tree_scale = Utils::random(layer.Scale.x() ,layer.Scale.y());
						veg_obj->Width = Utils::random(layer.Width.x(), layer.Width.y())*tree_scale;
						veg_obj->Height = Utils::random(layer.Height.x(), layer.Height.y())*tree_scale;
						veg_obj->TextureIndex = layer._TextureIndex;
						veg_obj->Position = inter - m_Offset;
						if(layer.MixInIntensity)
						{
							float intensity = (color.r() + color.g() + color.b())/3.0;
							color.set(intensity,intensity,intensity,color.a());
						}
						veg_obj->Color = color*layer.MixInColorRatio;
						veg_obj->Color += osg::Vec4(1,1,1,1)*rand_int;
						veg_obj->Color.set(veg_obj->Color.r(), veg_obj->Color.g(), veg_obj->Color.b(), 1.0);
						instances.push_back(veg_obj);
					}
				}
			}
		}
	}
	
	std::string BillboardQuadTreeScattering::_createFileName( unsigned int lv,	unsigned int x, unsigned int y ) const
	{
		std::stringstream sstream;
		sstream << m_FilenamePrefix << lv << "_X" << x << "_Y" << y << "." << m_SaveExt;
		return sstream.str();
	}

	osg::Node* BillboardQuadTreeScattering::_createLODRec(int ld, BillboardData &data, BillboardVegetationObjectVector instances, const osg::BoundingBox &bb,int x, int y)
	{
		if(ld < 6) //only show progress above lod 6, we don't want to spam the log
			std::cout << "Progress:" << (int)(100.0f*((float) m_CurrentTile/(float) m_NumberOfTiles)) <<  "% Tile:" << m_CurrentTile << " of:" << m_NumberOfTiles << std::endl;
		m_CurrentTile++;

		osg::ref_ptr<osg::Group> children_group = new osg::Group;

		//mesh_group is returned as raw pointer
		osg::Group* mesh_group = new osg::Group;
		double bb_size = (bb._max.x() - bb._min.x());
	
		BillboardVegetationObjectVector patch_instances;
		for(size_t i = 0; i < data.Layers.size(); i++)
		{
			if(ld == data.Layers[i]._QTLevel)
			{
				 _populateVegetationLayer(data.Layers[i], bb,patch_instances);
			}
		}

		if(patch_instances.size() > 0)
		{
			osg::Node* node = m_BRT->create(patch_instances, bb);
			mesh_group->addChild(node);
		}

		//split bounding box into four new children
		bool final_lod = (ld == m_FinalLOD);
		if(!final_lod)
		{
			double sx = (bb._max.x() - bb._min.x())*0.5;
			double sy = (bb._max.x() - bb._min.x())*0.5;
			osg::BoundingBox b1(bb._min, 
				osg::Vec3(bb._min.x() + sx,  bb._min.y() + sy  ,bb._max.z()));
			osg::BoundingBox b2(osg::Vec3(bb._min.x() + sx , bb._min.y()       ,bb._min.z()),
				osg::Vec3(bb._max.x(),       bb._min.y() + sy  ,bb._max.z()));

			osg::BoundingBox b3(osg::Vec3(bb._min.x() + sx,  bb._min.y() + sy   ,bb._min.z()),
				osg::Vec3(bb._max.x(),       bb._max.y()		,bb._max.z()));

			osg::BoundingBox b4(osg::Vec3(bb._min.x(),		 bb._min.y() + sy  ,bb._min.z()),
				osg::Vec3(bb._min.x() + sx,  bb._max.y()		,bb._max.z()));

			//first check that we are inside initial bounding box
			if(b1.intersects(m_InitBB))	children_group->addChild( _createLODRec(ld+1,data,instances,b1, x*2,   y*2));
			if(b2.intersects(m_InitBB))	children_group->addChild( _createLODRec(ld+1,data,instances,b2, x*2,   y*2+1));
			if(b3.intersects(m_InitBB)) children_group->addChild( _createLODRec(ld+1,data,instances,b3, x*2+1, y*2+1));
			if(b4.intersects(m_InitBB)) children_group->addChild( _createLODRec(ld+1,data,instances,b4, x*2+1, y*2)); 

			if(m_UsePagedLOD)
			{
				osg::PagedLOD* plod = new osg::PagedLOD;
				plod->setCenterMode( osg::PagedLOD::USER_DEFINED_CENTER );
				plod->setCenter(bb.center());
				double radius = sqrt(bb_size*bb_size);
				plod->setRadius(radius);
				float cutoff = radius*2;
				int c_index = 0;
				if(mesh_group->getNumChildren() > 0)
				{
					plod->addChild(mesh_group, 0, FLT_MAX );	
					c_index++;
				}
				const std::string filename = _createFileName(ld, x,y);
				plod->setFileName( c_index, filename );
				plod->setRange(c_index,0,cutoff);
				osgDB::writeNodeFile( *children_group, m_SavePath + filename );
				return plod;
			}
			else
			{
				osg::LOD* plod = new osg::LOD;
				plod->setCenterMode(osg::PagedLOD::USER_DEFINED_CENTER);
				plod->setCenter( bb.center());

				double radius = sqrt(bb_size*bb_size);
				plod->setRadius(radius);

				float cutoff = radius*2;
				plod->addChild(mesh_group, 0, FLT_MAX );
				plod->addChild(children_group, 0.0f, cutoff );
				return plod;
			}
		}
		else
			return mesh_group;
	}

	bool BillboardSortPredicate(const BillboardLayer &lhs, const BillboardLayer &rhs)
	{
		return lhs.ViewDistance > rhs.ViewDistance;
	}

	osg::Node* BillboardQuadTreeScattering::generate(const osg::BoundingBox &boudning_box,BillboardData &data, const std::string &output_file, bool use_paged_lod, const std::string &filename_prefix)
	{
		if(output_file != "")
		{
			m_UsePagedLOD = use_paged_lod;
			m_FilenamePrefix = filename_prefix;
			m_SavePath = osgDB::getFilePath(output_file);
			m_SavePath += "/";
			m_SaveExt = osgDB::getFileExtension(output_file);
		}
		else if(m_UsePagedLOD)
		{
			throw std::exception(std::string("BillboardQuadTreeScattering::generate - paged lod requested but no output file supplied").c_str());
		}
		

		//remove any previous render technique
		delete m_BRT;

		//m_BRT = new BRTShaderInstancing(data);
		m_BRT = new BRTGeometryShader(data);
	
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

		//distance sort mesh LODs
		std::sort(data.Layers.begin(), data.Layers.end(), BillboardSortPredicate);
		

		//Get max view dist
		for(size_t i = 0; i < data.Layers.size(); i++)
		{
			max_bb_size = std::max(max_bb_size,data.Layers[i].ViewDistance);
		}

		//set quad tree LOD level for each billboard layer
		for(size_t i = 0; i < data.Layers.size(); i++)
		{
			double temp_size  = max_bb_size;
			int ld = 0;
			while(temp_size > data.Layers[i].ViewDistance)
			{
				ld++;
				temp_size *= 0.5;
			}
			data.Layers[i]._QTLevel = ld;
			if(m_FinalLOD < ld)
				m_FinalLOD = ld;
		}

		//Create squared bounding box for top level quad tree tile
		osg::BoundingBox qt_bb;
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

		if(output_file != "")
		{
			osgDB::writeNodeFile(*transform, output_file);
			//osgDB::writeNodeFile(*transform, m_SavePath + m_FilenamePrefix + "master.ive");
			//osg::ProxyNode* pn = new osg::ProxyNode();
			//pn->setFileName(0,"master.ive");
			//pn->setDatabasePath("C:/temp/paged");
			//transform->addChild(pn);
			//osgDB::writeNodeFile( *transform, m_SavePath + "/transformation.osg" );
		}
		return transform;
	}
}
