#pragma once

#include "cell.h"
#include "design_info.h"

extern time_t tMoveCell;

Coor Cell::getPinCoor(size_t pin_count)
{
	Coor result;
	result.x = center.x + pins[pin_count].x_offset;
	result.y = center.y + pins[pin_count].y_offset;
	return result;
}

void Cell::moveCellCenter(Coor destination_coor)
{
	/* ********************************* */
	center = destination_coor;
	ll.x = destination_coor.x - width / 2;
	ll.y = destination_coor.y - height / 2;
	/* ********************************* */
}

coor_type Cell::getPinXOffset(int pin_index)
{
	return pins[pin_index].x_offset;
}

coor_type Cell::getPinYOffset(int pin_index)
{
	return pins[pin_index].y_offset;
}
