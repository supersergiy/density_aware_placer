#include "utilities.h"
#include "SubRow.h"
#include "design_info.h"
#include <algorithm>


extern time_t tSubrowUpdate;

/* ************************************************** */
inline bool compareCell_lx(size_t p_pos_1, size_t p_pos_2);
inline bool compareCell_center_x(size_t p_pos_1, size_t p_pos_2);
/* ************************************************** */

using namespace std;

pair<int, int> getCellRowSubrowIndex(int cell_index)
{
	Cell &curr_cell(design.cell_lib.at(cell_index));
	size_t row_pos = getRow(curr_cell.ll.y);
	size_t subrow_pos(0);

	for(int subrow_iter = 0; subrow_iter < (int)design.Row_Info.at(row_pos).getSubRowNum(); subrow_iter++)
	{
		if(curr_cell.getCenter().x >= design.Row_Info.at(row_pos).getSubRow(subrow_iter).get_lx() &&
		   curr_cell.getCenter().x <= design.Row_Info.at(row_pos).getSubRow(subrow_iter).get_rx())
		{
			subrow_pos = subrow_iter;
			break;
		} // if
	} // for

	return make_pair(row_pos, subrow_pos);
}

SubRow& getCellSubrow(int cell_index)
{
	pair<int, int> row_subrow = getCellRowSubrowIndex(cell_index);

	return design.Row_Info.at(row_subrow.first).getSubRow(row_subrow.second);
}

pair<int, int> getCoorRowSubrowIndex(Coor coor)
{
	size_t row_pos = getRow(coor.y);
	size_t subrow_pos(0);
	for(int subrow_iter = 0; subrow_iter < (int)design.Row_Info.at(row_pos).getSubRowNum(); subrow_iter++)
	{
		if(coor.x >= design.Row_Info.at(row_pos).getSubRow(subrow_iter).get_lx() &&
		   coor.x <= design.Row_Info.at(row_pos).getSubRow(subrow_iter).get_rx())
		{
			subrow_pos = subrow_iter;
		} // if
	} // for
	return make_pair(row_pos, subrow_pos);
}

SubRow& getCoorSubrow(Coor coor)
{
	size_t row_pos = getRow(coor.y);
	size_t subrow_pos(0);
	for(int subrow_iter = 0; subrow_iter < (int)design.Row_Info.at(row_pos).getSubRowNum(); subrow_iter++)
	{
		if(coor.x >= design.Row_Info.at(row_pos).getSubRow(subrow_iter).get_lx() &&
		   coor.x <= design.Row_Info.at(row_pos).getSubRow(subrow_iter).get_rx())
		{
			subrow_pos = subrow_iter;
		} // if
	} // for
	return design.Row_Info.at(row_pos).getSubRow(subrow_pos);
}

bool testIfAlreadyCongested(size_t p_cell_pos, Coor p_pos) {
	//return false;

	Cell &c(design.cell_lib[p_cell_pos]);
	size_t row_pos = getRow(c.getCenter().y);
	size_t subrow_pos(-1);

	for(int subrow_iter = 0; subrow_iter < (int)design.Row_Info.at(row_pos).getSubRowNum(); subrow_iter++)
	{
		if(c.getCenter().x >= design.Row_Info.at(row_pos).getSubRow(subrow_iter).get_lx() &&
		   c.getCenter().x <= design.Row_Info.at(row_pos).getSubRow(subrow_iter).get_rx())
		{
			subrow_pos = subrow_iter;
		} // if
	} // for

	size_t row_pos2 = getRow(p_pos.y);
	size_t subrow_pos2(-1);

	for(int subrow_iter = 0; subrow_iter < (int)design.Row_Info.at(row_pos2).getSubRowNum(); subrow_iter++)
	{
		if(p_pos.x >= design.Row_Info.at(row_pos2).getSubRow(subrow_iter).get_lx() &&
		   p_pos.x <= design.Row_Info.at(row_pos2).getSubRow(subrow_iter).get_rx())
		{
			subrow_pos2 = subrow_iter;
		} // if
	} // for

	if(subrow_pos2 == -1)
		cout << "error2\n";


	bool ans = design.cell_lib[p_cell_pos].getWidth() > design.Row_Info.at(row_pos2).getSubRow(subrow_pos2).getUnusedWidth();
	ans = ans || (row_pos == row_pos2 && subrow_pos == subrow_pos2);

	return ans;
}

