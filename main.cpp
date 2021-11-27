#include "main.h"

#include <ctime>
#include <iomanip>
#include <iostream>
#include <map>
#include <fstream>
#include <omp.h>

// For checking using

DesignInfo design;// input information library
BinLibrary bin_lib, bin_lib2;

using namespace std;

int mul;
int maximum_overlap;
int min_dist; // ???
int optimal_region_pump;
extern double hpwl;
extern double penalty;
extern double hpwl_density_threshold;
extern int swap_cell_counter;
float handsome_value;

vector<vector<bool> > success;
int _count;
vector<int> global_cells_to_swap;

#define COMPARE_FUNCTION compareCellPinNum

bool compareCellPinNum(int c1, int c2)
{
	/*
	if(design.cell_lib[c1].getPinNum() == design.cell_lib[c2].getPinNum())
	{
		return getSwapTargetDensity(SwapTarget(c1)) > getSwapTargetDensity(SwapTarget(c2));
	}
	*/
	return design.cell_lib[c1].getPinNum() > design.cell_lib[c2].getPinNum();
}

bool compareCellWidth(int c1, int c2)
{
	return design.cell_lib[c1].getWidth() > design.cell_lib[c2].getWidth();
}

bool compareCellDesity(int c1, int c2)
{
	return getSwapTargetDensity(SwapTarget(c1)) > getSwapTargetDensity(SwapTarget(c2));
}

int main(int argc, char *argv[])
{
	omp_set_num_threads(NUM_THREAD);
	DesignInfo blank;
    time_t fileI_start_ts = 0, fileI_end_ts = 0;
    time_t start_ts, end_ts;

    if(TIME_CHECK)
    {
        time(&start_ts);
        time(&fileI_start_ts);
    }

    if(argc != 9) {
    	cout << "Usage: ./cada040 -aux [aux file] -legal [legal file] -ut [target utilization] -max_disp [maximum displacement]" << endl;
    	exit(0);
    }

    for(int i = 0; i < argc; i++) {
		if(strcmp(argv[i], "-ut") == 0) {
			design.target_util = atof(argv[i + 1]);
		}
		else if(strcmp(argv[i], "-aux") == 0) {
			design.file_name = argv[i + 1];
			design.file_name = design.file_name.substr(0, design.file_name.size() - 4);
			//cout << design.file_name << endl;
		}
		else if(strcmp(argv[i], "-legal") == 0) {
			design.legal_file_name = argv[i + 1];
		}
		else if(strcmp(argv[i], "-max_disp") == 0 || strcmp(argv[i], "-max_displacement") == 0) {
			design.displacement = atoi(argv[i + 1]);
		}
	}

	// Read benchmark
	if(!readFile(design.file_name))
		return 1;

	if(TIME_CHECK){
		time(&fileI_end_ts);
	}

	initializeDesign();
/*
	int max_cell_width = 0;
	for(int cell_iter = 0; cell_iter < design.original_number_of_cells; cell_iter++)
	{
		if (design.cell_lib[cell_iter].getTerminal() == false){
			max_cell_width = max((int)getCellWidth(cell_iter), max_cell_width);
		}
	}
	cout << "max cell width: " << max_cell_width << endl;
*/
	maximum_overlap = design.displacement / 3000;
	optimal_region_pump = 0;
	mul = 0;
	min_dist = 0;
	int mul_value[10] = {0, 0, 5, 5, 5, 5, 5, 5, 5, 5};
	//double den_value[10] = {0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};
	//double den_value[10] = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};

//	int repetition_count;
	double improvement_value;

	if (design.displacement > 1000){
//		repetition_count = 5;
		improvement_value = 0.15;
		handsome_value = 12;
//		design.displacement -= 20;
	}
	else{
//		repetition_count = 4;
		improvement_value = 0.1;
		handsome_value = 10;
	}

#ifdef DEBUG
	resetTotalTimers();
#endif


	for (unsigned int i = 0; i < design.original_number_of_cells; i++){
		if (design.cell_lib[i].getTerminal() == false){
			global_cells_to_swap.push_back(i);
		}
	}

	swap_cell_counter = 0;

	double improvement = 100;
	double before, after = calculateScaledHPWL(findTotalHPWL());
	for(int i = 0; i < 8 && improvement > improvement_value; i++)
	{

		before = after;

		mul = mul_value[i];
		//hpwl_density_threshold = den_value[i];

//		sort(global_cells_to_swap.begin(), global_cells_to_swap.end(), compareCellPinNum);
		global_swap();
		if (maximum_overlap != 0){
			RowBase_legalization();
		}
		calculatePenalty();

//		sort(global_cells_to_swap.begin(), global_cells_to_swap.end(), compareCellPinNum);
		nearSwap();
		if (maximum_overlap != 0){
			RowBase_legalization();
		}
		calculatePenalty();

		if(design.displacement < 1000 && penalty != 0) {
			undenseSwap();
			if (maximum_overlap != 0){
				RowBase_legalization();
			}
			calculatePenalty();
		}
		if (maximum_overlap == 0){
			RowBase_legalization();
		}
		after = calculateScaledHPWL(findTotalHPWL());
		improvement = (before - after) * 100 / before;
//		cout << "Improvement: " << improvement << "%" << endl;
	}

	int ccc = 0;
	while((penalty != 0 || ccc == 0) && ccc < 3) {
		undenseSwap();
		RowBase_legalization();
//		cout << "last run HPWL: " << findTotalHPWL() << endl;
//		cout << "last run Scaled HPWL: " << calculateScaledHPWL(findTotalHPWL()) << endl;
//		cout << "last run Penalty: " << penalty << endl;
		ccc++;
		calculatePenalty();
	}

//	cout << "max disp: " << calculateMaximumDisplacement() << endl;
//	cout << "Final Scaled HPWL: " << calculateScaledHPWL(findTotalHPWL()) << endl;

	WritePlaceResult(design.file_name.substr(design.file_name.find_last_of("/\\") + 1), design);

	if(TIME_CHECK)
		time(&end_ts);

	if(TIME_CHECK){
		cout << "Read File for " << setw(5) << difftime(fileI_end_ts, fileI_start_ts) << " seconds" << endl;
		cout << "Total run time " << setw(5) << difftime(end_ts, start_ts) << " seconds" << endl;
	}
    return 0;
} // main
