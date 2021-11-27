#ifndef PLOT_RESULT_H
#define PLOT_RESULT_H

#include <string>

using namespace std;

void plot_result(string const &plotName, string const &designAuxName, bool plotCell, bool plotMacro,
                 bool plotPseudoMacro, bool plotNI, bool plotBin, bool plotToPng, bool shapeConcern);

void quick_plot(string plotName, bool plotCell, bool plotMacro,
                bool plotNI, bool plotBin, bool plotToPng, bool shapeConcern);

#endif // PLOT_RESULT_H