bool canBeSwaped(size_t cell_index, SwapTarget target) {
	bool a1, a2;

	if(target.is_whitespace) {
		a1 = !testIfAlreadyCongested(cell_index, target.center);
		a2 = a1;
	}
	else{
		size_t row_pos = getRow(design.cell_lib[cell_index].getCenter().y);
		size_t subrow_pos(-1);


		for(int subrow_iter = 0; subrow_iter < (int)design.Row_Info.at(row_pos).getSubRowNum(); subrow_iter++)
		{
			if(design.cell_lib[cell_index].getCenter().x >= design.Row_Info.at(row_pos).getSubRow(subrow_iter).get_lx() &&
					design.cell_lib[cell_index].getCenter().x <= design.Row_Info.at(row_pos).getSubRow(subrow_iter).get_rx())
			{
				subrow_pos = subrow_iter;
			} // if
		} // for

		size_t row_pos2 = getRow(design.cell_lib[target.cell_index].getCenter().y);
		size_t subrow_pos2(-1);

		for(int subrow_iter = 0; subrow_iter < (int)design.Row_Info.at(row_pos2).getSubRowNum(); subrow_iter++)
		{
			if(design.cell_lib[target.cell_index].getCenter().x >= design.Row_Info.at(row_pos2).getSubRow(subrow_iter).get_lx() &&
					design.cell_lib[target.cell_index].getCenter().x <= design.Row_Info.at(row_pos2).getSubRow(subrow_iter).get_rx())
			{
				subrow_pos2 = subrow_iter;
			} // if
		} // for

		if (row_pos == row_pos2 && subrow_pos == subrow_pos2){
			a1 = true;
			a2 = true;
		}
		else{
			a1 = design.cell_lib[cell_index].getWidth() <= (design.Row_Info.at(row_pos2).getSubRow(subrow_pos2).getUnusedWidth() + design.cell_lib[target.cell_index].getWidth());
			a2 = design.cell_lib[target.cell_index].getWidth() <= (design.Row_Info.at(row_pos).getSubRow(subrow_pos).getUnusedWidth() + design.cell_lib[cell_index].getWidth());
		}
	}
	return a1 && a2;
}

void moveSubrowCellLL(size_t p_cell_pos, Coor p_pos)
{
	// delete cell in original subrow
    size_t row_pos = getRow(design.cell_lib.at(p_cell_pos).ll.y);
    size_t subrow_pos(-1);

    subrow_pos = findSubRow(design.cell_lib.at(p_cell_pos).getCenter());

    Cell c = design.cell_lib.at(p_cell_pos);


    if (subrow_pos == -1){
    	cout << "error 1" << endl;
    }

    for(int cell_iter = 0; cell_iter < (int)design.Row_Info.at(row_pos).getSubRow(subrow_pos).getCellNum(); cell_iter++)
    {

        if(p_cell_pos == design.Row_Info.at(row_pos).getSubRow(subrow_pos).getCell(cell_iter)){
            design.Row_Info.at(row_pos).getSubRow(subrow_pos).deleteCell(cell_iter);

        }
    } // for


    // find target subrow
    row_pos = getRow(p_pos.y);
    subrow_pos = -1;
    p_pos.x += c.getWidth() / 2;

    /*for(int subrow_iter = 0; subrow_iter < (int)design.Row_Info.at(row_pos).getSubRowNum(); subrow_iter++)
    {
        //if(p_pos.x >= design.Row_Info.at(row_pos).getSubRow(subrow_iter).get_lx() &&
        //   p_pos.x <= design.Row_Info.at(row_pos).getSubRow(subrow_iter).get_rx())
		if(p_pos.x + c.getWidth() / 2 >= design.Row_Info.at(row_pos).getSubRow(subrow_iter).get_lx() &&
		   p_pos.x + c.getWidth() / 2 <= design.Row_Info.at(row_pos).getSubRow(subrow_iter).get_rx())
        {
            subrow_pos = subrow_iter;
        } // if
    } // for
	*/
    subrow_pos = findSubRow(p_pos);
    if (subrow_pos == -1){
		cout << "error3\n";
	}
    //update cell pos
    //design.cell_lib.at(p_cell_pos).ll = p_pos;
    // move into subrow

    design.Row_Info.at(row_pos).getSubRow(subrow_pos).addCell(p_cell_pos);
    // update subrow
    //design.Row_Info.at(row_pos).getSubRow(subrow_pos).updateCell();


    return;
}

