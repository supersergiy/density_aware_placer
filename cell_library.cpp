#include "main.h"
#include <math.h>
#include <limits.h>

extern time_t tMoveCellWithUpdate, tMoveCell;



Cell& CellLibrary::operator[](const int index)
{
	return cell_list[index];
}
/* ************************************************** */
Cell& CellLibrary::at(const int index)
{
	return cell_list.at(index);
}
/* ************************************************** */
void CellLibrary::moveCell(const int cell_index, Coor target_coor)
{
	if (getCellCenter(cell_index) == target_coor){
		return;
	}

	Coor target_coor_ll(target_coor.x - design.cell_lib[cell_index].getWidth() / 2, target_coor.y - design.cell_lib[cell_index].getHeight() / 2);
	Coor cell_center = getCellCenter(cell_index), lower_left = getCellLL(cell_index);

	bin_lib.moveCell(cell_index, target_coor); // bin update
	bin_lib2.moveCell(cell_index, target_coor);
	moveSubrowCellLL(cell_index, target_coor_ll);

	moveCellWithNoBinUpdate(cell_index, target_coor);

	return;
}

void CellLibrary::moveCellWithNoBinUpdate(const int index, Coor destination_coor)
{
	Cell &curr_cell(design.cell_lib[index]);
	Coor original_center = curr_cell.getCenter();
	Coor original_pin;

	/* ********************************* */
	curr_cell.moveCellCenter(destination_coor);
	/* ********************************* */

	for (int i = 0; i < curr_cell.pins.size(); i++){
		original_pin.x = original_center.x + curr_cell.pins[i].x_offset;
		original_pin.y = original_center.y + curr_cell.pins[i].y_offset;

		design.net_lib[curr_cell.pins[i].net_number].moveNetPin(index, original_pin, curr_cell.getPinCoor(i));
	}
}
/* ************************************************** */
size_t CellLibrary::size(void)
{
	return cell_list.size();
}
/* ************************************************** */
void CellLibrary::reserve (size_t n)
{
	cell_list.reserve(n);
}
/* ************************************************** */
void CellLibrary::push_back(const Cell c)
{
	cell_list.push_back(c);
}
/* ************************************************** */
vector<Cell>::iterator CellLibrary::begin(void)
{
	return cell_list.begin();
}
/* ************************************************** */
vector<Cell>::iterator CellLibrary::end(void)
{
	return cell_list.end();
}
/* ************************************************** */
vector<Cell>::const_iterator CellLibrary::begin(void) const
{
	return cell_list.begin();
}
/* ************************************************** */
vector<Cell>::const_iterator CellLibrary::end(void) const
{
	return cell_list.end();
}
