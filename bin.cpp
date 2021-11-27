#include "main.h"
#include "region.h"

using namespace std;

extern float bin1_threshold, bin2_threshold;

void Bin::cellMoveOut(int cell_index)
{
	Cell &c = design.cell_lib[cell_index];
	coor_type left, right, bot, top;
	coor_type _left, _right, _bot, _top;

	left = c.getCenter().x - (c.getWidth() / 2), right = c.getCenter().x + (c.getWidth() / 2);
	bot = c.getCenter().y - (c.getHeight() / 2), top = c.getCenter().y + (c.getHeight() / 2);

	_left = max(left, region.left_limit);
	_right = min(right, region.right_limit);
	_bot = max(bot, region.bot_limit);
	_top = min(top, region.top_limit);

	if (_left > _right || _bot > _top) {
		//cout << "negative area2!" << endl;
		return;
	}



	if(store_cells) {
		cellsInside.erase(cell_index);
	}
	cell_area -= (_right - _left) * (_top - _bot);
	cell_area = max((int)cell_area, 0);

	if(free_area <= 0.2 * total_area)
		density = 0;
	/*else if(store_cells == false && total_area <= bin2_threshold)
		density = 0;
	else if(store_cells == true  && total_area <= bin1_threshold)
		density = 0;*/
	else
		density = cell_area / (free_area);
	return;
}
void Bin::cellMoveIn(int cell_index)
{
	Cell &c = design.cell_lib[cell_index];
	coor_type left, right, bot, top;
	coor_type _left, _right, _bot, _top;

	left = c.getCenter().x - (c.getWidth() / 2), right = c.getCenter().x + (c.getWidth() / 2);
	bot = c.getCenter().y - (c.getHeight() / 2), top = c.getCenter().y + (c.getHeight() / 2);

	_left = max(left, region.left_limit);
	_right = min(right, region.right_limit);
	_bot = max(bot, region.bot_limit);
	_top = min(top, region.top_limit);
	if (_left > _right || _bot > _top) {
		//cout << "Negative area!" << endl;
		return;
	}


	if(store_cells) {
		cellsInside.insert(cell_index);
	}

	if(c.getTerminal() && !c.getNI()) {
		//terminal_area += (_right - _left) * (_top - _bot);
		free_area -= (_right - _left) * (_top - _bot);
		free_area = max((double)free_area, 0.0);
	}
	else if(!c.getTerminal()){
		cell_area += (_right - _left) * (_top - _bot);
	}
	if(free_area <= 0.2 * total_area)
		density = 0;
	/*else if(store_cells == false && total_area <= bin2_threshold)
		density = 0;
	else if(store_cells == true  && total_area <= bin1_threshold)
		density = 0;*/
	else
		density = cell_area / (free_area);

	return;
}

void Bin::cellMoveIn(int cell_index, Coor target)
{
	Cell &c = design.cell_lib[cell_index];
	coor_type left, right, bot, top;
	coor_type _left, _right, _bot, _top;

	left = target.x - (c.getWidth() / 2), right = target.x + (c.getWidth() / 2);
	bot = target.y - (c.getHeight() / 2), top = target.y + (c.getHeight() / 2);


	_left = max(left, region.left_limit);
	_right = min(right, region.right_limit);
	_bot = max(bot, region.bot_limit);
	_top = min(top, region.top_limit);
	if (_left > _right || _bot > _top) {
		//cout << "fjdioasjf" << endl;
		return;
	}


	if(store_cells) {
		cellsInside.insert(cell_index);
	}

	if(free_area <= 0.2 * total_area){
		density = 0;
	}
	/*else if(store_cells == false && total_area <= bin2_threshold)
		density = 0;
	else if(store_cells == true  && total_area <= bin1_threshold)
		density = 0;*/
	else {
		cell_area += (_right - _left) * (_top - _bot);
		density = cell_area / (free_area);
	}

	return;
}

double Bin::overlapAreaWithThisCell(Cell &c) {
	coor_type left, right, bot, top;
	coor_type _left, _right, _bot, _top;

	left = c.getCenter().x - (c.getWidth() / 2), right = c.getCenter().x + (c.getWidth() / 2);
	bot = c.getCenter().y - (c.getHeight() / 2), top = c.getCenter().y + (c.getHeight() / 2);

	_left = max(left, region.left_limit);
	_right = min(right, region.right_limit);
	_bot = max(bot, region.bot_limit);
	_top = min(top, region.top_limit);

	return (_right - _left) * (_top - _bot);
}

double Bin::overlapAreaWithThisCell(int cell_index) {

	Cell &c = design.cell_lib[cell_index];

	coor_type left, right, bot, top;
	coor_type _left, _right, _bot, _top;

	left = c.getCenter().x - (c.getWidth() / 2), right = c.getCenter().x + (c.getWidth() / 2);
	bot = c.getCenter().y - (c.getHeight() / 2), top = c.getCenter().y + (c.getHeight() / 2);

	_left = max(left, region.left_limit);
	_right = min(right, region.right_limit);
	_bot = max(bot, region.bot_limit);
	_top = min(top, region.top_limit);

	if(_left <= _right && _bot <= _top)
		return (_right - _left) * (_top - _bot);
	return 0;
}


