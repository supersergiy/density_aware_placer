#include "main.h"
#include "local_reordering.h"
#include "utilities.h"
#include "region.h"

#include <iostream>
#include <set>

#define CLUSTER_SIZE 2

/* ************************************************** */
void getBestOrder(Cluster& p_cluster, coor_type pr_cur_HPWL, size_t p_begin, size_t p_end);
void updateClusterCell(Cluster& p_cluster);
/* ************************************************** */
Cluster best_cluster;
coor_type best_HPWL;
int moved_cell;
/* ************************************************** */
void initial_cluster()
{
// Assume cell is sorted and legal
	//cout << "initial cluster begin" << endl;
    size_t cell_pos = 0;
    for(int row_iter = 0; row_iter < (int)design.Row_Info.size(); row_iter++)
    {
        for(int subrow_iter = 0; subrow_iter < (int)design.Row_Info.at(row_iter).getSubRowNum(); subrow_iter++)
        {
            SubRow& subrow_r = design.Row_Info.at(row_iter).getSubRow(subrow_iter);
            subrow_r.updateCell();
            subrow_r.clearCluster();
            for(int cell_iter = 0; cell_iter < (int)subrow_r.getCellNum(); cell_iter++)
            {
                cell_pos = subrow_r.getCell(cell_iter);
                if(cell_iter == 0 ||
                        getCellLeftBorder(cell_pos) > subrow_r.getCluster(subrow_r.getClusterNum() - 1).get_rx())
                {
                // add new cluster
                    Cluster new_cluster(getCellLeftBorder(cell_pos), subrow_r.getCoor());
                    new_cluster.setHeight(subrow_r.getHeight());
                    new_cluster.addCell(cell_pos, design.cell_lib.at(cell_pos).getWidth());

                    subrow_r.addCluster(new_cluster);
                } // if
                else
                {
                // add cell to cluster
                    subrow_r.getCluster(subrow_r.getClusterNum() - 1).addCell(cell_pos, design.cell_lib.at(cell_pos).getWidth());
                } // else
            } // for
        } // for
    } // for
/*
    // test cluster correctness
    for(int row_iter = 0; row_iter < (int)design.Row_Info.size(); row_iter++)
    {
    	for(int subrow_iter = 0; subrow_iter < (int)design.Row_Info.at(row_iter).getSubRowNum(); subrow_iter++)
    	{
    		SubRow& subrow_r = design.Row_Info.at(row_iter).getSubRow(subrow_iter);
			for(int cluster_iter = 0; cluster_iter < (int)subrow_r.getClusterNum(); cluster_iter++)
			{
				Cluster& cluster_c = subrow_r.getCluster(cluster_iter);
				coor_type total_cell_width = 0.0;
				if(cluster_c.get_lx() != getCellLeftBorder(cluster_c.getCell(0)))
					cout << "error: cluster lx wrong!!" << endl;
				for(int cell_iter = 0; cell_iter < (int)cluster_c.getCellNum(); cell_iter++)
				{
					total_cell_width += getCellWidth(cluster_c.getCell(cell_iter));
				}
				if(cluster_c.getWidth() != total_cell_width)
					cout << "error: cluster width wrong!!" << endl;
			}
    	}
    }
*/
    //cout << "initial cluster end" << endl;

    return;
} // initial_cluster

