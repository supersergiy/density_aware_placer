#include <iostream>
#include <algorithm>
#include "main.h"
#include "parameters.h"
#include "utilities.h"

using namespace std;

coor_type BinLibrary::getX(coor_type x) {
	return x - design.PMapFrame.getBoundary_ll().x;
}
coor_type BinLibrary::getY(coor_type y) {
	return y - design.PMapFrame.getBoundary_ll().y;
}

/* ************************************************** */
int BinLibrary::getIndex(coor_type x, coor_type y)
{
	return (int)(getY(y) / binHeight) * numBinX + (getX(x) / binWidth);
}
/* ************************************************** */
int BinLibrary::getIndex(int row, int col)
{
	return row * numBinX + col;
}
/* ************************************************** */
void BinLibrary::insertCell(int cell_index)
{	
	Cell &c(design.cell_lib[cell_index]);
	coor_type left, right, bot, top;
	int l, r, b, t;

	left = c.getCenter().x - (c.getWidth() / 2);
	right = c.getCenter().x + (c.getWidth() / 2);
	bot = c.getCenter().y - (c.getHeight() / 2);
	top = c.getCenter().y + (c.getHeight() / 2);

	if (top < design.PMapFrame.getBoundary_ll().y || bot > design.PMapFrame.getBoundary_ur().y ||
		right < design.PMapFrame.getBoundary_ll().x || left > design.PMapFrame.getBoundary_ur().x){
		return;
	}
	l = (int)(getX(left) / binWidth), r = (int)(getX(right) / binWidth);
	b = (int)(getY(bot) / binHeight), t = (int)(getY(top - 1) / binHeight);

	for (int i = max(b, 0); i <= min(t, numBinY - 1); i++) {
		for(int j = max(l, 0); j <= min(r, numBinX - 1); j++) {
			binSet[getIndex(i, j)].cellMoveIn(cell_index);
		}
	}
}
/* ************************************************** */
void BinLibrary::insertCell(int cell_index, Coor target)
{
	Cell c(design.cell_lib[cell_index]);
	coor_type left, right, bot;
	int l, r, b;

	left = target.x - (c.getWidth() / 2), right = target.x + (c.getWidth() / 2);
	bot = target.y - (c.getHeight() / 2);
	l = (int)(getX(left) / binWidth), r = (int)(getX(right) / binWidth);
	b = (int)(getY(bot) / binHeight);

	if (right < design.PMapFrame.getBoundary_ll().x || left > design.PMapFrame.getBoundary_ur().x){
		return;
	}

	for(int i = l; i <= r; i++) {
		float oldDensity, newDensity;

		oldDensity = min((int)(binSet[getIndex(b, i)].getDensity() * 100), 100);

		binSet[getIndex(b, i)].cellMoveIn(cell_index, target);

		newDensity = min((int)(binSet[getIndex(b, i)].getDensity() * 100), 100);

		for(int j = oldDensity + 1; j <= newDensity; j++) {
			if(j < 0)
				cout << "bug" << endl;
			densityTable[j]++;
		}
	}
}
/* ************************************************** */

float bin1_threshold, bin2_threshold;

