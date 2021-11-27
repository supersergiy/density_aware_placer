#include "main.h"
#include "input_statistics.h"
#include <fstream>
#include <iostream>
#include "net_pl.h"

using namespace std;

void displayCellStatistics(vector<Cell> &cells)
{
	ofstream output_file;
	double average_area = 0;
	double average_nonterminal_area = 0;
	int connected_to_small_nets_only = 0;
	int number_of_nonterminals = 0;
	int connected_to_big_nets_only = 0;
	int connected_to_2_net_and_to_smallest_net = 0;
	bool connected_to_smallest_net;
	bool small_net, big_net;

	output_file.open("cell_statistics.txt");

	for (int i = 0; i < cells.size(); i++){
		average_area += cells[i].getArea();
		if (cells[i].getTerminal() == false){
			number_of_nonterminals++;
			average_nonterminal_area += cells[i].getArea();
		}
		small_net = false;
		big_net = false;
		connected_to_smallest_net = false;

		for (int j = 0; j < cells[i].getPinNum(); j++){
			if (design.net_lib[cells[i].getNet(j)].getNetDegree() < 5){
				small_net = true;
				if (design.net_lib[cells[i].getNet(j)].getNetDegree() == 2){
					connected_to_smallest_net = true;
				}
			}
			else{
				small_net = false;
				big_net = true;
			}
		}

		if (connected_to_smallest_net && cells[i].getPinNum() == 2){
			connected_to_2_net_and_to_smallest_net++;
		}

		if (small_net && !big_net){
			connected_to_small_nets_only++;
		}
		if (big_net && !small_net){
			connected_to_big_nets_only++;
		}
	}
	average_area /= cells.size();
	average_nonterminal_area /= number_of_nonterminals;

	output_file << "Number of cells: " << cells.size() << endl;
	output_file << "Average cell area: " << average_area << endl;
	output_file << "Average nonterminal cell area: " << average_nonterminal_area << endl;
	output_file << "Number of cells connected to big nets only: " << connected_to_big_nets_only << endl;
	output_file << "Number of cells connected to small nets only: " << connected_to_small_nets_only << endl;
	output_file << "Number of 2-2 cells: " << connected_to_2_net_and_to_smallest_net << endl;

	cout << "Number of cells: " << cells.size() << endl;
	cout << "Average cell area: " << average_area << endl;
	cout << "Number of cells connected to big nets only: " << connected_to_big_nets_only << endl;
	cout << "Number of cells connected to small nets only: " << connected_to_small_nets_only << endl;
	cout << "Number of 2-2 cells: " << connected_to_2_net_and_to_smallest_net << endl;
}

void displayNetStatistics(void)
{
	double starting_HPWL = 0;
	double HPWL_from_small_nets = 0;
	double HPWL_from_degree_2_nets = 0;
	double HPWL_from_degree_3_nets = 0;
	double HPWL_from_degree_4_nets = 0;
	double average_HPWL_per_cell = 0;
	double HPWL_per_cell_for_degree_2_nets = 0;
	double HPWL_per_cell_for_degree_3_nets = 0;
	double HPWL_per_cell_for_degree_4_nets = 0;
	int total_number_of_nets = design.net_lib.size();
	int number_of_degree_2_nets = 0;
	int number_of_degree_3_nets = 0;
	int number_of_degree_4_nets = 0;
	ofstream output_file;
	Net_Pl *curr_net;

	output_file.open("net_statistics.txt");
	
	for (int i = 0; i < total_number_of_nets; i++){
		curr_net = &design.net_lib[i];
		starting_HPWL += curr_net->getHPWL();
		average_HPWL_per_cell += curr_net->getHPWL() / curr_net->getNetDegree();

		if (curr_net->getNetDegree() == 2){
			HPWL_from_degree_2_nets += curr_net->getHPWL();
			number_of_degree_2_nets++;
		}
		if (curr_net->getNetDegree() == 3){
			HPWL_from_degree_3_nets += curr_net->getHPWL();
			number_of_degree_3_nets++;
		}
		if (curr_net->getNetDegree() == 4){
			HPWL_from_degree_4_nets += curr_net->getHPWL();
			number_of_degree_4_nets++;
		}
	}

	average_HPWL_per_cell /= total_number_of_nets;
	HPWL_per_cell_for_degree_2_nets = HPWL_from_degree_2_nets / (number_of_degree_2_nets * 2);
	HPWL_per_cell_for_degree_3_nets = HPWL_from_degree_3_nets / (number_of_degree_3_nets * 3);
	HPWL_per_cell_for_degree_4_nets = HPWL_from_degree_4_nets / (number_of_degree_4_nets * 4);
	
	output_file << "Total number of nets: " << total_number_of_nets << endl;
	output_file << "Number of degree 2 nets: " << number_of_degree_2_nets << endl;
	output_file << "Number of degree 3 nets: " << number_of_degree_3_nets << endl;
	output_file << "Number of degree 4 nets: " << number_of_degree_4_nets << endl;
	output_file << endl;
	output_file << "Starting HPWL: " << starting_HPWL << endl;
	output_file << "HPWL from degree 2 nets: " << HPWL_from_degree_2_nets << endl;
	output_file << "HPWL from degree 3 nets: " << HPWL_from_degree_3_nets << endl;
	output_file << "HPWL from degree 4 nets: " << HPWL_from_degree_4_nets << endl;
	output_file << endl;
	output_file << "Average HPWL per cell for a net: " <<  average_HPWL_per_cell << endl;
	output_file << "HPWL per cell for degree 2 nets: " <<  HPWL_per_cell_for_degree_2_nets << endl;
	output_file << "HPWL per cell for degree 3 nets: " <<  HPWL_per_cell_for_degree_3_nets << endl;
	output_file << "HPWL per cell for degree 4 nets: " <<  HPWL_per_cell_for_degree_4_nets << endl;

	cout << "Total number of nets: " << total_number_of_nets << endl;
	cout << "Number of degree 2 nets: " << number_of_degree_2_nets << endl;
	cout << "Number of degree 3 nets: " << number_of_degree_3_nets << endl;
	cout << "Number of degree 4 nets: " << number_of_degree_4_nets << endl;
	cout << endl;
	cout << "Starting HPWL: " << starting_HPWL << endl;
	cout << "HPWL from degree 2 nets: " << HPWL_from_degree_2_nets << endl;
	cout << "HPWL from degree 3 nets: " << HPWL_from_degree_3_nets << endl;
	cout << "HPWL from degree 4 nets: " << HPWL_from_degree_4_nets << endl;
	cout << endl;
	cout << "Average HPWL per cell for a net: " <<  average_HPWL_per_cell << endl;
	cout << "HPWL per cell for degree 2 nets: " <<  HPWL_per_cell_for_degree_2_nets << endl;
	cout << "HPWL per cell for degree 3 nets: " <<  HPWL_per_cell_for_degree_3_nets << endl;
	cout << "HPWL per cell for degree 4 nets: " <<  HPWL_per_cell_for_degree_4_nets << endl;
}