int findSubRow(Coor p_pos)
{
	int row_pos = getRow(p_pos.y);

	int subrow_pos = -1;

	int cur_subrow_pos =  (design.Row_Info.at(row_pos).getSubRowNum() - 1) / 2;
	int left_limit = 0;
	int right_limit = design.Row_Info.at(row_pos).getSubRowNum() - 1;

	while(left_limit <= right_limit)
	{
		if(p_pos.x >= design.Row_Info.at(row_pos).getSubRow(cur_subrow_pos).get_lx() &&
		   p_pos.x <= design.Row_Info.at(row_pos).getSubRow(cur_subrow_pos).get_rx())
		{
		// in subrow

			return cur_subrow_pos;
		}
		else if(p_pos.x < design.Row_Info.at(row_pos).getSubRow(cur_subrow_pos).get_lx())
		{
		// left of subrow
			right_limit = cur_subrow_pos - 1;
			cur_subrow_pos = (left_limit + right_limit) / 2;
		}
		else if(p_pos.x > design.Row_Info.at(row_pos).getSubRow(cur_subrow_pos).get_rx())
		{
		// right of subrow
			left_limit = cur_subrow_pos + 1;
			cur_subrow_pos = (left_limit + right_limit) / 2;
		}
	}

	//if(cur_subrow_pos == -1)
		cout << "errrrrrrrrrrror" << endl;

} // findSubRow
/* ************************************************** */
extern DesignInfo design;

void SubRow::addCell(size_t p_cell)
{
	cell.push_back(p_cell);
	used_width += design.cell_lib[p_cell].getWidth();
}
void SubRow::deleteCell(size_t p_cell_pos)
{
	used_width -= design.cell_lib[getCell(p_cell_pos)].getWidth();
	cell.erase(cell.begin() + p_cell_pos);
}
void SubRow::updateCell()
{
    sort(cell.begin(), cell.end(), compareCell_center_x);
    return;
} // SubRow::updateCell
/* ************************************************** */
coor_type SubRow::getUsedArea(coor_type p_left, coor_type p_right)
{
// assume cells in subrow is sorted
// return total cell area in the boundary
    double used_area(0.0);
    double left_bound(0.0);
    double right_bound(0.0);
    for(int cell_iter = 0; cell_iter < cell.size(); cell_iter++)
    {
        /*
        if(design.cell_lib.at(cell.at(cell_iter)).ll.x + design.cell_lib.at(cell.at(cell_iter)).getWidth() < p_left)
            continue;
        else if(design.cell_lib.at(cell.at(cell_iter)).ll.x > p_right)
            break;
        */
        if(design.cell_lib.at(cell.at(cell_iter)).ll.x + design.cell_lib.at(cell.at(cell_iter)).getWidth() > p_left &&
           design.cell_lib.at(cell.at(cell_iter)).ll.x < p_right)
        {
            if(design.cell_lib.at(cell.at(cell_iter)).ll.x < p_left)
                left_bound = p_left;
            else
                left_bound = design.cell_lib.at(cell.at(cell_iter)).ll.x;
            if(design.cell_lib.at(cell.at(cell_iter)).ll.x + design.cell_lib.at(cell.at(cell_iter)).getWidth() > p_right )
                right_bound = p_right;
            else
                right_bound = design.cell_lib.at(cell.at(cell_iter)).ll.x + design.cell_lib.at(cell.at(cell_iter)).getWidth();
            used_area += right_bound - left_bound;
        } // else
    } // for

    return used_area * design.getStandardCellHeight();
} // SubRow::getUsedArea
/* ************************************************** */
void Cluster::copy(Cluster& p_cluster, int p_begin, int p_end)
{
    for(int cell_iter = p_begin; cell_iter < p_end; cell_iter++)
    {
        cell[cell_iter] = p_cluster.getCell(cell_iter);
    } // for

    return;
} // Cluster::copy
/* ************************************************** */
inline bool compareCell_lx(size_t p_pos_1, size_t p_pos_2)
{
    return design.cell_lib.at(p_pos_1).ll.x < design.cell_lib.at(p_pos_2).ll.x;
} // CompareCell_lx
/* ************************************************** */
inline bool compareCell_center_x(size_t p_pos_1, size_t p_pos_2)
{
    return design.cell_lib.at(p_pos_1).getCenter().x < design.cell_lib.at(p_pos_2).getCenter().x;
} // CompareCell_lx
/* ************************************************** */
