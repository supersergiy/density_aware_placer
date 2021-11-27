#pragma once

#include "cell_library.h"
#include "bin_library.h"
#include "net_pl.h"
#include "RowInfo.h"
#include <vector>
#include <string>

using namespace std;

class DesignInfo
{
//Bookshelf format
public:
	DesignInfo():NumNodes(0), NumTerminals(0), NumFixedTerminal(0), NumFixedNITerminal(0), NumMovable(0),
	    StandardCellHeight(0.0), NumNets(0), NumPins(0), NumRows(0), NumNonRectangularNodes(0), AvgCellArea(0.0),
	    PlaceImage(0.0), TotalWhiteSpace(0.0), RemainWhiteSpace(0.0), WhiteSpaceRatio(0.0), DesignDensity(0.0) {}


	/**/
	int displacement;
	float target_util;
	string file_name;
	string legal_file_name;

    // .aux file
// for placement infomation

    // from .nodes
    void initialNumNodes(size_t initial) {NumNodes = initial;}
    void initialNumTerminals(size_t initial) {NumTerminals = initial;}
    void initialNumMovable(size_t initial) {NumMovable = initial;}
    size_t getNumNodes() {return NumNodes;}
    size_t getNumTerminals() {return NumTerminals;}
    size_t getNumMovable() {return NumMovable;}
    void initialNumFixedTerminals(size_t initial) {NumFixedTerminal = initial;}
    void initialNumFixedNITerminals(size_t initial) {NumFixedNITerminal = initial;}
    void initialStandardCellHeight(double initial) {StandardCellHeight = initial;}

    size_t getNumFixedTerminal() {return NumFixedTerminal;}
    size_t getNumFixedNITerminal() {return NumFixedNITerminal;}
    double getStandardCellHeight() {return StandardCellHeight;}

    // from .nets
    void initialNumNets(size_t initial) {NumNets = initial;}
    void initialNumPins(size_t initial) {NumPins = initial;}
    size_t getNumNets() {return NumNets;}
    size_t getNumPins() {return NumPins;}

    // from .pl

    // from .scl file
    void initialNumRows(size_t initial) {NumRows = initial;}
    size_t getNumRows() {return NumRows;}

    // from .shapes
    void initialNumNonRectangularNodes(size_t initial){ NumNonRectangularNodes = initial;}
    size_t getNumNonRectangularNodes(){ return NumNonRectangularNodes;}

    int original_number_of_cells;
	CellLibrary cell_lib;
    vector<Net_Pl> net_lib;// net library
    vector<Shape> pseudo_macro_lib;// pseudo macro library. filling placement image which is not rectangle

    vector<RowInfo> Row_Info;
    vector<PlaceMap> PMap;// Place Image Sub Rectangle
    PlaceMap PMapFrame;// Place Image Frame
    coor_type starting_hpwl;

private:

    // .aux file
// for placement infomation

    // from .nodes
    size_t NumNodes;
    size_t NumTerminals;
    size_t NumMovable;// number of movable node = NumNodes - NumTerminals

    // from .nets
    size_t NumNets;
    size_t NumPins;

    // from .scl file
    size_t NumRows;
    // p.s. x-coordinate of right edge of the subrow = SubrowOrigin + NumSites * Sitespacing.

    // from .shapes
    size_t NumNonRectangularNodes;

    double StandardCellHeight;// equals to row height

    // from .pl

// for routing information
    // from .route
//    RouteInfo Route;

    // .wts file (Currently unused in the contest)

    // rough legalization using
    double AvgCellArea; // movable cell average area
    // routability cell bloating using
    double PlaceImage; // total place image.(including whitespace and macro)
    double TotalWhiteSpace;
    double RemainWhiteSpace; // RemainWhiteSpace = TotalWhiteSpace - BloatingArea
    double WhiteSpaceRatio; // the design's whitespace ratio
    double DesignDensity; // equal to totalCellArea/(PlaceImage-totalMacroArea)
    size_t NumFixedTerminal;
    size_t NumFixedNITerminal;
}; // DesignInfo

/* ************************************************** */
// common using
extern DesignInfo design;
