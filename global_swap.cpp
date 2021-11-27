#include "region.h"
#include "swap_target.h"
#include "global_swap.h"
#include "design_info.h"
#include "utilities.h"
#include "bin_library.h"
#include "SubRow.h"
#include "input_statistics.h"
#include <iostream>
#include <time.h>
#include <cmath>
#include <omp.h>


using namespace std;


extern int mul;
#ifdef DEBUG
extern time_t tTryToSwap, tProfit, tSwap, tSort, tSwapIntoBinSet;
extern time_t tDensityProfit, tGlobalSwap, tGetBestSwapTarget;
extern time_t t_doit1, t_doit2, t_doit1_wait, t_doit2_wait;
extern time_t tGlobalSwap_total, tNearSwap_total;
#endif
extern int maximum_overlap;
extern int min_dist;
extern double penalty;
extern float handsome_value;
extern vector<int> global_cells_to_swap;

extern double hpwl;
int swapped_with_white_space;
int swapped_with_cell;

double hpwl_density_threshold;

time_t timing_test;

int curr_cell1, curr_cell2;

int swap_cell_counter;

void global_swap(void)
{

#ifdef DEBUG
	time_t start_ts = 0, end_ts = 0;
	start_ts = timeInUs();
	resetTimers();
#endif

	vector<int> &cells_to_swap(global_cells_to_swap);					//stores indexes of the cells to swap in the cel_lib
	SwapTarget possible_target;

    swapped_with_white_space = 0;
    swapped_with_cell = 0;

	unsigned int i = 0;

	int curr_cell[NUM_THREAD * CELL_PER_THREAD];
	SwapTarget best_target[NUM_THREAD * CELL_PER_THREAD];
	float best_profit[NUM_THREAD * CELL_PER_THREAD];
	Region curr_optimal_region[NUM_THREAD * CELL_PER_THREAD];
	int distance_to_optimal;
	while(i < cells_to_swap.size()) {
		for (int j = 0; j < NUM_THREAD * CELL_PER_THREAD; j++){
			curr_cell[j] = -1;
			best_profit[j] = -1;
		}

		for (int j = 0; j < NUM_THREAD * CELL_PER_THREAD; j++){
			curr_cell[j] = cells_to_swap[i];
			findOptimalRegion(curr_cell[j], curr_optimal_region[j]);
			distance_to_optimal = getManhattanDistanceToRegion(getCellCenter(curr_cell[j]), curr_optimal_region[j]);

			while(i < cells_to_swap.size() && distance_to_optimal <= min_dist){
				i++;
				curr_cell[j] = cells_to_swap[i];
				findOptimalRegion(curr_cell[j], curr_optimal_region[j]);
				distance_to_optimal = getManhattanDistanceToRegion(getCellCenter(curr_cell[j]), curr_optimal_region[j]);
			}

			if (i >= cells_to_swap.size()){
				curr_cell[j] = -1;
				break;
			}
			i++;
		}

#pragma omp parallel for num_threads(NUM_THREAD)/* private(swap_cell_counter)*/
		for(int j = 0; j < NUM_THREAD; j++) {
			for (int k = 0; k < CELL_PER_THREAD; k++){
				if (curr_cell[CELL_PER_THREAD * j + k] != -1){
					getTargetFromRegion(curr_optimal_region[CELL_PER_THREAD * j + k], curr_cell[CELL_PER_THREAD * j + k],
							GLOBAL_THRESHOLD, false, best_target[CELL_PER_THREAD * j + k], best_profit[CELL_PER_THREAD * j + k]);
				}
			}
		}
		for (int j = 0; j < CELL_PER_THREAD * NUM_THREAD; j++){
			if (best_profit[j] > 0 && canBeSwaped(curr_cell[j], best_target[j]) && profitFromSwap(curr_cell[j], best_target[j]) > 0 &&
					bin_lib.computeOverlap(curr_cell[j], best_target[j]) <= maximum_overlap){
				permanentlySwapEntities(curr_cell[j], best_target[j]);
			}
		}
	}

	cout << "***************************************" << endl;
	cout << "5566 first blood!!!" << endl;

#ifdef DEBUG
	end_ts = timeInUs();
	tGlobalSwap = end_ts - start_ts;

	if (DISPLAY_SWAP_TARGET_STATISTICS){
		cout << "Cells not in optimal region: " << cells_not_in_or << endl;
		cout << "Cells successfully swapped:: " << cells_successfully_swapped << endl;
		cout << "Swapped with cell: " << swapped_with_cell << endl;
		cout << "Swapped with whitespace: " << swapped_with_white_space << endl;
		//cout << "Canceled due to multithread overlap: " << canceled << endl;
	}
	tGlobalSwap_total += tGlobalSwap;
//	cout << "independent: " << independent << endl;
//	cout << "dependent: " << dependent << endl;
	cout << "run time: " << timeInSec(tGlobalSwap) << endl;
#endif

	cout << "***************************************" << endl;

	if(TIME_CHECK){
		//displayTimers();
	}

	return;
 }// global_swap
