#include "VRTSoftware.h"
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
#include "VegetationCell.h"


namespace osgVegetation
{
	osg::Node* VRTSoftware::create(Cell* cell,osg::StateSet* stateset)
	{
		osg::Node* node = NULL;
		switch(m_Tech)
		{
		case VRT_BILLBOARD:
			node = createBillboardGraph(cell,stateset);
			break;
		case VRT_MATRIX:
			node = createTransformGraph(cell,stateset);
			break;
		case VRT_COPY:
			node = createXGraph(cell,stateset);
			break;
		}
		return node;
	}

	

	osg::Node* VRTSoftware::createBillboardGraph(Cell* cell,osg::StateSet* stateset)
	{
		bool needGroup = !(cell->_cells.empty());
		bool needBillboard = !(cell->_trees.empty());

		osg::Billboard* billboard = 0;
		osg::Group* group = 0;

		if (needBillboard)
		{
			billboard = new osg::Billboard;
			billboard->setStateSet(stateset);
			for(VegetationObjectList::iterator itr=cell->_trees.begin();
				itr!=cell->_trees.end();
				++itr)
			{
				VegetationObject& tree = **itr;
				billboard->addDrawable(createSprite(tree._width,tree._height,tree._color),tree._position);
			}
		}

		if (needGroup)
		{
			group = new osg::Group;
			for(Cell::CellList::iterator itr=cell->_cells.begin();
				itr!=cell->_cells.end();
				++itr)
			{
				group->addChild(createBillboardGraph(itr->get(),stateset));
			}

			if (billboard) group->addChild(billboard);

		}
		if (group) return group;
		else return billboard;
	}

	osg::Node* VRTSoftware::createXGraph(Cell* cell,osg::StateSet* stateset)
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
				geode->addDrawable(createOrthogonalQuads(tree._position,tree._width,tree._height,tree._color));
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
	}

	osg::Node* VRTSoftware::createTransformGraph(Cell* cell,osg::StateSet* stateset)
	{
		bool needGroup = !(cell->_cells.empty());
		bool needTrees = !(cell->_trees.empty());

		osg::Group* transform_group = 0;
		osg::Group* group = 0;

		if (needTrees)
		{
			transform_group = new osg::Group;

			osg::Geometry* geometry = createOrthogonalQuads(osg::Vec3(0.0f,0.0f,0.0f),1.0f,1.0f,osg::Vec4ub(255,255,255,255));

			for(VegetationObjectList::iterator itr=cell->_trees.begin();
				itr!=cell->_trees.end();
				++itr)
			{
				VegetationObject& tree = **itr;
				osg::MatrixTransform* transform = new osg::MatrixTransform;
				transform->setMatrix(osg::Matrix::scale(tree._width,tree._width,tree._height)*osg::Matrix::translate(tree._position));

				osg::Geode* geode = new osg::Geode;
				geode->setStateSet(stateset);
				geode->addDrawable(geometry);
				transform->addChild(geode);
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
				group->addChild(createTransformGraph(itr->get(),stateset));
			}

			if (transform_group) group->addChild(transform_group);

		}
		if (group) return group;
		else return transform_group;
	}

	
}