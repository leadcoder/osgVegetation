#include "VegetationScattering.h"
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
// for the grid data..
#include "terrain_coords.h"
#include "VegetationCell.h"
#include "VRTGeometryShader.h"
#include "VRTShaderInstancing.h"
#include "VRTSoftwareGeometry.h"
#include "VegetationUtils.h"
#include "VegetationTerrainQuery.h"

namespace osgVegetation
{
	VegetationScattering::VegetationScattering(osg::Node* terrain) : m_Terrain(terrain) 
	{
		m_VRT = new VRTShaderInstancing(true,true);
		m_TerrainQuery = new VegetationTerrainQuery(terrain);
	}

	void VegetationScattering::populateVegetationLayer(const BillboardVegetationLayer& layer,const  osg::BoundingBox& bb,BillboardVegetationObjectVector& object_list, double density_scale)
	{
		osg::Vec3 origin = bb._min; 
		osg::Vec3 size = bb._max - bb._min; 
		float max_tree_height = layer.Height.y();
		float max_tree_width = layer.Width.y();

		float min_tree_height = layer.Height.x();
		float min_tree_width = layer.Width.x();

		float min_scale = layer.Scale.x();
		float max_scale = layer.Scale.y();


		unsigned int num_objects_to_create = size.x()*size.y()*layer.Density*density_scale;
		object_list.reserve(object_list.size()+num_objects_to_create);

		for(unsigned int i=0;i<num_objects_to_create;++i)
		{
			osg::Vec3 pos(Utils::random(origin.x(),origin.x()+size.x()),Utils::random(origin.y(),origin.y()+size.y()),0);
			osg::Vec3 inter;
			osg::Vec4 color; 

			if(m_TerrainQuery->getTerrainData(pos,color,inter))
			{
				if(layer.HasMaterial(color))
				{
					BillboardVegetationObject* veg_obj = new BillboardVegetationObject;
					//TODO add color to layer
					float tree_scale = Utils::random(min_scale ,max_scale);
					veg_obj->Width = Utils::random(min_tree_width,max_tree_width)*tree_scale;
					veg_obj->Height = Utils::random(min_tree_height,max_tree_height)*tree_scale;
					veg_obj->TextureUnit = layer._TextureUnit;
					veg_obj->Position = inter;
					object_list.push_back(veg_obj);
				}
			}
		}
	}
	
	BillboardVegetationObjectVector VegetationScattering::generateVegetation(BillboardVegetationLayerVector &layers,const osg::BoundingBox& bb, double density_scale)
	{
		BillboardVegetationObjectVector trees;
		double const density= 1;
		for(size_t i = 0 ; i < layers.size();i++)
		{
			populateVegetationLayer(layers[i],bb,trees,density_scale);
		}
		return trees;
	}

	std::string VegetationScattering::createFileName( unsigned int lv,	unsigned int x, unsigned int y )
	{
		std::stringstream sstream;
		sstream << "quadtree_L" << lv << "_X" << x << "_Y" << y << ".ive";
		return sstream.str();
	}

	/*osg::Node* VegetationScattering::createPagedLODRec(int ld, osg::Node* terrain, VegetationLayerVector &layers, VegetationObjectVector &trees,float current_size, float final_patch_size, float target_patch_size, osg::Vec3 center,int x, int y)
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


	osg::Node* VegetationScattering::createLODRec(int ld, BillboardVegetationLayerVector &layers, BillboardVegetationObjectVector trees, const osg::BoundingBox &bb,int x, int y)
	{
		osg::ref_ptr<osg::Group> group = new osg::Group;
		osg::Group* mesh_group = new osg::Group;
		double bb_size = (bb._max.x() - bb._min.x());

		bool final_lod = false;

		if(bb_size < m_MinPatchSize)
			final_lod = true;
		
		if(bb_size < m_ViewDistance)
		{

			//calculate density ratio for this lod level
			int ratio = (m_FinalLOD - ld);
			int tiles = pow(2.0, ratio);
			double density_ratio = tiles*tiles;
			density_ratio = 1.0/density_ratio;

			if(trees.size() == 0)
			{
				trees = generateVegetation(layers,bb,density_ratio);
			}
			BillboardVegetationObjectVector patch_trees;
			
			//get trees inside patch
			/*for(size_t i = 0; i < trees.size(); i = i++)
			{
				//if(bb.contains(trees[i]->Position))
				{
					patch_trees.push_back(trees[i]);
				}
			}*/
			osg::Node* node = m_VRT->create(trees,bb);
			trees.clear();

			mesh_group->addChild(node);
			//osgDB::writeNodeFile(*node,"c:/temp/bbveg.ive");
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

			group->addChild( createLODRec(ld+1,layers,trees,b1, x*2,   y*2));
			group->addChild( createLODRec(ld+1,layers,trees,b2, x*2,   y*2+1));
			group->addChild( createLODRec(ld+1,layers,trees,b3, x*2+1, y*2+1));
			group->addChild( createLODRec(ld+1,layers,trees,b4, x*2+1, y*2)); 

			osg::LOD* plod = new osg::LOD;
			plod->setCenterMode(osg::PagedLOD::USER_DEFINED_CENTER);
			plod->setCenter( bb.center());

			double radius = sqrt(bb_size*bb_size);
			plod->setRadius(radius);

			float cutoff = radius*2;
			//regular terrain lod setup
			//plod->addChild(mesh_group, cutoff, FLT_MAX );
			plod->addChild(mesh_group, 0, FLT_MAX );
			plod->addChild(group, 0.0f, cutoff );
			return plod;
		}
		else
			return mesh_group;
		
	}

	osg::Node* VegetationScattering::create(BillboardVegetationData &data)
	{
		osg::ComputeBoundsVisitor  cbv;
		osg::BoundingBox &bb(cbv.getBoundingBox());
		m_Terrain->accept(cbv);
		bb._max.set(std::max(bb._max.x(),bb._max.y()),std::max(bb._max.x(),bb._max.y()), bb._max.z());
		bb._min.set(std::min(bb._min.x(),bb._min.y()),std::min(bb._min.x(),bb._min.y()), bb._min.z());
		
		m_ViewDistance = data.ViewDistance;
		m_VRT->setAlphaRefValue(data.AlphaRefValue);
		m_VRT->setAlphaBlend(data.UseAlphaBlend);
		m_VRT->setTerrainNormal(data.TerrainNormal);
		osg::Group* group = new osg::Group;
		group->setStateSet(m_VRT->createStateSet(data.Layers));

		double terrain_size = bb._max.x() - bb._min.x();

		m_MinPatchSize = m_ViewDistance/4;

		double temp_size  = terrain_size;
		m_FinalLOD =0;
		while(temp_size > m_MinPatchSize)
		{
			temp_size = temp_size/2.0; 
			m_FinalLOD++;
		}
		//m_FinalLOD = terrain_size/m_MinPatchSize;

		BillboardVegetationObjectVector trees;
		osg::Node* outnode = createLODRec(0, data.Layers, trees, bb,0,0);
		group->addChild(outnode);
		return group;
	}
}
