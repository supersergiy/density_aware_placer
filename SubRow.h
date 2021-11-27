#pragma once

#include "basic_types.h"
#include "swap_target.h"
#include "multithread.h"
#include <vector>

using namespace std;

void moveSubrowCellLL(size_t p_cell_pos, Coor p_pos);
bool testIfAlreadyCongested(size_t p_cell_pos, Coor p_pos);
bool canBeSwaped(size_t cell_index, SwapTarget target);
int findSubRow(Coor p_pos);


//clusters are not shared between threads, so clusters need no locks
class Cluster
{
public:
    Cluster(): coordinate(0.0), lx(0.0), width(0.0), height(0.0) {}
    Cluster(coor_type p_lx, coor_type p_coor): coordinate(p_coor), lx(p_lx), width(0.0), height(0.0) {}
    Cluster(coor_type p_lx, coor_type p_coor, coor_type p_width, coor_type p_height): coordinate(p_coor), lx(p_lx),
        width(p_width), height(p_height) {}

    void setBoundary(Coor p_ll, coor_type p_width, coor_type p_height) {lx = p_ll.x; coordinate = p_ll.y; width = p_width; height = p_height;}
    //void setCoor(Coor p_coor) {ll = p_coor;}
    void set_lx(coor_type p_lx) {lx = p_lx;}
    //void setWidth(coor_type p_width) {width = p_width;}
    void setHeight(coor_type p_height) {height = p_height;}
    void addCell(size_t p_cell, coor_type p_width)
    {
        width += p_width;
        cell.push_back(p_cell);
    } // addCell

    coor_type get_lx() {return lx;}
    coor_type get_rx() {return lx + width;}
    coor_type getCoor() {return coordinate;}
    coor_type getWidth() {return width;}
    coor_type getHeight() {return height;}
    size_t getCell(size_t p_pos) {return cell.at(p_pos);}
    size_t getCellNum() {return cell.size();}

    void deleteCell(size_t cell);
    coor_type getCellCenter();
    void swapCell(size_t p_pos_1, size_t p_pos_2) {swap(cell[p_pos_1], cell[p_pos_2]);}
    void copy(Cluster& p_cluster, int p_begin, int p_end);

private:
    coor_type lx;
    coor_type coordinate;// Y-coor
    coor_type width;
    coor_type height;

    vector<size_t> cell;

}; // Cluster

/* ************************************************** */
class SubRow
{
public:
    SubRow(): coordinate(0.0), lx(0.0), width(0.0), height(0.0), used_width(0.0) {}
    bool operator <(const SubRow& other) const{return lx < other.lx;} // for sorting

    void setBoundary(Coor p_ll, coor_type p_width, coor_type p_height) {lx = p_ll.x; coordinate = p_ll.y; width = p_width; height = p_height;}
    //void setCoor(coor_type p_coor) {coordinate = p_coor;}
    void setWidth(coor_type p_width) {width = p_width;}
    void setHeight(coor_type p_height) {height = p_height;}
    void setCluster(Cluster p_cluster, size_t p_pos) {cluster.at(p_pos) = p_cluster;}
    void addCluster(Cluster p_cluster) {cluster.push_back(p_cluster);}


    coor_type get_lx() {return lx;}
    coor_type get_rx() {return lx + width;}
    coor_type getCoor() {return coordinate;}
    coor_type getWidth() {return width;}
    coor_type getHeight() {return height;}

    //readers
    //is lock needed? no lock yet
    size_t getCell(size_t p_pos) {return cell.at(p_pos);}
    size_t getCellNum() {return cell.size();}

    //no locks
    Cluster& getCluster(size_t p_pos) {return cluster.at(p_pos);}
    size_t getClusterNum() {return cluster.size();}

    // cell
    //writers
    void updateCell();
    void addCell(size_t p_cell); /*{cell.push_back(p_cell); used_width += design.cell_lib[p_cell].width;}*/
    void deleteCell(size_t p_cell_pos);/* {/*used_width -= design.cell_lib[getCell(p_cell_pos)].width; cell.erase(cell.begin() + p_cell_pos);}*/
    // cluster
    //no locks
    void clearCluster() {cluster.clear();}
    void deleteCluster(size_t p_cluster_pos) {cluster.erase(cluster.begin() + p_cluster_pos);}
    //void updateCluster(size_t p_pos, Cluster p_new_cluster) {cluster.at(p_pos) = p_new_cluster;}
    //void findCluster();
    //void moveCluster();

    // for legalize using
    void setUsedWidth(coor_type p_width) {used_width = p_width;}
    coor_type getUsedWidth() {return used_width;}

    coor_type getUsedArea(coor_type p_left, coor_type p_right);
    coor_type getUnusedWidth() {return width - used_width;}

    // FatMinMin

private:
    //Coor ll;
    coor_type lx;
    coor_type coordinate;// Y-coor
    coor_type width;
    coor_type height;

    coor_type used_width;

    vector<size_t> cell;
    vector<Cluster> cluster;

}; // SubRow

SubRow& getCellSubrow(int cell_index);
SubRow& getCoorSubrow(Coor coor);
pair<int, int> getCellRowSubrowIndex(int cell_index);
pair<int, int> getWhitespaceRowSubrowIndex(SwapTarget target);
pair<int, int> getCoorRowSubrowIndex(Coor coor);
