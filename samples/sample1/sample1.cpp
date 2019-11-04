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

struct BoundingBoxCB : public osg::Drawable::ComputeBoundingBoxCallback
{
	BoundingBoxCB() {}
	BoundingBoxCB(const osg::BoundingBox &bbox) : _bbox(bbox) {};
	osg::BoundingBox computeBound(const osg::Drawable&) const { return _bbox; }
private:
	osg::BoundingBox _bbox;
};

osg::ref_ptr<osg::Node> createFlatGrid(double terrain_size, int samples)
{
	const osg::Vec3 origin(-terrain_size / 2.0, -terrain_size / 2.0, 0.0);
	const unsigned int numColumns = samples;
	const unsigned int numRows = samples;
	const osg::Vec2 size(terrain_size, terrain_size);
	osg::Geometry* geometry = new osg::Geometry;

	osg::Vec3Array& v = *(new osg::Vec3Array(numColumns*numRows));
	osg::Vec2Array& t = *(new osg::Vec2Array(numColumns*numRows));
	osg::Vec4ubArray& color = *(new osg::Vec4ubArray(1));

	color[0].set(255, 255, 255, 255);

	float rowCoordDelta = size.y() / (float)(numRows - 1);
	float columnCoordDelta = size.x() / (float)(numColumns - 1);

	float rowTexDelta = 1.0f / (float)(numRows - 1);
	float columnTexDelta = 1.0f / (float)(numColumns - 1);

	osg::Vec3 pos = origin;
	osg::Vec2 tex(0.0f, 0.0f);
	int vi = 0;
	for (unsigned int r = 0; r < numRows; ++r)
	{
		pos.x() = origin.x();
		tex.x() = 0.0f;
		for (unsigned int c = 0; c < numColumns; ++c)
		{
			v[vi].set(pos.x(), pos.y(), 0);
			t[vi].set(tex.x(), tex.y());
			pos.x() += columnCoordDelta;
			tex.x() += columnTexDelta;
			++vi;
		}
		pos.y() += rowCoordDelta;
		tex.y() += rowTexDelta;
	}

	geometry->setVertexArray(&v);
	geometry->setColorArray(&color);
	geometry->setColorBinding(osg::Geometry::BIND_OVERALL);
	geometry->setTexCoordArray(0, &t);

	osg::DrawElementsUShort& drawElements = *(new osg::DrawElementsUShort(GL_TRIANGLES, 3 * 2 * numRows * numColumns));
	geometry->addPrimitiveSet(&drawElements);
	int ei = 0;
	for (unsigned int r = 0; r < numRows - 1; ++r)
	{
		for (unsigned int c = 0; c < numColumns - 1; ++c)
		{
			drawElements[ei++] = r * numColumns + c;
			drawElements[ei++] = (r + 1)*numColumns + c;
			drawElements[ei++] = (r + 1)*numColumns + c + 1;

			drawElements[ei++] = r * numColumns + c;
			drawElements[ei++] = (r + 1)*numColumns + c + 1;
			drawElements[ei++] = r * numColumns + c + 1;

		}
	}
	osg::ref_ptr < osg::Geode> geode = new osg::Geode();
	geode->addDrawable(geometry);

	geometry->setComputeBoundingBoxCallback(new BoundingBoxCB(osg::BoundingBox(osg::Vec3d(-terrain_size / 2.0, -terrain_size / 2.0, 0),
		osg::Vec3d(terrain_size / 2.0, terrain_size / 2.0, 1))));
	return geode;
}


int main(int argc, char** argv)
{
	osgVegetation::Register.Scene.Shadow.Mode = osgVegetation::SM_LISPSM;
	osgVegetation::Register.Scene.FogMode = osgVegetation::FM_EXP2;

	Demo demo(argc, argv, osgVegetation::Register.Scene);

	osg::ref_ptr<osg::Group> root_node = new osg::Group();

	//Create the terrain geometry
	osg::ref_ptr<osg::Node> terrain = createFlatGrid(4000, 50);
	root_node->addChild(terrain);

	//Copy the terrain and prepare the copy for tessellation (convert it to patches)
	//We don't need full copy,  we can share some data because we only change primitives in this sample
	osg::ref_ptr<osg::Node> vegetation_terrain = dynamic_cast<osg::Node*>(terrain->clone(osg::CopyOp::DEEP_COPY_PRIMITIVES | osg::CopyOp::DEEP_COPY_DRAWABLES));
	osgVegetation::ConvertToPatches(vegetation_terrain);

	//Disable terrain selfshadow
	terrain->setNodeMask(osgVegetation::Register.Scene.Shadow.ReceivesShadowTraversalMask);

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
