#include "main.h"
#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>
#include <map>
#include <omp.h>
using namespace std;

extern time_t t_findTotalHPWLofConnectedNets, t_findNetsInCommon, t_fillVectorWithSwapTargetsInThisBin,
			t_hpwlChangeAfterMovingCellToCoor, t_hpwlChangeAfterSwappingTwoCells, t_findOptimalRegion;
extern int optimal_region_pump;

/* ************************************************** */
/*                 Cell shortcuts                     */

/* ************************************************** */
int getCellRow(int cell_index)
{
	coor_type row_height;
	row_height = design.Row_Info[0].getHeight();
	return getRow(getCellLL(cell_index).y);
}
/* ************************************************** */
size_t getRow(coor_type p_y)
{
    return (size_t)((p_y - design.PMapFrame.getBoundary_ll().y) / bin_lib.getRowHeight());
} // getRow

bool isInThisRow(int cell_index, int row_number)
{
	coor_type row_center;
	coor_type cell_top, cell_bot;

	row_center = getRowCenter(row_number);
	cell_top = getCellCenter(cell_index).y + getCellHalfHeight(cell_index);
	cell_bot = getCellCenter(cell_index).y - getCellHalfHeight(cell_index);

	return (row_center < cell_top) && (row_center > cell_bot);
}

