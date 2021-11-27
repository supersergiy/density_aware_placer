#include "main.h"
#include "basic_types.h"

bool Region::isInThisRegion(int cell_index)
{
	Coor curr_cell_center = design.cell_lib[cell_index].getCenter();
	
	if (curr_cell_center.y <= top_limit && 
		curr_cell_center.y >= bot_limit &&
		((getCellLeftBorder(cell_index) > left_limit && getCellLeftBorder(cell_index) < right_limit) ||
		 (getCellRightBorder(cell_index) > left_limit && getCellRightBorder(cell_index) < right_limit))){
			return true;
	}
	else{
		return false;
	}	
}
/* ************************************************** */
bool Region::isInThisRegion(Coor point)
{
	if (point.y <= top_limit && 
		point.y >= bot_limit &&
		point.x >= left_limit && point.x <= right_limit){
			 return true;
	}
	else{
		return false;
	}
}
/* ************************************************** */
bool Region::isInThisRegion(SwapTarget &s)
{
	if (s.is_cell){
#ifdef DEBUG
		cout << "entry: " << getCellCenter(s.cell_index).x << " " << getCellCenter(s.cell_index).y << endl;
#endif
		return (isInThisRegion(s.cell_index));
	}
	else{
		Coor whitespace_center;
		whitespace_center.x = s.whitespace_center_left.x + s.whitespace_width / 2;
		whitespace_center.y = s.whitespace_center_left.y;

		return (isInThisRegion(whitespace_center));
	}
}
/* ************************************************** */
bool Region::isTotallyInThisRegion(int cell_index)
{
	Coor curr_cell_center = design.cell_lib[cell_index].getCenter();

	if (curr_cell_center.y <= top_limit && //NOTE i don't add half cell height, while maybe I should
		curr_cell_center.y >= bot_limit &&
		getCellLeftBorder(cell_index) >= left_limit &&
		getCellRightBorder(cell_index) <= right_limit){
			return true;
	}
	else{
		return false;
	}	
}
/* ************************************************** */
void Region::getTopAndBottomRows(int &top_row, int &bot_row)
{
	coor_type row_height;
	row_height = design.Row_Info[0].getHeight();
	//bot_row = int(bot_limit / row_height);
	//top_row = int(top_limit / row_height);
	bot_row = getRow(bot_limit);
	top_row = getRow(top_limit - 0.1); //todo: !!!!!!!!!!!!!make beautiful!!!!!!!!!!!!!!!
}
/* ************************************************** */
ostream& operator<< (ostream &out, Region r)
{
	out << "Bot Left: (" << r.bot_limit << ", " << r.left_limit << ")" << endl;
	out << "Top Right: (" << r.top_limit << ", " << r.right_limit << ")" << endl;
    return out;
}