/* ************************************************** */
void local_reordering()
{
// Doing: local reordering
    cout << "[local_reordering.cpp] local reordering start" << endl;
    set<size_t> net_set;

    initial_cluster();
    moved_cell = 0;
    for(int row_iter = 0; row_iter < (int)design.Row_Info.size(); row_iter++)
    {
        //cout << row_iter << endl;
        for(int subrow_iter = 0; subrow_iter < (int)design.Row_Info.at(row_iter).getSubRowNum(); subrow_iter++)
        {
            for(int cluster_iter = 0; cluster_iter < (int)design.Row_Info.at(row_iter).getSubRow(subrow_iter).getClusterNum(); cluster_iter++)
            {
                Cluster& cluster = design.Row_Info.at(row_iter).getSubRow(subrow_iter).getCluster(cluster_iter);
                // calculate hpwl
                coor_type ori_hpwl = 0.0;

                if(cluster.getCellNum() == 1)
                {
                    continue;
                }

                net_set.clear();
                for(int cell_iter = 0; cell_iter < (int)cluster.getCellNum(); cell_iter++)
                {
                    for(int net_iter = 0; net_iter < (int)design.cell_lib[cluster.getCell(cell_iter)].getPinNum(); net_iter++)
                    {
                        net_set.insert(design.cell_lib[cluster.getCell(cell_iter)].getNet(net_iter));
                    } // for
                } // for
                for(set<size_t>::iterator net_iter = net_set.begin(); net_iter != net_set.end(); net_iter++)
                {
                    ori_hpwl += design.net_lib[(*net_iter)].getHPWL();
                } // for

                // try all possible premutation
                Cluster sub_best_cluster = cluster;
                best_cluster = cluster;
                best_HPWL = ori_hpwl;

                if(cluster.getCellNum() > CLUSTER_SIZE)
                {
                    for(int n_iter = 0; n_iter <= (int)cluster.getCellNum() - CLUSTER_SIZE; n_iter++)
                    {
                    // n consecutive cells
                        getBestOrder(sub_best_cluster, ori_hpwl, n_iter, n_iter + CLUSTER_SIZE);
                        sub_best_cluster.copy(best_cluster ,n_iter, n_iter + CLUSTER_SIZE);
                    } // for
                } // if
                else
                {
                    getBestOrder(sub_best_cluster, ori_hpwl, 0, cluster.getCellNum());
                    sub_best_cluster = best_cluster;
                } // else

                if(best_HPWL < ori_hpwl)
                {/*
                	if (best_HPWL != 0){
                	cout << "sergiy " << endl;
                	}*/
                    cluster = sub_best_cluster;
                    // update cluster cell
                    updateClusterCell(cluster);
                } // if
            } // for
        } // for
    } // for

	cout << "total moved cell: " << moved_cell << endl;
    cout << "[local_reordering.cpp] local reordering end" << endl;

    return;
} // local_reordering
/* ************************************************** */
void getBestOrder(Cluster& p_cluster ,coor_type pr_cur_HPWL, size_t p_begin, size_t p_end)
{

    if(p_end - p_begin == 1)
    {
        if(pr_cur_HPWL < best_HPWL)
        {
            best_HPWL = pr_cur_HPWL;
            best_cluster = p_cluster;
        }

        return;
    } // if

    coor_type hpwl_change = 0.0;
    coor_type new_lx = 0.0;
    for(int cell_iter = p_begin + 1; cell_iter < (int)p_end; cell_iter++)
    {
        //coor_type hpwl = best_hpwl;
        coor_type new_hpwl = pr_cur_HPWL;

        // swap cell
        p_cluster.swapCell(p_begin, cell_iter);

        // calculate hpwl
        new_lx = getCellLeftBorder(p_cluster.getCell(p_begin));
        hpwl_change = 0.0;
        vector<int> common_nets;
		findNetsInCommon(p_cluster.getCell(p_begin), p_cluster.getCell(cell_iter), common_nets);
        for(int cell_iter_2 = p_begin; cell_iter_2 < (int)(p_end - p_begin); cell_iter_2++)
        {
            Coor tmp_coor(new_lx + getCellWidth(p_cluster.getCell(cell_iter_2)) / 2, p_cluster.getCoor());
            hpwl_change += hpwlChangeAfterMovingCellToCoor(p_cluster.getCell(cell_iter_2), tmp_coor, common_nets);
            new_lx += getCellWidth(p_cluster.getCell(cell_iter_2));
        }
        new_hpwl += hpwl_change;

        getBestOrder(p_cluster, new_hpwl, p_begin + 1, p_end);

        p_cluster.swapCell(p_begin, cell_iter);
        new_hpwl -= hpwl_change;
    } // for

    return;
} // getBestOrder
/* ************************************************** */

