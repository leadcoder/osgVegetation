#include "MeshVegetationScattering.h"
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
#include "VRTMeshShaderInstancing.h"

namespace osgVegetation
{

	osg::Texture* getTextureLookUp2(const osgUtil::LineSegmentIntersector::Intersection& intersection,osg::Vec3& tc) 
	{
			osg::Geometry* geometry = intersection.drawable.valid() ? intersection.drawable->asGeometry() : 0;
			osg::Vec3Array* vertices = geometry ? dynamic_cast<osg::Vec3Array*>(geometry->getVertexArray()) : 0;

			if (vertices)
			{
				if (intersection.indexList.size()==3 && intersection.ratioList.size()==3)
				{
					unsigned int i1 = intersection.indexList[0];
					unsigned int i2 = intersection.indexList[1];
					unsigned int i3 = intersection.indexList[2];

					float r1 = intersection.ratioList[0];
					float r2 = intersection.ratioList[1];
					float r3 = intersection.ratioList[2];

					osg::Array* texcoords = (geometry->getNumTexCoordArrays()>0) ? geometry->getTexCoordArray(0) : 0;
					osg::FloatArray* texcoords_FloatArray = dynamic_cast<osg::FloatArray*>(texcoords);
					osg::Vec2Array* texcoords_Vec2Array = dynamic_cast<osg::Vec2Array*>(texcoords);
					osg::Vec3Array* texcoords_Vec3Array = dynamic_cast<osg::Vec3Array*>(texcoords);
					if (texcoords_FloatArray)
					{
						// we have tex coord array so now we can compute the final tex coord at the point of intersection.
						float tc1 = (*texcoords_FloatArray)[i1];
						float tc2 = (*texcoords_FloatArray)[i2];
						float tc3 = (*texcoords_FloatArray)[i3];
						tc.x() = tc1*r1 + tc2*r2 + tc3*r3;
					}
					else if (texcoords_Vec2Array)
					{
						// we have tex coord array so now we can compute the final tex coord at the point of intersection.
						const osg::Vec2& tc1 = (*texcoords_Vec2Array)[i1];
						const osg::Vec2& tc2 = (*texcoords_Vec2Array)[i2];
						const osg::Vec2& tc3 = (*texcoords_Vec2Array)[i3];
						tc.x() = tc1.x()*r1 + tc2.x()*r2 + tc3.x()*r3;
						tc.y() = tc1.y()*r1 + tc2.y()*r2 + tc3.y()*r3;
					}
					else if (texcoords_Vec3Array)
					{
						// we have tex coord array so now we can compute the final tex coord at the point of intersection.
						const osg::Vec3& tc1 = (*texcoords_Vec3Array)[i1];
						const osg::Vec3& tc2 = (*texcoords_Vec3Array)[i2];
						const osg::Vec3& tc3 = (*texcoords_Vec3Array)[i3];
						tc.x() = tc1.x()*r1 + tc2.x()*r2 + tc3.x()*r3;
						tc.y() = tc1.y()*r1 + tc2.y()*r2 + tc3.y()*r3;
						tc.z() = tc1.z()*r1 + tc2.z()*r2 + tc3.z()*r3;
					}
					else
					{
						return 0;
					}
				}

				const osg::TexMat* activeTexMat = 0;
				const osg::Texture* activeTexture = 0;

				if (intersection.drawable->getStateSet())
				{
					const osg::TexMat* texMat = dynamic_cast<osg::TexMat*>(intersection.drawable->getStateSet()->getTextureAttribute(0,osg::StateAttribute::TEXMAT));
					if (texMat) activeTexMat = texMat;

					const osg::Texture* texture = dynamic_cast<osg::Texture*>(intersection.drawable->getStateSet()->getTextureAttribute(0,osg::StateAttribute::TEXTURE));
					if (texture) activeTexture = texture;
				}

				for(osg::NodePath::const_reverse_iterator itr = intersection.nodePath.rbegin();
					itr != intersection.nodePath.rend() && (!activeTexMat || !activeTexture);
					++itr)
				{
					const osg::Node* node = *itr;
					if (node->getStateSet())
					{
						if (!activeTexMat)
						{
							const osg::TexMat* texMat = dynamic_cast<const osg::TexMat*>(node->getStateSet()->getTextureAttribute(0,osg::StateAttribute::TEXMAT));
							if (texMat) activeTexMat = texMat;
						}

						if (!activeTexture)
						{
							const osg::Texture* texture = dynamic_cast<const osg::Texture*>(node->getStateSet()->getTextureAttribute(0,osg::StateAttribute::TEXTURE));
							if (texture) activeTexture = texture;
						}
					}
				}

				if (activeTexMat)
				{
					osg::Vec4 tc_transformed = osg::Vec4(tc.x(), tc.y(), tc.z() ,0.0f) * activeTexMat->getMatrix();
					tc.x() = tc_transformed.x();
					tc.y() = tc_transformed.y();
					tc.z() = tc_transformed.z();

					if (activeTexture && activeTexMat->getScaleByTextureRectangleSize())
					{
						tc.x() *= static_cast<float>(activeTexture->getTextureWidth());
						tc.y() *= static_cast<float>(activeTexture->getTextureHeight());
						tc.z() *= static_cast<float>(activeTexture->getTextureDepth());
					}
				}

				return const_cast<osg::Texture*>(activeTexture);

			}
			return 0;
		}
	

