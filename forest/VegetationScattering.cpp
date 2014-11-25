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
	VegetationScattering::VegetationScattering(osg::Node* terrain, double view_dist) :	m_Terrain(terrain), 
		m_PatchTargetSize(view_dist)
	{
		m_ViewDistance = m_PatchTargetSize;
		m_VRT = new VRTShaderInstancing();
		m_TerrainQuery = new VegetationTerrainQuery(terrain);
	}


	void VegetationScattering::populateVegetationLayer(const BillboardVegetationLayer& layer,const  osg::BoundingBox& bb,BillboardVegetationObjectVector& object_list)
	{
		osg::Vec3 origin = bb._min; 
		osg::Vec3 size = bb._max - bb._min; 
		float max_TreeHeight = layer.Height.y();
		float max_TreeWidth = layer.Width.y();

		float min_TreeHeight = layer.Height.x();
		float min_TreeWidth = layer.Width.x();
		unsigned int num_objects_to_create = size.x()*size.y()*layer.Density;
		object_list.reserve(object_list.size()+num_objects_to_create);

		//m_IntersectionVisitor.reset();
	
		for(unsigned int i=0;i<num_objects_to_create;++i)
		{
			osg::Vec3 pos(random(origin.x(),origin.x()+size.x()),random(origin.y(),origin.y()+size.y()),0);
			//osg::ref_ptr<osgUtil::LineSegmentIntersector> intersector =
			//	new osgUtil::LineSegmentIntersector(pos,pos+osg::Vec3(0.0f,0.0f,1000));

			//m_IntersectionVisitor.setIntersector(intersector.get());
			//m_Terrain->accept(m_IntersectionVisitor);
			osg::Vec3 inter;
			osg::Vec4 color; 

			if(m_TerrainQuery->getTerrainData(pos,color,inter))
			{
				if(layer.HasMaterial(color))
				{
					BillboardVegetationObject* veg_obj = new BillboardVegetationObject;
					//TODO add color to layer
					veg_obj->Width = random(min_TreeWidth,max_TreeWidth);
					veg_obj->Height = random(min_TreeHeight,max_TreeHeight);
					veg_obj->TextureUnit = layer.TextureUnit;
					veg_obj->Position = inter;
					object_list.push_back(veg_obj);
				}
			}
			/*if (intersector->containsIntersections())
			{
				osgUtil::LineSegmentIntersector::Intersections& intersections = intersector->getIntersections();
				for(osgUtil::LineSegmentIntersector::Intersections::iterator itr = intersections.begin();
					itr != intersections.end();
					++itr)
				{
					const osgUtil::LineSegmentIntersector::Intersection& intersection = *itr;
					
					osg::Vec4 color; 
					osg::Vec3 tc;
					osg::Texture* texture = Utils::getTexture(intersection,tc);
					//check if dds, if so we will try to load alternative image file because we have no utils to decompress dds
					if(osgDB::getFileExtension(texture->getImage(0)->getFileName()) == "dds")
					{
						std::string new_texture_file = osgDB::getNameLessExtension(osgDB::getSimpleFileName(texture->getImage(0)->getFileName())) + ".rgb";
						//first check cache
						osg::Image* image = NULL;
						MaterialCacheMap::iterator iter = m_MaterialCache.find(new_texture_file);
						if(iter != m_MaterialCache.end())
						{
							image = iter->second.get();
						}
						else
						{
							m_MaterialCache[new_texture_file] = osgDB::readImageFile(new_texture_file);
							image = m_MaterialCache[new_texture_file].get();
						}
						if(image)
						{
							color = image->getColor(tc);
						}
					}
					else
						color = texture->getImage(0)->getColor(tc);

					
				}
			}*/

		}
	}
	
	BillboardVegetationObjectVector VegetationScattering::generateVegetation(BillboardVegetationLayerVector &layers,const osg::BoundingBox& bb)
	{
		BillboardVegetationObjectVector trees;
		double const density= 1;
		for(size_t i = 0 ; i < layers.size();i++)
		{
			populateVegetationLayer(layers[i],bb,trees);
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
		osg::ref_ptr<osg::Group> mesh_group = new osg::Group;
		double bb_size = (bb._max.x() - bb._min.x());
		if(bb_size < m_ViewDistance) //final lod
		{
			if(trees.size() == 0)
			{
				trees = generateVegetation(layers,bb);
			}

			BillboardVegetationObjectVector patch_trees;
			/*size_t step = 0;
			if(current_size < final_patch_size)
			{
				step = 1;
			}*/
			//get trees inside patch
			for(size_t i = 0; i < trees.size(); i++)
			{
				if(bb.contains(trees[i]->Position))
				{
					patch_trees.push_back(trees[i]);
				}
			}
			return m_VRT->create(patch_trees,bb);
		}

		//split bounding box for children
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

		/*osg::LOD* plod = new osg::LOD;
		plod->setCenterMode( osg::PagedLOD::USER_DEFINED_CENTER );
		plod->setCenter( bb.center());
		float radius = sqrt(current_size*current_size);
		plod->setRadius( radius);
		float cutoff = radius*2;
		plod->addChild(group,0,FLT_MAX);*/
		osg::LOD* plod = new osg::LOD;
		plod->setCenterMode(osg::PagedLOD::USER_DEFINED_CENTER);
		plod->setCenter( bb.center());
		
		double radius = sqrt(bb_size*bb_size);
		plod->setRadius(radius);
		
		float cutoff = radius*2;
		plod->setRange( 0, cutoff, FLT_MAX );
		plod->setRange( 1, 0.0f, cutoff );
		plod->addChild(mesh_group);
		plod->addChild(group);
		return plod;
	}

	osg::Node* VegetationScattering::create(BillboardVegetationLayerVector &layers)
	{
		osg::ComputeBoundsVisitor  cbv;
		osg::BoundingBox &bb(cbv.getBoundingBox());
		m_Terrain->accept(cbv);
		osg::Vec3 terrain_size = (bb._max - bb._min);

		osg::Group* group = new osg::Group;
		group->setStateSet(m_VRT->createStateSet(layers));

		BillboardVegetationObjectVector trees;
		osg::Node* outnode = createLODRec(0, layers, trees, bb,0,0);
		group->addChild(outnode);
		return group;
	}
}