/* ************************************************** */
/*                       HPWL                         */
/* ************************************************** */
double findTotalHPWL(void)
{
	double result = 0;
	for (int i = 0; i < design.net_lib.size(); i++){
		result += design.net_lib[i].getHPWL();
	}

	return result;
}
/* ************************************************** */
void printTotalHPWL(void)
{
#ifdef DEBUG
	cout << "Total HPWL: " << (int) findTotalHPWL() << endl;
#endif
}
/* ************************************************** */
coor_type findTotalHPWLofConnectedNets(int cell_index)
{
	coor_type result = 0;
	int curr_net_index;

	for (int i = 0; i < design.cell_lib[cell_index].getPinNum(); i++){
		Cell &curr_cell(design.cell_lib[cell_index]);
		Net_Pl &curr_net(design.net_lib[curr_cell.getNet(i)]);

		if (curr_net.isOneCellNet() == false){
			result += curr_net.getHPWL();
		}
	}

	return result;
}
/* ************************************************** */
coor_type findTotalHPWLofConnectedNets(int cell_index, vector<int> &nets_to_ignore)
{
	coor_type result = 0;
	int curr_net_index;
	bool skip;

	for (int i = 0; i < design.cell_lib[cell_index].getPinNum(); i++){
		Cell &curr_cell(design.cell_lib[cell_index]);
		//Cell curr_cell = design.cell_lib[cell_index];
		Net_Pl &curr_net(design.net_lib[curr_cell.getNet(i)]);

		skip = false;
		for (int j = 0; j < nets_to_ignore.size(); j++){
			if (nets_to_ignore[j] == curr_cell.getNet(i)){
				skip = true;
			}
		}
		if (!skip && curr_net.isOneCellNet() == false){
			curr_net_index = design.cell_lib[cell_index].getNet(i);
			result += design.net_lib[curr_net_index].getHPWL();
		}
	}
	return result;
}
/* ************************************************** */
coor_type hpwlChangeAfterMovingCellToCoor(int cell_index, Coor target_coor)
{
	coor_type hpwl_before_swap = findTotalHPWLofConnectedNets(cell_index);
	Cell &curr_cell(design.cell_lib[cell_index]);
	int curr_net;
	coor_type left, right, top, bot;
	Coor pin_position_after_move;
	coor_type hpwl_after_swap = 0;


	for (int i = 0; i < curr_cell.getPinNum(); i++){
		PinInfo &curr_pin(curr_cell.getPin(i));
		Net_Pl &curr_net(design.net_lib[curr_pin.net_number]);

		if (curr_net.isOneCellNet() == false){
			pin_position_after_move.x = target_coor.x + curr_pin.x_offset;
			pin_position_after_move.y = target_coor.y + curr_pin.y_offset;

			curr_net.getFourLimits(left, right, top, bot, cell_index);//ignores cell_index cell
			if (right < pin_position_after_move.x){
				right = pin_position_after_move.x;
			}
			if (left > pin_position_after_move.x){
				left = pin_position_after_move.x;
			}
			if (top < pin_position_after_move.y){
				top = pin_position_after_move.y;
			}
			if (bot > pin_position_after_move.y){
				bot = pin_position_after_move.y;
			}

			hpwl_after_swap += max((right - left), coor_type(0)) + max((top - bot), coor_type(0));
		}
	}
	return hpwl_after_swap - hpwl_before_swap;
}
/* ************************************************** */
coor_type hpwlChangeAfterMovingCellToCoor(int cell_index, Coor target_coor, vector<int> &nets_to_ignore)
{
	coor_type hpwl_before_swap = findTotalHPWLofConnectedNets(cell_index, nets_to_ignore);
	Cell &curr_cell(design.cell_lib[cell_index]);
	int curr_net;
	coor_type left, right, top, bot;
	Coor pin_position_after_move;
	coor_type hpwl_after_swap = 0;
	bool skip;

	for (int i = 0; i < curr_cell.getPinNum(); i++){
		PinInfo &curr_pin(curr_cell.getPin(i));
		Net_Pl &curr_net(design.net_lib[curr_pin.net_number]);

		skip = false;
		for (int j = 0; j < nets_to_ignore.size(); j++){
			if (curr_pin.net_number == nets_to_ignore[j]){
				skip = true;;
			}
		}
		if (!skip && curr_net.isOneCellNet() == false){
			pin_position_after_move.x = target_coor.x + curr_pin.x_offset;
			pin_position_after_move.y = target_coor.y + curr_pin.y_offset;

			curr_net.getFourLimits(left, right, top, bot, cell_index);
			if (right < pin_position_after_move.x){
				right = pin_position_after_move.x;
			}
			if (left > pin_position_after_move.x){
				left = pin_position_after_move.x;
			}
			if (top < pin_position_after_move.y){
				top = pin_position_after_move.y;
			}
			if (bot > pin_position_after_move.y){
				bot = pin_position_after_move.y;
			}

			hpwl_after_swap += (right - left) + (top - bot);
		}
	}
	return hpwl_after_swap - hpwl_before_swap;
}
/* ************************************************** */
coor_type hpwlChangeAfterSwappingTwoCells(int cell1_index, int cell2_index)
{
	coor_type result = 0;

	vector<int> common_nets;
	findNetsInCommon(cell1_index, cell2_index, common_nets);
	result += hpwlChangeAfterMovingCellToCoor(cell1_index, getCellCenter(cell2_index), common_nets);
	result += hpwlChangeAfterMovingCellToCoor(cell2_index, getCellCenter(cell1_index), common_nets);
	return result;
}
/* ************************************************** */
/*                  Optimal Region                    */
/* ************************************************** */
void findOptimalRegion(int cell_index, Region &r)
{
	size_t number_of_nets = design.cell_lib[cell_index].getPinNum();
	size_t curr_net_index;
	coor_type curr_net_left_limit, curr_net_right_limit, curr_net_top_limit, curr_net_bot_limit;
	vector<coor_type> x_boundaries, y_boundaries;
	Coor pin_to_ignore;
	int n;

	if (number_of_nets == 0){
		r.left_limit = 0;
		r.right_limit = 0;
		r.top_limit = 0;
		r.bot_limit = 0;
#ifdef DEBUG
		cout << "Warning: pinless cell" << endl;
#endif
		return;
	}

	x_boundaries.reserve(number_of_nets * 2);
	y_boundaries.reserve(number_of_nets * 2);

	for (int i = 0; i < design.cell_lib[cell_index].getPinNum(); i++){
		curr_net_index = design.cell_lib[cell_index].getNet(i);
		if (design.net_lib[curr_net_index].isOneCellNet() == false){
			pin_to_ignore = design.cell_lib[cell_index].getPinCoor(i);
			design.net_lib[curr_net_index].getFourLimits(curr_net_left_limit, curr_net_right_limit, curr_net_top_limit, curr_net_bot_limit, cell_index);
			x_boundaries.push_back(curr_net_left_limit - design.cell_lib[cell_index].getPinXOffset(i));
			x_boundaries.push_back(curr_net_right_limit - design.cell_lib[cell_index].getPinXOffset(i));
			y_boundaries.push_back(curr_net_top_limit - design.cell_lib[cell_index].getPinYOffset(i));
			y_boundaries.push_back(curr_net_bot_limit - design.cell_lib[cell_index].getPinYOffset(i));
		}
		else{
			number_of_nets--;
		}
	}

	if (number_of_nets == 0){
		r.left_limit = 0;
		r.right_limit = 0;
		r.top_limit = 0;
		r.bot_limit = 0;
#ifdef DEBUG
		//cout << "Warning: selfish cell" << endl;
#endif
		return;
	}
	//even number, so two medians
	//find the first median:
	n = number_of_nets - 1;
	nth_element(x_boundaries.begin(), x_boundaries.begin() + n, x_boundaries.end()); //rearanges elements so that n'th biggest element is in the n'th position
	r.left_limit = max(x_boundaries[n] - optimal_region_pump, design.PMapFrame.getBoundary_ll().x);
	r.left_limit = min(r.left_limit, design.PMapFrame.getBoundary_ur().x);
	//find the second median:
	n++;
	nth_element(x_boundaries.begin(), x_boundaries.begin() + n, x_boundaries.end());
	r.right_limit = min(design.PMapFrame.getBoundary_ur().x, x_boundaries[n] + optimal_region_pump);
	r.right_limit = max(r.right_limit, design.PMapFrame.getBoundary_ll().x);
	//repeat for y:
	n = number_of_nets - 1;
	nth_element(y_boundaries.begin(), y_boundaries.begin() + n, y_boundaries.end()); //rearanges elements so that n'th biggest element is in the n'th position
	r.bot_limit = max(y_boundaries[n] - optimal_region_pump, design.PMapFrame.getBoundary_ll().y);
	r.bot_limit = min(design.PMapFrame.getBoundary_ur().y, r.bot_limit);

	n++;
	nth_element(y_boundaries.begin(), y_boundaries.begin() + n, y_boundaries.end());
	r.top_limit = min(design.PMapFrame.getBoundary_ur().y, y_boundaries[n] + optimal_region_pump);
	r.bot_limit = max(r.bot_limit, design.PMapFrame.getBoundary_ll().y);

#ifdef DEBUG
	if (r.top_limit < r.bot_limit || r.left_limit > r.right_limit){
		int a;
		cout << "Gan!" << endl;
		int b;
	}
#endif
}
/* ************************************************** */
void findBetterRegion(int cell_index, Region &r)
{
	size_t number_of_nets = design.cell_lib[cell_index].getPinNum();
	size_t curr_net_index;
	coor_type curr_net_left_limit, curr_net_right_limit, curr_net_top_limit, curr_net_bot_limit;
	vector<coor_type> x_boundaries, y_boundaries;
	Coor pin_to_ignore;
	int n;

	if (number_of_nets == 0){
		r.left_limit = 0;
		r.right_limit = 0;
		r.top_limit = 0;
		r.bot_limit = 0;
#ifdef DEBUG
		cout << "Warning: pinless cell" << endl;
#endif
		return;
	}

	x_boundaries.reserve(number_of_nets * 2);
	y_boundaries.reserve(number_of_nets * 2);

	for (int i = 0; i < design.cell_lib[cell_index].getPinNum(); i++){
		curr_net_index = design.cell_lib[cell_index].getNet(i);
		if (design.net_lib[curr_net_index].isOneCellNet() == false){
			pin_to_ignore = design.cell_lib[cell_index].getPinCoor(i);
			design.net_lib[curr_net_index].getFourLimits(curr_net_left_limit, curr_net_right_limit, curr_net_top_limit, curr_net_bot_limit);
			x_boundaries.push_back(curr_net_left_limit - design.cell_lib[cell_index].getPinXOffset(i));
			x_boundaries.push_back(curr_net_right_limit - design.cell_lib[cell_index].getPinXOffset(i));
			y_boundaries.push_back(curr_net_top_limit - design.cell_lib[cell_index].getPinYOffset(i));
			y_boundaries.push_back(curr_net_bot_limit - design.cell_lib[cell_index].getPinYOffset(i));
		}
		else{
			number_of_nets--;
		}
	}

	if (number_of_nets == 0){
		r.left_limit = 0;
		r.right_limit = 0;
		r.top_limit = 0;
		r.bot_limit = 0;
#ifdef DEBUG
		//cout << "Warning: selfish cell" << endl;
#endif
		return;
	}
	//even number, so two medians
	//find the first median:
	n = number_of_nets - 1;
	nth_element(x_boundaries.begin(), x_boundaries.begin() + n, x_boundaries.end()); //rearanges elements so that n'th biggest element is in the n'th position
	r.left_limit = max(x_boundaries[n] + 1, design.PMapFrame.getBoundary_ll().x);
	r.left_limit = min(r.left_limit, design.PMapFrame.getBoundary_ur().x);
	//find the second median:
	n++;
	nth_element(x_boundaries.begin(), x_boundaries.begin() + n, x_boundaries.end());
	r.right_limit = min(design.PMapFrame.getBoundary_ur().x, x_boundaries[n] - 1);
	r.right_limit = max(r.right_limit, design.PMapFrame.getBoundary_ll().x);
	//repeat for y:
	n = number_of_nets - 1;
	nth_element(y_boundaries.begin(), y_boundaries.begin() + n, y_boundaries.end()); //rearanges elements so that n'th biggest element is in the n'th position
	r.bot_limit = max(y_boundaries[n] + 1, design.PMapFrame.getBoundary_ll().y);
	r.bot_limit = min(design.PMapFrame.getBoundary_ur().y, r.bot_limit);

	n++;
	nth_element(y_boundaries.begin(), y_boundaries.begin() + n, y_boundaries.end());
	r.top_limit = min(design.PMapFrame.getBoundary_ur().y, y_boundaries[n] - 1);
	r.bot_limit = max(r.bot_limit, design.PMapFrame.getBoundary_ll().y);

#ifdef DEBUG
	if (r.top_limit < r.bot_limit || r.left_limit > r.right_limit){
		int a;
		cout << "Gan!" << endl;
		int b;
	}
#endif
}
/* ************************************************** */
coor_type getDistanceToOptimalRegion(int cell_index)
{
	Region optimal_region;
	findOptimalRegion(cell_index, optimal_region);
	return getManhattanDistanceToRegion(design.cell_lib[cell_index].getCenter(), optimal_region);
}
/* ************************************************** */
int getManhattanDistanceToRegion(Coor c, Region r)
{
	int distance = 0;

	if (c.y < r.bot_limit){
		distance += r.bot_limit - c.y;
	}
	if (c.y > r.top_limit){
		distance += c.y - r.top_limit;
	}

	if (c.x < r.left_limit){
		distance += r.left_limit - c.x;
	}
	if (c.x > r.right_limit){
		distance += c.x - r.right_limit;
	}

	return distance;
}
/* ************************************************** */
coor_type cabs(coor_type v)
{
    if(v < 0)
        return -v;
    return v;
}