	MeshVegetationScattering::MeshVegetationScattering(osg::Node* terrain, double view_dist) : m_ViewDistance(view_dist),
		m_Terrain(terrain)
	{
		m_IntersectionVisitor.setReadCallback(new osgSim::DatabaseCacheReadCallback);
		m_VRT = new VRTMeshShaderInstancing();
		//m_VRTTexture = new VRTGeometryShader();
	}

	struct PagedReaderCallback : public osgUtil::IntersectionVisitor::ReadCallback
	{
		std::vector<osg::ref_ptr<osg::Node>  > pagedLODVec;

		virtual osg::Node* readNodeFile( const std::string& filename )
		{ 
			osg::ref_ptr<osg::Node> node = osgDB::readNodeFile(filename); 
			pagedLODVec.push_back(node);
			return node.get();
		}
	};

	void MeshVegetationScattering::populateVegetationLayer(const MeshVegetationLayer& layer, const osg::Vec3& origin, const osg::Vec3& size,MeshVegetationObjectVector& object_list)
	{
		unsigned int num_objects_to_create = size.x()*size.y()*layer.Density;
		object_list.reserve(object_list.size()+num_objects_to_create);
		m_IntersectionVisitor.reset();
		m_IntersectionVisitor.setLODSelectionMode(osgUtil::IntersectionVisitor::USE_HIGHEST_LEVEL_OF_DETAIL);

		float min_TreeHeight = layer.Height.x();
		float min_TreeWidth = layer.Width.x();

		float max_TreeHeight = layer.Height.y();
		float max_TreeWidth = layer.Width.y();


		for(unsigned int i=0;i<num_objects_to_create;++i)
		{
			if (m_Terrain)
			{
				osg::Vec3 pos(random(origin.x(),origin.x()+size.x()),random(origin.y(),origin.y()+size.y()),0);
				osg::ref_ptr<osgUtil::LineSegmentIntersector> intersector =
					new osgUtil::LineSegmentIntersector(pos,pos+osg::Vec3(0.0f,0.0f,1000));

				m_IntersectionVisitor.setIntersector(intersector.get());
				m_Terrain->accept(m_IntersectionVisitor);
				

				if (intersector->containsIntersections())
				{
					osgUtil::LineSegmentIntersector::Intersections& intersections = intersector->getIntersections();
					for(osgUtil::LineSegmentIntersector::Intersections::iterator itr = intersections.begin();
						itr != intersections.end();
						++itr)
					{
						const osgUtil::LineSegmentIntersector::Intersection& intersection = *itr;
					
						osg::Vec4 color; 
						osg::Vec3 tc;
						osg::Texture* texture = getTextureLookUp2(intersection,tc);
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

						if(layer.HasMaterial(color))
						{
							MeshVegetationObject* veg_obj = new MeshVegetationObject;

							//TODO add color to layer
							float intesity = random(layer.IntensitySpan.x(), layer.IntensitySpan.y());
							veg_obj->Color.set(intesity,intesity,intesity,1.0);
							veg_obj->Width = random(min_TreeWidth,max_TreeWidth);
							veg_obj->Height = random(min_TreeHeight,max_TreeHeight);
							veg_obj->Position = intersection.getWorldIntersectPoint();
							veg_obj->Rotation = osg::Quat(random(0,osg::PI),osg::Vec3(0,0,1));
							object_list.push_back(veg_obj);
						}
					}
				}
			}
		}
	}

