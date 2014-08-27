#include "VegetationCell.h"

//float random(float min,float max) { return min + (max-min)*(float)rand()/(float)RAND_MAX; }
//int random(int min,int max) { return min + (int)((float)(max-min)*(float)rand()/(float)RAND_MAX); }

namespace osgVegetation
{
	void Cell::computeBound()
	{
		_bb.init();
		for(CellList::iterator citr=_cells.begin();
			citr!=_cells.end();
			++citr)
		{
			(*citr)->computeBound();
			_bb.expandBy((*citr)->_bb);
		}

		for(VegetationObjectList::iterator titr=_trees.begin();
			titr!=_trees.end();
			++titr)
		{
			_bb.expandBy((*titr)->_position);
		}
	}

	bool Cell::divide(unsigned int maxNumTreesPerCell)
	{

		if (_trees.size()<=maxNumTreesPerCell) return false;

		computeBound();

		float radius = _bb.radius();
		float divide_distance = radius*0.7f;
		if (divide((_bb.xMax()-_bb.xMin())>divide_distance,(_bb.yMax()-_bb.yMin())>divide_distance,(_bb.zMax()-_bb.zMin())>divide_distance))
		{
			// recusively divide the new cells till maxNumTreesPerCell is met.
			for(CellList::iterator citr=_cells.begin();
				citr!=_cells.end();
				++citr)
			{
				(*citr)->divide(maxNumTreesPerCell);
			}
			return true;
		}
		else
		{
			return false;
		}
	}

	bool Cell::divide(bool xAxis, bool yAxis, bool zAxis)
	{
		if (!(xAxis || yAxis || zAxis)) return false;

		if (_cells.empty())
			_cells.push_back(new Cell(_bb));

		if (xAxis)
		{
			unsigned int numCellsToDivide=_cells.size();
			for(unsigned int i=0;i<numCellsToDivide;++i)
			{
				Cell* orig_cell = _cells[i].get();
				Cell* new_cell = new Cell(orig_cell->_bb);

				float xCenter = (orig_cell->_bb.xMin()+orig_cell->_bb.xMax())*0.5f;
				orig_cell->_bb.xMax() = xCenter;
				new_cell->_bb.xMin() = xCenter;

				_cells.push_back(new_cell);
			}
		}

		if (yAxis)
		{
			unsigned int numCellsToDivide=_cells.size();
			for(unsigned int i=0;i<numCellsToDivide;++i)
			{
				Cell* orig_cell = _cells[i].get();
				Cell* new_cell = new Cell(orig_cell->_bb);

				float yCenter = (orig_cell->_bb.yMin()+orig_cell->_bb.yMax())*0.5f;
				orig_cell->_bb.yMax() = yCenter;
				new_cell->_bb.yMin() = yCenter;

				_cells.push_back(new_cell);
			}
		}

		if (zAxis)
		{
			unsigned int numCellsToDivide=_cells.size();
			for(unsigned int i=0;i<numCellsToDivide;++i)
			{
				Cell* orig_cell = _cells[i].get();
				Cell* new_cell = new Cell(orig_cell->_bb);

				float zCenter = (orig_cell->_bb.zMin()+orig_cell->_bb.zMax())*0.5f;
				orig_cell->_bb.zMax() = zCenter;
				new_cell->_bb.zMin() = zCenter;

				_cells.push_back(new_cell);
			}
		}

		bin();

		return true;

	}

	void Cell::bin()
	{
		// put trees in appropriate cells.
		VegetationObjectList treesNotAssigned;
		for(VegetationObjectList::iterator titr=_trees.begin();
			titr!=_trees.end();
			++titr)
		{
			VegetationObject* tree = titr->get();
			bool assigned = false;
			for(CellList::iterator citr=_cells.begin();
				citr!=_cells.end() && !assigned;
				++citr)
			{
				if ((*citr)->contains(tree->_position))
				{
					(*citr)->addTree(tree);
					assigned = true;
				}
			}
			if (!assigned) treesNotAssigned.push_back(tree);
		}

		// put the unassigned trees back into the original local tree list.
		_trees.swap(treesNotAssigned);


		// prune empty cells.
		CellList cellsNotEmpty;
		for(CellList::iterator citr=_cells.begin();
			citr!=_cells.end();
			++citr)
		{
			if (!((*citr)->_trees.empty()))
			{
				cellsNotEmpty.push_back(*citr);
			}
		}
		_cells.swap(cellsNotEmpty);
	}
}