coor_type getDistanceBetweenPoints(Coor p1, Coor p2)
{
	coor_type distance = 0;

	distance += cabs(p1.x - p2.x);
	distance += cabs(p1.y - p2.y);

	return distance;
}

Region overlapOfTwoRegion(Region& r1, Region& r2)
{
	Region overlap_r;
	bool sergiy_go_home = false;

	// right limit
	if(r1.left_limit < r2.right_limit)
	{
		overlap_r.right_limit = min(r1.right_limit, r2.right_limit);
	}
	else
	{
		sergiy_go_home = true;
	}

	// left limit
	if(r1.right_limit > r2.left_limit)
	{
		overlap_r.left_limit = max(r1.left_limit, r2.left_limit);
	}
	else
	{
		sergiy_go_home = true;
	}

	// top limit
	if(r1.bot_limit < r2.top_limit)
	{
		overlap_r.top_limit = min(r1.top_limit, r2.top_limit);
	}
	else
	{
		sergiy_go_home = true;
	}

	// bot limit
	if(r1.top_limit > r2.bot_limit)
	{
		overlap_r.bot_limit = max(r1.bot_limit, r2.bot_limit);
	}
	else
	{
		sergiy_go_home = true;
	}


	if(sergiy_go_home == true)
	{
		cout << "sergiy go home yay!" << endl;
	}

	return overlap_r;
} // Region overlapOfTwoRegion(Region& r1, Region& r2)

