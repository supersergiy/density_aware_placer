#include "main.h"
#include "bin_library.h"

#include <fstream>

// output png file using. the priority is FIT_REALLY_SIZE->FIT_REALLY_ASPECT_RATIO
// if FIT_REALLY_SIZE and FIT_REALLY_ASPECT_RATIO are 0. output size will be "1920 x 1440".
#define FIT_REALLY_SIZE 0 // if true, the png file can be zoom in to see the clearly size.
                          // default is 0.
#define FIT_REALLY_ASPECT_RATIO 1 // if true, the png file will be the same aspect ratio with the design
                                  // default is 1.

void plot_result(string const &plotName, string const &designAuxName, bool plotCell, bool plotMacro,
                 bool plotPseudoMacro, bool plotNI, bool plotBin, bool plotToPng, bool shapeConcern){
// Doing: Plot design's result. (aka. cell center position, and macro, NI pins).
// According to "design.cell_lib.at(i).center.x" and "design.cell_lib.at(i).center.y"
// Rule: Output file to Gnuplot
//       shapeConcern = "plot macro with shapes by shape"
    //ofstream plotFile("plot_result.txt");
    string design_name = designAuxName;// design_name is benchmark name(w/o .aux)
    design_name.erase(design_name.end()-4, design_name.end());// erase ".aux"
    size_t erase_begin=0;
    size_t erase_end=0;
    for(int charCount=design_name.size()-1; charCount>=0; charCount--){
        if(design_name.at(charCount)=='/' || design_name.at(charCount)== '\\'){
            erase_end = (size_t)charCount;
            break;
        }
    }
    design_name.erase(design_name.begin()+erase_begin, design_name.begin()+erase_end+1);

    string fileName(plotName);
    fileName+="_";
    fileName+=design_name;
    string pngName(fileName);
    fileName+=".plt";
    ofstream plotFile(fileName.c_str());
    streambuf* cerr_buffer = cerr.rdbuf();
    cerr.rdbuf(plotFile.rdbuf());

    cerr << "# rectangle number is cell_pos +1" << endl;
    cerr << "# Because object tag have to > zero." << endl;

    if(plotToPng){
    // Doing: plot result to "plotName.png" file
    // Rule: in gnuplot only type "load 'plotName.plt'" is ok!
        if(FIT_REALLY_SIZE){
            cerr << "set terminal png fontscale 10 size " << design.PMapFrame.getBoundary_ur().x;
            cerr << ", " << design.PMapFrame.getBoundary_ur().y << endl;
        }
        else if(FIT_REALLY_ASPECT_RATIO){
            // set height 1000, and calculate width with same aspect ratio
            double Width = design.PMapFrame.getBoundary_ur().x;
            double Height = design.PMapFrame.getBoundary_ur().y;
            cerr << "set terminal png size " << (1000*Width/Height);
            cerr << ", " << 1000 << endl;
        }
        else{
            cerr << "set terminal png size 1920, 1440" << endl;
        }
        cerr << "set output '" << pngName << ".png'" << endl;
    }
    size_t objCount=1;// for output file set object "number" using.
                      // begin with 1(gnuplot rule: the number must > 1).
    if(plotCell){
    // Doing: plot movable cell to file
    // Rule: set object "cell_pos+1" rectangle at "center.x","center.y"
    //       size "Width","Height"
    // Color: blue and transparent ratio = 30%(cover)
        for(size_t cell_pos=0; cell_pos<design.cell_lib.size(); cell_pos++){
            if(design.cell_lib.at(cell_pos).getMove()){
                cerr << "set object " << objCount << " rectangle from ";
                objCount++;
                cerr << design.cell_lib.at(cell_pos).getCenter().x-
                        design.cell_lib.at(cell_pos).getWidth()/2.0 << ",";
                cerr << design.cell_lib.at(cell_pos).getCenter().y-
                        design.cell_lib.at(cell_pos).getHeight()/2.0 << " to ";
                cerr << design.cell_lib.at(cell_pos).getCenter().x+
                        design.cell_lib.at(cell_pos).getWidth()/2.0 << ",";
                cerr << design.cell_lib.at(cell_pos).getCenter().y+
                        design.cell_lib.at(cell_pos).getHeight()/2.0;
                cerr << " fc rgb 'blue' fs transparent solid 0.3";
                cerr << " border rgb 'blue'" << endl;
            }
        }
    }

    if(plotMacro){
    // Doing: plot fixed cell(macro and pins) to file(p.s. not NI pin)
    // Rule: set object "cell_pos+1" rectangle from "llx","lly" to "urx","ury"
    //       if plotMacro is true, shapeConcern can work.
    // Color: gray and behind
        for(size_t cell_pos=0; cell_pos<design.cell_lib.size(); cell_pos++){
            if(design.cell_lib.at(cell_pos).getMove()==false){
                if(design.cell_lib.at(cell_pos).getNI()==false){
                    if(shapeConcern && design.cell_lib.at(cell_pos).getManyShape()){
                    // if shapeConcern is true and this node is many shape type,
                    // then plot this node by many shape type
                        size_t NumShapes = design.cell_lib.at(cell_pos).getContainSize();
                        for(size_t shapeCount=0; shapeCount<NumShapes; shapeCount++){
                            cerr << "set object " << objCount << " rectangle from ";
                            objCount++;
                            cerr << design.cell_lib.at(cell_pos).getContain(shapeCount).ll.x << ",";
                            cerr << design.cell_lib.at(cell_pos).getContain(shapeCount).ll.y << " to ";
                            cerr << design.cell_lib.at(cell_pos).getContain(shapeCount).ll.x+
                                    design.cell_lib.at(cell_pos).getContain(shapeCount).width << ",";
                            cerr << design.cell_lib.at(cell_pos).getContain(shapeCount).ll.y+
                                    design.cell_lib.at(cell_pos).getContain(shapeCount).height;
                            cerr << " behind fc rgb 'gray'" << endl;
                        }
                    }
                    else{
                        cerr << "set object " << objCount << " rectangle from ";
                        objCount++;
                        cerr << design.cell_lib.at(cell_pos).ll.x << ",";
                        cerr << design.cell_lib.at(cell_pos).ll.y << " to ";
                        cerr << design.cell_lib.at(cell_pos).ll.x+
                                design.cell_lib.at(cell_pos).getWidth() << ",";
                        cerr << design.cell_lib.at(cell_pos).ll.y+
                                design.cell_lib.at(cell_pos).getHeight();
                        if(design.cell_lib.at(cell_pos).getManyShape())
                            cerr << " behind fc rgb 'orange'" << endl; // non-rectangle nodes shown by orange
                        else
                            cerr << " behind fc rgb 'gray'" << endl;
                    }
                }
            }
        }
    }

    if(plotPseudoMacro){
    // Doing: plot pseudo macro to file(p.s. not every design exists pseudo macro)
    // Rule: set object "cell_pos+1" rectangle from "llx","lly" to "urx","ury"
    //       if plotMacro is true, shapeConcern can work.
    // Exist: if design's PMapFrame is not rectangle, pseudo macro would exist to fill it
    // Color: black and behind
        for(size_t PMacro_pos=0; PMacro_pos<design.pseudo_macro_lib.size(); PMacro_pos++){
            cerr << "set object " << objCount << " rectangle from ";
            objCount++;
            cerr << design.pseudo_macro_lib.at(PMacro_pos).ll.x << ",";
            cerr << design.pseudo_macro_lib.at(PMacro_pos).ll.y << " to ";
            cerr << design.pseudo_macro_lib.at(PMacro_pos).ur.x << ",";
            cerr << design.pseudo_macro_lib.at(PMacro_pos).ur.y;
            cerr << " behind fc rgb 'black'" << endl;
        }
    }

    if(plotNI){
    // Doing: plot fixed cell(NI pin) to file
    // Rule: set object "cell_pos+1" rectangle from "llx","lly" to "urx","ury"
    // Color: 'white' and transparent
        for(size_t cell_pos=0; cell_pos<design.cell_lib.size(); cell_pos++){
            if(design.cell_lib.at(cell_pos).getMove()==false){
                if(design.cell_lib.at(cell_pos).getNI()){
                    cerr << "set object " << objCount << " rectangle from ";
                    objCount++;
                    cerr << design.cell_lib.at(cell_pos).ll.x << ",";
                    cerr << design.cell_lib.at(cell_pos).ll.y << " to ";
                    cerr << design.cell_lib.at(cell_pos).ll.x+
                            design.cell_lib.at(cell_pos).getWidth() << ",";
                    cerr << design.cell_lib.at(cell_pos).ll.y+
                            design.cell_lib.at(cell_pos).getHeight();
                    cerr << " fc rgb 'white' fs transparent solid 0" << endl;
                }
            }
        }
    }

    if(plotBin){
    // Doing: plot bin line
    // Rule: set arrow "lineCount+1" from x1,y1 to x2,y2 nohead lt 8 pt 5 lw 0.1
    // Color: lt 8 (brown)
        size_t lineCount=0;
        for(; lineCount<(size_t)bin_lib2.getNumBinX() - 1; lineCount++){
        // Vertical grid line
            cerr << "set arrow " << lineCount+1 << " from ";
            cerr << (lineCount+1)*(size_t)bin_lib2.getBinWidth() << ",0 to ";
            cerr << (lineCount+1)*(size_t)bin_lib2.getBinWidth() << ",";
            cerr << bin_lib2.getNumBinY()*bin_lib2.getBinHeight();
            cerr << " nohead lt 8 pt 5 lw 0.1" << endl;
        }
        for(size_t hLCount=0; hLCount<(size_t)bin_lib2.getNumBinY() - 1; hLCount++){
            cerr << "set arrow " << lineCount+1 << " from ";
            cerr << "0," <<  (hLCount+1)*(size_t)bin_lib2.getBinHeight() << " to ";
            cerr << bin_lib2.getNumBinX()*bin_lib2.getBinWidth() << ",";
            cerr << (hLCount+1)*(size_t)bin_lib2.getBinHeight();
            cerr << " nohead lt 8 pt 5 lw 0.1" << endl;
            lineCount++;
        }
    }

    cerr << "plot [0:" << design.PMapFrame.getBoundary_ur().x;
    cerr << "][0:" << design.PMapFrame.getBoundary_ur().y << "] 15 notitle" << endl;

    if(plotToPng){
        cerr << "unset output" << endl;
    }

    cerr.rdbuf(cerr_buffer);
}


