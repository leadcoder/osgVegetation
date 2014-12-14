#include "VRTSoftwareGeometry.h"
#include <osg/AlphaFunc>
#include <osg/Billboard>
#include <osg/BlendFunc>
#include <osg/Depth>
#include <osg/Geode>
#include <osg/Material>
#include <osg/Math>
#include <osg/MatrixTransform>
#include <osg/PolygonOffset>
#include <osg/Projection>
#include <osg/ShapeDrawable>
#include <osg/Texture2D>
#include <osg/TextureBuffer>
#include <osg/Image>
#include <osgDB/ReadFile>
#include "VegetationCell.h"


namespace osgVegetation
{

	void VRTSoftwareGeometry::createStateSet(VegetationLayerVector &layers)
	{
		if(layers.size() == 1)
		{
			if(layers[0].MeshNames[0].size() > 0)
			{
				m_VegMesh = osgDB::readNodeFile(layers[0].MeshNames[0]);
			}
			else
			{
				osg::Geometry* geometry = createOrthogonalQuads(osg::Vec3(0.0f,0.0f,0.0f),1.0f,1.0f,osg::Vec4ub(255,255,255,255));
				osg::Geode* geode = new osg::Geode;
				
				osg::StateSet* stateset = new osg::StateSet;
				osg::Texture2D *tex = new osg::Texture2D;
				tex->setWrap( osg::Texture2D::WRAP_S, osg::Texture2D::CLAMP );
				tex->setWrap( osg::Texture2D::WRAP_T, osg::Texture2D::CLAMP );
				tex->setImage(osgDB::readImageFile(layers[0].TextureName.c_str()));

				stateset->setTextureAttributeAndModes(0, tex, osg::StateAttribute::ON );
				//dstate->setTextureAttribute(0, new osg::TexEnv );
				stateset->setAttributeAndModes( new osg::BlendFunc, osg::StateAttribute::ON );
				osg::AlphaFunc* alphaFunc = new osg::AlphaFunc;
				alphaFunc->setFunction(osg::AlphaFunc::GEQUAL,0.05f);
				stateset->setAttributeAndModes( alphaFunc, osg::StateAttribute::ON );
				stateset->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
				stateset->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );
				geode->setStateSet(stateset);
				geode->addDrawable(geometry);
				m_VegMesh = geode; 
			}
		}
	}

	osg::Node* VRTSoftwareGeometry::create(Cell* cell)
	{
		osg::Node* node = NULL;
		node = createTransformGraph2(cell);
		return node;
	}

	/*osg::Node* VRTSoftwareGeometry::createXGraph(Cell* cell,osg::StateSet* stateset)
	{
		bool needGroup = !(cell->_cells.empty());
		bool needTrees = !(cell->_trees.empty());

		osg::Geode* geode = 0;
		osg::Group* group = 0;

		if (needTrees)
		{
			geode = new osg::Geode;
			geode->setStateSet(stateset);

			for(VegetationObjectList::iterator itr=cell->_trees.begin();
				itr!=cell->_trees.end();
				++itr)
			{
				VegetationObject& tree = **itr;
				geode->addDrawable(createOrthogonalQuads(tree.Position,tree.Width,tree.Height,tree.Color));
			}
		}

		if (needGroup)
		{
			group = new osg::Group;
			for(Cell::CellList::iterator itr=cell->_cells.begin();
				itr!=cell->_cells.end();
				++itr)
			{
				group->addChild(createXGraph(itr->get(),stateset));
			}

			if (geode) group->addChild(geode);

		}
		if (group) return group;
		else return geode;
	}*/

	osg::Node* VRTSoftwareGeometry::createTransformGraph2(Cell* cell)
	{
		bool needGroup = !(cell->_cells.empty());
		bool needTrees = !(cell->_trees.empty());

		osg::Group* transform_group = 0;
		osg::Group* group = 0;

		if (needTrees)
		{
			transform_group = new osg::Group;
			for(VegetationObjectList::iterator itr=cell->_trees.begin();
				itr!=cell->_trees.end();
				++itr)
			{
				VegetationObject& tree = **itr;
				osg::MatrixTransform* transform = new osg::MatrixTransform;
				transform->setMatrix(osg::Matrix::scale(tree.Width,tree.Width,tree.Height)*osg::Matrix::translate(tree.Position));
				transform->addChild(m_VegMesh);
				transform_group->addChild(transform);
			}
		}

		if (needGroup)
		{
			group = new osg::Group;
			for(Cell::CellList::iterator itr=cell->_cells.begin();
				itr!=cell->_cells.end();
				++itr)
			{
				group->addChild(createTransformGraph2(itr->get()));
			}

			if (transform_group) group->addChild(transform_group);
		}
		if (group) return group;
		else return transform_group;
	}
}