void findMaximumHandsomeRegion(int cell_index, Region &r)
{
	Coor cell_center = getCellCenter(cell_index);
	Region max_region;
	Region opt_region;
	coor_type width = min(150, design.displacement);

	max_region.left_limit = cell_center.x - width;
	max_region.right_limit = cell_center.x + width;
	max_region.bot_limit = cell_center.y - width;
	max_region.top_limit = cell_center.y + width;

	findBetterRegion(cell_index, opt_region);

	r = overlapOfTwoRegion(max_region, opt_region);

	return;
} // void findMaximumHandsomeRegion(int cell_index, Region &r)
/* ************************************************** */
/*                    Cell tools                      */
/* ************************************************** */
bool isTwoRegionOverlap(Region &r1, Region &r2)
{
	coor_type left, right, bot, top;
	left = max(r1.left_limit, r2.left_limit);
	right = min(r1.right_limit, r2.right_limit);
	bot = max(r1.bot_limit, r2.bot_limit);
	top = min(r1.top_limit, r2.top_limit);
	return ((left < right) && (bot < top));
	/*vector<int> bins1, bins2;
	bin_lib.fillVectorWithBinsInRegion(r1, bins1);
	bin_lib.fillVectorWithBinsInRegion(r2, bins2);
	for(int i = 0; i < bins1.size(); i++)
		for(int j = 0; j < bins2.size(); j++)
			if(bins1[i] == bins2[j])
				return true;
	return false;*/
}
bool isNetsDisjoint(int cell1_index, int cell2_index)
{
	int last = 0;
	int cell2_net_number = design.cell_lib[cell2_index].getPinNum();
	Cell &cell1(design.cell_lib[cell1_index]);
	Cell &cell2(design.cell_lib[cell2_index]);
	for (int i = 0; i < cell1.getPinNum(); i++){
		while(last < cell2_net_number && cell2.getNet(last) < cell1.getNet(i)){
			last++;
		}
		if (last == cell2.getPinNum()){
			break;
		}
		if (cell2.getNet(last) == cell1.getNet(i)){
			return false;
		}
	}
	return true;
}

bool isCellsDisjoint(int cell1_index, int cell2_index, Region &r1, Region &r2)
{
	if(!isTwoRegionOverlap(r1, r2))
		return true;
	return false;
}

