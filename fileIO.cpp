#include "main.h"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <tr1/unordered_map>
#include "net_pl.h"

using namespace std::tr1;

// For debug using
#define FILEIO_TEST_MODE 0 // 1 = on, 0 = off. initial is 0.

// For performance using (default is 1) must be 1.
#define BENCHMARK_WITH_FIRST_VERSION_LINE 1 // not legal comment. initial is 1.
#define STORE_NET_TO_NODE_POINTER 1 // (.nets file) hash speeding, initial is 1.
#define STORE_CELL_TO_NET 1 // store cell connected with which nets, initial is 1.

#define MANY_SHAPE_SPEEDING 1 // combine shape in macro with many shape to several big rectangle.
                              // initial is 1

/* ************************************************** */
// for hash using
class CellNameHashFunction
{
public:
    size_t operator ()(string p_cell_name) const
    {
        string::size_type found_last(0);
        found_last = p_cell_name.find_last_of("/\\");
        if(found_last == string::npos)
            found_last = 0;
        while(!isdigit(p_cell_name.at(found_last)))
            found_last++;
        return atoi(p_cell_name.substr(found_last).c_str());
    } // operator ()
}; // CellNameHashFunction
/* ************************************************** */
string benchmark_first_version_line;
unordered_map<string, int, CellNameHashFunction> cell_table;// cell search table.

/* ************************************************** */
bool readFile(string p_fileName);
inline void map_constructor(vector<Cell> &cell_lib);
inline int map_search(string &cellName);

