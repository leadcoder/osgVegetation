/* OpenSceneGraph example, osgforest.
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
*  THE SOFTWARE.
*/

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
#include <osgDB/WriteFile>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osg/Texture2DArray>

#include <osgUtil/LineSegmentIntersector>
#include <osgUtil/IntersectionVisitor>
#include <osgUtil/SmoothingVisitor>

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
#include "VegetationScattering.h"
#include "VRTSoftwareGeometry.h"
#include "VRTMeshShaderInstancing.h"

/*
// class to create the forest and manage the movement between various techniques.
namespace osgVegetation
{
	
class ForestTechniqueManager : public osg::Referenced
{
public:
	ForestTechniqueManager() {}
	typedef std::vector< osg::ref_ptr<VegetationObject> > TreeList;
	
	float random(float min,float max) { return min + (max-min)*(float)rand()/(float)RAND_MAX; }
	int random(int min,int max) { return min + (int)(((float)(max-min)*(float)rand()/(float)RAND_MAX) + 0.5f); }
	osg::Node* createPatch(osg::Node* terrain, osg::Vec3 origin, osg::Vec3 size, osg::StateSet *dstate);
	osg::Geode* createTerrain(const osg::Vec3& origin, const osg::Vec3& size);
	void createTreeList(osg::Node* terrain,const osg::Vec3& origin, const osg::Vec3& size,double density,TreeList& trees);
	osg::Node* createScene(unsigned int numTreesToCreates, unsigned int maxNumTreesPerCell);
	void createPatch(osg::Node* terrain, osg::StateSet *dstate, float patch_size, osg::LOD* node, float current_size);

	osg::Node* outputSubScene(int ld,osg::Node* terrain,osg::StateSet *dstate, float current_size, float target_patch_size, osg::Vec3 center,int x,int y);

};

osg::Geode* ForestTechniqueManager::createTerrain(const osg::Vec3& origin, const osg::Vec3& size)
{
	osg::Geode* geode = new osg::Geode();

	// ---------------------------------------
	// Set up a StateSet to texture the objects
	// ---------------------------------------
	osg::StateSet* stateset = new osg::StateSet();

	osg::Image* image = osgDB::readImageFile("Images/lz.rgb");
	if (image)
	{
		osg::Texture2D* texture = new osg::Texture2D;
		texture->setImage(image);
		stateset->setTextureAttributeAndModes(0,texture,osg::StateAttribute::ON);
	}

	geode->setStateSet( stateset );

	unsigned int numColumns = 38;
	unsigned int numRows = 39;
	unsigned int r;
	unsigned int c;

	// compute z range of z values of grid data so we can scale it.
	float min_z = FLT_MAX;
	float max_z = -FLT_MAX;
	for(r=0;r<numRows;++r)
	{
		for(c=0;c<numColumns;++c)
		{
			min_z = osg::minimum(min_z,vertex[r+c*numRows][2]);
			max_z = osg::maximum(max_z,vertex[r+c*numRows][2]);
		}
	}

	float scale_z = size.z()/(max_z-min_z);


	bool createGrid = false;
	if (createGrid)
	{

		osg::HeightField* grid = new osg::HeightField;
		grid->allocate(numColumns,numRows);
		grid->setOrigin(origin);
		grid->setXInterval(size.x()/(float)(numColumns-1));
		grid->setYInterval(size.y()/(float)(numRows-1));

		for(r=0;r<numRows;++r)
		{
			for(c=0;c<numColumns;++c)
			{
				grid->setHeight(c,r,(vertex[r+c*numRows][2]-min_z)*scale_z);
			}
		}

		geode->addDrawable(new osg::ShapeDrawable(grid));
	}
	else
	{
		osg::Geometry* geometry = new osg::Geometry;

		osg::Vec3Array& v = *(new osg::Vec3Array(numColumns*numRows));
		osg::Vec2Array& t = *(new osg::Vec2Array(numColumns*numRows));
		osg::Vec4ubArray& color = *(new osg::Vec4ubArray(1));

		color[0].set(255,255,255,255);

		float rowCoordDelta = size.y()/(float)(numRows-1);
		float columnCoordDelta = size.x()/(float)(numColumns-1);

		float rowTexDelta = 1.0f/(float)(numRows-1);
		float columnTexDelta = 1.0f/(float)(numColumns-1);

		osg::Vec3 pos = origin;
		osg::Vec2 tex(0.0f,0.0f);
		int vi=0;
		for(r=0;r<numRows;++r)
		{
			pos.x() = origin.x();
			tex.x() = 0.0f;
			for(c=0;c<numColumns;++c)
			{
				v[vi].set(pos.x(),pos.y(),pos.z()+(vertex[r+c*numRows][2]-min_z)*scale_z);
				t[vi].set(tex.x(),tex.y());
				pos.x()+=columnCoordDelta;
				tex.x()+=columnTexDelta;
				++vi;
			}
			pos.y() += rowCoordDelta;
			tex.y() += rowTexDelta;
		}

		geometry->setVertexArray(&v);
		geometry->setColorArray(&color, osg::Array::BIND_OVERALL);
		geometry->setTexCoordArray(0,&t);

		for(r=0;r<numRows-1;++r)
		{
			osg::DrawElementsUShort& drawElements = *(new osg::DrawElementsUShort(GL_QUAD_STRIP,2*numColumns));
			geometry->addPrimitiveSet(&drawElements);
			int ei=0;
			for(c=0;c<numColumns;++c)
			{
				drawElements[ei++] = (r+1)*numColumns+c;
				drawElements[ei++] = (r)*numColumns+c;
			}
		}

		geode->addDrawable(geometry);

		osgUtil::SmoothingVisitor sv;
		sv.smooth(*geometry);
	}

	return geode;
}

void ForestTechniqueManager::createTreeList(osg::Node* terrain,const osg::Vec3& origin, const osg::Vec3& size,double density,TreeList& trees)
{
	float max_TreeHeight = 1;//sqrtf(size.length2()/(float)numTreesToCreate);
	float max_TreeWidth = 1.2;//max_TreeHeight*0.5f;

	float min_TreeHeight = max_TreeHeight*0.5f;
	float min_TreeWidth = max_TreeWidth*0.5f;
	
	
	unsigned int numTreesToCreate = size.x()*size.y()*density;
	trees.reserve(trees.size()+numTreesToCreate);

	for(unsigned int i=0;i<numTreesToCreate;++i)
	{
		VegetationObject* tree = new VegetationObject;
		tree->Position.set(random(origin.x(),origin.x()+size.x()),random(origin.y(),origin.y()+size.y()),0);
		tree->Color.set(random(128,255),random(128,255),random(128,255),255);
		tree->Width = random(min_TreeWidth,max_TreeWidth);
		tree->Height = random(min_TreeHeight,max_TreeHeight);
		tree->TextureUnit = random(0,1);

		if (terrain)
		{
			osg::ref_ptr<osgUtil::LineSegmentIntersector> intersector =
				new osgUtil::LineSegmentIntersector(tree->Position,tree->Position+osg::Vec3(0.0f,0.0f,1000));

			osgUtil::IntersectionVisitor iv(intersector.get());

			terrain->accept(iv);

			if (intersector->containsIntersections())
			{
				osgUtil::LineSegmentIntersector::Intersections& intersections = intersector->getIntersections();
				for(osgUtil::LineSegmentIntersector::Intersections::iterator itr = intersections.begin();
					itr != intersections.end();
					++itr)
				{
					const osgUtil::LineSegmentIntersector::Intersection& intersection = *itr;
					tree->Position = intersection.getWorldIntersectPoint();
				}
			}
		}

		trees.push_back(tree);
	}
}
osg::Node* ForestTechniqueManager::createPatch(osg::Node* terrain, osg::Vec3 origin, osg::Vec3 size, osg::StateSet *dstate)
{
	TreeList trees;
	double const density= 1;
	int const maxNumTreesPerCell = 2000;
	createTreeList(terrain,origin,size,density,trees);
	osg::ref_ptr<Cell> cell = new Cell;
	cell->addTrees(trees);
	cell->divide(maxNumTreesPerCell);
	VRTGeometryShader graph;
	//VRTShaderInstancing graph;
	return graph.create(cell.get(),dstate);
}

std::string createFileName( unsigned int lv,
	unsigned int x, unsigned int y )
{
	std::stringstream sstream;
	sstream << "quadtree_L" << lv << "_X" << x << "_Y" << y << ".ive";
	return sstream.str();
}


osg::Node* ForestTechniqueManager::outputSubScene(int ld, osg::Node* terrain,osg::StateSet *dstate, float current_size, float target_patch_size, osg::Vec3 center,int x, int y)
{
	if(current_size < target_patch_size)
	{
		osg::Vec3 p_origin(center.x() - current_size*0.5, center.y() - current_size*0.5,center.z());
		osg::Vec3 p_size(current_size,current_size,1);
		osg::Node* f_node = createPatch(terrain,p_origin,p_size,dstate);
		return f_node;
	}

	osg::Group *group = new osg::Group;
	osg::Vec3 c1_center(center.x() - current_size*0.25,center.y() - current_size*0.25,center.z());
	osg::Vec3 c2_center(center.x() - current_size*0.25,center.y() + current_size*0.25,center.z());
	osg::Vec3 c3_center(center.x() + current_size*0.25,center.y() + current_size*0.25,center.z());
	osg::Vec3 c4_center(center.x() + current_size*0.25,center.y() - current_size*0.25,center.z());

	group->addChild( outputSubScene(ld+1,terrain,dstate, current_size*0.5, target_patch_size, c1_center, x*2,   y*2));
	group->addChild( outputSubScene(ld+1,terrain,dstate, current_size*0.5, target_patch_size, c2_center, x*2,   y*2+1));
	group->addChild( outputSubScene(ld+1,terrain,dstate, current_size*0.5, target_patch_size, c3_center, x*2+1, y*2+1));
	group->addChild( outputSubScene(ld+1,terrain,dstate, current_size*0.5, target_patch_size, c4_center, x*2+1, y*2)); 

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
}


void ForestTechniqueManager::createPatch(osg::Node* terrain, osg::StateSet *dstate, float patch_size, osg::LOD* node, float current_size)
{
	osg::Vec3 center = node->getCenter();
	if(current_size < patch_size*2)
	{
		//final lod
		osg::Vec3 p_origin(center.x() - current_size*0.5, center.y() - current_size*0.5,center.z());
		osg::Vec3 p_size(current_size,current_size,1);
		osg::Node* f_node = createPatch(terrain,p_origin,p_size,dstate);
		node->addChild(f_node,0,current_size*2);
		//f_node->setCullingActive(false);
	}
	else
	{
		float radius = sqrt(current_size*current_size);
		{
			osg::Vec3 c_center(center.x() - current_size*0.25, center.y() - current_size*0.25, center.z());
			osg::LOD* lod_t1 = new osg::LOD();
			lod_t1->setCenterMode( osg::PagedLOD::USER_DEFINED_CENTER );
			lod_t1->setCenter(c_center);
			lod_t1->setRadius(radius);
			//lod_t1->setCullingActive(false);
			osg::BoundingSphere sphere(c_center,radius);
			lod_t1->setInitialBound(sphere);

			createPatch(terrain, dstate, patch_size, lod_t1, current_size*0.5);
			node->addChild(lod_t1,0,current_size*2);
		}
		
		{
			osg::Vec3 c_center(center.x() - current_size*0.25,center.y() + current_size*0.25,center.z());
			osg::LOD* lod_t2 = new osg::LOD();
			lod_t2->setCenterMode(osg::PagedLOD::USER_DEFINED_CENTER );
			lod_t2->setRadius(radius);
			lod_t2->setCenter(c_center);
			//lod_t2->setCullingActive(false);
			
			osg::BoundingSphere sphere(c_center,radius);
			lod_t2->setInitialBound(sphere);

			createPatch(terrain, dstate, patch_size, lod_t2, current_size*0.5);
			node->addChild(lod_t2,0,current_size*2);
		}
		
		{
			osg::Vec3 c_center(center.x() + current_size*0.25,center.y() + current_size*0.25,center.z());
			osg::LOD* lod_t3 = new osg::LOD();
			lod_t3->setCenterMode( osg::PagedLOD::USER_DEFINED_CENTER );
			lod_t3->setRadius(radius);
			lod_t3->setCenter(c_center);
			//lod_t3->setCullingActive(false);
			osg::BoundingSphere sphere(c_center,radius);
			lod_t3->setInitialBound(sphere);


			createPatch(terrain, dstate, patch_size, lod_t3, current_size*0.5);
			node->addChild(lod_t3,0,current_size*2);
		}
		
		{
			osg::Vec3 c_center(center.x() + current_size*0.25,center.y() - current_size*0.25,center.z());
			osg::LOD* lod_t4 = new osg::LOD();
			lod_t4->setCenterMode( osg::PagedLOD::USER_DEFINED_CENTER );
			lod_t4->setRadius(radius);
			lod_t4->setCenter(c_center);
			//lod_t4->setCullingActive(false);
			osg::BoundingSphere sphere(c_center,radius);
			lod_t4->setInitialBound(sphere);

			createPatch(terrain, dstate, patch_size, lod_t4, current_size*0.5);
			node->addChild(lod_t4,0,current_size*2);
		}
	}
}


osg::Node* ForestTechniqueManager::createScene(unsigned int numTreesToCreates, unsigned int maxNumTreesPerCell)
{
	osg::Vec3 origin(0.0f,0.0f,0.0f);
	osg::Vec3 size(1000.0f,1000.0f,200.0f);

	std::cout<<"Creating terrain...";
	osg::ref_ptr<osg::Node> terrain = osgDB::readNodeFile("C:/temp/kvarn/Grid0/tiles/0x1_3_3x3.ive.osg");
	//osg::ref_ptr<osg::Node> terrain = createTerrain(origin,size);
	std::cout<<"done."<<std::endl;
	osg::ComputeBoundsVisitor  cbv;
	osg::BoundingBox &bb(cbv.getBoundingBox());
	terrain->accept(cbv);

	//calculate number of patches
	osg::Vec3 terrain_size = (bb._max - bb._min);
	const float patch_size = 50;
	const float view_dist = patch_size*4;

	int num_p_x = terrain_size.x() / patch_size;
	int num_p_y = terrain_size.y() / patch_size;
	
	//osg::Texture2D *tex = new osg::Texture2D;
	//tex->setWrap( osg::Texture2D::WRAP_S, osg::Texture2D::CLAMP );
	//tex->setWrap( osg::Texture2D::WRAP_T, osg::Texture2D::CLAMP );
	//tex->setImage(osgDB::readImageFile("Images/veg_grass02.dds"));

	osg::Texture2DArray* tex = new osg::Texture2DArray;
	tex->setTextureSize(256, 256, 2);
	tex->setUseHardwareMipMapGeneration(false);   
	//tex->setTextureDepth(2);
	tex->setImage(0, osgDB::readImageFile("Images/veg_grass02.dds"));
	tex->setImage(1, osgDB::readImageFile("Images/veg_grass03.dds"));

	tex->setFilter(osg::Texture2DArray::MIN_FILTER, 
		osg::Texture2DArray::NEAREST);
	tex->setFilter(osg::Texture2DArray::MAG_FILTER, 
		osg::Texture2DArray::NEAREST);
	tex->setWrap(osg::Texture::WRAP_S, 
		osg::Texture::CLAMP_TO_BORDER);
	tex->setWrap(osg::Texture::WRAP_T, 
		osg::Texture::CLAMP_TO_BORDER);
	tex->setWrap(osg::Texture::WRAP_R, 
		osg::Texture::CLAMP_TO_BORDER);

	//ss->setTextureAttribute(1, tex); 


	//tex->setFilter( osg::Texture2DArray::MIN_FILTER, 
	//	osg::Texture2DArray::NEAREST); 
	//tex->setFilter( osg::Texture2DArray::MAG_FILTER, 
	//	osg::Texture2DArray::NEAREST); 


	osg::StateSet *dstate = new osg::StateSet;
	{

		dstate->setTextureAttributeAndModes(0, tex, osg::StateAttribute::ON );
		dstate->setTextureAttribute(0, tex,	osg::StateAttribute::ON);
		//dstate->setTextureAttribute(0, tex, osg::StateAttribute::ON);
		//dstate->setTextureMode(0, osg::Texture2DArray::GL_TEXTURE2D_ARRAY, osg::StateAttribute::ON);
		//dstate->setTextureAttribute(0, new osg::TexEnv );
		dstate->setAttributeAndModes( new osg::BlendFunc, osg::StateAttribute::ON );
		osg::AlphaFunc* alphaFunc = new osg::AlphaFunc;
		alphaFunc->setFunction(osg::AlphaFunc::GEQUAL,0.05f);
		dstate->setAttributeAndModes( alphaFunc, osg::StateAttribute::ON );
		dstate->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
		dstate->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );

		//osg::Uniform* baseTextureSampler = new osg::Uniform("baseTexture",0);
		osg::Uniform* baseTextureSampler = new osg::Uniform(osg::Uniform::SAMPLER_2D_ARRAY, "baseTexture", 2);
		dstate->addUniform(baseTextureSampler);
	}
	osg::Group* group = new osg::Group;
	/*for(int i = 0; i < num_p_x; i++)
	{
		for(int j = 0; j < num_p_y; j++)
		{
			osg::Vec3 p_origin(bb._min.x() + patch_size*i , bb._min.y() + patch_size*j,bb._min.z());
			osg::Vec3 p_size(patch_size,patch_size,bb._max.z() - bb._min.z());
			osg::LOD* lod_node =  new osg::LOD();
			lod_node->setCenter(p_origin + p_size*0.5);
			osg::Node* node = createPatch(terrain,p_origin,p_size,dstate);
			lod_node->addChild(node,0,view_dist);
			group->addChild(lod_node);
		}
	}
	osgDB::writeNodeFile(*group,"c:/temp/test.osg");*/
	
	/*osg::LOD* lod_node =  new osg::LOD();
	lod_node->setCenterMode( osg::PagedLOD::USER_DEFINED_CENTER );
	lod_node->setCenter(bb.center());
	lod_node->setRadius(sqrt(terrain_size.x()*terrain_size.x()));
	//lod_node->setCullingActive(false);
	//osg::BoundingSphere sphere(bb.center(),sqrt(terrain_size.x()*terrain_size.x()));
	//lod_node->setInitialBound(sphere);
	createPatch(terrain,dstate, patch_size,lod_node,terrain_size.x());
	group->addChild(lod_node);
	group->addChild(terrain.get());
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

	/*osg::Group* scene = new osg::Group;
	//scene->addChild(terrain.get());
	scene->addChild(group);
	return scene;
}
}*/