void findNetsInCommon(int cell1_index, int cell2_index, vector<int> &common_nets)
{
	int last = 0;
	int cell2_net_number = design.cell_lib[cell2_index].getPinNum();
	Cell &cell1(design.cell_lib[cell1_index]);
	Cell &cell2(design.cell_lib[cell2_index]);

	for (int i = 0; i < cell1.getPinNum(); i++){
		while(last < cell2_net_number && cell2.getNet(last) < cell1.getNet(i)){
			last++;
		}
		if (last == cell2.getPinNum()){
			break;
		}
		if (cell2.getNet(last) == cell1.getNet(i)){
			common_nets.push_back(cell2.getNet(last));
		}
	}
}
/* ************************************************** */
bool compareCellsYX(int i1, int i2)
{
	Coor center1, center2;
	center1 = design.cell_lib[i1].getCenter();
	center2 = design.cell_lib[i2].getCenter();

	if (center1.y < center2.y){
		return true;
	}
	else if (center1.y > center2.y){
		return false;
	}
	else if (center1.x < center2.x){
		return true;
	}
	else{
		return false;
	}
}
/* ************************************************** */
bool compareCellsX(int i1, int i2)
{
	Coor center1, center2;
	center1 = design.cell_lib[i1].getCenter();
	center2 = design.cell_lib[i2].getCenter();

	if (center1.x < center2.x){
		return true;
	}
	else{
		return false;
	}
}

/* ************************************************** */
bool compareCellsLX(int i1, int i2)
{
	coor_type left1, left2;
	left1 = getCellLeftBorder(i1);
	left2 = getCellLeftBorder(i2);

	if (left1 < left2){
		return true;
	}
	else{
		return false;
	}
}
/* ************************************************** */
coor_type getRowCenter(int row_number)
{
	coor_type row_height;
	row_height = design.Row_Info[row_number].getHeight();
	return design.Row_Info[row_number].getCoor() + row_height / 2;
}
/* ************************************************** */
Coor getCellLL(int cell_index)
{
	Coor c;
	c.x = design.cell_lib[cell_index].getCenter().x - design.cell_lib[cell_index].getWidth() / 2;
	c.y = design.cell_lib[cell_index].getCenter().y - design.cell_lib[cell_index].getHeight() / 2;
	return c;
}
/* ************************************************** */
void getCellsInGivenRow(int row_number, vector<int> &cell_pool, vector<int> &result)
{
	for (int i = 0; i < cell_pool.size(); i++){
		if (isInThisRow(cell_pool[i], row_number)){
			result.push_back(cell_pool[i]);
		}
	}
}

void fillRowVectors(int top_row, int bot_row, vector<int> &cell_pool, vector<vector<int> > &result)
{
	for (int i = 0; i < cell_pool.size(); i++){
		for (int curr_row = bot_row; curr_row <= top_row; curr_row++){
			if (isInThisRow(cell_pool[i], curr_row)){
				result[curr_row - bot_row].push_back(cell_pool[i]);
			}
		}
	}
}
/* ************************************************** */
/*             SwapTarget manipulations               */
/* ************************************************** */
void fillVectorWithSwapTargetsInThisBin(int bin_index, vector<SwapTarget> &swap_targets)
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