/* ************************************************** */
bool tryToSwapCellIntoThisRegion(Region r, int cell_index, int threshold, bool swap_with_best_only)
{
	vector<int> bins_in_region;

	bin_lib.fillVectorWithBinsInRegion(r, bins_in_region, cell_index);

	bool result = tryToSwapCellIntoBinSet(bins_in_region, cell_index, threshold, swap_with_best_only, true);

	return result;
}
/* ************************************************** */

bool tryToSwapCellIntoBinSet(vector<int> &bin_collection, int cell_index, int threshold, bool swap_with_best_only, bool multi_thread)
{
	vector<SwapTarget> swap_targets_in_curr_bin;
	float best_target_profit, curr_target_profit;
	SwapTarget best_swap_target, curr_swap_target;
	best_target_profit = 0;

	if(multi_thread) {

		for (unsigned int i = 0; i < bin_collection.size(); i += 4){
			/*multi-thread*/

			SwapTarget curr_swap_target[4];
			float curr_target_profit[4];
			bool outOfBound[4] = {};

	#pragma omp parallel sections num_threads(NUM_THREAD)
			{
	#pragma omp section
				{
					getBestSwapTarget(bin_collection[i], cell_index, curr_swap_target[0], curr_target_profit[0]);
				}
	#pragma omp section
				{
					if(i + 1 < bin_collection.size())
					{
						getBestSwapTarget(bin_collection[i + 1], cell_index, curr_swap_target[1], curr_target_profit[1]);
					}
					else
					{
						outOfBound[1] = true;
					}
				}
	#pragma omp section
				{
					if(i + 2 < bin_collection.size())
					{
						getBestSwapTarget(bin_collection[i + 2], cell_index, curr_swap_target[2], curr_target_profit[2]);
					}
					else
					{
						outOfBound[2] = true;
					}
				}
	#pragma omp section
				{
					if(i + 3 < bin_collection.size())
					{
						getBestSwapTarget(bin_collection[i + 3], cell_index, curr_swap_target[3], curr_target_profit[3]);
					}
					else
					{
						outOfBound[3] = true;
					}
				}
			}
			for(int j = 0; j < 4; j++) {
				if (!outOfBound[j] && curr_target_profit[j] > best_target_profit){
					best_target_profit = curr_target_profit[j];
					best_swap_target = curr_swap_target[j];
				}
			}

			if (!swap_with_best_only && best_target_profit > threshold){
				if (best_swap_target.is_cell){
					swapped_with_cell++;
				}
				else{
					swapped_with_white_space++;
				}

				permanentlySwapEntities(cell_index, best_swap_target);
				return true;
			}
			swap_targets_in_curr_bin.clear();
		}
	}
	else {
		for (unsigned int i = 0; i < bin_collection.size(); i++){
			getBestSwapTarget(bin_collection[i], cell_index, curr_swap_target, curr_target_profit);
			if (curr_target_profit > best_target_profit){
				best_target_profit = curr_target_profit;
				best_swap_target = curr_swap_target;
			}
			if (!swap_with_best_only && best_target_profit > threshold){
				if (best_swap_target.is_cell){
					swapped_with_cell++;
				}
				else{
					swapped_with_white_space++;
				}
				if ((profitFromSwap(cell_index, best_swap_target) > 0)){
					permanentlySwapEntities(cell_index, best_swap_target);
				}
				return true;
			}
			swap_targets_in_curr_bin.clear();
		}
	}

	if (best_target_profit > 0){
		if (best_swap_target.is_cell){
			swapped_with_cell++;
		}
		else{
			swapped_with_white_space++;
		}
		if(canBeSwaped(cell_index, best_swap_target))
			permanentlySwapEntities(cell_index, best_swap_target);
		return true;
	}
	return false;
}
/* ************************************************** */
void getTargetFromRegion(Region r, int cell_index, int threshold, bool swap_with_best_only, SwapTarget &result, float &best_profit)
{
	vector<int> bins_in_region;

	bin_lib.fillVectorWithBinsInRegion(r, bins_in_region, cell_index);

	getTargetFromBinSet(bins_in_region, cell_index, threshold, swap_with_best_only, result, best_profit);
}
/* ************************************************** */
void getTargetFromBinSet(vector<int> &bin_collection, int cell_index, int threshold, bool swap_with_best_only, SwapTarget &result, float &best_profit)
{
	vector<SwapTarget> swap_targets_in_curr_bin;
	float best_target_profit = -1, curr_target_profit = -1;
	SwapTarget best_swap_target, curr_swap_target;

	for (unsigned int i = 0; i < bin_collection.size(); i++){
		getBestSwapTarget(bin_collection[i], cell_index, curr_swap_target, curr_target_profit);

		if (curr_target_profit > best_target_profit){
			best_target_profit = curr_target_profit;
			best_swap_target = curr_swap_target;
		}
		if (!swap_with_best_only && best_target_profit > threshold){
			result = best_swap_target;
			best_profit = best_target_profit;
			return;
		}
		swap_targets_in_curr_bin.clear();
	}

	if (best_target_profit > 0){
		result = best_swap_target;
		best_profit = best_target_profit;
		return;
	}
	else{
		best_profit = -1;
		return;
	}

}
void getBestSwapTarget(int bin_index, int cell_index, SwapTarget &result, float &best_profit)
{
	vector<SwapTarget> swap_targets_in_curr_bin;
	best_profit = 0;


	if (!bin_lib[bin_index].canBeUse){
		return;
	}

	Region curr_bin_region = bin_lib[bin_index].getRegion();
	if (getManhattanDistanceToRegion(design.cell_lib[cell_index].getOriginalCenter(),
									 curr_bin_region) >= design.displacement){
		return;
	}
	fillVectorWithSwapTargetsInThisBin(bin_index, swap_targets_in_curr_bin);
//	bin_lib[bin_index].fillVectorWithSwapTargetsInThisBin(swap_targets_in_curr_bin);

 	float curr_target_profit;
  	for (unsigned int j = 0; j < swap_targets_in_curr_bin.size(); j++){
		if (swap_targets_in_curr_bin[j].is_cell == false || swap_targets_in_curr_bin[j].cell_index != cell_index){//don't want to swap cell with itself
			curr_target_profit = profitFromSwap(cell_index, swap_targets_in_curr_bin[j]);

			if (curr_target_profit > best_profit){
				best_profit = curr_target_profit;
				result = swap_targets_in_curr_bin[j];
			}
		}
	}
}
/* ************************************************** */
coor_type profitFromSwap(int cell_index, SwapTarget swap_target)
{
	coor_type hpwl_change = 0;
	coor_type overlap_penalty = 0;
	coor_type density_profit = 0;

	double heuristic_value;
	pair<int, int> cell_bins;
	pair<int, int> target_bins;
	bool can_be_swaped;

	can_be_swaped = canBeSwaped(cell_index, swap_target);

	if (can_be_swaped == false){
		return -5566;
	}

	if (swap_target.is_cell){
		if (getDistanceBetweenPoints(design.cell_lib[cell_index].getOriginalCenter(), getCellCenter(swap_target.cell_index))
				> design.displacement){
				return -5566;
		}
		if (getDistanceBetweenPoints(design.cell_lib[cell_index].getCenter(), getCellOriginalCenter(swap_target.cell_index))
				> design.displacement){
				return -5566;
		}
		if (getSwapTargetDensity(SwapTarget(cell_index)) > design.target_util && getSwapTargetDensity(swap_target) < getSwapTargetDensity(SwapTarget(cell_index)) && getCellWidth(swap_target.cell_index) - getCellWidth(cell_index) > 0){
			return -5566;
		}
		if (getSwapTargetDensity(swap_target) > design.target_util && getSwapTargetDensity(SwapTarget(cell_index)) < getSwapTargetDensity(swap_target)  && getCellWidth(swap_target.cell_index) - getCellWidth(cell_index) < 0){
			return -5566;
		}


		overlap_penalty = abs(getCellWidth(cell_index) - getCellWidth(swap_target.cell_index));
//		overlap_penalty = max(getCellWidth(cell_index) - getSwapTargetIfIsCellandGetWidth(swap_target), 0.0f);
//		overlap_penalty += max(getCellWidth(swap_target.cell_index) - getCellWidth(cell_index), 0.0f);


		//SwapTarget aaa(cell_index);
		//overlap_penalty = bin_lib.computeOverlap(cell_index, swap_target) + bin_lib.computeOverlap(swap_target.cell_index, aaa);

		if (overlap_penalty > maximum_overlap){
			return -5566;
		}
		overlap_penalty = 0;

		hpwl_change = hpwlChangeAfterSwappingTwoCells(cell_index, swap_target.cell_index);


		//density_profit += getDensityProfit(SwapTarget(cell_index), getCellWidth(swap_target.cell_index) - getCellWidth(cell_index));
		//density_profit += getDensityProfit(swap_target, getCellWidth(cell_index) - getCellWidth(swap_target.cell_index));
	}
	else{
		if (getDistanceBetweenPoints(design.cell_lib[cell_index].getOriginalCenter(), swap_target.center)
						> design.displacement){
			return -5566;
		}
		if (getSwapTargetDensity(swap_target) > design.target_util){
			return -5566;
		}
		Coor tmp;
		overlap_penalty = computeOverlapPenalty(cell_index, swap_target);

		if (overlap_penalty > maximum_overlap){
			return -5566;
		}
		tmp.x = swap_target.whitespace_center_left.x + swap_target.whitespace_width / 2;
		tmp.y = swap_target.whitespace_center_left.y;
		hpwl_change = hpwlChangeAfterMovingCellToCoor(cell_index, tmp);

	}
	density_profit += getDoubleEdgedDensityProfit(cell_index, swap_target);

//	heuristic_value = (- hpwl_change) * (1 - hpwl_density_threshold) - overlap_penalty * mul + density_profit * hpwl_density_threshold;
	heuristic_value = (- hpwl_change) - overlap_penalty * mul + density_profit;



	return heuristic_value;
}
/* ************************************************** */
void permanentlySwapEntities(int cell_index, SwapTarget &swap_target)
{

	swap_cell_counter++;
	if(swap_cell_counter >= 500) {
		calculatePenalty();
//		hpwl = findTotalHPWL();
		swap_cell_counter = 0;
	}

	Coor tmp;
//	set<int> touched_nets;

//	for (int i = 0; i < design.cell_lib[cell_index].getPinNum(); i++){
//		touched_nets.insert(design.cell_lib[cell_index].getNet(i));
//	}

	if (swap_target.is_whitespace){
//		for (set<int>::iterator it = touched_nets.begin(); it != touched_nets.end(); ++it){
//			hpwl -= design.net_lib[*it].getHPWL();
//		}

		tmp.x = swap_target.whitespace_center_left.x + swap_target.whitespace_width / 2;
		tmp.y = swap_target.whitespace_center_left.y;
		design.cell_lib.moveCell(cell_index, tmp);
	}
	else{
		tmp = design.cell_lib[cell_index].getCenter();

//		for (int i = 0; i < design.cell_lib[swap_target.cell_index].getPinNum(); i++){
//			touched_nets.insert(design.cell_lib[swap_target.cell_index].getNet(i));
//		}
//
//		for (set<int>::iterator it = touched_nets.begin(); it != touched_nets.end(); it++){
//			hpwl -= design.net_lib[*it].getHPWL();
//		}

		design.cell_lib.moveCell(cell_index, design.cell_lib[swap_target.cell_index].getCenter());
		design.cell_lib.moveCell(swap_target.cell_index, tmp);

	}
//	for (set<int>::iterator it = touched_nets.begin(); it != touched_nets.end(); ++it){
//		hpwl += design.net_lib[*it].getHPWL();
//	}
}
/* ************************************************** */
coor_type computeOverlapPenalty(int cell_index, SwapTarget &swap_target)
{
	if (swap_target.is_whitespace == true){
		return max(coor_type(0.0), getCellWidth(cell_index) - swap_target.whitespace_width);
	}
	else{
		return abs(getCellWidth(cell_index) - getCellWidth(swap_target.cell_index));
	}
}
/* ************************************************** */

