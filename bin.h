#pragma once

#include "basic_types.h"
#include "region.h"
#include "parameters.h"
#include "multithread.h"
#include "cell.h"
#include <set>

using namespace std;

class Bin {
private:

public:
	set<int> cellsInside;
	Region region;
	coor_type density;

	bool store_cells;
	coor_type total_area, cell_area, free_area;
	bool canBeUse;

	Bin(){}
	Bin(Region _r, bool store, coor_type bin_area) : region(_r), density(0.0), cell_area(0.0), canBeUse(false), store_cells(store)
	{
//		total_area = ( region.top_limit - region.bot_limit ) * (region.right_limit - region.left_limit);
		total_area = /*max((region.right_limit - region.left_limit) * (region.top_limit - region.bot_limit), (coor_type)0.0)*/ bin_area;
		free_area = total_area;
	}
	float getDensity(void);
	Region getRegion(void);
	float overlapArea(int cell_index);
	void cellMoveOut(int cell_index);
	void cellMoveIn(int cell_index);
	void cellMoveIn(int cell_index, Coor target);

	coor_type getNewDensity(coor_type new_area)
	{
		if(free_area <= 0.2 * total_area)
			return 0;
		else {
			if(new_area < 0) {
				return max(((cell_area + new_area) / free_area), (coor_type)0.0);
			}
			else {
				return min(((cell_area + new_area) / free_area), (coor_type)1.0);
			}
		}
	}

	double overlapAreaWithThisCell(Cell &c);

	double overlapAreaWithThisCell(int cell_index);


#ifdef SERGIY_IS_NOT_BITCH
	void getCellsInside(set<int> &s)
	{
		s = set<int>(cellsInside);
	}
	void getCellsInside(vector<int> &v)
	{
		v = vector<int>(cellsInside.begin(), cellsInside.end());
	}
#endif
	bool isTotallyInsideMacroBlock() { return total_area == 0; }
	bool cellIsInThisBin(int cell_index);
	bool pointIsInThisBin(Coor c);
	bool isOverlapWithThisRegion(Region r);
	coor_type areaOfOverlapWithThisCell(int cell_coord);
	void fillVectorWithSwapTargetsInThisBin(vector<SwapTarget> &swap_targets);

};
