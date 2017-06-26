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

#include <osg/ArgumentParser>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>
#include <osgGA/KeySwitchMatrixManipulator>
#include <osgGA/StateSetManipulator>
#include <osgGA/AnimationPathManipulator>
#include <osgGA/TerrainManipulator>

#include <osgTerrain/Terrain>
#include <osgTerrain/TerrainTile>
#include <osgTerrain/GeometryTechnique>
#include <osg/Version>
#include <osgTerrain/Layer>

#include <osgFX/MultiTextureControl>
#include <osg/PositionAttitudeTransform>
#include <osg/PatchParameter>
#include <osg/Program>
#include <osg/AlphaFunc>
#include <osg/BlendFunc>

#include <iostream>
#include "VegetationUtils.h"
#include "Tessellation.h"

#ifndef OSG_VERSION_GREATER_OR_EQUAL
#define OSG_VERSION_GREATER_OR_EQUAL(MAJOR, MINOR, PATCH) ((OPENSCENEGRAPH_MAJOR_VERSION>MAJOR) || (OPENSCENEGRAPH_MAJOR_VERSION==MAJOR && (OPENSCENEGRAPH_MINOR_VERSION>MINOR || (OPENSCENEGRAPH_MINOR_VERSION==MINOR && OPENSCENEGRAPH_PATCH_VERSION>=PATCH))))
#endif

class ConvertToDrawPatches : public osg::NodeVisitor
{
public:
	virtual void apply(osg::Geode& node)
	{
		for (size_t drawable_num = 0; drawable_num <
			node.getNumDrawables(); ++drawable_num)
		{
			osg::Drawable* drawable = node.getDrawable(drawable_num);
			osg::Geometry* geom = drawable->asGeometry();
			if (!geom)
			{
				continue;
			}
			//merge all strips
			std::vector<unsigned int> index_list;
			size_t primset_num = 0;
			while(primset_num  < geom->getNumPrimitiveSets())
			{
				if (geom->getPrimitiveSet(primset_num)->getMode() == osg::PrimitiveSet::TRIANGLE_STRIP)
				{
					osg::PrimitiveSet* ps = geom->getPrimitiveSet(primset_num);
					int num_i = geom->getPrimitiveSet(primset_num)->getNumIndices();
					for (int j = 2; j < num_i; j++)
					{
						index_list.push_back(ps->index(j));
						if (j % 2)
						{
							index_list.push_back(ps->index(j - 1));
							index_list.push_back(ps->index(j - 2));
						}
						else
						{
							index_list.push_back(ps->index(j - 2));
							index_list.push_back(ps->index(j - 1));
						}
						//std::cout << draw->index(j);
					}
					geom->removePrimitiveSet(primset_num);
				}
				else if (geom->getPrimitiveSet(primset_num)->getMode() == osg::PrimitiveSet::TRIANGLES)
				{
					osg::PrimitiveSet* ps = geom->getPrimitiveSet(primset_num);
					ps->setMode(GL_PATCHES);
					primset_num++;
				}
				else
					primset_num++;
			}

			osg::DrawElementsUShort& drawElements = *(new osg::DrawElementsUShort(GL_PATCHES, index_list.size()));
			geom->addPrimitiveSet(&drawElements);
			for (int i = 0; i < index_list.size(); i++)
			{
				drawElements[i] = index_list[i];
			}

		}
		traverse(node);
	}
};


