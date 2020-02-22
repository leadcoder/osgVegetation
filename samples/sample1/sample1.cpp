#include "ov_BillboardLayerConfig.h"
#include "ov_BillboardLayerStateSet.h"
#include "ov_TerrainSplatShadingStateSet.h"
#include "ov_Utils.h"
#include <osg/ArgumentParser>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>
#include <osgGA/KeySwitchMatrixManipulator>
#include <osgGA/StateSetManipulator>
#include <osgGA/AnimationPathManipulator>
#include <osgGA/TerrainManipulator>
#include <osg/Version>
#include <osg/PositionAttitudeTransform>
#include <osg/Fog>
#include <osg/ShapeDrawable>
#include <iostream>
#include "ov_Demo.h"
#include "ov_DemoTerrain.h"

namespace osgVegetation 
{
	GlobalRegister Register;
}

int main(int argc, char** argv)
{
	Demo demo(argc, argv);
	
	//Enable fog
	demo.EnableFog(osg::Fog::LINEAR);
	//Enable shadows
	demo.EnableShadow(Demo::SM_VDSM1);
	

	osg::ref_ptr<osg::Group> root_node = new osg::Group();

	//Create the terrain geometry and add it to scene
	osg::ref_ptr<osg::Node> terrain = createFlatGrid(4000, 50);
	root_node->addChild(terrain);

	//Copy the terrain and prepare the copy for tessellation (convert it to patches)
	//We don't need full copy,  we can share some data because we only change primitives in this sample
	osg::ref_ptr<osg::Node> vegetation_terrain = dynamic_cast<osg::Node*>(terrain->clone(osg::CopyOp::DEEP_COPY_PRIMITIVES | osg::CopyOp::DEEP_COPY_DRAWABLES));
	osgVegetation::ConvertToPatches(vegetation_terrain);

	//Disable terrain selfshadow (after copy/clone, we wan't vegetation to cast shadows)
	terrain->setNodeMask(osgVegetation::Register.ReceivesShadowTraversalMask);

	//Setup two grass layers, 
	//the first layer is more dens but with shorter render distance,
	//the second more sparse with longer render distance.
	osg::Vec4 grass_splat_threashold(0.5, 0.5, 0.5, 0.5);
	std::vector<osgVegetation::BillboardLayerConfig> layers;
	osgVegetation::BillboardLayerConfig grass_layer0(osgVegetation::BillboardLayerConfig::BLT_GRASS);
	grass_layer0.MaxDistance = 100;
	grass_layer0.Density = 0.1;
	grass_layer0.ColorImpact = 0.0;
	grass_layer0.CastShadow = false;
	grass_layer0.Billboards.push_back(osgVegetation::BillboardLayerConfig::Billboard("billboards/veg_plant03.png", osg::Vec2f(4, 2), 1.0, 0.008));
	grass_layer0.Billboards.push_back(osgVegetation::BillboardLayerConfig::Billboard("billboards/veg_plant01.png", osg::Vec2f(2, 2), 1.0, 0.002));
	grass_layer0.Billboards.push_back(osgVegetation::BillboardLayerConfig::Billboard("billboards/grass2.png", osg::Vec2f(2, 1), 1.0, 1.0));
	layers.push_back(grass_layer0);

	osgVegetation::BillboardLayerConfig grass_layer1(osgVegetation::BillboardLayerConfig::BLT_GRASS);
	grass_layer1.MaxDistance = 30;
	grass_layer1.Density = 0.4;
	grass_layer1.CastShadow = false;
	grass_layer1.ColorImpact = 0.0;
	grass_layer1.Billboards.push_back(osgVegetation::BillboardLayerConfig::Billboard("billboards/grass2.png", osg::Vec2f(2, 1), 1.0, 1.0));
	layers.push_back(grass_layer1);

	//Setup tree layer, 
	osgVegetation::BillboardLayerConfig tree_data(osgVegetation::BillboardLayerConfig::BLT_ROTATED_QUAD);
	tree_data.MaxDistance = 740;
	tree_data.Density = 0.001;
	tree_data.ColorImpact = 0.0;
	tree_data.Billboards.push_back(osgVegetation::BillboardLayerConfig::Billboard("billboards/fir01_bb.png", osg::Vec2f(10, 16), 1.6, 1.0));
	layers.push_back(tree_data);

	//Create layers from config and add them to root node
	for (size_t i = 0; i < layers.size(); i++)
	{
		//first create the effect node
		osg::ref_ptr<osgVegetation::BillboardLayerEffect> bb_layer = new osgVegetation::BillboardLayerEffect(layers[i]);

		//...then add the terrain to be rendered with this effect
		bb_layer->addChild(vegetation_terrain);

		//Last, add layer to scene
		root_node->addChild(bb_layer);
	}

	demo.GetSceneRoot()->addChild(root_node);
	demo.Run();
	return 0;
}