void displayOptimalRegionStatistics(vector<Cell> &cells)
{
	double average_optimal_region_area = 0;
	double average_optimal_region_area_for_boundary = 0;
	double average_distance_to_optimal_region = 0;

	int point_optimal_region = 0;
	int number_of_boundary_cells = 0;
	int total_number_of_nonterminals = 0;
	int number_of_line_regions = 0;
	Region r;
	int huge_or = 0;
	ofstream output_file;
	output_file.open("optimal_region_statistics.txt");



	for (int i = 0; i < cells.size(); i++){
		if (cells[i].getTerminal() == false){
			findOptimalRegion(i, r);
			total_number_of_nonterminals++;

			if (cells[i].getCenter().y < r.bot_limit){
				average_distance_to_optimal_region += r.bot_limit - cells[i].getCenter().y;
			}
			if (cells[i].getCenter().y > r.top_limit){
				average_distance_to_optimal_region += cells[i].getCenter().y - r.top_limit;
			}

			if (cells[i].getCenter().x < r.left_limit){
				average_distance_to_optimal_region += r.left_limit - cells[i].getCenter().x;
			}
			if (cells[i].getCenter().x > r.right_limit){
				average_distance_to_optimal_region += cells[i].getCenter().x - r.right_limit;
			}

			

			if (r.top_limit == r.bot_limit && r.right_limit == r.left_limit){
				point_optimal_region++;

			}
			if ((r.top_limit - r.bot_limit) * (r.right_limit - r.left_limit) > 10000){
				huge_or++;
			}

			average_optimal_region_area += (r.top_limit - r.bot_limit) * (r.right_limit - r.left_limit);
			if (!r.isInThisRegion(i)){
				average_optimal_region_area_for_boundary += (r.top_limit - r.bot_limit) * (r.right_limit - r.left_limit);
				number_of_boundary_cells++;
			}
			else{
				if ((r.top_limit - r.bot_limit) * (r.right_limit - r.left_limit) == 0){
					number_of_line_regions++;
				}
			}
		}
		else{
			int terminal;
			terminal = true;
			r.left_limit = terminal;
		}
	}
	

	cout << number_of_line_regions << endl;
	cout << huge_or << endl;
	average_optimal_region_area_for_boundary /= number_of_boundary_cells;
	average_optimal_region_area /= total_number_of_nonterminals;
	average_distance_to_optimal_region /= number_of_boundary_cells;

	output_file << "Total number of cells: " << total_number_of_nonterminals << endl;
	output_file << "Number of boundary cells: " << number_of_boundary_cells << endl;
	output_file << "Average optimal region area: " << average_optimal_region_area << endl;
	output_file << "Average optimal region area for boundary cells: " << average_optimal_region_area_for_boundary << endl;
	output_file << "A point-like optimal region: " << point_optimal_region << endl;
	output_file << "Average distance to OR: " << average_distance_to_optimal_region << endl;

	cout << endl;
	cout << "Total number of cells: " << total_number_of_nonterminals << endl;
	cout << "Number of boundary cells: " << number_of_boundary_cells << endl;
	cout << "Average optimal region area: " << average_optimal_region_area << endl;
	cout << "Average optimal region area for boundary cells: " << average_optimal_region_area_for_boundary << endl;
	cout << "A point-like optimal region:" << point_optimal_region << endl;
	cout << "Average distance to OR: " << average_distance_to_optimal_region << endl;


}

void displayInputStatistics(void)
{
	//displayCellStatistics(design.cell_lib.cells;
	displayNetStatistics();
	//displayOptimalRegionStatistics();
}