//class Billboard
//{
//public:
//	Billboard(const std::string &texture, const osg::Vec2f &size): TextureName(texture),
//		Size(size)
//	{
//		
//	}
//
//	~Billboard()
//	{
//
//	}
//	osg::Vec2f Size;
//	std::string TextureName;
//private:
//};
//
//class VegetationData
//{
//public:
//	VegetationData(float max_dist = 150, float density = 8): MaxDistance(max_dist),
//		Density(density)
//	{
//		
//		m_TexArray = osgVegetation::Utils::loadTextureArray(tex_names);
//	}
//
//	~VegetationData()
//	{
//
//	}
//
//	float Density;
//	float MaxDistance;
//	std::vector<Billboard> Billboards;
//
//	osg::ref_ptr<osg::Texture2DArray> GetTexArray() const
//	{
//		std::vector<std::string> tex_names;
//		for (size_t i = 0; i < Billboards.size(); i++)
//		{
//			tex_names.push_back(Billboards[i].TextureName);
//		}
//		return osgVegetation::Utils::loadTextureArray(tex_names);
//	}
//private:
//};
//
//void PrepareVegLayer(osg::StateSet* stateset, VegetationData& data)
//{
//	osg::Program* program = new osg::Program;
//	stateset->setAttribute(program);
//	osg::Uniform* baseTextureSampler = new osg::Uniform("baseTexture", 0);
//	stateset->addUniform(baseTextureSampler);
//	osg::Uniform* vegTextureSampler = new osg::Uniform("vegTexture", 1);
//	stateset->addUniform(vegTextureSampler);
//
//	
//	stateset->addUniform(new osg::Uniform("vegMaxDistance", data.MaxDistance));
//	stateset->addUniform(new osg::Uniform("vegDensity", data.Density));
//	
//
//	
//
//	int num_billboards = static_cast<int>(data.Billboards.size());
//	osg::Uniform* numBillboards = new osg::Uniform("numBillboards", num_billboards);
//	stateset->addUniform(numBillboards);
//
//	const int MAX_BILLBOARDS = 10;
//	osg::Uniform *billboardUniform = new osg::Uniform(osg::Uniform::FLOAT_VEC4, "billboardData", MAX_BILLBOARDS);
//	for (unsigned int i = 0; i < MAX_BILLBOARDS; ++i)
//	{
//		//Setdefault values
//		billboardUniform->setElement(i, osg::Vec4f(1.0f, 1.0f, 1.0f, 1.0f));
//	}
//
//	for (unsigned int i = 0; i < data.Billboards.size(); ++i)
//	{
//		osg::Vec2 size =  data.Billboards[i].Size;
//		billboardUniform->setElement(i, osg::Vec4f(size.x(), size.y(), 1.0f, 1.0f));
//	}
//
//	stateset->addUniform(billboardUniform);
//	osg::AlphaFunc* alphaFunc = new osg::AlphaFunc;
//	alphaFunc->setFunction(osg::AlphaFunc::GEQUAL, 0.9);
//	stateset->setAttributeAndModes(alphaFunc, osg::StateAttribute::ON);
//
//	//if (data.UseAlphaBlend)
//	{
//		stateset->setAttributeAndModes(new osg::BlendFunc, osg::StateAttribute::ON);
//		stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
//	}
//	
//	stateset->setTextureAttributeAndModes(1, data.GetTexArray(), osg::StateAttribute::ON);
//
//	/*program->addShader(osg::Shader::readShaderFile(osg::Shader::VERTEX, osgDB::findDataFile("shaders/terrain_vertex.glsl")));
//	program->addShader(osg::Shader::readShaderFile(osg::Shader::TESSCONTROL, osgDB::findDataFile("shaders/terrain_tess_ctrl.glsl")));
//	program->addShader(osg::Shader::readShaderFile(osg::Shader::TESSEVALUATION, osgDB::findDataFile("shaders/terrain_tess_eval.glsl")));
//	program->addShader(osg::Shader::readShaderFile(osg::Shader::FRAGMENT, osgDB::findDataFile("shaders/terrain_fragment.glsl")));
//	*/
//	program->addShader(osg::Shader::readShaderFile(osg::Shader::VERTEX, osgDB::findDataFile("shaders/veg_vertex.glsl")));
//	program->addShader(osg::Shader::readShaderFile(osg::Shader::TESSCONTROL, osgDB::findDataFile("shaders/veg_tess_ctrl.glsl")));
//	program->addShader(osg::Shader::readShaderFile(osg::Shader::TESSEVALUATION, osgDB::findDataFile("shaders/veg_tess_eval.glsl")));
//	program->addShader(osg::Shader::readShaderFile(osg::Shader::GEOMETRY, osgDB::findDataFile("shaders/veg_geometry.glsl")));
//	program->addShader(osg::Shader::readShaderFile(osg::Shader::FRAGMENT, osgDB::findDataFile("shaders/veg_fragment.glsl")));
//	program->setParameter(GL_GEOMETRY_VERTICES_OUT_EXT, 4);
//	program->setParameter(GL_GEOMETRY_OUTPUT_TYPE_EXT, GL_TRIANGLE_STRIP);
//	stateset->setAttribute(new osg::PatchParameter(3));
//}

