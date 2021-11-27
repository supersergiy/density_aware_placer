#pragma once

#include "cell.h"
#include "region.h"
#include "basic_types.h"

using namespace std;

class CellLibrary{
public:
	vector<Cell> cell_list;
	Cell& operator[](const int index);
	Cell& at(const int index);
	size_t size(void);
	void reserve (size_t n);
	void push_back(const Cell c);
	vector<Cell>::iterator begin(void);
	vector<Cell>::iterator end(void);
	vector<Cell>::const_iterator begin(void) const;
	vector<Cell>::const_iterator end(void) const;

	void moveCellWithNoBinUpdate(const int index, Coor destination_coor);
	void moveCell(const int index, Coor target_coor);
	int getLeftNeighbor(const int index) const;
	int getRightNeighbor(const int index) const;
	void fillVectorWithCellsInRegion(Region r, vector<int> &cells);
	
	coor_type getBinDensity(int cell_index);
};