void Bin::fillVectorWithSwapTargetsInThisBin(vector<SwapTarget> &swap_targets)
{
	int top_row, bot_row, curr_row;
	Region bin_region;

	vector<coor_type> rightest, leftest;
	coor_type prev_leftest, prev_rightest;

	SwapTarget new_target;

	vector<vector<int> > cells_in_row;
	int last_cell_in_curr_row;

	set<int> cell_set;
	vector<int> all_cells;

	all_cells = vector<int>(cellsInside.begin(), cellsInside.end());


	bin_region = region;
	bin_region.getTopAndBottomRows(top_row, bot_row);

	rightest.resize(all_cells.size());
	leftest.resize(all_cells.size());

	new_target.is_cell = false;
	new_target.is_whitespace = true;

	cells_in_row.resize(top_row - bot_row + 1);


	//calculate whitespaces
	fillRowVectors(top_row, bot_row, all_cells, cells_in_row);

	for (int i = 0; i < top_row - bot_row + 1; i++){
		curr_row = bot_row + i;
		if (cells_in_row[i].size() == 0){
			new_target.whitespace_center_left = Coor(bin_region.left_limit, getRowCenter(curr_row));
			new_target.whitespace_width = bin_region.right_limit - bin_region.left_limit;
			new_target.center.x = new_target.whitespace_center_left.x + new_target.whitespace_width / 2;
			new_target.center.y = getRowCenter(curr_row);

			swap_targets.push_back(new_target);
		}
		else{
			sort(cells_in_row[i].begin(), cells_in_row[i].end(), compareCellsX);

			prev_rightest = bin_region.left_limit;
			for (int curr_cell = 0;  curr_cell < cells_in_row[i].size(); curr_cell++){
				rightest[curr_cell] = max(prev_rightest, getCellRightBorder(cells_in_row[i][curr_cell]));
				prev_rightest = rightest[curr_cell];
			}

			prev_leftest = bin_region.right_limit;
			for (int curr_cell = cells_in_row[i].size() - 1; curr_cell >= 0; curr_cell--){
				leftest[curr_cell] = min(prev_leftest, getCellLeftBorder(cells_in_row[i][curr_cell]));
				prev_leftest = leftest[curr_cell];
			}

			prev_rightest = bin_region.left_limit;
			for (int curr_cell = 0; curr_cell < cells_in_row[i].size(); curr_cell++){
				if (prev_rightest < leftest[curr_cell]){
					new_target.whitespace_center_left = Coor(prev_rightest, getRowCenter(curr_row));
					new_target.whitespace_width =  leftest[curr_cell] - prev_rightest;
					new_target.center.x = new_target.whitespace_center_left.x + new_target.whitespace_width / 2;
					new_target.center.y = getRowCenter(curr_row);

					swap_targets.push_back(new_target);
				}
				prev_rightest = max(prev_rightest, getCellRightBorder(cells_in_row[i][curr_cell]));
			}


			last_cell_in_curr_row = cells_in_row[i].size() - 1;
			if (rightest[last_cell_in_curr_row] < bin_region.right_limit){
				new_target.whitespace_center_left = Coor(rightest[last_cell_in_curr_row], getRowCenter(curr_row));
				new_target.whitespace_width = bin_region.right_limit - rightest[last_cell_in_curr_row];
				new_target.center.x = new_target.whitespace_center_left.x + new_target.whitespace_width / 2;
				new_target.center.y = getRowCenter(curr_row);

				swap_targets.push_back(new_target);
			}
		}
		//cells_in_curr_row.clear();
	}
	//all whitespaces found

	//0.5 sec
	for (int i = 0; i < all_cells.size(); i++){
		if (!design.cell_lib[all_cells[i]].getTerminal()){
			swap_targets.push_back(all_cells[i]);
		}
	}
	//all movable cells found
}

float Bin::overlapArea(int cell_index) {
	Cell &c = design.cell_lib[cell_index];
	coor_type left, right, bot, top;
	coor_type _left, _right, _bot, _top;
	left = c.getCenter().x - (c.getWidth() / 2), right = c.getCenter().x + (c.getWidth() / 2);
	bot = c.getCenter().y - (c.getHeight() / 2), top = c.getCenter().y + (c.getHeight() / 2);

	_left = max(left, region.left_limit);
	_right = min(right, region.right_limit);
	_bot = max(bot, region.bot_limit);
	_top = min(top, region.top_limit);

	if (_left > _right || _bot > _top) {
		return 0;
	}

	return (_right - _left) * (_top - _bot);
}

float Bin::getDensity() 
{ 
	return density;
}
Region Bin::getRegion() { return region; }
bool Bin::cellIsInThisBin(int cell_index) 
{ 
	return (cellsInside.find(cell_index) != cellsInside.end());
}
bool Bin::pointIsInThisBin(Coor c)
{
	if(c.x >= region.left_limit && c.x <= region.right_limit && c.y >= region.bot_limit && c.y <= region.top_limit)
		return true;
	return false;
}
bool Bin::isOverlapWithThisRegion(Region r)
{
	coor_type left, right, bot, top;
	left = max(r.left_limit, region.left_limit);
	right = min(r.right_limit, region.right_limit);
	bot = max(r.bot_limit, region.bot_limit);
	top = min(r.top_limit, region.top_limit);
	return ((left < right) && (bot < top));
}