int main(int argc, char** argv)
{
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

		std::string pathfile;
		char keyForAnimationPath = '5';
		while (arguments.read("-p", pathfile))
		{
			osgGA::AnimationPathManipulator* apm = new osgGA::AnimationPathManipulator(pathfile);
			if (apm || !apm->valid())
			{
				unsigned int num = keyswitchManipulator->getNumMatrixManipulators();
				keyswitchManipulator->addMatrixManipulator(keyForAnimationPath, "Path", apm);
				keyswitchManipulator->selectMatrixManipulator(num);
				++keyForAnimationPath;
			}
		}

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

	//load terrain
	osg::ref_ptr<osg::Node> terrain = osgDB::readNodeFile("lz.osg");

	if (!terrain)
	{
		std::cerr << "Terrain mesh not found\n";
		return 0;
	}
	//Create root node
	osg::Group* group = new osg::Group;
	group->addChild(terrain);

	

	//Add sample data path
	osgDB::Registry::instance()->getDataFilePathList().push_back("../data");

	osg::MatrixTransform* top = dynamic_cast<osg::MatrixTransform*>(terrain.get());
	osg::Geode* geode = top->getChild(1)->asGeode();

	//setup shaders

	if(true)
	{ 
		ConvertToDrawPatches convert;
		geode = dynamic_cast<osg::Geode*>(geode->clone(osg::CopyOp::DEEP_COPY_ALL));
		geode->accept(convert);

		//top->addChild(geode);
		osgVegetation::VegetationData data(100,8);
		data.Billboards.push_back(osgVegetation::Billboard("billboards/grass0.png",osg::Vec2f(1,1)));
		//data.Billboards.push_back(Billboard("billboards/fir01_bb.png", osg::Vec2f(2, 2)));
	
		osg::Group* veg_group = new osg::Group();
		top->addChild(veg_group);
		osg::Group* veg_layer1 = new osg::Group();
		veg_group->addChild(veg_layer1);
		osgVegetation::VegetationData::PrepareVegLayer(veg_layer1->getOrCreateStateSet(), data);
		veg_layer1->addChild(geode);

		osgVegetation::VegetationData tree_data(340,1);
		tree_data.Billboards.push_back(osgVegetation::Billboard("billboards/fir01_bb.png", osg::Vec2f(4, 8)));

		osg::Group* veg_layer2 = new osg::Group();
		veg_group->addChild(veg_layer2);
		osgVegetation::VegetationData::PrepareVegLayer(veg_layer2->getOrCreateStateSet(), tree_data);
		veg_layer2->addChild(geode);
		
	}
	// add a viewport to the viewer and attach the scene graph.
	viewer.setSceneData(group);

	viewer.getCamera()->getGraphicsContext()->getState()->setUseModelViewAndProjectionUniforms(true);
//	viewer.getCamera()->getGraphicsContext()->getState()->setUseModelViewAndProjectionUniforms(true);
	//viewer.getCamera()->getGraphicsContext()->getState()->resetVertexAttributeAlias(false, 8);
	//viewer.getCamera()->getGraphicsContext()->getState()->setUseVertexAttributeAliasing(true);
	// run the viewers main loop
	return viewer.run();

}
