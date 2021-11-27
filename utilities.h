#pragma once

#include "basic_types.h"
#include "swap_target.h"
#include "design_info.h"
#include <map>

using namespace std;

void calculatePenalty();

double calculateFastPenalty(map<int, double> &m);

/* ************************************************** */
/*                  Cell Shortcuts                    */
/* ************************************************** */
inline bool isNI(int cell_index)
{
	return design.cell_lib[cell_index].getNI();
}

inline bool isTerminal(int cell_index)
{
	return design.cell_lib[cell_index].getTerminal();
}

inline coor_type getCellHalfWidth(int index)
{
	return design.cell_lib[index].getWidth() / 2;
}

inline coor_type getCellLeftBorder(int index)
{
	return design.cell_lib[index].getCenter().x - getCellHalfWidth(index);
}

inline coor_type getCellRightBorder(int index)
{
	return design.cell_lib[index].getCenter().x + getCellHalfWidth(index);
}

inline Coor getCellCenter(int index)
{
	return design.cell_lib[index].getCenter();
}

inline Coor getCellOriginalCenter(int index)
{
	return design.cell_lib[index].getOriginalCenter();
}

inline coor_type getCellWidth(int index)
{
	return design.cell_lib[index].getWidth();
}

inline coor_type getCellHeight(int index)
{
	return design.cell_lib[index].getHeight();
}

inline coor_type getCellHalfHeight(int index)
{
	return design.cell_lib[index].getHeight() / 2;
}

inline int getCellRow(int cell_index);

size_t getRow(coor_type p_y);

bool isInThisRow(int cell_index, int row_number);


/* ************************************************** */
/*                       HPWL                         */
/* ************************************************** */
double findTotalHPWL(void);

void printTotalHPWL(void);

coor_type findTotalHPWLofConnectedNets(int index);

coor_type findTotalHPWLofConnectedNets(int index, vector<int> &nets_to_ignore);

coor_type hpwlChangeAfterMovingCellToCoor(int cell_index, Coor target_coor);

coor_type hpwlChangeAfterMovingCellToCoor(int cell_index, Coor target_coor, vector<int> &nets_to_ignore);

coor_type hpwlChangeAfterSwappingTwoCells(int cell1_index, int cell2_index);

/* ************************************************** */
/*                  Optimal Region                    */
/* ************************************************** */
void findOptimalRegion(int cell_index, Region &r);

void findBetterRegion(int cell_index, Region &r);

int getManhattanDistanceToRegion(Coor c, Region r);

coor_type getDistanceToOptimalRegion(int cell_index);

coor_type getDistanceBetweenPoints(Coor p1, Coor p2);

Region overlapOfTwoRegion(Region& r1, Region& r2);

void findMaximumHandsomeRegion(int cell_index, Region &r);
/* ************************************************** */
/*                    Cell Tools                      */
/* ************************************************** */
bool compareCellsYX(int i1, int i2);

bool compareCellsX(int i1, int i2);

bool compareCellsLX(int i1, int i2);

void findNetsInCommon(int cell1_index, int cell2_index, vector<int> &common_nets);

coor_type getRowCenter(int row_number);

Coor getCellLL(int cell_index);

void getCellsInGivenRow(int row_number, vector<int> &cell_pool, vector<int> &result);

void fillRowVectors(int top_row, int bot_row, vector<int> &cell_pool, vector<vector<int> > &result);


/* ************************************************** */
/*              SwapTarget Manipulations              */
/* ************************************************** */
void fillVectorWithSwapTargetsInThisBin(int bin_index, vector<SwapTarget> &swap_targets);

void fillVectorWithSwapTargetsInThisRegion(Region &r, vector<SwapTarget> &swap_targets);

void fillVectorWithSwapTargetsInThisBinLeftSort(int bin_index, vector<SwapTarget> &swap_targets);

struct FartherstSortStruc
{
public:
	Coor point;
	FartherstSortStruc(Coor p) : point(p) {};


	bool operator() (int i, int j )
	{
		return getManhattanDistanceToRegion(point, bin_lib[i].getRegion()) > getManhattanDistanceToRegion(point, bin_lib[j].getRegion());
	}
};

float distanceBetween(int cell_index, SwapTarget swap_target);

bool sortByWhitespaceWidth(SwapTarget a, SwapTarget b);

bool areIndependent(int *cells, SwapTarget *targets, float *profit, int cell_num);
/*
inline coor_type getSwapTargetIfIsCellandGetWidth(SwapTarget st)
{
	return getCellWidth(st.cell_index) + min(st.left_ww, st.right_ww) * 2;
}*/
/* ************************************************** */
/*                      Density                       */
/* ************************************************** */
bool densitySorter(int i1, int i2);

int getDensityPercentile(int cell_index);

float getSwapTargetDensity(SwapTarget swap_target);

/* ************************************************** */
/*                      Timing                        */
/* ************************************************** */

int timeInUs(void);

void resetTimers(void);

void displayTimers(void);

void resetTotalTimers(void);

void displayTotalTimers(void);

float timeInSec(int t);

bool isCellsDisjoint(int cell1_index, int cell2_index, Region &r1, Region &r2);

/* Multithreading */

void enterReader(void);

void exitReader(void);
