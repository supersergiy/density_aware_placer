#include "bin.h"
#include "bin_library.h"
#include "design_info.h"
#include "parameters.h"
#include "main.h"
#include "global_swap.h"
#include "utilities.h"
#define ABU2_WGT 10
#define ABU5_WGT  4
#define ABU10_WGT 2
#define ABU20_WGT 1

int num[4];
double avg2, avg5, avg10, avg20;
double fast_avg[4];
/*boundary*/
double handsome[4];

double abu2, abu5, abu10, abu20;
double penalty;

extern double hpwl;
extern int swap_cell_counter;


bool binDensityComparator(float a, float b) {
	return a > b;
}

inline int getPercentileRange(double density)
{
	if (density >= handsome[0]){
		return 0;
	}
	else if (density >= handsome[1]){
		return 1;
	}
	else if (density >= handsome[2]){
		return 2;
	}
	else if (density >= handsome[3]){
		return 3;
	}
	else{
		return -1;
	}
}

inline int getPercentileRange(int percentile)
{
	if (percentile < 2){
		return 0;
	}
	else if (percentile < 5){
		return 1;
	}
	else if (percentile < 10){
		return 2;
	}
	else if (percentile < 20){
		return 3;
	}
	else{
		return -1;
	}
}

void calculateABUGamma() {

	vector<double> v;

	for (int i = 0; i < bin_lib2.size(); i++){
		if(bin_lib2[i].getRegion().left_limit < bin_lib2[i].getRegion().right_limit &&
				bin_lib2[i].getRegion().bot_limit < bin_lib2[i].getRegion().top_limit)
			v.push_back(bin_lib2[i].getDensity());
	}

	sort(v.begin(), v.end(), binDensityComparator);

	num[0] = bin_lib2.size() * ( 2.0 / 100 );
	num[1] = bin_lib2.size() * ( 5.0 / 100 );
	num[2] = bin_lib2.size() * ( 10.0 / 100 );
	num[3] = bin_lib2.size() * ( 20.0 / 100 );

	avg2 = avg5 = avg10 = avg20 = 0;
	for(int i = 0; i < bin_lib2.size() && i < num[3]; i++) {

		/*if(v[i].isTotallyInsideMacroBlock()) {
			n++;
			diff++;
			continue;
		}*/
		if(i < num[0]) {
			avg2 += v[i];
			handsome[0] = v[i];
		}
		if(i < num[1]) {
			avg5 += v[i];
			handsome[1] = v[i];
		}
		if(i < num[2]) {
			avg10 += v[i];
			handsome[2] = v[i];
		}
		if(i < num[3]) {
			avg20 += v[i];
			handsome[3] = v[i];
		}
	}
	avg2 /= num[0];
	avg5 /= num[1];
	avg10 /= num[2];
	avg20 /= num[3];

	fast_avg[0] = avg2;
	fast_avg[1] = avg5;
	fast_avg[2] = avg10;
	fast_avg[3] = avg20;
}

void calculatePenalty() {
	calculateABUGamma();

	abu2 = max(avg2 / design.target_util - 1.0, 0.0);
	abu5 = max(avg5 / design.target_util - 1.0, 0.0);
	abu10 = max(avg10 / design.target_util - 1.0, 0.0);
	abu20 = max(avg20 / design.target_util - 1.0, 0.0);

	penalty = (ABU2_WGT*abu2+ABU5_WGT*abu5+ABU10_WGT*abu10+ABU20_WGT*abu20)/(double)(ABU2_WGT+ABU5_WGT+ABU10_WGT+ABU20_WGT);

}

double calculateFastPenalty(map<int, double> &m) {

//#pragma omp critical
//{
//	swap_cell_counter++;
//	if(swap_cell_counter >= 5000) {
//		calculatePenalty();
//		swap_cell_counter = 0;
////		hpwl = findTotalHPWL();
//	}
//}

	int curr_bin;
	double density_before, density_after;
	int percentile_before, percentile_after;
	double local_fast_avg[4];

	local_fast_avg[0] = fast_avg[0];
	local_fast_avg[1] = fast_avg[1];
	local_fast_avg[2] = fast_avg[2];
	local_fast_avg[3] = fast_avg[3];

	for(map<int, double>::iterator it = m.begin(); it != m.end(); ++it) {
		curr_bin = it->first;
		density_before = bin_lib2[curr_bin].getDensity();
		density_after = bin_lib2[curr_bin].getNewDensity(it->second);

		int range_before = getPercentileRange(density_before);
		int range_after = getPercentileRange(density_after);

//		percentile_before = bin_lib2.densityTable[min((int)(density_before * 100), 100)] * 100 / bin_lib2.size();
//		percentile_after = bin_lib2.densityTable[min((int)(density_after * 100), 100)] * 100 / bin_lib2.size();
//
//		int range_before = getPercentileRange(percentile_before);
//		int range_after = getPercentileRange(percentile_after);


		if(range_before != -1 && range_before == range_after) {
			local_fast_avg[range_before] = (local_fast_avg[range_before] * num[range_before] - (density_before - density_after))
											/ num[range_before];
		}
		else {
			if(range_before != -1) {
				local_fast_avg[range_before] = (local_fast_avg[range_before] * num[range_before] + handsome[range_before] - density_before)
											/ num[range_before];
			}
			if(range_after != -1) {
				local_fast_avg[range_after] = (local_fast_avg[range_after] * num[range_after] - handsome[range_after] + density_after)
											/ num[range_after];
			}
		}
	}

	double fast_abu2, fast_abu5, fast_abu10, fast_abu20;
	fast_abu2 = max(local_fast_avg[0] / design.target_util - 1.0, 0.0);
	fast_abu5 = max(local_fast_avg[1]/ design.target_util - 1.0, 0.0);
	fast_abu10 = max(local_fast_avg[2] / design.target_util - 1.0, 0.0);
	fast_abu20 = max(local_fast_avg[3] / design.target_util - 1.0, 0.0);

	return (ABU2_WGT*fast_abu2+ABU5_WGT*fast_abu5+ABU10_WGT*fast_abu10+ABU20_WGT*fast_abu20)/(double)(ABU2_WGT+ABU5_WGT+ABU10_WGT+ABU20_WGT);

}

