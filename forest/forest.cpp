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
//#include "VegetationCell.h"
//#include "VRTGeometryShader.h"
//#include "VRTShaderInstancing.h"
#include "MeshVegetationScattering.h"
//#include "VRTSoftwareGeometry.h"
#include "VRTMeshShaderInstancing.h"
#include "VegetationScattering.h"

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
	osg::Group* group = new osg::Group;
	group->addChild(terrain);

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

	osgVegetation::MeshVegetationLayer treelayer1; 
	treelayer1.Density = 0.03;
	treelayer1.Height.set(0.7,1.2);
	treelayer1.Width.set(0.9,1.1);
	treelayer1.IntensitySpan.set(1.1,1.5);
	
	treelayer1.MeshLODs.push_back(osgVegetation::MeshLod("trees/pine01_no_alpha.osg.osg.osg",300));
	treelayer1.MeshLODs.push_back(osgVegetation::MeshLod("trees/pine01_no_alpha.osg.osg",150));
	treelayer1.MeshLODs.push_back(osgVegetation::MeshLod("trees/pine01_no_alpha.osg",75));
	treelayer1.Materials.push_back(material_map[WOODS]);

	osgVegetation::MeshVegetationLayer treelayer2; 
	treelayer2.Density = 0.01;
	treelayer2.Height.set(7,12);
	treelayer2.Width.set(9,11);
	//	grass1.TextureName = "Images/veg_grass02.dds";
	treelayer2.IntensitySpan.set(0.7,1.0);
	//grass1.MeshName = "cube_mapped_torus.osgt";
	treelayer2.MeshLODs.push_back(osgVegetation::MeshLod("C:/temp/osgearth/osgearth/data/pinetree.ive",300));
	treelayer2.Materials.push_back(material_map[WOODS]);
	//grass1.MeshNames.push_back("pine01.ive");
	//grass1.MeshName = "C:/temp/osgearth/osgearth/data/pinetree.ive";
	//grass1.MeshName = "C:/temp/osgearth/osgearth/data/loopix/tree7.osgb";
	//grass1.MeshName = "C:/dev/GASSData/gfx/osg/3dmodels/genesis/patria.3ds";

	osgVegetation::MeshVegetationLayerVector layers;
	//layers.push_back(treelayer1);
	layers.push_back(treelayer2);
	
	//osgVegetation::MeshVegetationScattering vs(terrain.get(),200);
	//osg::Node* veg_node = vs.create(layers);
	//group->addChild(veg_node);
	//osgDB::writeNodeFile(*veg_node,"c:/temp/test.ive");

	
	osgVegetation::BillboardVegetationData undergrowth_data(50,true,0.5,true);
	
	osgVegetation::BillboardVegetationLayer  grass2("Images/veg_grass02.dds"); 
	grass2.Density = 3.5;
	grass2.Height.set(0.3,0.6);
	grass2.Width.set(0.25,0.35);
	grass2.Scale.set(1.5,3);
	grass2.Materials.push_back(material_map[GRASS]);
	grass2.Materials.push_back(material_map[WOODS]);
	undergrowth_data.Layers.push_back(grass2);

	osgVegetation::BillboardVegetationLayer  grass3("Images/veg_plant03.dds"); 
	grass3.Density = 0.1;
	grass3.Height.set(0.6,1.2);
	grass3.Width.set(0.5,0.7);
	grass3.Scale.set(1.5,3);
	grass3.Materials.push_back(material_map[GRASS]);
	grass3.Materials.push_back(material_map[WOODS]);
	undergrowth_data.Layers.push_back(grass3);

	osgVegetation::BillboardVegetationData tree_data(400,true,0.08,false);

	osgVegetation::BillboardVegetationLayer  spruce("Images/spruce01.dds");
	spruce.Density = 0.03;
	spruce.Height.set(5,5);
	spruce.Width.set(2,2);
	spruce.Scale.set(2,3);
	spruce.Materials.push_back(material_map[WOODS]);
	tree_data.Layers.push_back(spruce);

	osgVegetation::BillboardVegetationLayer  pine("Images/pine01.dds"); 
	pine.Density = 0.03;
	pine.Height.set(5,5);
	pine.Width.set(2,2);
	pine.Scale.set(2,3);
	pine.Materials.push_back(material_map[WOODS]);
	tree_data.Layers.push_back(pine);

	osgVegetation::BillboardVegetationLayer  birch("Images/birch01.dds");
	birch.Density = 0.03;
	birch.Height.set(4,4);
	birch.Width.set(4,4);
	birch.Scale.set(2,3);
	birch.Materials.push_back(material_map[WOODS]);
	tree_data.Layers.push_back(birch);

	//osgVegetation::VegetationScattering bs(terrain.get(),400);
	//osg::Node* bb_node = bs.create(bblayers);

	osgVegetation::VegetationScattering scattering(terrain.get());
	osg::Node* ug_node = scattering.create(undergrowth_data);
	group->addChild(ug_node);
	osg::Node* tree_node = scattering.create(tree_data);
	group->addChild(tree_node);

	//osgDB::writeNodeFile(*bb_node,"c:/temp/bbveg.ive");

	// osg:Light allows us to set default parameter values to be used by osg::LightSource
	// note: we do not use light number 0, because we do not want to override the osg default headlights
	osg::Light* pLight = new osg::Light;
	//pLight->setLightNum( 4 );						
	pLight->setDiffuse( osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f) );
	pLight->setPosition( osg::Vec4(1,0,1,0) );		// last param	w = 0.0 directional light (direction)
	//				w = 1.0 point light (position)
	// light source
	osg::LightSource* pLightSource = new osg::LightSource;    
	pLightSource->setLight( pLight );

	group->addChild( pLightSource );
	
	viewer.setSceneData(group);
	
	return viewer.run();
}