bool compareBinDesity(int b1, int b2)
{
	return bin_lib[b1].getDensity() < bin_lib[b1].getDensity();
}


void nearSwap(void)
{
	bool result;
	SwapTarget best_target;
	vector<int> neighbor_bins;
	pair<int, int>	cell_bins;

	vector<int> &cells_to_swap(global_cells_to_swap);			//stores indexes of the cells to swap in the cel_lib


#ifdef DEBUG
	int success = 0, fail = 0;
	time_t startTime, endTime;
	startTime = timeInUs();
#endif

	for (int i = 0; i < cells_to_swap.size(); i++) {
		bool handsome_bool;
		if(design.displacement < 1000) {
			handsome_bool = getDistanceToOptimalRegion(cells_to_swap[i]) >= max(getCellHalfWidth(cells_to_swap[i]), handsome_value);
		}
		else {
			handsome_bool = getDistanceToOptimalRegion(cells_to_swap[i]) >= max(getCellHalfWidth(cells_to_swap[i]), handsome_value);
		}
		if (design.cell_lib[cells_to_swap[i]].getTerminal() == false && handsome_bool){

			cell_bins = bin_lib.getBinOfThisSwapTarget(SwapTarget(cells_to_swap[i]));
			neighbor_bins.push_back(bin_lib.getLeftBin(cell_bins.first));
			neighbor_bins.push_back(bin_lib.getRightBin(cell_bins.first));
			neighbor_bins.push_back(bin_lib.getTopBin(cell_bins.first));
			neighbor_bins.push_back(bin_lib.getBotBin(cell_bins.first));


			neighbor_bins.push_back(bin_lib.getTopBin(bin_lib.getLeftBin(cell_bins.first)));
			neighbor_bins.push_back(bin_lib.getBotBin(bin_lib.getLeftBin(cell_bins.first)));
			neighbor_bins.push_back(bin_lib.getTopBin(bin_lib.getRightBin(cell_bins.first)));
			neighbor_bins.push_back(bin_lib.getBotBin(bin_lib.getRightBin(cell_bins.first)));

			neighbor_bins.push_back(bin_lib.getTopBin(bin_lib.getTopBin(cell_bins.first)));
			neighbor_bins.push_back(bin_lib.getBotBin(bin_lib.getBotBin(cell_bins.first)));
			neighbor_bins.push_back(bin_lib.getLeftBin(bin_lib.getLeftBin(cell_bins.first)));
			neighbor_bins.push_back(bin_lib.getRightBin(bin_lib.getRightBin(cell_bins.first)));

			neighbor_bins.push_back(cell_bins.first);


			sort(neighbor_bins.begin(), neighbor_bins.end(), compareBinDesity);

		 	result = tryToSwapCellIntoBinSet(neighbor_bins, cells_to_swap[i], 0 , false, true);

#ifdef DEBUG
			success += result;
			fail += !result;
#endif

			neighbor_bins.clear();

//			Region handsome_region;
//			SwapTarget best_target;
//			float best_profit;
//
//			findMaximumHandsomeRegion(i, handsome_region);
//			getTargetFromRegion(handsome_region, i, GLOBAL_THRESHOLD, false, best_target, best_profit);
//
//			if(best_profit > 0)
//			{
//				permanentlySwapEntities(i, best_target);
//			}
		}
	}

	cout << "***********************************************" << endl;
	cout << "426 move!!!" << endl;

#ifdef DEBUG
	cout << "cells successfully swapped: " << success << endl;
	cout << "cells failed: " << fail << endl;
	endTime = timeInUs();
	cout << "time: " << timeInSec(endTime - startTime) << endl;
	tNearSwap_total += endTime - startTime;
#endif
	cout << "***********************************************" << endl;
}