void BinLibrary::buildBinLibrary(int bin_size, bool store, int bin_width)
{
	rowHeight = design.Row_Info[0].getHeight(), siteWidth = design.Row_Info[0].getSitewidth();
	if(bin_width == 0) {
		binWidth = binHeight = bin_size * rowHeight;
	}
	else {
		binHeight = bin_size * rowHeight;
		binWidth = bin_width * rowHeight;
	}

	if(bin_size == 9) {
		bin2_threshold = binWidth * binHeight * 0.2;
	}
	else {
		bin1_threshold = binWidth * binHeight * 0.2;
	}

	height = 0.0, width = 0.0;
	store_cells = store;

	width = design.PMapFrame.getBoundary_ur().x - design.PMapFrame.getBoundary_ll().x;
	height = design.PMapFrame.getBoundary_ur().y - design.PMapFrame.getBoundary_ll().y;

	numBinX = (int)((width + binWidth - 1) / binWidth);
	numBinY = (int)((height + binHeight - 1) / binHeight);

	int bin_num = numBinX * numBinY;
	binSet.resize(bin_num);

	for(coor_type i = design.PMapFrame.getBoundary_ll().x; i < design.PMapFrame.getBoundary_ur().x; i += binWidth){
		for(coor_type j = design.PMapFrame.getBoundary_ll().y; j < design.PMapFrame.getBoundary_ur().y; j += binHeight)
		{
			Region r;

			r.left_limit = i, r.right_limit = min(i + binWidth, design.PMapFrame.getBoundary_ur().x);
			r.bot_limit = j, r.top_limit = min(j + binHeight, design.PMapFrame.getBoundary_ur().y);

			bool overlap = false;
			coor_type bin_area = 0;
			for(int k = 0; k < design.PMap.size(); k++) {
				float left, right, bot, top;
				left = max(i, design.PMap[k].getBoundary_ll().x);
				right = min(i + binWidth, design.PMap[k].getBoundary_ur().x);
				bot = max(j, design.PMap[k].getBoundary_ll().y);
				top = min(j + binHeight, design.PMap[k].getBoundary_ur().y);
				if(left < right && bot < top) {
					if(store_cells) {
						r.left_limit = left, r.right_limit = right;
						r.bot_limit = bot, r.top_limit = top;
					}
					bin_area += (right - left) * (top - bot);
					overlap = true;
					//break;
				}
			}

			Bin bin(r, store_cells, bin_area);

			if(overlap)
				bin.canBeUse = true;
			else {
				bin.canBeUse = false;
				bin.free_area = 0;
				bin.density = 0;
			}

			binSet[getIndex(i, j)] = bin;
		}
	}
	for(int i = 0; i < design.cell_lib.size(); i++){
		if (design.cell_lib[i].getManyShape()){

		}
		else{
			insertCell(i);
		}
	}

	densityTable.resize(101);
	for(int i = 0; i < densityTable.size(); i++)
		densityTable[i] = 0;
	for(int i = 0; i < binSet.size(); i++) {
		if(binSet[i].getDensity() * 100 > 100) {
			cout << i << ' ' << binSet[i].getDensity() << endl;
		}
		for(int j = 0; j <= binSet[i].getDensity() * 100; j++) {
			if(j > 100){
				continue;
			}
			densityTable[j]++;
		}
	}

}
/* ************************************************** */
void BinLibrary::moveCell(int cell_index, Coor target)
{
	pair<int, int> p = BinLibrary::getBinOfThisCell(cell_index);

	for(int i = p.first; i <= p.second; i++) {
		float oldDensity, newDensity;
		oldDensity = min((int)(binSet[i].getDensity() * 100), 100);

		binSet[i].cellMoveOut(cell_index);

		newDensity = min((int)(binSet[i].getDensity() * 100), 100);

		for(int j = newDensity + 1; j <= oldDensity; j++) {
			if(j < 0)
				cout << "bug" <<endl;
			densityTable[j]--;
		}
	}

	insertCell(cell_index, target);
}
/* ************************************************** */
void BinLibrary::fillVectorWithBinsTotallyInRegion(Region r, vector<int> &bins)
{
	coor_type gap1 = binWidth - 1, gap2 = binHeight - 1;
	int left = (int)((getX(r.left_limit) + gap1) / binWidth), right = (int)((getX(r.right_limit) - gap1) / binWidth);
	int bot = (int)((getY(r.bot_limit) + gap2) / binHeight), top = (int)((getY(r.top_limit) - gap2) / binHeight);
	for(int i = left; i <= right; i++)
		for(int j = bot; j <= top; j++)
		{
			if(getIndex(j, i) < binSet.size() && binSet[getIndex(j, i)].canBeUse)
				bins.push_back(getIndex(j, i));
		}
}
/* ************************************************** */
void BinLibrary::fillVectorWithBinsPartlyInRegion(Region r, vector<int> &bins)
{
	coor_type gap1 = binWidth - 1, gap2 = binHeight - 1;
	int left = (int)(getX(r.left_limit) / binWidth), _left = (int)((getX(r.left_limit) + gap1) / binWidth);
	int right = (int)(getX(r.right_limit) / binWidth), _right = (int)((getX(r.right_limit) - gap1) / binWidth);
	int bot = (int)(getY(r.bot_limit) / binHeight), _bot = (int)((getY(r.bot_limit) + gap2) / binHeight);
	int top = (int)(getY(r.top_limit) / binHeight), _top = (int)((getY(r.top_limit) - gap2) / binHeight);
	// left border
	if(left < _left)
		for(int i = bot; i <= top; i++)
			if(getIndex(i, left) < binSet.size() && binSet[getIndex(i, left)].canBeUse)
				bins.push_back(getIndex(i, left));
	// right border
	if(right > _right)
		for(int i = bot; i <= top; i++)
			if(getIndex(i, right) < binSet.size() && binSet[getIndex(i, right)].canBeUse)
				bins.push_back(getIndex(i, right));
	// bottom border
	if(bot < _bot)
		for(int i = left; i <= right; i++)
			if(getIndex(bot, i) < binSet.size() && binSet[getIndex(bot, i)].canBeUse)
				bins.push_back(getIndex(bot, i));
	// top border
	if(top > _top)
		for(int i = left; i <= right; i++)
			if(getIndex(top, i) < binSet.size() && binSet[getIndex(top, i)].canBeUse)
				bins.push_back(getIndex(top, i));
}
/* ************************************************** */
struct density_compare {
	bool operator()(int bin_index1, int bin_index2) {
		if(bin_lib[bin_index1].getDensity() < bin_lib[bin_index2].getDensity())
			return true;
//		else if(bin_lib[bin_index1].getDensity() == bin_lib[bin_index2].getDensity()) {
//			return bin_index1 < bin_index2;
//		}
		return false;
	}
};

