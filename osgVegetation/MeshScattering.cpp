#include "MeshScattering.h"
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
#include <osg/TexMat>
#include <osgDB/WriteFile>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <osg/Texture2DArray>

#include <osgUtil/LineSegmentIntersector>
#include <osgUtil/IntersectionVisitor>
#include <osgUtil/SmoothingVisitor>
#include <osgSim/LineOfSight>

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
#include "terrain_coords.h"
#include "BRTGeometryShader.h"
#include "BRTShaderInstancing.h"
#include "MRTShaderInstancing.h"
#include "VegetationUtils.h"
#include "TerrainQuery.h"

namespace osgVegetation
{
	MeshScattering::MeshScattering(osg::Node* terrain, double view_dist) : m_ViewDistance(view_dist),
		m_Terrain(terrain)
	{
		//m_Cache = new osgSim::DatabaseCacheReadCallback;
		//m_IntersectionVisitor.setReadCallback(m_Cache);
		m_TerrainQuery = new TerrainQuery(terrain);
		m_MRT = new MRTShaderInstancing();
	}

	void MeshScattering::populateVegetationLayer(const MeshLayer& layer, osg::BoundingBox &box ,MeshVegetationObjectVector& object_list)
	{
		osg::Vec3 origin =  box._min;
		const osg::Vec3 size = box._max - box._min;

		unsigned int num_objects_to_create = size.x()*size.y()*layer.Density;
		object_list.reserve(object_list.size()+num_objects_to_create);
		//m_IntersectionVisitor.reset();
		//m_IntersectionVisitor.setLODSelectionMode(osgUtil::IntersectionVisitor::USE_HIGHEST_LEVEL_OF_DETAIL);

		float min_TreeHeight = layer.Height.x();
		float min_TreeWidth = layer.Width.x();

		//float min_scale = layer.Scale.x();
		//float max_scale = layer.Scale.y();

		float max_TreeHeight = layer.Height.y();
		float max_TreeWidth = layer.Width.y();

		for(unsigned int i=0;i<num_objects_to_create;++i)
		{
			if (m_Terrain)
			{
				osg::Vec3 pos(random(origin.x(),origin.x()+size.x()),random(origin.y(),origin.y()+size.y()),0);
				osg::Vec4 color;
				osg::Vec4 mat_color;
				osg::Vec3 inter;
				if(m_TerrainQuery->getTerrainData(pos,color,mat_color,inter))
				{
					if(layer.hasMaterial(mat_color))
					{
						MeshObject* veg_obj = new MeshObject;
						//TODO add color to layer
						float intesity = random(layer.IntensitySpan.x(), layer.IntensitySpan.y());
						veg_obj->Color.set(intesity,intesity,intesity,1.0);
						
						float tree_scale = 1.0;//random(min_scale ,max_scale);
						veg_obj->Width = random(min_TreeWidth,max_TreeWidth)*tree_scale;
						veg_obj->Height = random(min_TreeHeight,max_TreeHeight)*tree_scale;
						veg_obj->Position = inter;
						veg_obj->Rotation = osg::Quat(random(0,osg::PI),osg::Vec3(0,0,1));
						object_list.push_back(veg_obj);
					}
				}
			
			}
		}
	}

	MeshVegetationMap MeshScattering::generateVegetation(MeshLayerVector &layers, osg::BoundingBox &box)
	{
		MeshVegetationMap objects;
		for(size_t i = 0 ; i < layers.size();i++)
		{
			MeshVegetationObjectVector trees;
			populateVegetationLayer(layers[i],box,trees);
			objects[i] = trees;
		}
		return objects;
	}


	std::string MeshScattering::createFileName( unsigned int lv,	unsigned int x, unsigned int y )
	{
		std::stringstream sstream;
		sstream << "quadtree_L" << lv << "_X" << x << "_Y" << y << ".ive";
		return sstream.str();
	}

