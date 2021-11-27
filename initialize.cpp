#include "initialize.h"
#include "pretreatment.h"
#include "utilities.h"

extern double hpwl;

void initializeNets(void)
{
	for (int i = 0; i < design.net_lib.size(); i++){
		design.net_lib[i].initializeNet();
		design.net_lib[i].initialOneCellNet();
	}
}

void initializeBins(void)
{
	bin_lib.buildBinLibrary(3, true, 4);
	bin_lib2.buildBinLibrary(9, false);
}

void initializeDesign(void)
{
	initializeShapes();
	initializeBins();
	initializeNets();
	initialSubRow();
	hpwl = findTotalHPWL();
	calculatePenalty();
}

void sortCellPins(void)
{
	for (int i = 0; i < design.cell_lib.size(); i++){
		sort(design.cell_lib.at(i).pins.begin(), design.cell_lib.at(i).pins.begin());
	}
}

void initializeShapes(void)
{
	Shape curr_shape;
	Cell new_cell;
	Coor center;

	design.original_number_of_cells = design.cell_lib.size();

	for (int i = 0; i < design.original_number_of_cells; i++){
		if (design.cell_lib[i].getNumShapes() && !design.cell_lib[i].getNI()){
			for (int j = 0; j < design.cell_lib[i].getContainSize(); j++){
				curr_shape = design.cell_lib[i].getContain(j);

				new_cell.initialObjname(curr_shape.Name);
				new_cell.initialWidth(curr_shape.width);
				new_cell.initialHeight(curr_shape.height);
				new_cell.initialTerminal(true);
				new_cell.initialNI(false);
				new_cell.initialMove(false);
				new_cell.initialManyShape(false);
				new_cell.initialNumShapes(0);
				new_cell.initialArea(curr_shape.width * curr_shape.height);
				new_cell.initialLL(curr_shape.ll);
				center.x = curr_shape.ll.x + curr_shape.width / 2;
				center.y = curr_shape.ll.y + curr_shape.height / 2;

				new_cell.initialCenter(center);

				design.cell_lib.push_back(new_cell);
			}
		}
	}
}
