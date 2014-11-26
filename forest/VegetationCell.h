#pragma once
#include <osg/BoundingBox>
#include <osg/Referenced>
#include <osg/vec4>
#include <osg/vec3>
#include <osg/Vec4ub>
#include <osg/ref_ptr>
#include <vector>
#include "VegetationLayer.h"


namespace osgVegetation
{

	/*class Cell : public osg::Referenced
	{
	public:
		typedef std::vector< osg::ref_ptr<Cell> > CellList;

		Cell():_parent(0) {}
		Cell(osg::BoundingBox& bb):_parent(0), _bb(bb) {}

		void addCell(Cell* cell) { cell->_parent=this; _cells.push_back(cell); }

		void addTree(VegetationObject* tree) { _trees.push_back(tree); }

		void addTrees(const VegetationObjectVector& trees) { _trees.insert(_trees.end(),trees.begin(),trees.end()); }

		void computeBound();

		bool contains(const osg::Vec3& position) const { return _bb.contains(position); }

		bool divide(unsigned int maxNumTreesPerCell=10);

		bool divide(bool xAxis, bool yAxis, bool zAxis);

		void bin();
		Cell*               _parent;
		osg::BoundingBox    _bb;
		CellList            _cells;
		VegetationObjectVector            _trees;
	};*/
}