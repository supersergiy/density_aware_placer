#include "region.h"
#include "swap_target.h"
#include "global_swap.h"
#include "design_info.h"
#include "utilities.h"
#include "bin_library.h"
#include "SubRow.h"
#include "dynamic_swap.h"
#include "input_statistics.h"
#include <iostream>
#include <time.h>
#include <cmath>

using namespace std;
/*
void dynamicSwap(void)
{
	vector<Region> good_region;
	vector<bool> allowed;
	vector<int> bins_in_good_region;
	bool job_done;
	int success = 0;
	int good_swap = 0, bad_swap = 0;
	float curr_target_profit, best_target_profit;
	int best_target;

	good_region.resize(design.cell_lib.size());
	allowed.resize(design.cell_lib.size());
	for (int i = 0; i < design.cell_lib.size(); i++){
		if (!isTerminal(i)){
			findBetterRegion(i, good_region[i]);
			allowed[i] = true;;
		}
		else{
			allowed[i] = false;
		}
	}

	for (int i = 0; i < design.cell_lib.size(); i++){
		if (i % 30000 == 0){
			cout << "=" << endl;
		}
		if (!isTerminal(i) && allowed[i]){// && getManhattanDistanceToRegion(getCellCenter(i), good_region[i]) != 0){
			Region i_optimal_region;
			findBetterRegion(i, i_optimal_region);
			job_done = false;
			best_target_profit = 0;
			bin_lib.fillVectorWithBinsInRegion(i_optimal_region, bins_in_good_region);
			for (int j = 0; j < bins_in_good_region.size() && job_done == false; j++){
				Bin &curr_bin(bin_lib[bins_in_good_region[j]]);
				Region it_optimal_region;
				for (set<int>::iterator it = curr_bin.cellsInside.begin(); it != curr_bin.cellsInside.end() && job_done == false; it++){
					if (!isTerminal(*it) && allowed[*it]){// && getManhattanDistanceToRegion(getCellCenter(*it), good_region[*it]) != 0){
						//findOptimalRegion(*it, it_optimal_region);
						if (getManhattanDistanceToRegion(getCellCenter(*it), i_optimal_region) == 0 &&
							getManhattanDistanceToRegion(getCellCenter(i), good_region[*it]) == 0){

							curr_target_profit = profitFromSwap(i, SwapTarget(*it));

							if (curr_target_profit > best_target_profit){
								best_target_profit = curr_target_profit;
								best_target = *it;
							}
							/*
							SwapTarget tmp(*it);
							int before = findTotalHPWL();
							permanentlySwapEntities(i, tmp);
							int after = findTotalHPWL();
							if (before < after){
								bad_swap++;
							}
							else{
								good_swap++;
							}
							success++;
							//allowed[i] = false;
							//allowed[*it] = false;
							job_done = true;
						}
					}
				}
			}
			if (best_target_profit > 0){
				SwapTarget tmp(best_target);
				permanentlySwapEntities(i, tmp);
				success++;
			}
			bins_in_good_region.clear();
		}
	}
	cout << "Success number: " << success << endl;
	cout << "Good swap: " << good_swap << endl;
	cout << "Bad swap: " << bad_swap << endl;
}
*/