double calculateScaledHPWL(double hpwl) {

	calculatePenalty();

	return hpwl * (1 + penalty);
}

coor_type calculateMaximumDisplacement(void)
{
	coor_type result = 0;
	int out_of_bound = 0;

	for (int i = 0; i < design.original_number_of_cells; i++){
		Cell &curr_cell(design.cell_lib[i]);
		if (getDistanceBetweenPoints(curr_cell.getCenter(), curr_cell.getOriginalCenter()) > design.displacement * 0.98){
			out_of_bound++;
		}
		if (!curr_cell.getTerminal() && getDistanceBetweenPoints(curr_cell.getCenter(), curr_cell.getOriginalCenter()) > result){
			result = getDistanceBetweenPoints(curr_cell.getCenter(), curr_cell.getOriginalCenter());
		}
	}
	cout << "out of bound: " << out_of_bound << endl;
	return result;
}

void checkCellInSubrow(void)
{
	for (int i = 0; i < design.Row_Info.size(); i++){
		for (int j = 0; j < design.Row_Info[i].getSubRowNum(); j++){
			SubRow &curr_subrow = design.Row_Info[i].getSubRow(j);
			for (int k = 0; k < curr_subrow.getCellNum(); k++){
				if (getCellCenter(curr_subrow.getCell(k)).x < curr_subrow.get_lx() ||
						getCellCenter(curr_subrow.getCell(k)).x > curr_subrow.get_rx()){
					cout << "kaka de dong zi dong zi dong zi tiao zhen tiao zhen tiao zhen jiao wo jie jie!" << endl;
				}
			}
		}
	}
}

void fixingHPWLBug(void)
{
	int upto10k = 0, upto100k = 0, upto200k = 0, total = 0;
	int curr_net_wl;
	int limit[100];
	int ammount[100];

	for (int i = 0; i < 100; i++){
		limit[i] = 10 * i;
		ammount[i] = 0;
	}

	for (int i = 0; i < design.net_lib.size(); i++){
		curr_net_wl = int (design.net_lib[i].getHPWL() + 0.5);
		total += curr_net_wl;
		if (i < 30 && i >= 20){
			cout << i << ": " << curr_net_wl << " / " << design.net_lib[i].getHPWL() << endl;;
		}
		for (int j = 0; j < 100; j++){
			if (i < limit[j]){
				ammount[j] += curr_net_wl;
			}
		}
	}
	cout << "total: " << total << endl;
	for (int i = 0; i < 100; i++){
		//cout << i << ": " << ammount[i] << endl;
	}

}

extern int min_dist;

void fixingProfit(void)
{
	int cells_not_in_or = 0;
	vector<int> cells_to_swap;
	cells_to_swap.reserve((design.cell_lib.size()));

	for (int i = 0; i < design.cell_lib.size(); i++){
		if (design.cell_lib[i].getTerminal() == false){
			cells_to_swap.push_back(i);
		}
	}
	SwapTarget target1, target2;
	coor_type before, after;
	for (int i = 0; i < cells_to_swap.size(); i++) {
		int curr_cell;
		Region curr_optimal_region;
		curr_cell = cells_to_swap[i];
		findOptimalRegion(curr_cell, curr_optimal_region);

		if (getManhattanDistanceToRegion(getCellCenter(curr_cell), curr_optimal_region) > min_dist &&
			getManhattanDistanceToRegion(getCellCenter(curr_cell), curr_optimal_region) <= design.displacement
			){
			cells_not_in_or++;
			before = calculateScaledHPWL(findTotalHPWL());
//			if (i == 171){
//				cout << "pause" << endl;
				//cout << calculateScaledHPWL(findTotalHPWL()) << endl;
//			}
			bool result = tryToSwapCellIntoThisRegion(curr_optimal_region, curr_cell, GLOBAL_THRESHOLD, false);
//			if (i == 171){
			before = calculateScaledHPWL(findTotalHPWL());
//
			if (result && before < after){
				cout << "bad swap: " << i << endl;
			}
		}
	}
}
