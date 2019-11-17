#include "ov_BillboardLayerConfig.h"
#include "ov_MeshLayerConfig.h"
#include "ov_LayerGenerator.h"
#include "ov_BillboardLayerStateSet.h"
#include "ov_TerrainSplatShadingStateSet.h"
#include "ov_Utils.h"
#include <osgDB/ReadFile>
#include <iostream>
#include "ov_Demo.h"
#include "ov_DemoTerrain.h"


namespace osgVegetation
{
	GlobalRegister Register;
}


int main(int argc, char** argv)
{
	//Enable LiSPSM shadows
	osgVegetation::Register.Scene.Shadow.Mode = osgVegetation::SM_LISPSM;
	
	//Enable fog
	osgVegetation::Register.Scene.Fog.Mode = osgVegetation::FM_EXP2;
	

	Demo demo(argc, argv, osgVegetation::Register.Scene);

	osg::ref_ptr<osg::Group> root_node = new osg::Group();

	//Create the terrain geometry and add it to scene
	osg::ref_ptr<osg::Node> terrain = createFlatGrid(4000, 50);
	root_node->addChild(terrain);

	//Copy the terrain and prepare the copy for tessellation (convert it to patches)
	//We don't need full copy,  we can share some data because we only change primitives in this sample
	osg::ref_ptr<osg::Node> vegetation_terrain = dynamic_cast<osg::Node*>(terrain->clone(osg::CopyOp::DEEP_COPY_PRIMITIVES | osg::CopyOp::DEEP_COPY_DRAWABLES));
	osgVegetation::ConvertToPatches(vegetation_terrain);

	//Disable terrain selfshadow (after copy/clone, we wan't vegetation to cast shadows)
	terrain->setNodeMask(osgVegetation::Register.Scene.Shadow.ReceivesShadowTraversalMask);

	//Setup two grass layers, 
	//the first layer is more dens but with shorter render distance,
	//the second more sparse with longer render distance.
	osg::Vec4 grass_splat_threashold(0.5, 0.5, 0.5, 0.5);
	std::vector<osg::ref_ptr<osgVegetation::ILayerConfig> > layers;
	osg::ref_ptr <osgVegetation::BillboardLayerConfig> grass_layer0 = new osgVegetation::BillboardLayerConfig(osgVegetation::BillboardLayerConfig::BLT_GRASS);
	grass_layer0->MaxDistance = 100;
	grass_layer0->Density = 0.1;
	grass_layer0->ColorImpact = 0.0;
	grass_layer0->CastShadow = false;
	grass_layer0->Billboards.push_back(osgVegetation::BillboardLayerConfig::Billboard("billboards/veg_plant03.png", osg::Vec2f(4, 2), 1.0, 0.008));
	grass_layer0->Billboards.push_back(osgVegetation::BillboardLayerConfig::Billboard("billboards/veg_plant01.png", osg::Vec2f(2, 2), 1.0, 0.002));
	grass_layer0->Billboards.push_back(osgVegetation::BillboardLayerConfig::Billboard("billboards/grass2.png", osg::Vec2f(2, 1), 1.0, 1.0));
	layers.push_back(grass_layer0);
	
	osg::ref_ptr <osgVegetation::BillboardLayerConfig> grass_layer1 = new osgVegetation::BillboardLayerConfig(osgVegetation::BillboardLayerConfig::BLT_GRASS);
	grass_layer1->MaxDistance = 30;
	grass_layer1->Density = 0.4;
	grass_layer1->CastShadow = false;
	grass_layer1->ColorImpact = 0.0;
	grass_layer1->Billboards.push_back(osgVegetation::BillboardLayerConfig::Billboard("billboards/grass2.png", osg::Vec2f(2, 1), 1.0, 1.0));
	layers.push_back(grass_layer1);

	//Setup tree layer, 
	osg::ref_ptr <osgVegetation::MeshLayerConfig> tree_layer = new osgVegetation::MeshLayerConfig(1000);
	osgVegetation::MeshTypeConfig mesh;
	const float end_dist = 200.0f;
	mesh.MeshLODs.push_back(osgVegetation::MeshTypeConfig::MeshLODConfig("trees/fir01_l0.osg", osg::Vec4(0.0f, 0.0f, 100.0f, 110.0f)));
	mesh.MeshLODs.push_back(osgVegetation::MeshTypeConfig::MeshLODConfig("trees/fir01_l1.osg", osg::Vec4(100.0f, 110.0f, end_dist, end_dist + 10)));
	tree_layer->MeshTypes.push_back(mesh);
	layers.push_back(tree_layer);

	osgVegetation::LayerGenerator generator(layers);
	osg::ref_ptr<osg::Group> vegetation_node = generator.CreateVegetationNode(vegetation_terrain);
	root_node->addChild(vegetation_node);

	demo.GetSceneRoot()->addChild(root_node);
	demo.Run();
	return 0;
}