void undenseSwap(void)
{

	bool result;
	SwapTarget best_target;
	vector<int> neighbor_bins;
	pair<int, int>	cell_bins;

	vector<int> &cells_to_swap(global_cells_to_swap);				//stores indexes of the cells to swap in the cel_lib

#ifdef DEBUG
	int success = 0, fail = 0;
	time_t startTime, endTime;
	startTime = timeInUs();
#endif

	for (int i = 0; i < cells_to_swap.size(); i++) {
		if (design.cell_lib[cells_to_swap[i]].getTerminal() == false && getSwapTargetDensity(SwapTarget(cells_to_swap[i])) > design.target_util){

			cell_bins = bin_lib.getBinOfThisSwapTarget(SwapTarget(cells_to_swap[i]));
			neighbor_bins.push_back(bin_lib.getLeftBin(cell_bins.first));
			neighbor_bins.push_back(bin_lib.getRightBin(cell_bins.first));
			neighbor_bins.push_back(bin_lib.getTopBin(cell_bins.first));
			neighbor_bins.push_back(bin_lib.getBotBin(cell_bins.first));


			neighbor_bins.push_back(bin_lib.getTopBin(bin_lib.getLeftBin(cell_bins.first)));
			neighbor_bins.push_back(bin_lib.getBotBin(bin_lib.getLeftBin(cell_bins.first)));
			neighbor_bins.push_back(bin_lib.getTopBin(bin_lib.getRightBin(cell_bins.first)));
			neighbor_bins.push_back(bin_lib.getBotBin(bin_lib.getRightBin(cell_bins.first)));

			neighbor_bins.push_back(bin_lib.getTopBin(bin_lib.getTopBin(cell_bins.first)));
			neighbor_bins.push_back(bin_lib.getBotBin(bin_lib.getBotBin(cell_bins.first)));
			neighbor_bins.push_back(bin_lib.getRightBin(bin_lib.getRightBin(cell_bins.first)));
			neighbor_bins.push_back(bin_lib.getLeftBin(bin_lib.getLeftBin(cell_bins.first)));


			neighbor_bins.push_back(cell_bins.first);

			sort(neighbor_bins.begin(), neighbor_bins.end(), compareBinDesity);


			result = tryToSwapCellIntoBinSet(neighbor_bins, cells_to_swap[i], 0 , false, true);
#ifdef DEBUG
			success += result;
			fail += !result;
#endif

			neighbor_bins.clear();
		}
	}
	cout << "***********************************************" << endl;
	cout << "426 move -- undense" << endl;

#ifdef DEBUG
	cout << "cells successfully swapped: " << success << endl;
	cout << "cells failed: " << fail << endl;
	endTime = timeInUs();
	cout << "time: " << timeInSec(endTime - startTime) << endl;
	tNearSwap_total += endTime - startTime;
#endif
	cout << "***********************************************" << endl;
}