/* ************************************************** */
bool readFile(string p_fileName)
{
// Doing: Read Input Benchmark File
// Parameters
//      p_fileName: .aux file directory
// Return Value
//      true if read file success
//      false otherwise

    cout << "[fileIO.cpp] Starting Reading Benchmarks..." << endl;

    // calculating file path name
    string input_file_dir = p_fileName;
    string::size_type found_last;
    found_last = input_file_dir.find_last_of("/\\");
    input_file_dir.erase(found_last + 1);

    string line, word, firstword;// buffer for read file
    Cell CellBuffer;// Temp buffer for Read file.
    Net_Pl NetBuffer;

// Read .aux file begin
    // read other input file name
    string aux_file_name = p_fileName + ".aux";
    ifstream fin_aux(aux_file_name.c_str());
    if(fin_aux.fail())
    {
        cerr << "Error:[fileIO.cpp] Cannot open such file \"" << aux_file_name << "\" !" << endl;
        return false;
    } // if
    getline(fin_aux, line);
    istringstream ss_aux(line);
    ss_aux >> word >> word >> word;
    string nodes_file = input_file_dir + word;
    ss_aux >> word;
    string nets_file = input_file_dir + word;
    ss_aux >> word;
    string wts_file = input_file_dir + word;
    ss_aux >> word;
    string pl_file = design.legal_file_name;
    ss_aux >> word;
    string scl_file = input_file_dir + word;
    ss_aux >> word;
    string shapes_file = input_file_dir + word;
/*
    ss_aux >> word;
    string route_file = input_file_dir + word;
*/
/*
    ss_aux >> word;
    string legal_file = input_file_dir + word;
*/
    fin_aux.close();
// Read .aux file end

// Read .nodes file begin
    ifstream fin_nodes(nodes_file.c_str());
    if(fin_nodes.fail())
    {
        cerr << "Error:[fileIO.cpp] Cannot open such file \"" << nodes_file << "\" !" << endl;
        return false;
    } // if
    if(BENCHMARK_WITH_FIRST_VERSION_LINE)
        getline(fin_nodes, line);// Skip the first line. (UCLA nodes 1.0)
    int nodesLineCount = 0;// for .nodes file cell construct count
    int FixedTerminalCount = 0;
    int FixedNITerminalCount = 0;
    istringstream ss_nodes;
    while(getline(fin_nodes, line))
    {
        ss_nodes.clear();
        ss_nodes.str(line);
        //istringstream ss(line);
        while(ss_nodes >> firstword)
        {
            if(firstword == "#")
                break;// Next line
            else if(firstword == "NumNodes")
            {
                ss_nodes >> word >> word;
                design.initialNumNodes(atoi(word.c_str()));
                design.cell_lib.reserve(design.getNumNodes());// Speeding vector construct
                break;// Next line
            } // else if
            else if(firstword == "NumTerminals")
            {
                ss_nodes >> word >> word;
                design.initialNumTerminals(atoi(word.c_str()));
                break;// Next line
            } // else if
            else
            {
                CellBuffer.initialObjname(firstword);
                ss_nodes >> word;// width
                CellBuffer.initialWidth(atof(word.c_str()));
                ss_nodes >> word;// height
                CellBuffer.initialHeight(atof(word.c_str()));
                double area = CellBuffer.getWidth() * CellBuffer.getHeight();
                CellBuffer.initialArea(area);
                CellBuffer.initialMove(true);
                if(nodesLineCount >= (int)(design.getNumNodes() - design.getNumTerminals()))
                {
                    ss_nodes >> word;// "terminal" or "terminal_NI"
                    if(word == "terminal"){
                        CellBuffer.initialTerminal(true);
                        CellBuffer.initialMove(false);
                        FixedTerminalCount++;
                    } // if
                    else if(word == "terminal_NI")
                    {
                    	FixedNITerminalCount++;
                        CellBuffer.initialTerminal(true);
                        CellBuffer.initialNI(true);
                        CellBuffer.initialMove(false);
                    } // else if
                } // if

                design.cell_lib.push_back(CellBuffer);
                CellBuffer.clear();
                nodesLineCount++;
                break;// Next line
            } // else
        } // while
    } // while
    design.initialNumFixedTerminals(FixedTerminalCount);
    design.initialNumFixedNITerminals(FixedNITerminalCount);
    design.initialNumMovable(design.getNumNodes() - design.getNumTerminals());
    design.initialStandardCellHeight(design.cell_lib.at(0).getHeight());
    fin_nodes.close();
    if(STORE_NET_TO_NODE_POINTER)
        map_constructor(design.cell_lib.cell_list);// For net to node speeding
// Read .nodes file end

// Read .nets file begin
    ifstream fin_nets(nets_file.c_str());
    if(fin_nets.fail())
    {
        cerr << "Error:[fileIO.cpp] Cannot open such file \"" << nets_file << "\" !" << endl;
        return false;
    } // if

    if(BENCHMARK_WITH_FIRST_VERSION_LINE)
        getline(fin_nets, line);// Skip the first line. (UCLA nodes 1.0)
    istringstream ss_nets;
    istringstream ss_nets_nodes;
    while(getline(fin_nets, line)){
        ss_nets.clear();
        ss_nets.str(line);
        while(ss_nets >> firstword)
        {
            if(firstword == "#")
                break;// Next line
            else if(firstword == "NumNets")
            {
                ss_nets >> word >> word;
                design.initialNumNets(atoi(word.c_str()));
                design.net_lib.reserve(design.getNumNets());// Speeding vector construct
                break;// Next line
            } // else if
            else if(firstword == "NumPins")
            {
                ss_nets >> word >> word;
                design.initialNumPins(atoi(word.c_str()));
                break;// Next line
            } // else if
            else
            {
                ss_nets >> word >> word;
                NetBuffer.initialNetDegree(atoi(word.c_str()));
                ss_nets >> word;
                NetBuffer.initialNetName(word);
                for(int nodeIter = 0; nodeIter < NetBuffer.getNetDegree(); nodeIter++)
                {
                    getline(fin_nets, line);
                    ss_nets_nodes.clear();
                    ss_nets_nodes.str(line);
                    NetNode tempNetNode;

                    ss_nets_nodes >> word;
                    tempNetNode.NodeName = word;
                    ss_nets_nodes >> word;
                    if(word == "I")
                        tempNetNode.IorO = 1;// 1 means I
                    else
                        tempNetNode.IorO = 0;// 0 means O
                    ss_nets_nodes >> word >> word;
                    tempNetNode.pin_xoffset = atof(word.c_str());
                    ss_nets_nodes >> word;
                    tempNetNode.pin_yoffset = atof(word.c_str());

                    if(STORE_NET_TO_NODE_POINTER)
                    {
						tempNetNode.CellPosition = map_search(tempNetNode.NodeName);// Speeding
                        if(STORE_CELL_TO_NET)
							design.cell_lib.at(tempNetNode.CellPosition).addPin(design.net_lib.size(), tempNetNode.pin_xoffset, tempNetNode.pin_yoffset);
                    } // if

                    NetBuffer.initialcell(tempNetNode);
                } // for
                design.net_lib.push_back(NetBuffer);
                NetBuffer.clear();
                break;// Next line
            } // else
        } // while
    } // while
    fin_nets.close();
// Read .nets file end

// Read .pl file begin
    ifstream fin_pl(pl_file.c_str());
    if(fin_pl.fail())
    {
        cerr << "Error:[fileIO.cpp] Cannot open such file \"" << pl_file << "\" !" << endl;
        return false;
    } // if

    if(BENCHMARK_WITH_FIRST_VERSION_LINE)
    {
        getline(fin_pl, line);// Skip the first line. (UCLA nodes 1.0)
        benchmark_first_version_line = line; // store to output placement result using
    } // if
    int plLineCount = 0;// Count the line number
    istringstream ss_pl;
    while(getline(fin_pl, line)){
        ss_pl.clear();
        ss_pl.str(line);
        while(ss_pl >> firstword){
            if(firstword == "#")
                break;// Next line
            else// assume the order is same as .nodes file
            {
                if(firstword == design.cell_lib.at(plLineCount).getObjName()){
                    ss_pl >> word;
                    design.cell_lib.at(plLineCount).ll.x = atof(word.c_str());
                    design.cell_lib.at(plLineCount).initialCenterX(atof(word.c_str()) + (design.cell_lib.at(plLineCount).getWidth() / 2.0));
                    design.cell_lib.at(plLineCount).initialOriginalCenterX(atof(word.c_str()) + (design.cell_lib.at(plLineCount).getWidth() / 2.0));
                    ss_pl >> word;
                    design.cell_lib.at(plLineCount).ll.y = atof(word.c_str());
                    design.cell_lib.at(plLineCount).initialCenterY(atof(word.c_str()) + (design.cell_lib.at(plLineCount).getHeight() / 2.0));
                    design.cell_lib.at(plLineCount).initialOriginalCenterY(atof(word.c_str()) + (design.cell_lib.at(plLineCount).getHeight() / 2.0));
                } // if
                else
                    cerr << "[fileIO.cpp]Error: the pl file order is not same as nodes file" <<endl;
                plLineCount++;
                break;// Next line
            } // else
        } // while
    } // while
    fin_pl.close();
// Read .pl file end

// Read .scl file begin
    ifstream fin_scl (scl_file.c_str());
    if(fin_scl.fail())
    {
        cerr << "Error:[fileIO.cpp] Cannot open such file \"" << scl_file << "\" !" << endl;
        return false;
    } // if

    if(BENCHMARK_WITH_FIRST_VERSION_LINE)
        getline(fin_scl, line);// Skip the first line. (UCLA nodes 1.0)
    RowInfo RowBuffer;
    istringstream ss_scl;
    while(getline(fin_scl, line))
    {
        ss_scl.clear();
        ss_scl.str(line);
        while(ss_scl >> firstword)
        {
            if(firstword == "#")
                break;// Next line
            else if(firstword == "NumRows")
            {
                ss_scl >> word >> word;
                design.initialNumRows(atoi(word.c_str()));
                design.Row_Info.reserve(design.getNumRows());// Speeding
                break;// Next line
            } // else if
            else if(firstword == "CoreRow")
            {
                ss_scl >> word;// Horizontal
                while(word != "End")
                {
                    fin_scl >> word;
                    if(word == "Coordinate")
                    {
                        fin_scl >> word >> word;// Coor
                        RowBuffer.initialCoor(atof(word.c_str()));
                    } // if
                    if(word == "Height")
                    {
                        fin_scl >> word >> word;// Height
                        RowBuffer.initialHeight(atof(word.c_str()));
                    } // if
                    if(word == "Sitewidth")
                    {
                        fin_scl >> word >> word;// Sitewidth
                        RowBuffer.initialSitewidth(atof(word.c_str()));
                    } // if
                    if(word == "Sitespacing")
                    {
                        fin_scl >> word >> word;// Sitespacing
                        RowBuffer.initialSitespacing(atof(word.c_str()));
                    } // if
                    if(word == "Siteorient")
                    {
                        fin_scl >> word >> word;// Siteorient
                        if(word == "Y")
                            RowBuffer.initialSiteorient(true);
                        else
                            RowBuffer.initialSiteorient(false);
                    } // if
                    if(word == "Sitesymmetry")
                    {
                        fin_scl >> word >> word;// Sitesymmetry
                        if(word == "Y")
                            RowBuffer.initialSitesymmetry(true);
                        else
                            RowBuffer.initialSitesymmetry(false);
                    } // if
                    if(word == "SubrowOrigin")
                    {
                        fin_scl >> word >> word;// SubrowOrigin
                        RowBuffer.initialSubrowOrigin(atof(word.c_str()));
                    } // if
                    if(word == "NumSites")
                    {
                        fin_scl >> word >> word;// NumSites
                        RowBuffer.initialNumSites(atof(word.c_str()));
                    } // if
                } // while
                RowBuffer.initialWidth(RowBuffer.getNumSites() * RowBuffer.getSitewidth());
                RowBuffer.calRESubRow();// calculating this subRow Right-End x Coordinate
                design.Row_Info.push_back(RowBuffer);
                RowBuffer.clear();
            } // else if
        } // while
    } // while
    fin_scl.close();

    // Caculating placement boundary. Related with row ordering.
    // Creating PMap vector(contain many rectangle based on row_info)
    double rowRec_ll_x = design.Row_Info.at(0).getSubrowOrigin();
    double rowRec_ll_y = design.Row_Info.at(0).getCoor();
    double rowRec_ur_x = design.Row_Info.at(0).getRESubRow();
    double rowRec_ur_y = rowRec_ll_y+design.Row_Info.at(0).getHeight();
    for(vector<RowInfo>::iterator rowCount = (design.Row_Info.begin() + 1); rowCount != design.Row_Info.end(); rowCount++)
    {
        if((rowCount->getSubrowOrigin() == rowRec_ll_x) && (rowRec_ur_x == (rowCount->getRESubRow())))
        {
            rowRec_ur_y += rowCount->getHeight();
        }
        else
        {
        // new rectangle
            PlaceMap tempPMap(Coor(rowRec_ll_x, rowRec_ll_y), Coor(rowRec_ur_x, rowRec_ur_y));
            design.PMap.push_back(tempPMap);
            rowRec_ll_x = rowCount->getSubrowOrigin();
            rowRec_ll_y = rowCount->getCoor();
            rowRec_ur_x = rowCount->getRESubRow();
            rowRec_ur_y = rowRec_ll_y + rowCount->getHeight();
        }
        if(rowCount+1 == design.Row_Info.end())
        {
            PlaceMap tempPMap(Coor(rowRec_ll_x, rowRec_ll_y), Coor(rowRec_ur_x, rowRec_ur_y));
            design.PMap.push_back(tempPMap);
        }
    }

    // calculating PMapFrame(the bounding box of all row)
    vector<RowInfo>::iterator RowBegin = design.Row_Info.begin();
    vector<RowInfo>::iterator RowLast = design.Row_Info.end() - 1;
    Coor PMapFrame_ll(RowBegin->getSubrowOrigin(), RowBegin->getCoor());
    Coor PMapFrame_ur(RowBegin->getRESubRow(), RowLast->getCoor()+RowLast->getHeight());
    for(vector<PlaceMap>::iterator PMapIter = design.PMap.begin(); PMapIter != design.PMap.end(); PMapIter++)
    {
        if(PMapFrame_ll.x > (PMapIter->getBoundary_ll().x))
           PMapFrame_ll.x = (PMapIter->getBoundary_ll().x);
        if(PMapFrame_ll.y > (PMapIter->getBoundary_ll().y))
           PMapFrame_ll.y = (PMapIter->getBoundary_ll().y);

        if(PMapFrame_ur.x < (PMapIter->getBoundary_ur().x))
           PMapFrame_ur.x = (PMapIter->getBoundary_ur().x);
        if(PMapFrame_ur.y < (PMapIter->getBoundary_ur().y))
           PMapFrame_ur.y = (PMapIter->getBoundary_ur().y);
    }
    design.PMapFrame.initialBoundary(PMapFrame_ll, PMapFrame_ur);
/*
    if(FILEIO_TEST_MODE)
    {
        cerr << endl << "[FILEIO_TEST_MODE]" << endl;
        for(size_t i = 0; i < design.Row_Info.size(); i++)
        {
            if(design.PMap.getBoundary_ll().x != design.Row_Info.at(i).getSubrowOrigin())
            {
                cerr << "Error:[fileIO.cpp] PMap boundary is not a retangle(llx error)." << endl;
            } // if
            if(design.PMap.getBoundary_ur().x !=
               design.Row_Info.at(i).getSubrowOrigin()+(design.Row_Info.at(i).getNumSites()*design.Row_Info.at(i).getSitespacing()))
            {
                cerr << "Error:[fileIO.cpp] PMap boundary is not a retangle(urx error)." << endl;
            } // if
            if(design.PMap.getBoundary_ll().y > design.Row_Info.at(i).getCoor())
            {
                cerr << "Error:[fileio.cpp] PMap boundary error(lly error)." << endl;
            } // if
            if(design.PMap.getBoundary_ur().y <= design.Row_Info.at(i).getCoor())
            {
                cerr << "Error:[fileio.cpp] PMap boundary error(ury error)." << endl;
            } // if
        } // for
        cout << "PMap ll and ur: (" << design.PMap.getBoundary_ll().x << ", " << design.PMap.getBoundary_ll().y << ")";
        cout << " (" << design.PMap.getBoundary_ur().x << ", " << design.PMap.getBoundary_ur().y << ")" << endl;
    } // if
*/
// Read .scl file end

// Read .shapes file begin
    ifstream fin_shapes(shapes_file.c_str());
    if(fin_shapes.fail())
    {
        cerr << "Error:[fileIO.cpp] Cannot open such file \"" << shapes_file << "\" !" << endl;
        return false;
    } // if

    if(BENCHMARK_WITH_FIRST_VERSION_LINE)
        getline(fin_shapes, line);// Skip the first line. (UCLA nodes 1.0)
    vector<Cell>::iterator tempCellPos = design.cell_lib.begin();// For speeding
    Coor merge_ll; // For ManyShape speeding
    double merge_width=0.0, merge_height=0.0; // For ManyShape speeding
    string merge_from, merge_end;
    istringstream ss_shapes;
    istringstream ss_shapes_rec;
    while(getline(fin_shapes, line))
    {
        ss_shapes.clear();
        ss_shapes.str(line);
        while(ss_shapes >> firstword)
        {
            if(firstword == "#")
                break;// Next line
            else if(firstword == "NumNonRectangularNodes")
            {
                ss_shapes >> word >> word;
                design.initialNumNonRectangularNodes(atoi(word.c_str()));
                break;// Next line
            } // else if
            else
            {// Assume the shapes file order is same as nodes file's order
                for(; ;tempCellPos++)
                {
                    if(tempCellPos == design.cell_lib.end())
                        tempCellPos = design.cell_lib.begin();
                    else if(firstword == tempCellPos->getObjName())
                        break;
                } // for
                tempCellPos->initialManyShape(true);
                ss_shapes >> word >> word;// number of this node's shape
                tempCellPos->initialNumShapes(atoi(word.c_str()));
                for(int i = 0; i < tempCellPos->getNumShapes(); i++)
                {
                    Shape ShapeBuffer;
                    getline(fin_shapes, line);
                    ss_shapes_rec.clear();
                    ss_shapes_rec.str(line);
                    ss_shapes_rec >> word;
                    ShapeBuffer.Name = word;
                    ss_shapes_rec >> word;
                    ShapeBuffer.ll.x = atof(word.c_str());
                    ss_shapes_rec >> word;
                    ShapeBuffer.ll.y = atof(word.c_str());
                    ss_shapes_rec >> word;
                    ShapeBuffer.width = atof(word.c_str());
                    ss_shapes_rec >> word;
                    ShapeBuffer.height = atof(word.c_str());

                    if(MANY_SHAPE_SPEEDING == false)
                    {
                        ShapeBuffer.calculate_ur();
                        tempCellPos->initialContain(ShapeBuffer);
                    } // if
                    else
                    {
                    // merge many shape to one big rectangle
                        if(i == tempCellPos->getNumShapes() - 1)
                        {
                        // means last shape
                            if(ShapeBuffer.width == merge_width && ShapeBuffer.ll.x == merge_ll.x)
                            {
                            // last merge rectangle
                                merge_from += ShapeBuffer.Name;
                                merge_height += ShapeBuffer.height;
                                Shape MergeBuffer(merge_ll, merge_width, merge_height);
                                MergeBuffer.Name = merge_from;
                                tempCellPos->initialContain(MergeBuffer);
                            } // if
                            else
                            {
                            // last shape is last rectangle
                                // last rectangle
                                Shape MergeBuffer(merge_ll, merge_width, merge_height);
                                merge_from += "last-1_shape";
                                MergeBuffer.Name = merge_from;
                                tempCellPos->initialContain(MergeBuffer);
                                // last shape is last rectangle
                                ShapeBuffer.calculate_ur();
                                tempCellPos->initialContain(ShapeBuffer);
                            } // else
                        } // if
                        else if((i == 0) || (ShapeBuffer.ll.x != merge_ll.x) || (ShapeBuffer.width != merge_width))
                        {
                        // means not last shape and the shape is not in merge rectangle
                            // store merge rectangle
                            if(i != 0)
                            {
                                merge_from += merge_end;
                                Shape MergeBuffer(merge_ll, merge_width, merge_height);
                                MergeBuffer.Name = merge_from;
                                tempCellPos->initialContain(MergeBuffer);
                            } // if
                            // record new rectangle
                            merge_from = ShapeBuffer.Name;
                            merge_from += "_To_";
                            merge_width = ShapeBuffer.width;
                            merge_height = ShapeBuffer.height;
                            merge_ll.x = ShapeBuffer.ll.x;
                            merge_ll.y = ShapeBuffer.ll.y;
                        } // else if
                        else
                        {
                        // means not last shape and the shape is in merge rectangle
                            merge_height += ShapeBuffer.height;
                            merge_end = ShapeBuffer.Name;
                        } // else
                    } // else
                } // for
            } // else
        } // while
    } // while
    fin_shapes.close();
// Read .shapes file end
/*
// Read .route file begin
    ifstream fin_route(route_file.c_str());
    if(fin_route.fail())
    {
        cerr << "Error:[fileIO.cpp] Cannot open such file \"" << route_file << "\" !" << endl;
        return false;
    } // if

    if(BENCHMARK_WITH_FIRST_VERSION_LINE)
        getline(fin_route, line);// Skip the first line. (route 1.0)
    RouteInfo RouteBuffer;
    istringstream ss_route;
    while(getline(fin_route, line))
    {
        ss_route.clear();
        ss_route.str(line);
        while(ss_route >> firstword)
        {
            if(firstword == "#")
                break;// Next line
            else if(firstword == "Grid")
            {
                ss_route >> word >> word;
                RouteBuffer.num_x_grids = atoi(word.c_str());
                ss_route >> word;
                RouteBuffer.num_y_grids = atoi(word.c_str());
                ss_route >> word;
                RouteBuffer.num_layers = atoi(word.c_str());
                RouteBuffer.route_layer.resize(RouteBuffer.num_layers);
                // Above line is building size for initial.
                break;// Next line
            } // else if
            else if(firstword == "VerticalCapacity")
            {
                ss_route >> word;// ":"
                for(int i = 0; i < RouteBuffer.num_layers; i++)
                {
                    ss_route >> word;
                    RouteBuffer.route_layer.at(i).VerticalCapacity = atoi(word.c_str());
                } // for
                break;// Next line
            } // else
            else if(firstword == "HorizontalCapacity")
            {
                ss_route >> word;// ":"
                for(int i = 0; i < RouteBuffer.num_layers; i++)
                {
                    ss_route >> word;
                    RouteBuffer.route_layer.at(i).HorizontalCapacity = atoi(word.c_str());
                } // for
                break;// Next line
            } // else if
            else if(firstword == "MinWireWidth")
            {
                ss_route >> word;// ":"
                for(int i = 0; i < RouteBuffer.num_layers; i++)
                {
                    ss_route >> word;
                    RouteBuffer.route_layer.at(i).MinWireWidth = atoi(word.c_str());
                } // for
                break;// Next line
            } // else if
            else if(firstword == "MinWireSpacing")
            {
                ss_route >> word;// ":"
                for(int i = 0; i < RouteBuffer.num_layers; i++)
                {
                    ss_route >> word;
                    RouteBuffer.route_layer.at(i).MinWireSpacing = atoi(word.c_str());
                } // for
                break;// Next line
            } // else if
            else if(firstword == "ViaSpacing")
            {
                ss_route >> word;// ":"
                for(int i = 0; i < RouteBuffer.num_layers; i++)
                {
                    ss_route >> word;
                    RouteBuffer.route_layer.at(i).ViaSpacing = atoi(word.c_str());
                } // for
                break;// Next line
            } // else if
            else if(firstword == "GridOrigin")
            {
                ss_route >> word >> word;
                RouteBuffer.grid_lowerleft_x = atoi(word.c_str());
                ss_route >> word;
                RouteBuffer.grid_lowerleft_y = atoi(word.c_str());
                break;// Next line
            } // else if
            else if(firstword == "TileSize")
            {
                ss_route >> word >> word;
                RouteBuffer.tile_width = atoi(word.c_str());
                ss_route >> word;
                RouteBuffer.tile_height = atoi(word.c_str());
                break;// Next line
            } // else if
            else if(firstword == "BlockagePorosity")
            {
                ss_route >> word >> word;
                RouteBuffer.BlockagePorosity = atoi(word.c_str());
                break;// Next line
            } // else if
            else if(firstword == "NumNiTerminals")
            {
                ss_route >> word >> word;
                RouteBuffer.NumNiTerminals = atoi(word.c_str());
                for(size_t i = design.getNumNodes() - design.getNumTerminals(); i < design.getNumNodes(); i++)
                {
                    if(design.cell_lib.at(i).getNI())
                    {
                        fin_route >> word >> word;
                        design.cell_lib.at(i).initialNIRouteLayer(atoi(word.c_str()));
                    } // if
                } // for
            } // else if
            else if(firstword == "NumBlockageNodes")
            {
                ss_route >> word >> word;
                RouteBuffer.NumBlockageNodes = atoi(word.c_str());
                fin_route >> word;// First block name
                int blockCount = 0;
                for(size_t i = design.getNumNodes() - design.getNumTerminals(); i < design.getNumNodes(); i++)
                {
                    if(design.cell_lib.at(i).getObjName() == word)
                    {
                        vector<int> LayerListBuffer;
                        design.cell_lib.at(i).initialBlockageNodes(true);
                        fin_route >> word;
                        design.cell_lib.at(i).initialNumBlockedLayer(atoi(word.c_str()));
                        for(int j = 0; j < design.cell_lib.at(i).getNumBlockedLayer(); j++)
                        {
                            fin_route >> word;
                            LayerListBuffer.push_back(atoi(word.c_str()));
                        } // for
                        design.cell_lib.at(i).initialBlockedLayer(LayerListBuffer);
                        blockCount++;
                        if(blockCount<RouteBuffer.NumBlockageNodes)
                            fin_route >> word;// Next block name
                    } // if
                } // for
            } // else if
            else if(firstword == "NumEdgeCapacityAdjustments")
            {

            } // else if
        } // while
    } // while
    design.initialRouteInfo(RouteBuffer);
    fin_route.close();
*/
/*
    // Placement Image(PMap) is not same as Global Router Image(Grid boundary)
    if(FILEIO_TEST_MODE){
        if(design.PMap.getBoundary_ur().x != design.PMap.getBoundary_ll().x+((design.getRoute().num_x_grids)*(design.getRoute().tile_width))){
            cerr << "Error:[fileio.cpp] Grid tile width error!" << endl;
            cerr << "  PMap_ll_x + TileWidth*num_x_grid: ";
            cerr << design.PMap.getBoundary_ll().x+((design.getRoute().num_x_grids)*(design.getRoute().tile_width)) << endl;
        }
        if(design.PMap.getBoundary_ur().y != design.PMap.getBoundary_ll().y+((design.getRoute().num_y_grids)*(design.getRoute().tile_height))){
            cerr << "Error:[fileio.cpp] Grid tile height error!" << endl;
            cerr << "  PMap_ll_y + TileHeight*num_y_grid: ";
            cerr << design.PMap.getBoundary_ll().y+((design.getRoute().num_y_grids)*(design.getRoute().tile_height)) << endl;
        }
    }
*/
// Read .route file end
/*
// Read .legal file begin
    ifstream fin_legal(legal_file.c_str());
    if(fin_legal.fail())
    {
        cerr << "Error:[fileIO.cpp] Cannot open such file \"" << pl_file << "\" !" << endl;
        return false;
    } // if

    if(BENCHMARK_WITH_FIRST_VERSION_LINE)
        getline(fin_legal, line);// Skip the first line. (UCLA nodes 1.0)
    int plLineCount = 0;// Count the line number
    istringstream ss_legal;
    while(getline(fin_legal, line)){
        ss_legal.clear();
        ss_legal.str(line);
        while(ss_legal >> firstword){
            if(firstword == "#")
                break;// Next line
            else// assume the order is same as .nodes file
            {
                if(firstword == design.cell_lib.at(plLineCount).getObjName()){
                    ss_legal >> word;
                    design.cell_lib.at(plLineCount).ll.x = atof(word.c_str());
                    design.cell_lib.at(plLineCount).center.x =
                            atof(word.c_str()) + (design.cell_lib.at(plLineCount).getWidth() / 2.0);
                    ss_legal >> word;
                    design.cell_lib.at(plLineCount).ll.y = atof(word.c_str());
                    design.cell_lib.at(plLineCount).center.y =
                            atof(word.c_str()) + (design.cell_lib.at(plLineCount).getHeight() / 2.0);
                } // if
                else
                    cerr << "[fileIO.cpp]Error: the legal file order is not same as nodes file" <<endl;
                plLineCount++;
                break;// Next line
            } // else
        } // while
    } // while
    fin_legal.close();
// Read .legal file end
*/
    cout << "Complete!" << endl;


	// dump information
	// .nodes
	cout << "Total Nodes                 : " << design.getNumNodes() << endl;
	cout << "Terminal Nodes              : " << design.getNumTerminals() << endl;
	cout << "Fixed Terminal Nodes        : " << design.getNumFixedTerminal() << endl;
	cout << "Fixed_NI Terminal Nodes     : " << design.getNumFixedNITerminal() << endl;
	// .nets
	cout << "Total Nets                  : " << design.getNumNets() << endl;
	cout << "Total Pins                  : " << design.getNumPins() << endl;
	// .scl
	cout << "Total Rows                  : " << design.getNumRows() << endl;
	// .shape
	cout << "Total Non-Rectangular Nodes : " << design.getNumNonRectangularNodes() << endl;

	/**** to do
	// assume cell order is  movable -> terminal -> terminal NI
	// assume row is consecutive
	// assume macro is row structure

	*/


// [Test] Part
    if(FILEIO_TEST_MODE){
        test_nodesFile(nodes_file);
        test_netsFile(nets_file);
        test_plFile(pl_file);
        test_sclFile(scl_file);
        test_shapesFile(shapes_file);
        //test_routeFile(route_file);
    } // if

    return true;
} // readFile
/* ************************************************** */
void WritePlaceResult(string fileName, DesignInfo &design)
{
// Doing: Write legalized placement result in Banana_"FileName".pl

    /*string input_name = FileName;// input_name is benchmark name(w/o .aux)
    input_name.erase(input_name.end()-4, input_name.end());// erase ".aux"
    size_t erase_begin=0;
    size_t erase_end=0;
    for(int charCount=input_name.size()-1; charCount>=0; charCount--)
    {
        if(input_name.at(charCount)=='/' || input_name.at(charCount)== '\\')
        {
            erase_end = (size_t)charCount;
            break;
        } // if
    } // for
    input_name.erase(input_name.begin()+erase_begin, input_name.begin()+erase_end+1);*/

    string output_name(fileName + "-cada040.pl");
    ofstream Rout_pl(output_name.c_str());
    if(BENCHMARK_WITH_FIRST_VERSION_LINE)
    {
        Rout_pl << benchmark_first_version_line << endl << endl;
    } // if
    Rout_pl << fixed << setprecision(2); // set same syntax in precision to 2
    for(size_t i=0; i<design.original_number_of_cells; i++)
    {
        Rout_pl << "             " << design.cell_lib.at(i).getObjName();

        Rout_pl << "             " << design.cell_lib.at(i).ll.x;
        Rout_pl << "             " << design.cell_lib.at(i).ll.y;

        Rout_pl << "    :  ";
        if(design.cell_lib.at(i).getOrientation())
            Rout_pl << "Y";
        else
            Rout_pl << "N";
        if(design.cell_lib.at(i).getMove()==false)
            Rout_pl << "  " << "/FIXED";
        if(design.cell_lib.at(i).getNI())
            Rout_pl << "_NI";
        Rout_pl << endl;
    } // for
    Rout_pl.precision(6); // back to default
    Rout_pl.close();
    cout << "Producing placement result in '"<< output_name << "'...";
    cout << "done!" << endl;
} // WritePlaceResult
/* ************************************************** */
inline void map_constructor(vector<Cell> &cell_lib)
{
    cell_table.rehash(cell_lib.size()); // for speeding
    for(size_t i = 0; i < cell_lib.size(); i++)
    {
        cell_table.insert(make_pair(cell_lib.at(i).getObjName(), i));
    } // for
} // map_constructor
/* ************************************************** */
inline int map_search(string &cellName)
{// giving cell name return cell_lib position
    return cell_table.find(cellName)->second;
} // map_search
/* ************************************************** */