void quick_plot(string plotName, bool plotCell, bool plotMacro,
                bool plotNI, bool plotBin, bool plotToPng, bool shapeConcern)
{
// Doing: Plot design's result very quickly. (aka. cell center position, and macro, NI pins).
// According to "design.cell_lib.at(i).center.x" and "design.cell_lib.at(i).center.y"
// Rule: Output file to Gnuplot
//       shapeConcern = "plot macro with shapes by shape"
    vector<vector<size_t> > plotMap;
    string fileName(plotName);
    fileName+=".plt";
    ofstream plotFile(fileName.c_str());
    streambuf* cerr_buffer = cerr.rdbuf();
    cerr.rdbuf(plotFile.rdbuf());

    const size_t unitGrid = design.Row_Info.at(0).getHeight();
    const size_t plotMap_width = design.PMapFrame.getBoundary_ur().x / unitGrid;
    const size_t plotMap_Height = design.PMapFrame.getBoundary_ur().y / unitGrid;
    const size_t c_blue = 5;
    const size_t c_grey = 10;

    cerr << "# rectangle number is cell_pos +1" << endl;
    cerr << "# Because object tag have to > zero." << endl;
    cerr << "set pm3d map corners2color c1" << endl;
    cerr << "set cbrange [0:10]" << endl;   // color range
    cerr << "unset key" << endl;
    cerr << "unset xtics" << endl;
    cerr << "unset ytics" << endl;
    //cerr << "set cbtics 0, 10, 115" << endl;
    cerr << "unset colorbox" << endl;   // remove color bar
    cerr << "set palette defined ( 0 \"white\", 5 \"navy\", 10 \"grey\" )" << endl;
    //10 \"forest-green\", 20 \"green\", 30 \"dark-yellow\", 40 \"yellow\" ,50 \"gold\", 60 \"orange\", 70 \"red\", 80 \"dark-pink\", 90 \"light-magenta\", 100 \"purple\" )" << endl;

    plotMap.resize(plotMap_width, vector<size_t>(plotMap_Height, 0));

    if(plotToPng) {
    // Doing: plot result to "plotName.png" file
    // Rule: in gnuplot only type "load 'plotName.plt'" is ok!
        cerr << "set terminal png crop truecolo fontscale 2 size "
             << plotMap_width << ", " << plotMap_Height << endl;
        cerr << "set output '" << plotName << ".png'" << endl;
    } // if

    if(plotCell){
    // Doing: plot movable cell to file
    // Rule: set object "cell_pos" rectangle at "center.x","center.y"
    //       size "Width","Height"
    // Color: blue
        size_t cell_x_left = 0;
        size_t cell_x_right = 0;
        size_t cell_y_lower = 0;
        size_t cell_y_upper = 0;
        for(size_t cell_pos=0; cell_pos < design.cell_lib.size(); cell_pos++){
            if(design.cell_lib.at(cell_pos).getMove()){
                cell_x_left = (size_t)(design.cell_lib.at(cell_pos).getCenter().x-design.cell_lib.at(cell_pos).getWidth()/2.0) / unitGrid;
                cell_x_right = (size_t)(design.cell_lib.at(cell_pos).getCenter().x+design.cell_lib.at(cell_pos).getWidth()/2.0) / unitGrid;
                cell_y_lower = (size_t)(design.cell_lib.at(cell_pos).getCenter().y-design.cell_lib.at(cell_pos).getHeight()/2.0) / unitGrid;
                cell_y_upper = (size_t)(design.cell_lib.at(cell_pos).getCenter().y+design.cell_lib.at(cell_pos).getHeight()/2.0) / unitGrid;
                if(cell_x_left < 0)
                    cell_x_left = 0;
                if(cell_x_right >= plotMap_width)
                    cell_x_right = plotMap_width - 1;
                if(cell_y_lower < 0)
                    cell_y_lower = 0;
                if(cell_y_upper >= plotMap_Height)
                    cell_y_upper = plotMap_Height - 1;
                for(size_t cell_x = cell_x_left; cell_x <= cell_x_right; cell_x++) {
                    for(size_t cell_y = cell_y_lower; cell_y <= cell_y_upper; cell_y++) {
                        plotMap.at(cell_x).at(cell_y) = c_blue;
                    } // for
                } // for
            } // if
        } // for
    } // if

    if(plotMacro){
    // Doing: plot fixed cell(macro and pins) to file(p.s. not NI pin)
    // Rule: set object "cell_pos" rectangle from "llx","lly" to "urx","ury"
    //       if plotMacro is true, shapeConcern can work.
    // Color: gray and behind
        for(size_t cell_pos = 0; cell_pos < design.cell_lib.size(); cell_pos++){
            if(design.cell_lib.at(cell_pos).getMove() == false){
                if(design.cell_lib.at(cell_pos).getNI()==false){
                    if(shapeConcern && design.cell_lib.at(cell_pos).getManyShape()){
                    // if shapeConcern is true and this node is many shape type,
                    // then plot this node by many shape type
                        size_t NumShapes = design.cell_lib.at(cell_pos).getContainSize();
                        for(size_t shapeCount=0; shapeCount<NumShapes; shapeCount++){
                            size_t macro_x_left = (size_t)design.cell_lib.at(cell_pos).getContain(shapeCount).ll.x / unitGrid;
                            size_t macro_x_right = (size_t)(design.cell_lib.at(cell_pos).getContain(shapeCount).ll.x
                                                            + design.cell_lib.at(cell_pos).getContain(shapeCount).width) / unitGrid;
                            size_t macro_y_lower = (size_t)design.cell_lib.at(cell_pos).getContain(shapeCount).ll.y / unitGrid;
                            size_t macro_y_upper = (size_t)(design.cell_lib.at(cell_pos).getContain(shapeCount).ll.y
                                                            + design.cell_lib.at(cell_pos).getContain(shapeCount).height) / unitGrid;
                            if(macro_x_left < 0)
                                macro_x_left = 0;
                            if(macro_x_right >= plotMap_width)
                                macro_x_right = plotMap_width - 1;
                            if(macro_y_lower < 0)
                                macro_y_lower = 0;
                            if(macro_y_upper >= plotMap_Height)
                                macro_y_upper = plotMap_Height - 1;
                            for(size_t macro_x = macro_x_left; macro_x <= macro_x_right; macro_x++) {
                                for(size_t macro_y = macro_y_lower; macro_y <= macro_y_upper; macro_y++) {
                                    if(plotMap.at(macro_x).at(macro_y) == 0)
                                        plotMap.at(macro_x).at(macro_y) = c_grey;
                                } // for
                            } // for
                        } // for
                    } else {
                        size_t macro_x_left = (size_t)(design.cell_lib.at(cell_pos).getCenter().x-design.cell_lib.at(cell_pos).getWidth()/2.0) / unitGrid;
                        size_t macro_x_right = (size_t)(design.cell_lib.at(cell_pos).getCenter().x+design.cell_lib.at(cell_pos).getWidth()/2.0) / unitGrid;
                        size_t macro_y_lower = (size_t)(design.cell_lib.at(cell_pos).getCenter().y-design.cell_lib.at(cell_pos).getHeight()/2.0) / unitGrid;
                        size_t macro_y_upper = (size_t)(design.cell_lib.at(cell_pos).getCenter().y+design.cell_lib.at(cell_pos).getHeight()/2.0) / unitGrid;
                        if(macro_x_left < 0)
                            macro_x_left = 0;
                        if(macro_x_right >= plotMap_width)
                            macro_x_right = plotMap_width - 1;
                        if(macro_y_lower < 0)
                            macro_y_lower = 0;
                        if(macro_y_upper >= plotMap_Height)
                            macro_y_upper = plotMap_Height - 1;
                        for(size_t macro_x = macro_x_left; macro_x <= macro_x_right; macro_x++) {
                            for(size_t macro_y = macro_y_lower; macro_y <= macro_y_upper; macro_y++) {
                                if(plotMap.at(macro_x).at(macro_y) == 0)
                                    plotMap.at(macro_x).at(macro_y) = c_grey;
                            } // for
                        } // for
                    } // if
                } // if
            } // if
        } // for
    } // if

    cerr << "splot [0:" << plotMap_width;
    cerr << "][0:" << plotMap_Height << "] '-' matrix" << endl;

    for(size_t plotMap_y = 0; plotMap_y < plotMap_Height; plotMap_y++) {
        for(size_t plotMap_x = 0; (int)plotMap_x < plotMap_width; plotMap_x++) {
            cerr << plotMap.at(plotMap_x).at(plotMap_y) << " ";
        } // for
        cerr << endl;
    } // for
    cerr << "e" << endl << "e" << endl;

    if(plotToPng){
        cerr << "unset output" << endl;
    } // if

    cerr.rdbuf(cerr_buffer);
    plotFile.close();


    system("gnuplot sergiy_bitch.plt");
    cout << "Quick plot finished!" << endl;

    return;
} // quick_plot