//	enterReader();
//	cout << "fillTargets needs to be modified!!" << endl;
	cell_set = set<int>(bin_lib[bin_index].cellsInside);
	//exitReader();

	all_cells = vector<int>(cell_set.begin(), cell_set.end());
	bin_region = bin_lib[bin_index].getRegion();
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
/*
void fillVectorWithSwapTargetsInThisBin(int bin_index, vector<SwapTarget> &swap_targets)
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

	cell_set = set<int>(bin_lib[bin_index].cellsInside);

	all_cells = vector<int>(cell_set.begin(), cell_set.end());
	bin_region = bin_lib[bin_index].getRegion();
	bin_region.getTopAndBottomRows(top_row, bot_row);

	rightest.resize(all_cells.size());
	leftest.resize(all_cells.size());

	new_target.is_cell = false;
	new_target.is_whitespace = true;

	cells_in_row.resize(top_row - bot_row + 1);

	//new
	vector<SwapTarget> targets_for_cells_in_row;

	//calculate whitespaces

	fillRowVectors(top_row, bot_row, all_cells, cells_in_row); //fills cells_in_row[i] with cells in the i'th row of the bin

	for (int i = 0; i < top_row - bot_row + 1; i++){
		curr_row = bot_row + i; //i is the row count from 0 to n, where n is the number of rows in a bin.
								//curr_row is the design row count.

		if (cells_in_row[i].size() == 0){ //empty row case
			new_target.whitespace_center_left = Coor(bin_region.left_limit, getRowCenter(curr_row));
			new_target.whitespace_width = bin_region.right_limit - bin_region.left_limit;
			new_target.center.x = new_target.whitespace_center_left.x + new_target.whitespace_width / 2;
			new_target.center.y = getRowCenter(curr_row);

			swap_targets.push_back(new_target);
		}
		else{//nonempty row case
			sort(cells_in_row[i].begin(), cells_in_row[i].end(), compareCellsX);//sort by centerX

			//new
			targets_for_cells_in_row.resize(cells_in_row[i].size());
			for (int j = 0; j < cells_in_row[i].size(); j++){
				targets_for_cells_in_row[j].is_cell = true;
				targets_for_cells_in_row[j].is_whitespace = false;
				targets_for_cells_in_row[j].cell_index = cells_in_row[i][j];
			}

			//the next part computes rightests and leftests.
			//it can be done in a better way but, well, we have no time

			//rightest[i] is the rightest cell border up to cell i in the curr row
			prev_rightest = bin_region.left_limit;
			for (int curr_cell = 0;  curr_cell < cells_in_row[i].size(); curr_cell++){
				rightest[curr_cell] = max(prev_rightest, getCellRightBorder(cells_in_row[i][curr_cell]));
				prev_rightest = rightest[curr_cell];
			}

			//see rightest
			prev_leftest = bin_region.right_limit;
			for (int curr_cell = cells_in_row[i].size() - 1; curr_cell >= 0; curr_cell--){
				leftest[curr_cell] = min(prev_leftest, getCellLeftBorder(cells_in_row[i][curr_cell]));
				prev_leftest = leftest[curr_cell];
			}

			//computing whitespaces
			prev_rightest = bin_region.left_limit;
			for (int curr_cell = 0; curr_cell < cells_in_row[i].size(); curr_cell++){
				if (prev_rightest < leftest[curr_cell]){
					new_target.whitespace_center_left = Coor(prev_rightest, getRowCenter(curr_row));
					new_target.whitespace_width =  leftest[curr_cell] - prev_rightest;
					new_target.center.x = new_target.whitespace_center_left.x + new_target.whitespace_width / 2;
					new_target.center.y = getRowCenter(curr_row);

					swap_targets.push_back(new_target);

					//assumes that SwapTarget has left_ww and right_ww elements added (ww == whitespace width)
					targets_for_cells_in_row[curr_cell].left_ww = new_target.whitespace_width;
					if (curr_cell != 0){
						targets_for_cells_in_row[curr_cell - 1].right_ww = new_target.whitespace_width;
					}
				}
				else{
					targets_for_cells_in_row[curr_cell].left_ww = 0;
					if (curr_cell != 0){
						targets_for_cells_in_row[curr_cell - 1].right_ww = 0;
					}
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

				targets_for_cells_in_row[last_cell_in_curr_row].right_ww = new_target.whitespace_width;

			}
			else{
				targets_for_cells_in_row[last_cell_in_curr_row].right_ww = 0;
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
}*/



float distanceBetween(int cell_index, SwapTarget &swap_target)
{
	if (swap_target.is_whitespace){
		return getDistanceBetweenPoints(getCellCenter(cell_index), swap_target.center);
	}
	else{
		return getDistanceBetweenPoints(getCellCenter(cell_index), getCellCenter(swap_target.cell_index));
	}
}

bool sortByWhitespaceWidth(SwapTarget a, SwapTarget b)
{
	return a.whitespace_width < b.whitespace_width;
}