	/*osg::Node* MeshVegetationScattering::createPagedLODRec(int ld, osg::Node* terrain, VegetationLayerVector &layers, VegetationObjectVector &trees,float current_size, float final_patch_size, float target_patch_size, osg::Vec3 center,int x, int y)
	{
	if(current_size < target_patch_size)
	{
	osg::Vec3 p_origin(center.x() - current_size*0.5, center.y() - current_size*0.5,center.z());
	osg::Vec3 p_size(current_size,current_size,1);
	VegetationObjectVector trees = generateVegetation(terrain,layers,p_origin,p_size);
	//osg::Node* f_node = createPatch(terrain,layers,p_origin,p_size);
	//return f_node;
	}

	osg::Group *group = new osg::Group;
	osg::Vec3 c1_center(center.x() - current_size*0.25,center.y() - current_size*0.25,center.z());
	osg::Vec3 c2_center(center.x() - current_size*0.25,center.y() + current_size*0.25,center.z());
	osg::Vec3 c3_center(center.x() + current_size*0.25,center.y() + current_size*0.25,center.z());
	osg::Vec3 c4_center(center.x() + current_size*0.25,center.y() - current_size*0.25,center.z());

	group->addChild( createPagedLODRec(ld+1,terrain,layers, trees, current_size*0.5, target_patch_size, final_patch_size,c1_center, x*2,   y*2));
	group->addChild( createPagedLODRec(ld+1,terrain,layers, trees, current_size*0.5, target_patch_size, final_patch_size,c2_center, x*2,   y*2+1));
	group->addChild( createPagedLODRec(ld+1,terrain,layers, trees, current_size*0.5, target_patch_size, final_patch_size,c3_center, x*2+1, y*2+1));
	group->addChild( createPagedLODRec(ld+1,terrain,layers, trees, current_size*0.5, target_patch_size, final_patch_size,c4_center, x*2+1, y*2)); 

	osg::PagedLOD* plod = new osg::PagedLOD;
	std::string filename = createFileName(ld, x,y);
	//plod->insertChild( 0, geode.get() );
	plod->setFileName( 0, filename );
	osgDB::writeNodeFile( *group, "C:/temp/paged/" + filename );
	plod->setCenterMode( osg::PagedLOD::USER_DEFINED_CENTER );
	plod->setCenter( center );
	float radius = sqrt(current_size*current_size);
	plod->setRadius( radius);
	float cutoff = radius*2;
	//plod->setRange( 0, cutoff, FLT_MAX );
	plod->setRange( 0, 0.0f, cutoff );
	return plod;
	}*/

