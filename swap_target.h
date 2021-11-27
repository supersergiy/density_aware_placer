#pragma once 

#include "basic_types.h"

class SwapTarget{
public:
	bool is_cell, is_whitespace;
	int cell_index;
	Coor whitespace_center_left, center;
	coor_type whitespace_width;	
	coor_type right_ww, left_ww;

	//determines the priority of the swap target, todo
	bool operator<(const SwapTarget &a) const
	{
		return false;
	}
	
	bool operator==(const SwapTarget &a) const
	{
		bool result = (is_cell == true && a.is_cell == true && cell_index == a.cell_index) ||
				(is_whitespace == true && a.is_whitespace == true
				&& whitespace_center_left.x == a.whitespace_center_left.x
				&& whitespace_center_left.y == a.whitespace_center_left.y
				&& whitespace_width == a.whitespace_width);

		return result;
	}

	SwapTarget(void)
	{
	}

	SwapTarget(const int &c_index)
	{
		is_cell = true;
		is_whitespace = false;
		cell_index = c_index;
	}
};