bool areIndependent(int *cells, SwapTarget *targets, float *profit, int cell_num)
{
	map<int, int> nets_used, bins_used;
	map<pair<int, int>, int>subrows_used;

	map<int, bool> curr_cell_nets;
	pair<int, int> curr_cell_bins;
	pair<int, int> curr_cell_subrow;
	pair<int, int> curr_target_bins;
	pair<int, int> curr_target_subrow;

	for (int i = 0; i < cell_num; i++){
		if (profit[i] > 0){
			//get used nets
			Cell &curr_cell(design.cell_lib[cells[i]]);
			for (int j = 0; j < curr_cell.getPinNum(); j++){
				curr_cell_nets[curr_cell.getNet(j)] = true;
			}
			for (map<int, bool>::iterator it = curr_cell_nets.begin(); it != curr_cell_nets.end(); it++){
				nets_used[it->first]++;
			}
			curr_cell_nets.clear();

			if (targets[i].is_cell){
				Cell &curr_cell(design.cell_lib[targets[i].cell_index]);
				for (int j = 0; j < curr_cell.getPinNum(); j++){
					curr_cell_nets[curr_cell.getNet(j)] = true;
				}
				for (map<int, bool>::iterator it = curr_cell_nets.begin(); it != curr_cell_nets.end(); it++){
					nets_used[it->first]++;
				}
				curr_cell_nets.clear();
			}

			//get used bins
			SwapTarget dummy;

			dummy.is_cell = false;
			dummy.is_whitespace = true;
			dummy.whitespace_width = getCellWidth(cells[i]);

			if (targets[i].is_cell){
				dummy.whitespace_width = max(dummy.whitespace_width, getCellWidth(targets[i].cell_index));
			}

			dummy.center = getCellCenter(cells[i]);
			dummy.whitespace_center_left.x = dummy.center.x - dummy.whitespace_width / 2;
			dummy.whitespace_center_left.y = dummy.center.y;
			curr_cell_bins = bin_lib.getBinOfThisSwapTarget(dummy);
			for (int j = curr_cell_bins.first; j <= curr_cell_bins.second; j++){
				bins_used[j]++;
			}


			if (targets[i].is_cell){
				dummy.center = getCellCenter(targets[i].cell_index);
			}
			else{
				dummy.center = targets[i].center;
			}

			dummy.whitespace_center_left.x = dummy.center.x - dummy.whitespace_width / 2;
			dummy.whitespace_center_left.y = dummy.center.y;

			curr_cell_bins = bin_lib.getBinOfThisSwapTarget(dummy);
			for (int j = curr_cell_bins.first; j <= curr_cell_bins.second; j++){
				bins_used[j]++;
			}

			//get used subrows
			/*curr_cell_subrow = getCoorRowSubrowIndex(getCellCenter(cells[i]));
			subrows_used[curr_cell_subrow]++;

			if (targets[i].is_cell){
				curr_target_subrow = getCoorRowSubrowIndex(getCellCenter(targets[i].cell_index));
			}
			else{
				curr_target_subrow = getCoorRowSubrowIndex(targets[i].center);
			}
			subrows_used[curr_cell_subrow]++;*/
		}
	}

	for (map<int, int>::iterator it = nets_used.begin(); it != nets_used.end(); it++){
		if (it->second > 1){
			//cout << "net in common!" << endl;
			return false;
		}
	}

	for (map<int, int>::iterator it = bins_used.begin(); it != bins_used.end(); it++){
		if (it->second > 1){
			//cout << "bin in common!" << endl;
			return false;
		}
	}

	/*for (map<pair<int, int>, int>::iterator it = subrows_used.begin(); it != subrows_used.end(); it++){
		if (it->second > 1){
			//cout << "subrow in common!" << endl;
			return false;
		}
	}*/
	//cout << "true" << endl;
	return true;


}

/* ************************************************** */
/*                      Density                       */
/* ************************************************** */
bool densitySorter(int i1, int i2)
{
	return bin_lib[i1].getDensity() < bin_lib[i2].getDensity();
}
/* ************************************************** */
int getDensityPercentile(int cell_index)
{
	float percentile = 0;
	float average_density = 0;
	int bin_number, curr_bin;
	pair<int, int> cell_bins;
	int right_bin, left_bin;

	cell_bins = bin_lib2.getBinOfThisCell(cell_index);

	left_bin = cell_bins.first;
	right_bin = cell_bins.second;

	bin_number = 1 + right_bin - left_bin;
	curr_bin = left_bin;
	for (int i = 0; i < bin_number; i++){
		average_density += bin_lib2[curr_bin].getDensity();
		curr_bin++;
	}
	average_density /= bin_number;


	return bin_lib2.densityTable[min((int)(average_density * 100), 100)] * 100 / bin_lib.size();
}
/* ************************************************** */
float getSwapTargetDensity(SwapTarget swap_target)
{
	float average_density, max_density = -1;
	int bin_number, right_bin, left_bin, curr_bin;
	pair<int, int>	cell_bins = bin_lib2.getBinOfThisSwapTarget(swap_target);

	left_bin = cell_bins.first;
	right_bin = cell_bins.second;

	bin_number = 1 + right_bin - left_bin;
	curr_bin = left_bin;
	average_density = 0;
	for (int i = 0; i < bin_number; i++){
		if (bin_lib2[curr_bin].getDensity() > max_density){
			max_density = bin_lib2[curr_bin].getDensity();
		}
		curr_bin++;
	}

	return max_density;
}