int main( int argc, char **argv )
{
	// use an ArgumentParser object to manage the program arguments.
	osg::ArgumentParser arguments(&argc,argv);

	// construct the viewer.
	osgViewer::Viewer viewer(arguments);

	//unsigned int numTreesToCreate = 2000;
	//arguments.read("--trees",numTreesToCreate);
	//unsigned int maxNumTreesPerCell = sqrtf(static_cast<float>(numTreesToCreate));
	//arguments.read("--trees-per-cell",maxNumTreesPerCell);
	//osg::ref_ptr<osgVegetation::ForestTechniqueManager> ttm = new osgVegetation::ForestTechniqueManager;

	// add the stats handler
	viewer.addEventHandler(new osgViewer::StatsHandler);

	//viewer.addEventHandler(new TechniqueEventHandler(ttm.get()));
	viewer.addEventHandler(new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()));


	osg::ref_ptr<osgGA::KeySwitchMatrixManipulator> keyswitchManipulator = new osgGA::KeySwitchMatrixManipulator;

	keyswitchManipulator->addMatrixManipulator( '1', "Trackball", new osgGA::TrackballManipulator() );
	keyswitchManipulator->addMatrixManipulator( '2', "Flight", new osgGA::FlightManipulator() );
	keyswitchManipulator->addMatrixManipulator( '3', "Drive", new osgGA::DriveManipulator() );
	keyswitchManipulator->addMatrixManipulator( '4', "Terrain", new osgGA::TerrainManipulator() );
	keyswitchManipulator->addMatrixManipulator( '5', "Orbit", new osgGA::OrbitManipulator() );
	keyswitchManipulator->addMatrixManipulator( '6', "FirstPerson", new osgGA::FirstPersonManipulator() );
	keyswitchManipulator->addMatrixManipulator( '7', "Spherical", new osgGA::SphericalManipulator() );
	viewer.setCameraManipulator( keyswitchManipulator.get() );
	
	//Add texture search paths
	osgDB::Registry::instance()->getDataFilePathList().push_back("C:/temp/kvarn/Grid0/tiles");
	osgDB::Registry::instance()->getDataFilePathList().push_back("C:/temp/kvarn/Grid0/material_textures");  

	
	
	//osg::ref_ptr<osg::Node> terrain = osgDB::readNodeFile("C:/temp/kvarn/Grid0/tiles/0x1_3_3x3.ive.osg");
	//osg::ref_ptr<osg::Node> terrain = osgDB::readNodeFile("C:/temp/kvarn/Grid0/tiles/0x0_0_0x0.ive");
	osg::ref_ptr<osg::Node> terrain = osgDB::readNodeFile("C:/temp/kvarn/proxy.osg");


	enum MaterialEnum
	{
		GRASS,
		ROAD,
		WOODS,
		DIRT
	};
	std::map<MaterialEnum,osgVegetation::MaterialColor> material_map;
	material_map[GRASS] = osgVegetation::MaterialColor(0,0,0,1);
	material_map[WOODS] = osgVegetation::MaterialColor(0,1,0,1);
	material_map[ROAD] = osgVegetation::MaterialColor(0,0,1,1);
	material_map[DIRT] = osgVegetation::MaterialColor(1,0,0,1);


	osgVegetation::VegetationLayerVector layers;
	osgVegetation::VegetationLayer grass1; 
	grass1.Density = 0.03;
	grass1.Height.set(0.7,1.2);
	grass1.Width.set(0.9,1.1);
	grass1.TextureName = "Images/veg_grass02.dds";
	grass1.MaxColor.set(255,200,255);
	grass1.MinColor.set(220,170,220);
	//grass1.MeshName = "cube_mapped_torus.osgt";
	//grass1.MeshName = "cow.osgt";
	grass1.MeshName = "pine01.ive";
	//grass1.MeshName = "C:/temp/osgearth/osgearth/data/pinetree.ive";
	//grass1.MeshName = "C:/temp/osgearth/osgearth/data/loopix/tree7.osgb";
	//grass1.MeshName = "C:/dev/GASSData/gfx/osg/3dmodels/genesis/patria.3ds";
	grass1.Materials.push_back(material_map[WOODS]);
	grass1.TextureUnit = 0;
	layers.push_back(grass1);

	osgVegetation::VegetationLayer grass2; 
	grass2.Density = 0.1;
	grass2.Height.set(0.3,0.6);
	grass2.Width.set(0.25,0.35);
	grass2.TextureName = "Images/veg_grass02.dds";
	grass2.Materials.push_back(material_map[GRASS]);
	//layers.push_back(grass2);

	osgVegetation::VegetationLayer grass3; 
	grass3.Density = 0.1;
	grass3.Height.set(0.6,1.2);
	grass3.Width.set(0.5,0.7);
	grass3.TextureName = "Images/veg_plant03.dds";
	grass3.Materials.push_back(material_map[GRASS]);
	grass3.Materials.push_back(material_map[WOODS]);
//	layers.push_back(grass3);


	osg::Group* group = new osg::Group;
	group->addChild(terrain);
	osgVegetation::VRTMeshShaderInstancing*msi = new osgVegetation::VRTMeshShaderInstancing();
	osgVegetation::VegetationScattering vs(msi,103);
	//osgVegetation::VegetationScattering vs(new osgVegetation::VRTGeometryShader(),50);
	//osgVegetation::VegetationScattering vs(new osgVegetation::VRTShaderInstancing(),2250);
	//osgVegetation::VegetationScattering vs(new osgVegetation::VRTSoftwareGeometry(),50);
	osg::Node* veg_node = vs.create(terrain.get(), layers);
	veg_node->setStateSet(msi->m_StateSet);
	group->addChild(veg_node);
	
	osgDB::writeNodeFile(*veg_node,"c:/temp/test.ive");
	viewer.setSceneData(group);
	
	return viewer.run();
}