void updateClusterCell(Cluster& p_cluster)
{
    coor_type new_lx = p_cluster.get_lx();

    for(int cell_iter = 0; cell_iter < (int)p_cluster.getCellNum(); cell_iter++)
    {
        Coor tmp_coor(new_lx + getCellHalfWidth(p_cluster.getCell(cell_iter)), p_cluster.getCoor() + p_cluster.getHeight() / 2);
        if(getCellCenter(p_cluster.getCell(cell_iter)).x != tmp_coor.x ||
           getCellCenter(p_cluster.getCell(cell_iter)).y != tmp_coor.y)
        {
        	moved_cell++;
        	design.cell_lib.moveCell(p_cluster.getCell(cell_iter), tmp_coor);
        }
        new_lx += getCellWidth(p_cluster.getCell(cell_iter));
    } // for

    return;
} // updateClusterCell
/* ************************************************** */
void local_shifting()
{
	for(int row_iter = 0; row_iter < design.Row_Info.size(); row_iter++)
	{
		for(int subrow_iter = 0; subrow_iter < design.Row_Info[row_iter].getSubRowNum(); subrow_iter++)
		{
			SubRow& subrow_r = design.Row_Info[row_iter].getSubRow(subrow_iter);
			subrow_r.updateCell();
			coor_type last_rx = 0.0;
			coor_type next_lx = 0.0;
			Region optimal_region;
			bool on_the_left_of_region = false;
			bool on_the_right_of_region = false;
			for(int cell_iter = 0; cell_iter < subrow_r.getCellNum(); cell_iter++)
			{
				// update left and right
				if(cell_iter == 0)
				{
					last_rx = subrow_r.get_lx();
				}
				else
				{
					last_rx = getCellRightBorder(subrow_r.getCell(cell_iter - 1));
				}

				if(cell_iter != subrow_r.getCellNum() - 1)
				{
					next_lx = getCellLeftBorder(subrow_r.getCell(cell_iter + 1));
				}
				else
				{
					next_lx = subrow_r.get_rx();
				}

				findOptimalRegion(subrow_r.getCell(cell_iter), optimal_region);
				on_the_left_of_region = false;
				on_the_right_of_region = false;
				if(getCellRightBorder(subrow_r.getCell(cell_iter)) < optimal_region.left_limit)
				{
				// left of region
					on_the_left_of_region = true;
				}
				else if(getCellLeftBorder(subrow_r.getCell(cell_iter)) > optimal_region.right_limit)
				{
				// right of region
					on_the_right_of_region = true;
				}

				// check white space
				if(on_the_left_of_region == true)
				{
					if(next_lx > getCellRightBorder(subrow_r.getCell(cell_iter)))
					{
						// move cell
						coor_type new_cell_rx = 0.0;
						//new_cell_rx = min(next_lx, optimal_region.left_limit);
						pair<coor_type, coor_type> bin_pair = bin_lib2.giveHungHao(subrow_r.getCell(cell_iter));
						new_cell_rx = min(next_lx, bin_pair.second);
						Coor tmp_coor(new_cell_rx - getCellHalfWidth(subrow_r.getCell(cell_iter)), subrow_r.getCoor() + subrow_r.getHeight() / 2);
						design.cell_lib.moveCell(subrow_r.getCell(cell_iter), tmp_coor);
					}
				}
				else if(on_the_right_of_region == true)
				{
					if(last_rx < getCellLeftBorder(subrow_r.getCell(cell_iter)))
					{
						// move cell
						coor_type new_cell_lx = 0.0;
						//new_cell_lx = max(last_rx, optimal_region.right_limit);
						pair<coor_type, coor_type> bin_pair = bin_lib2.giveHungHao(subrow_r.getCell(cell_iter));
						new_cell_lx = max(last_rx, bin_pair.first);
						Coor tmp_coor(new_cell_lx + getCellHalfWidth(subrow_r.getCell(cell_iter)), subrow_r.getCoor() + subrow_r.getHeight() / 2);
						design.cell_lib.moveCell(subrow_r.getCell(cell_iter), tmp_coor);
					}
				}
			}
		}
	}

	return;
} // void local_shifting

