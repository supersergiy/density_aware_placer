#pragma once

#include "swap_target.h"
#include "basic_types.h"
#include "bin.h"
#include "parameters.h"
#include <map>

class BinLibrary
{
private:
	vector<Bin> binSet;
	coor_type height, width;
	coor_type rowHeight, siteWidth;
	coor_type binHeight, binWidth;
	int numBinX, numBinY;

	void insertCell(int cell_index);
	void insertCell(int cell_index, Coor target);
	coor_type getX(coor_type x);
	coor_type getY(coor_type y);
public:
	bool store_cells;
	vector<int> densityTable;
	vector<Bin>& getBinSet() {return binSet;}
	void buildBinLibrary(int bin_size, bool store, int bin_width = 0);
	void moveCell(int cell_index, Coor target);
	int getIndex(coor_type x, coor_type y);
	int getIndex(int row, int col);
	int getNumBinX() { return numBinX; }
	int getNumBinY() { return numBinY; }
	int getLeftBin(int bin){return (bin + 1) < size() ? (bin + 1) : bin;}
	int getRightBin(int bin){return (bin - 1) > 0 ? (bin - 1) : bin;}
	int getTopBin(int bin){return (bin + getNumBinX()) < size() ? (bin + getNumBinX()) : bin;}
	int getBotBin(int bin){return (bin - getNumBinX()) > 0 ? (bin - getNumBinX()) : bin;}

	coor_type getBinHeight() { return binHeight; }
	coor_type getBinWidth() {return binWidth; }
	coor_type getRowHeight() { return rowHeight; }
	coor_type getSiteWidth() { return siteWidth; }
	coor_type getDesignHeight() { return height; }
	coor_type getDesignWidth() { return width; }

	// target uses center
	coor_type computeOverlap(int cell_index, Coor target, int cell_to_ignore);
	coor_type computeOverlap(int cell_index, SwapTarget &target);

	Bin& operator[](const int index) { return binSet[index]; }
	int size() { return binSet.size(); }
	void fillVectorWithBinsTotallyInRegion(Region r, vector<int> &bins); //totally in region bins only
	void fillVectorWithBinsPartlyInRegion(Region r, vector<int> &bins); //partly in region only
	bool distanceBetweenCellAndBin(int bin_index, int cell_index);
	void fillVectorWithBinsInRegion(Region r, vector<int> &bins, int curr_cell = -1); //partly or totally in region, ie all bins that overlap with region
	int getBinOfThisPoint(Coor c);
	pair<int, int> getBinOfThisCell(int cell_index);
	pair<int, int> getBinOfThisCell(Cell &c);
	pair<int, int> getBinOfThisSwapTarget(SwapTarget s);
	pair<coor_type, coor_type> giveHungHao(int cell_index);
	void densityChangeAfterSwap(int cell_index, SwapTarget target, map<int, double> &size_diff);
};

extern BinLibrary bin_lib, bin_lib2;
