#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include <iterator>
#include <set>
#include <vector>
#include <iostream>
#include <cstring>
#include <time.h>
#include "parameters.h"

using namespace std;

/* ************************************************** */
typedef float coor_type;
/* ************************************************** */
struct Coor
{
    coor_type x, y;
    Coor():x(0.0), y(0.0) {}
    Coor(coor_type X, coor_type Y):x(X), y(Y) {}
    Coor(Coor const &CONSTRUCTOR):x(CONSTRUCTOR.x), y(CONSTRUCTOR.y) {}
    void setCoor(coor_type p_x, coor_type p_y) {x = p_x, y = p_y;}
    bool operator==(Coor c){return (x == c.x) && (y == c.y);}
}; // Coor
/* ************************************************** */
struct Shape
{
// The cell's shape(composed by retangle)
    string Name;
    Coor ll;// lower left x,y
    Coor ur;
    coor_type width;
    coor_type height;
    Shape():ll(0.0, 0.0), ur(0.0, 0.0), width(0.0), height(0.0) {}
    Shape(Coor Mergell, coor_type WIDTH, coor_type HEIGHT):ll(Mergell), width(WIDTH), height(HEIGHT) {calculate_ur();}
    Shape(Coor LL, Coor UR):ll(LL), ur(UR) {calculate_Width_and_Height();}
    void calculate_ur() {ur.x = ll.x + width; ur.y = ll.y + height;}
    void calculate_Width_and_Height() {width = ur.x - ll.x; height = ur.y - ll.y;}
}; // Shape
/* ************************************************** */
struct NetNode
{
// Net to Node
    string NodeName;
    bool IorO;// 1=I, 0=O.
    double pin_xoffset, pin_yoffset;                    //!!!Not having center position makes my code ugly!!!
    //vector<Cell>::iterator CellConnected;
    size_t CellPosition;// Store the connected cell's position of cell_lib
    NetNode():IorO(0),pin_xoffset(0.0),pin_yoffset(0.0),CellPosition(0){}
    // the net connected with cell order in cell library.
    // e.g. cell_lib[net_lib.cell[0].CellPosition] and cell_lib[net_lib.cell[1].CellPosition]
    //      are connected.
}; // NetNode
/* ************************************************** */
struct RouteLayerInfo        //!!Do we still need this?
{// each layer's route info
    int VerticalCapacity;
    int HorizontalCapacity;
    int MinWireWidth;
    int MinWireSpacing;
    int ViaSpacing;
    RouteLayerInfo():VerticalCapacity(0),HorizontalCapacity(0),
    MinWireWidth(0),MinWireSpacing(0),ViaSpacing(0){}
};
/* ************************************************** */
class PinInfo{
public:
	coor_type x_offset, y_offset;
	int net_number;
	PinInfo(int net_num, coor_type x_off, coor_type y_off)
	{
		net_number = net_num;
		x_offset = x_off;
		y_offset = y_off;
	}

	bool operator<(const PinInfo &p) const
	{
		return net_number < p.net_number;
	}

	bool operator==(const PinInfo &p) const
	{
		return net_number < p.net_number;
	}


};
/* ************************************************** */
class PlaceMap
{// Placement's cell boundary
public:
    PlaceMap():Boundary_ll(0.0, 0.0), Boundary_ur(0.0, 0.0) {}
    PlaceMap(Coor ll, Coor ur):Boundary_ll(ll), Boundary_ur(ur) {}
/*
    void initialBoundary_ll_x(coor_type b_ll_x){Boundary_ll.x = b_ll_x;}
    void initialBoundary_ll_y(coor_type b_ll_y){Boundary_ll.y = b_ll_y;}
    void initialBoundary_ur_x(coor_type b_ur_x){Boundary_ur.x = b_ur_x;}
    void initialBoundary_ur_y(coor_type b_ur_y){Boundary_ur.y = b_ur_y;}
*/
    void initialBoundary(Coor ll, Coor ur) {Boundary_ll = ll; Boundary_ur = ur;}

    Coor getBoundary_ll() {return Boundary_ll;}
    Coor getBoundary_ur() {return Boundary_ur;}

private:
    Coor Boundary_ll;// Recording placement boundary low left coor.
    Coor Boundary_ur;// Recording placement boundary up right coor.
}; // PlaceMap
/* ************************************************** */
