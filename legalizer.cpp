#include "main.h"
#include "legalizer.h"
#include <cmath>
#include <ctime>
#include <iostream>

using namespace std;

#ifdef DEBUG
extern time_t tLegalization_total;
#endif

void collapse(RowInfo& p_row, size_t p_subrow_pos, size_t p_cluster_pos);

void diplacement_checker(void)
{
	Region max_region;
	int out_of_band = 1;

	while(out_of_band != 0){
		cout << "Try to lose weight" << endl;
		out_of_band = 0;

		for (int i = 0; i < design.original_number_of_cells; i++){
			Cell &curr_cell(design.cell_lib[i]);
			if (!curr_cell.getTerminal() && getDistanceBetweenPoints(curr_cell.getCenter(), curr_cell.getOriginalCenter()) > design.displacement){
				if (!tryToSwapToMaxRegion(i)){
					//cout << "fuck~~~~" << endl;
					out_of_band++;
				}
			}
		}
	}
}

void mergeCluster(SubRow& p_subrow, size_t p_cluster_pos_1, size_t p_cluster_pos_2);
/* ************************************************** */
coor_type Cluster::getCellCenter()
{
//	return (getCellLeftBorder(cell[0]) + getCellRightBorder(cell[cell.size() - 1])) / 2;

    if(cell.size() % 2 == 0)
    {
        return (design.cell_lib.at(cell.at(cell.size() / 2 - 1)).getCenter().x +
                design.cell_lib.at(cell.at(cell.size() / 2)).getCenter().x) / 2;
    } // if
    else
    {
        return design.cell_lib.at(cell.at(cell.size() / 2)).getCenter().x;
    } // else
} // Cluster::getCellCenter
/* ************************************************** */
void RowBase_legalization()
{
// Doing: legalize each subrow
//        if subrow is full then try Abacus legalization

#ifdef DEBUG
    time_t start_t, end_t;
    start_t = timeInUs();
#endif
    cout << "[they_are_taking_the_hobbits_to_isengard.cpp] Row Base Start" << endl;

    vector<size_t> unplaced_cell;

// try to legalize cells in subrow
    // initial used width of subrow
#pragma omp parallel for num_threads(4)
    for(int row_iter = 0; row_iter < (int)design.Row_Info.size(); row_iter++)
    {
        for(int subrow_iter = 0; subrow_iter < (int)design.Row_Info.at(row_iter).getSubRowNum(); subrow_iter++)
        {
            // calculate used area
            SubRow& subrow_r = design.Row_Info.at(row_iter).getSubRow(subrow_iter);
            coor_type used_width(0.0);
            for(int cell_iter = 0; cell_iter < (int)subrow_r.getCellNum(); cell_iter++)
            {
                used_width += design.cell_lib.at(subrow_r.getCell(cell_iter)).getWidth();
            } // for
            subrow_r.setUsedWidth(used_width);
        } // for
    } // for

#ifdef DEBUG
    // remove redundant cell in subrow and put into unplaced_cell
    for(int row_iter = 0; row_iter < (int)design.Row_Info.size(); row_iter++)
    {
        for(int subrow_iter = 0; subrow_iter < (int)design.Row_Info.at(row_iter).getSubRowNum(); subrow_iter++)
        {
            // remove cell
            SubRow& subrow_r = design.Row_Info.at(row_iter).getSubRow(subrow_iter);
            size_t cell_pos = -1;
            while(subrow_r.getUsedWidth()  > subrow_r.getWidth())
            {
                if(subrow_r.getCellNum() > 0)
                {
                    cell_pos = subrow_r.getCell(subrow_r.getCellNum() - 1);
                    unplaced_cell.push_back(cell_pos);
                    //to delete gan!

                    //subrow_r.setUsedWidth(subrow_r.getUsedWidth() - design.cell_lib.at(cell_pos).getWidth());
                    subrow_r.deleteCell(subrow_r.getCellNum() - 1);
                } // if
                else
                {
                    cout << "Error: subrow width not fit" << endl;
                    break;
                } // else
            } // if
        } // for
    } // for/
#endif

    // initial cluster and place cell in subrow
#pragma omp parallel for num_threads(4)
    for(int row_iter = 0; row_iter < (int)design.Row_Info.size(); row_iter++)
    {
        for(int subrow_iter = 0; subrow_iter < (int)design.Row_Info.at(row_iter).getSubRowNum(); subrow_iter++)
        {
            // initial cluster
            SubRow& subrow_r = design.Row_Info.at(row_iter).getSubRow(subrow_iter);
            /*/***//*/*/
            subrow_r.updateCell();

            size_t cell_pos = -1;
            int int_pos = -1;
            subrow_r.clearCluster();
            for(int cell_iter = 0; cell_iter < (int)subrow_r.getCellNum(); cell_iter++)
            {
                cell_pos = subrow_r.getCell(cell_iter);
                if(cell_iter == 0 || design.cell_lib.at(cell_pos).ll.x >= subrow_r.getCluster(subrow_r.getClusterNum() - 1).get_rx())
                {
                // add new cluster
                    // align sitespace
                    int_pos = (int)(design.cell_lib.at(cell_pos).ll.x / design.Row_Info.at(row_iter).getSitespacing()) *
                              design.Row_Info.at(row_iter).getSitespacing();
                    if(int_pos < subrow_r.get_lx())
                    	int_pos = subrow_r.get_lx();
                    else if(int_pos + design.cell_lib.at(cell_pos).getWidth() > subrow_r.get_rx())
                    	int_pos = subrow_r.get_rx() - design.cell_lib.at(cell_pos).getWidth();

                    Cluster new_cluster(int_pos, subrow_r.getCoor());
                    new_cluster.setHeight(subrow_r.getHeight());
                    new_cluster.addCell(cell_pos, design.cell_lib.at(cell_pos).getWidth());

                    subrow_r.addCluster(new_cluster);
                    collapse(design.Row_Info.at(row_iter), subrow_iter, subrow_r.getClusterNum() - 1);
                } // if
                else
                {
                // collapse cluster
                    subrow_r.getCluster(subrow_r.getClusterNum() - 1).addCell(cell_pos, design.cell_lib.at(cell_pos).getWidth());
                    collapse(design.Row_Info.at(row_iter), subrow_iter, subrow_r.getClusterNum() - 1);
                } // else if
            } // for
        } // for
    } // for


    // write legalized position to cell
    for(int row_iter = 0; row_iter < (int)design.Row_Info.size(); row_iter++)
    {
    	int cur_lx = 0;
        for(int subrow_iter = 0; subrow_iter < (int)design.Row_Info.at(row_iter).getSubRowNum(); subrow_iter++)
        {
            for(int cluster_iter = 0; cluster_iter < (int)design.Row_Info.at(row_iter).getSubRow(subrow_iter).getClusterNum(); cluster_iter++)
            {
                Cluster& cluster = design.Row_Info.at(row_iter).getSubRow(subrow_iter).getCluster(cluster_iter);
                cur_lx = cluster.get_lx();
                for(int cell_iter = 0; cell_iter < (int)cluster.getCellNum(); cell_iter++)
                {
                	Coor target_coor;

                	target_coor.y = cluster.getCoor() + cluster.getHeight() / 2;
                	target_coor.x = (coor_type)cur_lx + design.cell_lib.at(cluster.getCell(cell_iter)).getWidth() / 2;
                	design.cell_lib.moveCell(cluster.getCell(cell_iter), target_coor);
                    cur_lx += design.cell_lib.at(cluster.getCell(cell_iter)).getWidth();
                } // for
            } // for
        } // for
    } // for

    cout << "unplaced_cell.size: " << unplaced_cell.size() << endl;
    if(unplaced_cell.size() > 0)
        cout << "legalization is not legal" << endl;

    //cout << "cell : " << maxMoveCell << "-" << design.cell_lib.at(maxMoveCell).getObjName() << " has the max movement " << maxCost << endl;
    //cout << "total HPWL move : " << totalmove << endl;

#ifdef DEBUG
    end_t = timeInUs();
    cout << "The duration : " << timeInSec(end_t - start_t) << " sec" << endl;
    tLegalization_total += end_t - start_t;
#endif
    cout << "[they_are_taking_the_hobbits_to_isengard.cpp] Row Base Finish" << endl;

    return;
} // RowBase_legalization
/* ************************************************** */
void collapse(RowInfo& p_row, size_t p_subrow_pos, size_t p_cluster_pos)
{
// place cluster
    SubRow& subrow = p_row.getSubRow(p_subrow_pos);
    coor_type cluster_pos = 0.0;
    int int_cluster_pos = -1;
    coor_type x_min = subrow.get_lx();
    coor_type x_max = subrow.get_rx();

    //cluster_pos = subrow.getCluster(p_cluster_pos).getCellCenter() - subrow.getCluster(p_cluster_pos).getWidth() / 2;
    coor_type cluster_lx_center_cell = 0.0;
    //coor_type cluster_cell_center = subrow.getCluster(p_cluster_pos).getCellCenter();
    for(int cell_iter = 0; cell_iter < (int)(subrow.getCluster(p_cluster_pos).getCellNum() - 1) / 2; cell_iter++)
    {
    	cluster_lx_center_cell += design.cell_lib.at(subrow.getCluster(p_cluster_pos).getCell(cell_iter)).getWidth();
    } // for
    if(subrow.getCluster(p_cluster_pos).getCellNum() % 2 == 0)
    {
    	cluster_lx_center_cell += design.cell_lib.at(subrow.getCluster(p_cluster_pos).getCell((subrow.getCluster(p_cluster_pos).getCellNum() - 1) / 2)).getWidth();
    }
    else
    {
    	cluster_lx_center_cell += design.cell_lib.at(subrow.getCluster(p_cluster_pos).getCell((subrow.getCluster(p_cluster_pos).getCellNum() - 1) / 2)).getWidth() / 2;
    }
    cluster_pos = subrow.getCluster(p_cluster_pos).getCellCenter() - cluster_lx_center_cell;

    // align sitespace
    int_cluster_pos = (int)(cluster_pos / p_row.getSitespacing()) * p_row.getSitespacing();

    // adjust cluster out of subrow
    if(int_cluster_pos < x_min)
    {
        int_cluster_pos = x_min;
    } // if
    else if(int_cluster_pos + subrow.getCluster(p_cluster_pos).getWidth() > x_max)
    {
        int_cluster_pos = x_max - subrow.getCluster(p_cluster_pos).getWidth();
    } // else if

    subrow.getCluster(p_cluster_pos).set_lx(int_cluster_pos);

    // overlap between cluster
    if(p_cluster_pos > 0)
    {
        if(subrow.getCluster(p_cluster_pos - 1).get_rx() > subrow.getCluster(p_cluster_pos).get_lx())
        {
        // overlap with left cluster
            // merge cluster
            mergeCluster(subrow, p_cluster_pos - 1, p_cluster_pos);
            collapse(p_row, p_subrow_pos, p_cluster_pos - 1);
        } // if
    } // if
    else if(p_cluster_pos + 1 < subrow.getClusterNum())
    {
        if(subrow.getCluster(p_cluster_pos + 1).get_lx() < subrow.getCluster(p_cluster_pos).get_rx())
        {
        // overlap with right cluster
            // merge cluster
            mergeCluster(subrow, p_cluster_pos, p_cluster_pos + 1);
            collapse(p_row, p_subrow_pos, p_cluster_pos);
        } // if
    } // else if

    return;
} // collapse
/* ************************************************** */
void mergeCluster(SubRow& p_subrow, size_t p_cluster_pos_1, size_t p_cluster_pos_2)
{
// merge cluster 2 into cluster 1
    // add cell
    size_t cell_pos = -1;
    for(size_t cell_iter = 0; cell_iter < p_subrow.getCluster(p_cluster_pos_2).getCellNum(); cell_iter++)
    {
        cell_pos = p_subrow.getCluster(p_cluster_pos_2).getCell(cell_iter);
        p_subrow.getCluster(p_cluster_pos_1).addCell(cell_pos, design.cell_lib.at(cell_pos).getWidth());
    }
    p_subrow.deleteCluster(p_cluster_pos_2);

    return;
} // mergeCluster
