#include "MeshQuadTreeScattering.h"
#include <osg/AlphaFunc>
#include <osg/Billboard>
#include <osg/BlendFunc>
#include <osg/Depth>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Material>
#include <osg/Math>
#include <osg/MatrixTransform>
#include <osg/ShapeDrawable>
#include <osg/StateSet>
#include <osg/Texture2D>
#include <osg/TextureBuffer>
#include <osg/Image>
#include <osg/TexEnv>
#include <osg/ComputeBoundsVisitor>
#include <osg/PagedLOD>
#include <osg/ProxyNode>


#include <osgDB/WriteFile>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <osg/Texture2DArray>
//#include <iostream>
#include <sstream>

#include "MRTShaderInstancing.h"
#include "VegetationUtils.h"
#include "ITerrainQuery.h"

namespace osgVegetation
{
	MeshQuadTreeScattering::MeshQuadTreeScattering(ITerrainQuery* tq) : m_VRT(NULL),
		m_TerrainQuery(tq),
		m_UsePagedLOD(false),
		m_FilenamePrefix("quadtree_")
	{

	}

	void MeshQuadTreeScattering::_populateVegetationLayer(const MeshLayer& layer,const  osg::BoundingBox& bb,MeshVegetationObjectVector& instances)
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
			osg::Vec4 mat_color;
			float rand_int = Utils::random(layer.ColorIntensity.x(),layer.ColorIntensity.y());
			osg::Vec3 offset_pos = pos + m_Offset;
			if(m_InitBB.contains(pos))
			{
				if(m_TerrainQuery->getTerrainData(offset_pos,color,mat_color,inter))
				{
					if(layer.hasMaterial(mat_color))
					{
						MeshObject* veg_obj = new MeshObject;
						//TODO add color to layer
						float tree_scale = Utils::random(layer.Scale.x() ,layer.Scale.y());
						veg_obj->Width = Utils::random(layer.Width.x(),layer.Width.y())*tree_scale;
						veg_obj->Height = Utils::random(layer.Height.x(),layer.Height.y())*tree_scale;
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

	std::string MeshQuadTreeScattering::_createFileName(unsigned int lv,	unsigned int x, unsigned int y )
	{
		std::stringstream sstream;
		sstream << m_FilenamePrefix << lv << "_X" << x << "_Y" << y << ".ive";
		return sstream.str();
	}

	osg::Node* MeshQuadTreeScattering::_createLODRec(int ld, MeshData &data, MeshVegetationObjectVector instances, const osg::BoundingBox &bb,int x, int y)
	{
		if(ld < 6) //only show progress above lod 6, we don't want to spam the log
			std::cout << "Progress:" << (int)(100.0f*((float) m_CurrentTile/(float) m_NumberOfTiles)) <<  "% Create Tile:" << m_CurrentTile << " of:" << m_NumberOfTiles << std::endl;
		m_CurrentTile++;

		osg::ref_ptr<osg::Group> children_group = new osg::Group;

		//mesh_group is returned as raw pointer so we don't use smart pointer
		osg::Group* mesh_group = new osg::Group;
		double bb_size = (bb._max.x() - bb._min.x());

		bool final_lod = (ld == m_FinalLOD);

		for(size_t i = 0; i < data.Layers.size(); i++)
		{
			int mesh_lod = -1;
			int max_lod = 0;
			for(size_t j = 0; j < data.Layers[i].MeshLODs.size(); j++)
			{
				if(ld >= data.Layers[i].MeshLODs[j]._LODLevel && data.Layers[i].MeshLODs[j]._LODLevel > max_lod)
				{
					mesh_lod = j;
					max_lod = data.Layers[i].MeshLODs[j]._LODLevel;
				}

				if(j == 0 && ld == data.Layers[i].MeshLODs[j]._LODLevel)
				{
					//create data
					data.Layers[i]._Instances.clear();
					_populateVegetationLayer(data.Layers[i], bb, data.Layers[i]._Instances);
				}
			}
			
			
			

			if(mesh_lod >= 0)
			{
				//filter trees inside box
				MeshVegetationObjectVector patch_instances;
				for(size_t j = 0; j < data.Layers[i]._Instances.size(); j++)
				{
					if(bb.contains(data.Layers[i]._Instances[j]->Position))
						patch_instances.push_back(data.Layers[i]._Instances[j]);
					//osg::Vec3 p = data.Layers[i]._Instances[j]->Position;
					//p.set(p.x(),p.y(),p.z() + 1.0);
					//data.Layers[i]._Instances[j]->Position = p;
				}
				osg::Node* node = m_VRT->create(patch_instances, data.Layers[i].MeshLODs[mesh_lod].MeshName, bb);
				mesh_group->addChild(node);
			}
		}

		//split bounding box into four new children
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
				//regular terrain LOD setup
				
				int c_index = 0;
				if(mesh_group->getNumChildren() > 0)
				{
					//plod->addChild(mesh_group, 0, FLT_MAX );
					plod->addChild(mesh_group, cutoff, FLT_MAX );
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
				//regular terrain LOD setup
				plod->addChild(mesh_group, cutoff, FLT_MAX );
				//plod->addChild(mesh_group, 0, FLT_MAX );
				plod->addChild(children_group, 0.0f, cutoff );
				return plod;
			}
		}
		else
			return mesh_group;

	}
	osg::Node* MeshQuadTreeScattering::generate(const osg::BoundingBox &boudning_box,MeshData &data, const std::string &page_lod_path, const std::string &filename_prefix)
	{
		m_FilenamePrefix = filename_prefix;
		if(page_lod_path != "")
		{
			m_UsePagedLOD = true;
			m_SavePath = page_lod_path;
		}

		//remove  previous render tech
		delete m_VRT;

		//m_VRT = new BRTShaderInstancing(data);
		m_VRT = new MRTShaderInstancing(data);
		

		//get max bb side, we want square area for to begin quad tree splitting
		double max_bb_size = std::max(boudning_box._max.x() - boudning_box._min.x(), 
			boudning_box._max.y() - boudning_box._min.y());

		max_bb_size = std::max(max_bb_size,data.ViewDistance);
	
		//Offset vegetation by using new origin at boudning_box._min
		m_Offset = boudning_box._min;

		//Create initial bounding box at new origin
		m_InitBB._min.set(0,0,0);
		m_InitBB._max = boudning_box._max - boudning_box._min;

		//Create squared bounding box as starting point for the quadtree process
		osg::BoundingBox qt_bb;
		qt_bb._max.set(max_bb_size, max_bb_size, boudning_box._max.z() - boudning_box._min.z());
		qt_bb._min.set(0,0,0);

		//add offset matrix
		osg::MatrixTransform* transform = new osg::MatrixTransform;
		transform->setMatrix(osg::Matrix::translate(m_Offset));

		//save for later...
		

		//reset
		m_FinalLOD =0;
		m_StartLOD =0;
		m_NumberOfTiles = 0;
		m_CurrentTile = 0;

		//Get LOD level to begin scattering at
		/*while(temp_size > data.ViewDistance)
		{
			m_StartLOD++;
			temp_size *= 0.5;
		}*/

		for(size_t i = 0; i < data.Layers.size(); i++)
		{
			for(size_t j = 0; j < data.Layers[i].MeshLODs.size(); j++)
			{
				double temp_size  = max_bb_size;
				int ld = 0;
				while(temp_size > data.Layers[i].MeshLODs[j].MaxDistance)
				{
					ld++;
					temp_size *= 0.5;
				}
				data.Layers[i].MeshLODs[j]._LODLevel = ld;
				if(m_FinalLOD < ld)
					m_FinalLOD = ld;
			}
		}
		
		//get total number of tiles to process, used for progress report
		int ld = 0;
		while(ld < m_FinalLOD)
		{
			int side_tile_count = 2 << ld;
			m_NumberOfTiles += side_tile_count*side_tile_count;
			ld++;
		}

		//Start recursive scattering process
		MeshVegetationObjectVector instances;
		osg::Node* outnode = _createLODRec(0, data, instances, qt_bb,0,0);

		//Add state set to top node
		outnode->setStateSet((osg::StateSet*) m_VRT->getStateSet()->clone(osg::CopyOp::DEEP_COPY_STATESETS));

		if(m_UsePagedLOD)
		{
			transform->addChild(outnode);
			osgDB::writeNodeFile(*transform, m_SavePath + m_FilenamePrefix + "master.ive");
			//osg::ProxyNode* pn = new osg::ProxyNode();
			//pn->setFileName(0,"master.ive");
			//pn->setDatabasePath("C:/temp/paged");
			//transform->addChild(pn);
			//osgDB::writeNodeFile( *transform, m_SavePath + "/transformation.osg" );
		}
		else
		{
			transform->addChild(outnode);
		}

		return transform;
	}
}
