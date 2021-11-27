#pragma once

#include "basic_types.h"
#include "swap_target.h"
#include "region.h"
#include <algorithm>

using namespace std;

bool tryToSwapCellIntoThisRegion(Region r, int cell_index, int threshold, bool swap_with_best_only = false);
/* ************************************************** */
bool tryToSwapCellIntoBinSet(vector<int> &bin_collection, int cell_index, int threshold, bool swap_with_best_only = false, bool multi_thread = false);
/* ************************************************** */
void getTargetFromBinSet(vector<int> &bin_collection, int cell_index, int threshold, bool swap_with_best_only, SwapTarget &result, float &best_profit);
/* ************************************************** */
void getTargetFromRegion(Region r, int cell_index, int threshold, bool swap_with_best_only, SwapTarget &result, float &best_profit);
/* ************************************************** */
void getBestSwapTarget(int bin_index, int cell_index, SwapTarget &result, float &best_profit);
/* ************************************************** */
void findOptimalRegion(int index, Region &r);
/* ************************************************** */
coor_type profitFromSwap(int cell_index, SwapTarget swap_target);
/* ************************************************** */
void permanentlySwapEntities(int cell_index, SwapTarget &swap_target);
/* ************************************************** */
void permanentlySwapEntities_ll(int cell_index, SwapTarget &swap_target);
/* ************************************************** */
void SortSwapTargets(vector<SwapTarget> &swap_targets);
/* ************************************************** */
coor_type computeOverlapPenalty(int cell_index, Coor target_coor);
/* ************************************************** */
coor_type computeOverlapPenalty(int cell_index, SwapTarget &target);
/* ************************************************** */
coor_type profitFromSwapApproximation(int cell_index, SwapTarget swap_target);
/* ************************************************** */
bool tryToSwapToMaxRegion(int cell_index);
/* ************************************************** */
void nearSwap(void);
/* ************************************************** */
void undenseSwap();
/* ************************************************** */
double getDoubleEdgedDensityProfit(int cell_id, SwapTarget swap_target);