bool BinLibrary::distanceBetweenCellAndBin(int bin_index, int cell_index) {
//	return true;
	return (getManhattanDistanceToRegion(getCellOriginalCenter(cell_index), binSet[bin_index].getRegion()) <= (design.displacement + 40));
}

void BinLibrary::fillVectorWithBinsInRegion(Region r, vector<int> &bins, int curr_cell)
{
	set<int, density_compare> sbins;
	int left = (int)(getX(r.left_limit) / binWidth), right = (int)(getX(r.right_limit) / binWidth);
	int bot = (int)(getY(r.bot_limit) / binHeight), top = (int)(getY(r.top_limit) / binHeight);

	for(int i = max(left, 0); i <= max(right, 0); i++){
		for(int j = max(bot, 0); j <= max(top, 0); j++) {
			int index = getIndex(j, i);
			if(index < binSet.size() && binSet[index].canBeUse && !binSet[index].isTotallyInsideMacroBlock()
					&& curr_cell != -1 && distanceBetweenCellAndBin(index, curr_cell)) {
				sbins.insert(index);
				if(sbins.size() > I_AM_HANDSOME)
					sbins.erase(--sbins.end());
			}
		}
	}

	bins = vector<int>(sbins.begin(), sbins.end());
}
/* ************************************************** */
int BinLibrary::getBinOfThisPoint(Coor c)
{
	return getIndex(c.x, c.y);
}
/* ************************************************** */
pair<int, int> BinLibrary::getBinOfThisCell(int cell_index)
{
	Cell &c(design.cell_lib[cell_index]);

	coor_type left, right, bot;

	left = c.getCenter().x - (c.getWidth() / 2), right = c.getCenter().x + (c.getWidth() / 2);
	bot = c.getCenter().y - (c.getHeight() / 2);

	int row = (int)(getY(bot) / binHeight) * numBinX;

	int l =  row + (int)(getX(left) / binWidth), r = row + (int)(getX(right - 1) / binWidth);

	l = min(l, (int)binSet.size() - 1);
	r = min(r, (int)binSet.size() - 1);

	return make_pair(l, r);
}

pair<int, int> BinLibrary::getBinOfThisCell(Cell &c)
{
	coor_type left, right, bot;

	left = c.getCenter().x - (c.getWidth() / 2), right = c.getCenter().x + (c.getWidth() / 2);
	bot = c.getCenter().y - (c.getHeight() / 2);

	int row = (int)(getY(bot) / binHeight) * numBinX;

	int l =  row + (int)(getX(left) / binWidth), r = row + (int)(getX(right - 1) / binWidth);

	l = min(l, (int)binSet.size() - 1);
	r = min(r, (int)binSet.size() - 1);

	return make_pair(l, r);
}

pair<coor_type, coor_type> BinLibrary::giveHungHao(int cell_index)
{
	pair<int, int> p = getBinOfThisCell(cell_index);
	return make_pair(binSet[p.first].getRegion().left_limit, binSet[p.second].getRegion().right_limit);
}
/* ************************************************** */
pair<int, int> BinLibrary::getBinOfThisSwapTarget(SwapTarget s)
{
	if(s.is_cell)
		return BinLibrary::getBinOfThisCell(s.cell_index);

	coor_type left = s.whitespace_center_left.x, right = s.whitespace_center_left.x + s.whitespace_width;

	int row = (int)(getY(s.whitespace_center_left.y) / binHeight) * numBinX;

	int l = row + (int)(getX(left) / binWidth), r = row + (int)(getX(right) / binWidth);

	l = min(l, (int)binSet.size() - 1);
    r = min(r, (int)binSet.size() - 1);

	return make_pair(l, r);
}
/* *************************************************** */

