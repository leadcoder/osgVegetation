#pragma once
#include <osg/BoundingBox>
#include <osg/Referenced>
#include <osg/vec4>
#include <osg/vec3>
#include <osg/Vec4ub>
#include <osg/ref_ptr>
#include <vector>

namespace osgVegetation
{
	class VegetationObject : public osg::Referenced
	{
	public:
		VegetationObject():
		  _color(255,255,255,255),
			  _width(1.0f),
			  _height(1.0f),
			  _type(0) {}

		  VegetationObject(const osg::Vec3& position, const osg::Vec4ub& color, float width, float height, unsigned int type):
		  _position(position),
			  _color(color),
			  _width(width),
			  _height(height),
			  _type(type) {}

		  osg::Vec3       _position;
		  osg::Vec4ub     _color;
		  float           _width;
		  float           _height;
		  unsigned int    _type;
	};

	typedef std::vector< osg::ref_ptr<VegetationObject> > VegetationObjectList;
	class Cell : public osg::Referenced
	{
	public:
		typedef std::vector< osg::ref_ptr<Cell> > CellList;

		Cell():_parent(0) {}
		Cell(osg::BoundingBox& bb):_parent(0), _bb(bb) {}

		void addCell(Cell* cell) { cell->_parent=this; _cells.push_back(cell); }

		void addTree(VegetationObject* tree) { _trees.push_back(tree); }

		void addTrees(const VegetationObjectList& trees) { _trees.insert(_trees.end(),trees.begin(),trees.end()); }

		void computeBound();

		bool contains(const osg::Vec3& position) const { return _bb.contains(position); }

		bool divide(unsigned int maxNumTreesPerCell=10);

		bool divide(bool xAxis, bool yAxis, bool zAxis);

		void bin();
		Cell*               _parent;
		osg::BoundingBox    _bb;
		CellList            _cells;
		VegetationObjectList            _trees;
	};
}