	osg::Node* MeshScattering::createLODRec(int ld, MeshLayerVector &layers, MeshVegetationMap trees, osg::BoundingBox &boundingBox,int x, int y)
	{
		double radius = boundingBox.radius();
		double cutoff = radius*2.0;
		//m_Cache->clearDatabaseCache();
		std::cout << "Current LOD:" << ld << " Tile:" << x << " y:" << y << "\n";

		osg::ref_ptr<osg::Group> group = new osg::Group;

		if(ld == m_StartLOD)
		{
			trees = generateVegetation(layers,boundingBox);
		}
		osg::Group* mesh_group= new osg::Group();
		if(ld >= m_StartLOD)
		{
			bool final_lod = true;
			MeshVegetationMap::iterator iter = trees.begin();
			while(iter != trees.end())
			{
				MeshVegetationObjectVector patch_trees;

				int layer = iter->first;
				for(size_t i = 0; i < iter->second.size(); i++)
				{
					MeshObject* tree = iter->second[i];
					//if(fabs(tree->Position.x() - center.x()) < current_size*0.5 && 
					//	fabs(tree->Position.y() - center.y()) < current_size*0.5)
					if(boundingBox.contains(tree->Position))
					{
						patch_trees.push_back(tree);
					}
				}

				//Pick mesh LOD
				int mesh_lod = ld - m_StartLOD;

				for(size_t i =0 ; i < layers[layer].MeshLODs.size();i++)
				{
					if(layers[layer].MeshLODs[i].MaxDistance < cutoff)
					{
						mesh_lod = i;
						break;
					}
				}

				if((mesh_lod + 1) < static_cast<int>(layers[layer].MeshLODs.size())) //check if we have more lod's, if not stop recursive calls
				{
					final_lod = false;
				}

				if(layers[layer].MeshLODs.size() > 0)
				{
					if(mesh_lod >=  static_cast<int>(layers[layer].MeshLODs.size()))
					{
						mesh_lod = layers[layer].MeshLODs.size()-1;
					}
					const std::string mesh_name = layers[layer].MeshLODs[mesh_lod].MeshName;
					osg::Node* node = m_MRT->create(patch_trees,mesh_name,boundingBox);
					mesh_group->addChild(node);
				}
				iter++;
			}
			if(final_lod)
			{
				return mesh_group;
			}
		}

		//split bounding box for children
		double sx = (boundingBox._max.x() - boundingBox._min.x())*0.5;
		double sy = (boundingBox._max.x() - boundingBox._min.x())*0.5;
		osg::BoundingBox b1(boundingBox._min, 
			osg::Vec3(boundingBox._min.x() + sx,  boundingBox._min.y() + sy  ,boundingBox._max.z()));
		osg::BoundingBox b2(osg::Vec3(boundingBox._min.x() + sx , boundingBox._min.y()       ,boundingBox._min.z()),
			osg::Vec3(boundingBox._max.x(),       boundingBox._min.y() + sy  ,boundingBox._max.z()));

		osg::BoundingBox b3(osg::Vec3(boundingBox._min.x() + sx,  boundingBox._min.y() + sy   ,boundingBox._min.z()),
			osg::Vec3(boundingBox._max.x(),       boundingBox._max.y()		,boundingBox._max.z()));

		osg::BoundingBox b4(osg::Vec3(boundingBox._min.x(),		 boundingBox._min.y() + sy  ,boundingBox._min.z()),
			osg::Vec3(boundingBox._min.x() + sx,  boundingBox._max.y()		,boundingBox._max.z()));

		group->addChild( createLODRec(ld+1,layers,trees,b1, x*2,   y*2));
		group->addChild( createLODRec(ld+1,layers,trees,b2, x*2,   y*2+1));
		group->addChild( createLODRec(ld+1,layers,trees,b3, x*2+1, y*2+1));
		group->addChild( createLODRec(ld+1,layers,trees,b4, x*2+1, y*2)); 

		osg::LOD* plod = new osg::LOD;
		plod->setCenterMode( osg::PagedLOD::USER_DEFINED_CENTER );
		plod->setCenter( boundingBox.center());
		plod->setRadius( radius);
		plod->setRange( 0, cutoff, FLT_MAX );
		plod->setRange( 1, 0.0f, cutoff );
		plod->addChild(mesh_group);
		plod->addChild(group);
		return plod;
	}

	osg::Node* MeshScattering::create(MeshLayerVector &layers)
	{
		osg::ComputeBoundsVisitor  cbv;
		osg::BoundingBox &bb(cbv.getBoundingBox());
		m_Terrain->accept(cbv);
		osg::Vec3 terrain_size = (bb._max - bb._min);

		osg::Group* group = new osg::Group;
		group->setStateSet(m_MRT->createStateSet(layers));

		double max_dist = 0;
		for(size_t i =0 ; i < layers.size();i++)
		{
			for(size_t j =0 ; j < layers[i].MeshLODs.size();j++)
			{
				if(layers[i].MeshLODs[j].MaxDistance > max_dist)
				{
					max_dist = layers[i].MeshLODs[j].MaxDistance; 
				}
			}
		}

		//Calculate start LOD
		double t_size = std::max<double>(terrain_size.x(),terrain_size.y());
		double radius = sqrt(t_size*t_size);
		double cutoff = radius*2;

		double temp_size  = cutoff;
		m_StartLOD=-1;
		while(temp_size > max_dist)
		{
			temp_size = temp_size/2.0; 
			m_StartLOD++;
		}
		MeshVegetationMap trees;

		osg::Node* outnode = createLODRec(0, layers, trees, bb ,0,0);
		group->addChild(outnode);
		return group;
	}
}