// target uses center coordinate
coor_type BinLibrary::computeOverlap(int cell_index, SwapTarget& target) {
	if(target.is_whitespace) {
		return BinLibrary::computeOverlap(cell_index, target.center, -1);
	}
	return BinLibrary::computeOverlap(cell_index, getCellCenter(target.cell_index), target.cell_index) +
			BinLibrary::computeOverlap(target.cell_index, getCellCenter(cell_index), cell_index);
}
coor_type BinLibrary::computeOverlap(int cell_index, Coor target, int cell_to_ignore)
{
	int index = getIndex(target.x, target.y);
	vector<int> candidate;
	for(set<int>::iterator it = binSet[index].cellsInside.begin(); it != binSet[index].cellsInside.end(); ++it) {
		if(*it != cell_index && *it != cell_to_ignore && getCellCenter(*it).y == target.y)
			candidate.push_back(*it);
	}
	if(index > 0) {
		for(set<int>::iterator it = binSet[index - 1].cellsInside.begin(); it != binSet[index - 1].cellsInside.end(); ++it) {
			if(*it != cell_index && *it != cell_to_ignore && getCellCenter(*it).y == target.y)
				candidate.push_back(*it);
		}
	}
	if(index + 1 < binSet.size()) {
		for(set<int>::iterator it = binSet[index + 1].cellsInside.begin(); it != binSet[index + 1].cellsInside.end(); ++it) {
			if(*it != cell_index && *it != cell_to_ignore && getCellCenter(*it).y == target.y)
				candidate.push_back(*it);
		}
	}
	coor_type overlap = 0;
	coor_type target_left_border = target.x - design.cell_lib[cell_index].getWidth() / 2;
	coor_type target_right_border = target.x + design.cell_lib[cell_index].getWidth() / 2;
	coor_type m_left = target_left_border;   // cluster left
	coor_type m_right = target_right_border; // cluster right
	for(int i = 0; i < candidate.size(); i++) {
		coor_type overlap_left_border = max(target_left_border, getCellLeftBorder(candidate[i]));
		coor_type overlap_right_border = min(target_right_border, getCellRightBorder(candidate[i]));

		if(overlap_left_border < overlap_right_border) {
			m_left = min(m_left, overlap_left_border);
			m_right = max(m_right, overlap_right_border);

			overlap += overlap_right_border - overlap_left_border;
		}
	}

	if(overlap == 0)
		return 0;

	coor_type x_left = m_left - 100;
	coor_type x_right = m_right + 100;
	for(int i = 0; i < candidate.size(); i++) {
		coor_type cell_left_border = getCellLeftBorder(candidate[i]);
		coor_type cell_right_border = getCellRightBorder(candidate[i]);

		if(cell_right_border <= m_left)
			x_left = max(x_left, cell_right_border);
		if(cell_left_border >= m_right)
			x_right = min(x_right, cell_left_border);
	}
	coor_type white_space = (m_left - x_left) + (x_right - m_right);
	//overlap -= white_space;
	return overlap + max(coor_type(0), overlap - white_space) * 5;
}

void BinLibrary::densityChangeAfterSwap(int cell_index, SwapTarget target, map<int, double> &size_diff)
{
	pair<int, int> bins;
//	map<int, double> size_diff;

	bins = getBinOfThisCell(cell_index);
	for(int i = bins.first; i <= bins.second; i++) {
//		m[i].first = binSet[i].getDensity();
		size_diff[i] -= binSet[i].overlapAreaWithThisCell(cell_index);
	}

	if(target.is_cell) {
		bins = getBinOfThisSwapTarget(target);
		for(int i = bins.first; i <= bins.second; i++) {
//			m[i].first = binSet[i].getDensity();
//			if(target.is_cell) {
				size_diff[i] -= binSet[i].overlapAreaWithThisCell(target.cell_index);
//			}
		}
	}


	Cell new_cell1, new_cell2;
	if(target.is_whitespace) {
		new_cell1 = design.cell_lib[cell_index];
		new_cell1.initialCenterX(target.center.x);
		new_cell1.initialCenterY(target.center.y);
	}
	else {
		new_cell1 = design.cell_lib[cell_index];
		new_cell1.initialCenterX(design.cell_lib[target.cell_index].getCenter().x);
		new_cell1.initialCenterY(design.cell_lib[target.cell_index].getCenter().y);

		new_cell2 = design.cell_lib[target.cell_index];
		new_cell2.initialCenterX(design.cell_lib[cell_index].getCenter().x);
		new_cell2.initialCenterY(design.cell_lib[cell_index].getCenter().y);
	}

	bins = getBinOfThisCell(new_cell1);
	for(int i = bins.first; i <= bins.second; i++) {
//		m[i].first = binSet[i].getDensity();
		size_diff[i] += binSet[i].overlapAreaWithThisCell(new_cell1);
	}

	if(target.is_cell) {
		bins = getBinOfThisCell(new_cell2);
		for(int i = bins.first; i <= bins.second; i++) {
			size_diff[i] += binSet[i].overlapAreaWithThisCell(new_cell2);
		}
	}
}
