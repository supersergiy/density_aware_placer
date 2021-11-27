#pragma once

#include "basic_types.h"
#include <string>
#include <vector>
#include "region.h"

using namespace std;

class Cell
{
public:
	// only movable cell can move center and ll, and only movable cell can use RL_ll and RL_center
	Coor ll;// (initial position.[fixed])The cell retangle's lower left coordinate

    /* test */
    Region precomputed_optimal_region;
    /* test */


    Cell():ll(0.0,0.0), center(0.0,0.0), width(0.0), height(0.0), Area(0.0), congestedTimes(0), orientation(false), move(true),
    terminal(false), NI(false), manyShape(false), NumShapes(0), NIRouteLayer(0), BlockageNodes(false), NumBlockedLayer(0){}


    void initialObjname(string initial) {ObjName = initial;}
    void initialWidth(double initial) {width = initial;}
    void initialHeight(double initial) {height = initial;}
    void initialTerminal(bool initial) {terminal = initial;}
    void initialNI(bool initial) {NI = initial;}
    void initialMove(bool initial) {move = initial;}
    void initialManyShape(bool initial) {manyShape = initial;}
    void initialNumShapes(int initial) {
        NumShapes=initial;
        contain.reserve(initial);// Speeding
    }
    void initialContain(Shape initial) {contain.push_back(initial);}
    void initialNIRouteLayer(int initial){ NIRouteLayer=initial;}
    void initialBlockageNodes(bool initial){ BlockageNodes=initial;}
    void initialNumBlockedLayer(int initial){ NumBlockedLayer=initial;}
    void initialBlockedLayer(vector<int> initial){ BlockedLayer=initial;}
    void initialArea(double area) {Area = area;}
	void initialCenterX(coor_type x) {center.x = x;}
	void initialCenterY(coor_type y) {center.y = y;}
	void initialCenter(Coor c) {center = c;}
	void initialLL(Coor c) {ll = c;}
	void initialOriginalCenterX(coor_type x) {original_center.x = x;}
	void initialOriginalCenterY(coor_type y) {original_center.y = y;}
	void initialOriginalCenter(Coor c) {original_center = c;}

    void clear() {Cell(); ObjName.clear(); contain.clear(); BlockedLayer.clear();}

	void moveCellCenter(Coor c);

	Coor getCenter(void) const {return center; }
	Coor getOriginalCenter(void) const {return original_center;}
    string getObjName() {return ObjName;}
    double getWidth() {return width;}
    double getHeight() {return height;}
    bool getTerminal() {return terminal;}
    bool getNI() {return NI;}
    bool getOrientation() {return orientation;}
    bool getMove() {return move;}
    bool getManyShape() {return manyShape;};
    int getNumShapes() {return NumShapes;}
    size_t getContainSize() {return contain.size();}
    Shape getContain(size_t p_pos) {return contain.at(p_pos);}

    int getNIRouteLayer() {return NIRouteLayer;}
    bool getBlockageNodes() {return BlockageNodes;}
    int getNumBlockedLayer() {return NumBlockedLayer;}
    vector<int> getBlockedLayer() const {return BlockedLayer;}

    void addPin(int netlib_Pos, coor_type x_off, coor_type y_off) {pins.push_back(PinInfo(netlib_Pos, x_off, y_off));}

	size_t getNet(size_t pin_count) {return pins.at(pin_count).net_number;}
	PinInfo& getPin(size_t pin_count) {return pins.at(pin_count);}
	Coor getPinCoor(size_t pin_count);
    size_t getPinNum() {return pins.size();}
    double getArea() {return Area;}
    coor_type getPinXOffset(int pin_index);
    coor_type getPinYOffset(int pin_index);

    Coor getUR() {return Coor(ll.x + width, ll.y + height);}

 /*   // routability driven using
    void setBloatingWidth(double width) {BloatingWidth = width;}
    void setBloatingArea(double area) {BloatingArea = area;}
    double getBloatingArea() {return BloatingArea;}
    double getBloatingWidth() {return BloatingWidth;}
    void calBloatingArea() {BloatingArea = BloatingWidth*height;}
    void initialCellBloating() {BloatingWidth = width; BloatingArea = Area;}
    void updateCongestedTimes() {congestedTimes++;}
    size_t getCongestedTimes()const {return congestedTimes;}
*/

    vector<PinInfo> pins;// Store the cell connected with which nets

private:

	Coor center;// (move position[move with iterations])The cell retangle's center coordinate
	// cell center was moved to private to enforce using moveCell
	Coor original_center;
    string ObjName;
    double width, height;
    double Area; // origin cell area, equal to (width*height)

    size_t congestedTimes; // recording the cell in the congested grid times.
    // cell character
    bool orientation;// true=can, false=cannot. all nodes is false(default)
    bool move;// true=can, false=cannot
    // pin using
    bool terminal;
    bool NI;// terminal NI or not
    // macro using
    bool manyShape;// true: cell!=shape, false: cell==shape
    int NumShapes;// No merge shapesize. when ManyShape speeding is off, same as contain.size()
    vector<Shape> contain;// the cell contains retangle
                          // the Number of shapes is contain.size()
    // routing information
    int NIRouteLayer;// if NI is true, this info is used.
    bool BlockageNodes;// from .route file
    int NumBlockedLayer;// same as BlockedLayer.size()
    vector<int> BlockedLayer;// List of blocked layer

}; // Cell
