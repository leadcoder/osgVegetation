/* OpenSceneGraph example, osgterrain.
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

#include "ov_BillboardLayer.h"
#include "ov_BillboardNodeGenerator.h"
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
#include "ov_DemoTerrain.h"
#include "ov_DemoShadow.h"


std::vector<osgVegetation::BillboardLayer> GetVegetationLayers()
{
	std::vector<osgVegetation::BillboardLayer> layers;
	osgVegetation::BillboardLayer grass_data(100, 0.1, 1.0, 0.4, 0.1, 5);
	grass_data.Type = osgVegetation::BillboardLayer::BLT_GRASS;
	//grass_data.Type = osgVegetation::BillboardLayer::BLT_CROSS_QUADS;
	grass_data.Billboards.push_back(osgVegetation::BillboardLayer::Billboard("billboards/veg_plant03.png", osg::Vec2f(4, 2), 0.9, 0.008));
	grass_data.Billboards.push_back(osgVegetation::BillboardLayer::Billboard("billboards/veg_plant01.png", osg::Vec2f(2, 2), 0.9, 0.002));
	grass_data.Billboards.push_back(osgVegetation::BillboardLayer::Billboard("billboards/grass2.png", osg::Vec2f(2, 1), 1.1, 1.0));
	layers.push_back(grass_data);

	osgVegetation::BillboardLayer grass_data2(30, 0.4, 1.0, 0.4, 0.1, 5);
	grass_data2.Type = osgVegetation::BillboardLayer::BLT_GRASS;
	grass_data2.Billboards.push_back(osgVegetation::BillboardLayer::Billboard("billboards/grass2.png", osg::Vec2f(2, 1), 1.0, 1.0));
	layers.push_back(grass_data2);

	//osgVegetation::BillboardLayer tree_data(2740, 0.001, 0.5, 0.7, 0.1, 2);
	osgVegetation::BillboardLayer tree_data(2740, 0.01, 0.5, 0.7, 0.1, 2);
	//tree_data.Type = osgVegetation::BillboardLayer::BLT_CROSS_QUADS;
	tree_data.Type = osgVegetation::BillboardLayer::BLT_ROTATED_QUAD;
	tree_data.Billboards.push_back(osgVegetation::BillboardLayer::Billboard("billboards/fir01_bb.png", osg::Vec2f(10, 16), 1.2, 1.0));
	layers.push_back(tree_data);
	return layers;
}

osg::ref_ptr<osg::Group> CreateTerrainAndVegetation()
{
	const double terrain_size = 2000;
	const bool share_terrain_geometry = true;
	
	//Create terrain geometry used for both terrain layer and  vegetation layers
	osg::ref_ptr<osg::Node> terrain_geometry = CreateDemoTerrain(terrain_size);
	osg::ref_ptr<osg::Node> terrain_geometry_vegetation = terrain_geometry;
	if(share_terrain_geometry)
		osgVegetation::ConvertToPatches(terrain_geometry);
	else
	{
		terrain_geometry_vegetation = dynamic_cast<osg::Node*>(terrain_geometry->clone(osg::CopyOp::DEEP_COPY_ALL));
		osgVegetation::ConvertToPatches(terrain_geometry_vegetation);
	}

	//Create main node for both terrain and vegetation
	osg::ref_ptr<osg::Group> root = new osg::Group();

	//root->addChild(CreateDemoTerrain(terrain_size));
	
	//setup shared texture resources.
	{
		osg::StateSet* stateset = root->getOrCreateStateSet();
		osg::ref_ptr<osg::Image> image = osgDB::readRefImageFile("Images/lz.rgb");
		if (image)
		{
			osg::Texture2D* texture = new osg::Texture2D;
			texture->setImage(image);
			stateset->setTextureAttributeAndModes(0, texture, osg::StateAttribute::ON);
		}
	}

	//Create terrain layer node
	{
		//add some detail mapping based on landcover
		osgVegetation::TerrainConfiguration tdm;
		tdm.DetailLayers.push_back(osgVegetation::DetailLayer(std::string("terrain/detail/detail_grass_mossy.dds"), 0.1));
		tdm.DetailLayers.push_back(osgVegetation::DetailLayer(std::string("terrain/detail/detail_dirt.dds"), 0.1));
		tdm.DetailLayers.push_back(osgVegetation::DetailLayer(std::string("terrain/detail/detail_grass_mossy.dds"), 0.1));
		tdm.DetailLayers.push_back(osgVegetation::DetailLayer(std::string("terrain/detail/detail_dirt.dds"), 0.1));
		tdm.UseTessellation = share_terrain_geometry;

		osgVegetation::TerrainGenerator terrain_generator(tdm);
		osg::ref_ptr<osg::Group> terrain_layer = terrain_generator.Create(terrain_geometry);
		
		//Disable terrain self shadowning
		terrain_layer->setNodeMask(ReceivesShadowTraversalMask);
		//add terrain layer to top node
		root->addChild(terrain_layer);
	}

	//Create vegetation layer node
	{
		std::vector<osgVegetation::BillboardLayer> veg_config = GetVegetationLayers();
		osgVegetation::BillboardNodeGenerator node_generator(veg_config);
		root->addChild(node_generator.CreateNode(terrain_geometry_vegetation));
		
	}
	return root;
}

int main(int argc, char** argv)
{
	osgVegetation::SceneConfiguration config;
	config.ShadowMode = osgVegetation::SM_LISPSM;
	config.FogMode = osgVegetation::FM_EXP2;

	osg::ArgumentParser arguments(&argc, argv);

	// construct the viewer.
	osgViewer::Viewer viewer(arguments);

	// set up the camera manipulators.
	{
		osg::ref_ptr<osgGA::KeySwitchMatrixManipulator> keyswitchManipulator = new osgGA::KeySwitchMatrixManipulator;
		keyswitchManipulator->addMatrixManipulator('1', "Trackball", new osgGA::TrackballManipulator());
		keyswitchManipulator->addMatrixManipulator('2', "Flight", new osgGA::FlightManipulator());
		keyswitchManipulator->addMatrixManipulator('3', "Drive", new osgGA::DriveManipulator());
		keyswitchManipulator->addMatrixManipulator('4', "Terrain", new osgGA::TerrainManipulator());
		viewer.setCameraManipulator(keyswitchManipulator.get());
	}

	// add the state manipulator
	viewer.addEventHandler(new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()));

	// add the stats handler
	viewer.addEventHandler(new osgViewer::StatsHandler);

	// add the record camera path handler
	viewer.addEventHandler(new osgViewer::RecordCameraPathHandler);

	// add the window size toggle handler
	viewer.addEventHandler(new osgViewer::WindowSizeHandler);

	osg::DisplaySettings::instance()->setNumMultiSamples(4);

	//Add sample data path
	osgDB::Registry::instance()->getDataFilePathList().push_back("../data");
	
	osg::ref_ptr<osg::Group> root_node = CreateShadowNode(config.ShadowMode);
	
	osg::ref_ptr<osg::Group> terrain_and_vegetation_node = CreateTerrainAndVegetation();

	//apply scene settings to terrain and vegetation shaders
	osgVegetation::SetSceneDefinitions(terrain_and_vegetation_node->getOrCreateStateSet(), config);
	
	root_node->addChild(terrain_and_vegetation_node);
	
	if (!root_node)
	{
		osg::notify(osg::NOTICE) << "Warning: no valid data loaded, please specify a database on the command line." << std::endl;
		return 1;
	}
	
	//Add light and shadows
	osg::Light* light = new osg::Light;
	light->setDiffuse(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
	osg::Vec4 light_pos(1, 1.0, 1, 0);
	light->setPosition(light_pos);		// last param	w = 0.0 directional light (direction)
	osg::Vec3f light_dir(-light_pos.x(), -light_pos.y(), -light_pos.z());
	light_dir.normalize();
	light->setDirection(light_dir);
	light->setAmbient(osg::Vec4(0.4f, 0.4f, 0.4f, 1.0f));

	osg::LightSource* light_source = new osg::LightSource;
	light_source->setLight(light);
	root_node->addChild(light_source);

	viewer.setThreadingModel(osgViewer::ViewerBase::SingleThreaded);

	if (config.FogMode != osgVegetation::FM_DISABLED) //Add fog effect?
	{
		const osg::Vec4 fog_color(0.5, 0.6, 0.7, 1.0);
		osg::StateSet* state = root_node->getOrCreateStateSet();
		osg::ref_ptr<osg::Fog> fog = new osg::Fog();
		state->setMode(GL_FOG, osg::StateAttribute::ON);
		state->setAttributeAndModes(fog.get());
		fog->setMode(osg::Fog::Mode(config.FogMode));
		fog->setDensity(0.0005);
		fog->setColor(fog_color);
		viewer.getCamera()->setClearColor(fog_color);
	}

	
	
	viewer.setSceneData(root_node);
	viewer.setUpViewInWindow(100, 100, 800, 600);

	viewer.getCamera()->getGraphicsContext()->getState()->setUseModelViewAndProjectionUniforms(true);

	// run the viewers main loop
	while (!viewer.done())
	{
		float t = viewer.getFrameStamp()->getSimulationTime() * 0.5;
		light_pos.set(sinf(t), cosf(t), 1, 0.0f);
		light->setPosition(light_pos);
		light_dir.set(-light_pos.x(), -light_pos.y(), -light_pos.z());
		light_dir.normalize();
		light->setDirection(light_dir);
		
		viewer.frame();
	}
	return 0;
}