bool tryToSwapToMaxRegion(int cell_index)
{
	Region max_region;
	Cell &curr_cell(design.cell_lib[cell_index]);

	max_region.right_limit = curr_cell.getOriginalCenter().x + design.displacement;
	max_region.left_limit = curr_cell.getOriginalCenter().x - design.displacement;
	max_region.top_limit = curr_cell.getOriginalCenter().y + design.displacement;
	max_region.bot_limit = curr_cell.getOriginalCenter().y - design.displacement;

	return tryToSwapCellIntoThisRegion(max_region, cell_index, 0 / 5566, true);
}

double getDoubleEdgedDensityProfit(int cell_id, SwapTarget swap_target)
{
	/*float percentile_before1, percentile_after1, percentile_before2, percentile_after2;
	int multiplier;
	float density_before1, density_after1, density_before2, density_after2;
	coor_type size_difference;
	float max_density = -1;

	int bin_number, right_bin, left_bin, curr_bin, worst_bin;

	if (swap_target.is_whitespace){
		size_difference = getCellWidth(cell_id);
	}
	else{
		size_difference = getCellWidth(cell_id) - getCellWidth(swap_target.cell_index);
	}

	pair<int, int>	bins = bin_lib2.getBinOfThisSwapTarget(cell_id);

	size_difference *= getCellHeight(cell_id);

	left_bin = bins.first;
	right_bin = bins.second;

	bin_number = 1 + right_bin - left_bin;
	curr_bin = left_bin;
	max_density = -1;
	for (int i = 0; i < bin_number; i++){
		if (bin_lib2[curr_bin].getDensity() > max_density){
			worst_bin = curr_bin;
			max_density = bin_lib2[curr_bin].getDensity();
		}
		curr_bin++;
	}

	density_before1 = bin_lib2[worst_bin].getDensity() * 100;
	density_after1 = bin_lib2[worst_bin].getNewDensity(-size_difference) * 100;

	bins = bin_lib2.getBinOfThisSwapTarget(swap_target);

	left_bin = bins.first;
	right_bin = bins.second;

	bin_number = 1 + right_bin - left_bin;
	curr_bin = left_bin;
	max_density = -1;
	int worst_bin2;
	for (int i = 0; i < bin_number; i++){
		if (bin_lib2[curr_bin].getDensity() > max_density){
			worst_bin2 = curr_bin;
			max_density = bin_lib2[curr_bin].getDensity();
		}
		curr_bin++;
	}

	density_before2 = bin_lib2[worst_bin2].getDensity() * 100;
	density_after2 = bin_lib2[worst_bin2].getNewDensity(size_difference) * 100;

	if(worst_bin2 == worst_bin) {
		return -5566;
	}

	if (size_difference < 0){
		swap(density_before1, density_before2);
		swap(density_after1, density_after2);
		size_difference *= -1;
	}

	percentile_before1 = bin_lib2.densityTable[min((int)(density_before1), 100)] * 100 / bin_lib2.size();
	percentile_after1 = bin_lib2.densityTable[min((int)(density_after1), 100)] * 100 / bin_lib2.size();

	percentile_before2 = bin_lib2.densityTable[min((int)(density_before2), 100)] * 100 / bin_lib2.size();
	percentile_after2 = bin_lib2.densityTable[min((int)(density_after2), 100)] * 100 / bin_lib2.size();

	//zhuan
	if (density_before1 > design.target_util * 100 && density_after2 < design.target_util * 100){
		return (density_before1 - max(design.target_util * 100, density_after1)) * 1000;
	}
	//shu
	else if (density_before1 < design.target_util * 100 && density_before2 > density_before1 && density_after2 > design.target_util * 100){
		return (density_after2 - max(density_before2, design.target_util * 100)) * -1000;
	}
	else{
		return -556600;
	}*/
	map<int, double> size_dif;

	bin_lib2.densityChangeAfterSwap(cell_id, swap_target, size_dif);

	double penalty_after = calculateFastPenalty(size_dif);

	float scaled_diff = float((penalty - penalty_after) * hpwl);

	return scaled_diff;
}