	MeshVegetationMap MeshVegetationScattering::generateVegetation(MeshVegetationLayerVector &layers, osg::Vec3 origin, osg::Vec3 size)
	{
		MeshVegetationMap objects;
		for(size_t i = 0 ; i < layers.size();i++)
		{
			MeshVegetationObjectVector trees;
			populateVegetationLayer(layers[i],origin,size,trees);
			objects[i] = trees;
		}
		return objects;
	}


	std::string MeshVegetationScattering::createFileName( unsigned int lv,	unsigned int x, unsigned int y )
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

	osg::Node* MeshVegetationScattering::createLODRec(int ld, MeshVegetationLayerVector &layers, MeshVegetationMap trees, float current_size, osg::Vec3 center,int x, int y)
	{
		float radius = sqrt(current_size*current_size);
		float cutoff = radius*2;

		osg::ref_ptr<osg::Group> group = new osg::Group;
		
		if(ld == m_StartLOD)
		{
			//generate vegetation data
			osg::Vec3 p_origin(center.x() - current_size*0.5, center.y() - current_size*0.5,center.z());
			osg::Vec3 p_size(current_size,current_size,1);
			trees = generateVegetation(layers,p_origin,p_size);
		}
		osg::Group* mesh_group= new osg::Group();
		if(ld >= m_StartLOD)
		{
			osg::Vec3 min_p(center.x() - current_size*0.5, center.y() - current_size*0.5,center.z());
			osg::Vec3 max_p = min_p + osg::Vec3(current_size,current_size,1);
			osg::BoundingBox bb(min_p,max_p);
			
			
			bool final_lod = true;
			MeshVegetationMap::iterator iter = trees.begin();
			while(iter != trees.end())
			{
				MeshVegetationObjectVector patch_trees;

				int layer = iter->first;
				for(size_t i = 0; i < iter->second.size(); i++)
				{
					MeshVegetationObject* tree = iter->second[i];
					if(fabs(tree->Position.x() - center.x()) < current_size*0.5 && 
						fabs(tree->Position.y() - center.y()) < current_size*0.5)
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
				
				if((mesh_lod + 1) < layers[layer].MeshLODs.size()) //check if we have more lods, if not stop recursive calls
				{
					final_lod = false;
				}
				
				if(layers[layer].MeshLODs.size() > 0)
				{
					if(mesh_lod >= layers[layer].MeshLODs.size())
					{
						mesh_lod = layers[layer].MeshLODs.size()-1;
					}
					const std::string mesh_name = layers[layer].MeshLODs[mesh_lod].MeshName;
					osg::Node* node = m_VRT->create(patch_trees,mesh_name,bb);
					mesh_group->addChild(node);
				}
				iter++;
			}
			if(final_lod)
			{
				return mesh_group;
			}
		}

		osg::Vec3 c1_center(center.x() - current_size*0.25,center.y() - current_size*0.25,center.z());
		osg::Vec3 c2_center(center.x() - current_size*0.25,center.y() + current_size*0.25,center.z());
		osg::Vec3 c3_center(center.x() + current_size*0.25,center.y() + current_size*0.25,center.z());
		osg::Vec3 c4_center(center.x() + current_size*0.25,center.y() - current_size*0.25,center.z());

		group->addChild( createLODRec(ld+1,layers,trees,current_size*0.5, c1_center, x*2,   y*2));
		group->addChild( createLODRec(ld+1,layers,trees,current_size*0.5, c2_center, x*2,   y*2+1));
		group->addChild( createLODRec(ld+1,layers,trees,current_size*0.5, c3_center, x*2+1, y*2+1));
		group->addChild( createLODRec(ld+1,layers,trees,current_size*0.5, c4_center, x*2+1, y*2)); 
	
		osg::LOD* plod = new osg::LOD;
		plod->setCenterMode( osg::PagedLOD::USER_DEFINED_CENTER );
		plod->setCenter( center );
		plod->setRadius( radius);
		plod->setRange( 0, cutoff, FLT_MAX );
		plod->setRange( 1, 0.0f, cutoff );
		plod->addChild(mesh_group);
		plod->addChild(group);
		return plod;
	}

	osg::Node* MeshVegetationScattering::create(MeshVegetationLayerVector &layers)
	{
		osg::ComputeBoundsVisitor  cbv;
		osg::BoundingBox &bb(cbv.getBoundingBox());
		m_Terrain->accept(cbv);
		osg::Vec3 terrain_size = (bb._max - bb._min);

		osg::Group* group = new osg::Group;
		group->setStateSet(m_VRT->createStateSet(layers));

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
		osg::Node* outnode = createLODRec(0, layers, trees, terrain_size.x(), bb.center(),0,0);
		group->addChild(outnode);
		
		return group;

		//osgDB::writeNodeFile(*group,"c:/temp/test.ive");

		/*osg::Node* outnode = outputSubScene(0, terrain,dstate, terrain_size.x(), patch_size, bb.center(),0,0);
		group->addChild(outnode);
		group->addChild(terrain.get());
		osgDB::writeNodeFile(*group,"C:/temp/paged/test.ive");*/

		/*	size = (bb._max - bb._min);
		std::cout<<"Creating tree locations...";std::cout.flush();
		TreeList trees;
		createTreeList(terrain.get(),origin,size,numTreesToCreates,trees);
		std::cout<<"done."<<std::endl;

		std::cout<<"Creating cell subdivision...";
		osg::ref_ptr<Cell> cell = new Cell;
		cell->addTrees(trees);
		cell->divide(maxNumTreesPerCell);
		std::cout<<"done."<<std::endl;

		osg::Texture2D *tex = new osg::Texture2D;
		tex->setWrap( osg::Texture2D::WRAP_S, osg::Texture2D::CLAMP );
		tex->setWrap( osg::Texture2D::WRAP_T, osg::Texture2D::CLAMP );
		tex->setImage(osgDB::readImageFile("Images/tree0.rgba"));

		osg::StateSet *dstate = new osg::StateSet;
		{
		dstate->setTextureAttributeAndModes(0, tex, osg::StateAttribute::ON );
		dstate->setTextureAttribute(0, new osg::TexEnv );
		dstate->setAttributeAndModes( new osg::BlendFunc, osg::StateAttribute::ON );
		osg::AlphaFunc* alphaFunc = new osg::AlphaFunc;
		alphaFunc->setFunction(osg::AlphaFunc::GEQUAL,0.05f);
		dstate->setAttributeAndModes( alphaFunc, osg::StateAttribute::ON );
		dstate->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
		dstate->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );
		}
		osg::Group* group = new osg::Group;
		{
		VRTGeometryShader graph;
		//VRTShaderInstancing graph;
		std::cout<<"Creating osg::Billboard based forest...";
		group->addChild(graph.create(cell.get(),dstate));

		osgDB::writeNodeFile(*group,"c:/temp/test.osg");
		//exit(0);
		}*/
	}
}
