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
		  Color(255,255,255,255),
			  Width(1.0f),
			  Height(1.0f),
			  TextureUnit(0) {}

		  VegetationObject(const osg::Vec3& position, const osg::Vec4ub& color, float width, float height, unsigned int type):
		  Position(position),
			  Color(color),
			  Width(width),
			  Height(height),
			  TextureUnit(type) {}

		  osg::Vec3       Position;
		  osg::Vec4ub     Color;
		  float           Width;
		  float           Height;
		  unsigned int    TextureUnit;
		  std::string     MeshName;
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