/* ************************************************** */
/*                      Timing                        */
/* ************************************************** */
time_t tTryToSwap, tComputeOverlap, tProfit, tSwap, tSort, tMoveCellWithUpdate, tFindHpwlConnectedNets;
time_t t_findTotalHPWLofConnectedNets, t_findNetsInCommon, t_fillVectorWithSwapTargetsInThisBin;
time_t t_hpwlChangeAfterMovingCellToCoor, t_hpwlChangeAfterSwappingTwoCells;
time_t tMoveCell, tGetFourLimits, tGetBestSwapTarget;
time_t tDensityProfit, t_findOptimalRegion, tSwapIntoBinSet;
time_t tBin;
time_t tSubrowUpdate;
time_t tGlobalSwap, tLegalization, tNearSwap;
time_t tGlobalSwap_total, tLegalization_total, tNearSwap_total;
time_t t_doit1, t_doit2, t_doit1_wait, t_doit2_wait;
time_t tGlobalCellMoving;
/* ************************************************** */
int timeInUs(void)
{
	struct timeval  tv;
	gettimeofday(&tv, NULL);

	return (tv.tv_sec) * 1000 * 1000 + (tv.tv_usec);
	return 0;
}
/* ************************************************** */
void resetTimers(void)
{
	tTryToSwap = tProfit = tSwap = tMoveCell = tGetFourLimits = tMoveCellWithUpdate = 0;
	t_findTotalHPWLofConnectedNets = t_findNetsInCommon= t_fillVectorWithSwapTargetsInThisBin = 0;
	t_hpwlChangeAfterMovingCellToCoor = t_hpwlChangeAfterSwappingTwoCells = tDensityProfit = 0;
	t_findOptimalRegion = tGetBestSwapTarget = tSwapIntoBinSet = 0;
	tGlobalCellMoving = 0;
	tSubrowUpdate = 0;
	tGlobalSwap = 0;
	t_doit1 = t_doit2 = t_doit1_wait = t_doit2_wait = 0;
}
/* ************************************************** *//* ************************************************** */
void resetTotalTimers(void)
{
	tGlobalSwap_total = 0;
	tLegalization_total = 0;
	tNearSwap_total = 0;
}

void displayTotalTimers(void)
{
	cout << "Total Global Swap " << timeInSec(tGlobalSwap_total) << " seconds" << endl;
	cout << "Total Near Swap " << timeInSec(tNearSwap_total) << " seconds" << endl;
	cout << "Total Legalization " << timeInSec(tLegalization_total) << " seconds" << endl;
}
/* ************************************************** */
void displayTimers(void)
{
#ifdef DEBUG
	cout << "Runime distribution:" << endl;
	cout << "TryToSwap " << timeInSec(tTryToSwap) << " seconds" << endl;
	cout << "ProfitFromSwap " << timeInSec(tProfit) << " seconds" << endl;
	cout << "t_findTotalHPWLofConnectedNets " << timeInSec(t_findTotalHPWLofConnectedNets) << " seconds" << endl;
	cout << "t_findNetsInCommon " << timeInSec(t_findNetsInCommon) << " seconds" << endl;
	cout << "t_fillVectorWithSwapTargetsInThisBin " << timeInSec(t_fillVectorWithSwapTargetsInThisBin) << " seconds" << endl;
	cout << "t_hpwlChangeAfterSwappingTwoCells " << timeInSec(t_hpwlChangeAfterSwappingTwoCells) << " seconds" << endl;
	cout << "t_hpwlChangeAfterMovingCellToCoor " << timeInSec(t_hpwlChangeAfterMovingCellToCoor) << " seconds" << endl;
	cout << "MoveCell without updating " << timeInSec(tMoveCell) << " seconds" << endl;
	cout << "MoveCell with updating " << timeInSec(tMoveCellWithUpdate) << " seconds" << endl;
	cout << "getFourLimits " << timeInSec(tGetFourLimits) << " seconds" << endl;
	cout << "Density Profit " << timeInSec(tDensityProfit) << "sec" << endl;
	cout << "Subrow updtate time " << timeInSec(tSubrowUpdate) << endl;
	cout << "Find OR time: " << timeInSec(t_findOptimalRegion) << " seconds"<< endl;
	cout << "getBestSwapTarget: " << timeInSec(tGetBestSwapTarget) << " seconds" << endl;
	cout << "tSwapIntoBinSet: " << timeInSec(tSwapIntoBinSet) << "seconds" << endl;
	cout << "thread1 work: " << timeInSec(t_doit1) << endl;
	cout << "thread2 work: " << timeInSec(t_doit2) << endl;
	cout << "Global Swap " << timeInSec(tGlobalSwap) << " seconds" << endl;

	cout << "tGlobalCellMoving: " << timeInSec(tGlobalCellMoving) << " seconds" << endl;
#endif
}
/* ************************************************** */
float timeInSec(int t)
{
	return float(t) / 1000000;
}

/* Multithreading */
#ifdef SERGIY_IS_NOT_BITCH
omp_lock_t mutex, wrt;
int readcount;

void enterReader(void)
{
	omp_set_lock(&mutex);
	readcount++;
	if (readcount == 1){
		omp_set_lock(&wrt);
	}
	omp_unset_lock(&mutex);
}
/* ************************************************** */
void exitReader(void)
{
	omp_set_lock(&mutex);
	readcount--;
	if (readcount == 0){
		omp_unset_lock(&wrt);
	}
	omp_unset_lock(&mutex);
}
#endif
