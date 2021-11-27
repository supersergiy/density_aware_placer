#pragma once

#include "basic_types.h" 
#include "swap_target.h"
#include <iostream>

using namespace std;

class Region{
public:
	coor_type left_limit, right_limit, top_limit, bot_limit;
	
	Region(void)
	{
		left_limit = -1;
		right_limit = -1;
		top_limit = -1;
		bot_limit = -1;
	}

	bool isInThisRegion(int cell_index);
	bool isInThisRegion(Coor point);
	bool isInThisRegion(SwapTarget &s); 

	bool isTotallyInThisRegion(int cell_index);

	friend ostream& operator<< (ostream &out, Region r);

	void getTopAndBottomRows(int &top_row, int &bot_row);

	void Clear(void)
	{
		left_limit = -1;
		right_limit = -1;
		top_limit = -1;
		bot_limit = -1;
	}
};