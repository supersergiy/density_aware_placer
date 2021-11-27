#include "SubRow.h"
#include "pretreatment.h"
#include "cell_library.h"
#include "cell.h"
#include "RowInfo.h"
#include "design_info.h"
#include <algorithm>
#include <iostream>

// for debug using
#define SUBROW_CUT_TEST_MODE 0 // 1 = on, 0 = off. initial is 0.
#define SUBROW_SORTED_TEST_MODE 0 // 1 = on, 0 = off. initial is 0.
#define ALL_CELL_IN_SUBROW 0 // 1 = on, 0 = off. initial is 0.
/* ************************************************** */
inline bool macroOverlapWithArea(size_t rowIndex, Coor macro_ll, Coor macro_ur);
inline bool compareCell_lx(size_t p_pos_1, size_t p_pos_2);
/* ************************************************** */
void initialSubRow()
{
// start from NI
    cout << "[pretreatment.cpp] initial subrow start" << endl;

    // assign macro to row
    for(int cell_iter = design.getNumMovable(); cell_iter < (int)design.cell_lib.size(); cell_iter++)
    {
        if(design.cell_lib.at(cell_iter).getNI() == false)
        {
#pragma omp parallel for num_threads(NUM_THREAD)
            for(int row_iter = 0; row_iter < (int)design.Row_Info.size(); row_iter++)
            {
                if(macroOverlapWithArea(row_iter, design.cell_lib.at(cell_iter).ll, design.cell_lib.at(cell_iter).getUR()))
                {
                    design.Row_Info.at(row_iter).AddMacro(cell_iter);
                } // if
            } // for
        } // if
        else
            break;
    } // for

    // cut row
    Coor row_ll;
    SubRow row_subrow;
    size_t macro_pos(0);
    coor_type cut_left_x(-1.0);
    coor_type cut_right_x(-1.0);
    SubRow new_subrow;
    for(int row_iter = 0; row_iter < (int)design.Row_Info.size(); row_iter++)
    {
        row_ll.setCoor(design.Row_Info.at(row_iter).getSubrowOrigin(), design.Row_Info.at(row_iter).getCoor());
        row_subrow.setBoundary(row_ll, design.Row_Info.at(row_iter).getWidth(), design.Row_Info.at(row_iter).getHeight());
        design.Row_Info.at(row_iter).addSubRow(row_subrow);

        //if(design.Row_Info.at(row_iter).getMacroNum() == 0)
        //    continue;

        for(int macro_iter = 0; macro_iter < (int)design.Row_Info.at(row_iter).getMacroNum(); macro_iter++)
        {
            macro_pos = design.Row_Info.at(row_iter).getMacro(macro_iter);
            cut_left_x = -1.0;
            cut_right_x = -1.0;
            // assign cut position
            if(design.cell_lib.at(macro_pos).getManyShape() == true)
            {
            // macro is "shape"
                for(int shape_iter = 0; shape_iter < (int)design.cell_lib.at(macro_pos).getContainSize(); shape_iter++)
                {
                    Shape macro_contain = design.cell_lib.at(macro_pos).getContain(shape_iter);
                    if(macroOverlapWithArea(row_iter, macro_contain.ll, macro_contain.ur))
                    {
                        if(cut_left_x == -1 || macro_contain.ll.x < cut_left_x)
                            cut_left_x = macro_contain.ll.x;
                        if(cut_right_x == -1 || macro_contain.ur.x > cut_right_x)
                            cut_right_x = macro_contain.ur.x;
                    } // if
                } // for
            } // if
            else
            {
            // macro isn't "shape"
                cut_left_x = design.cell_lib.at(macro_pos).ll.x;
                cut_right_x = design.cell_lib.at(macro_pos).ll.x + design.cell_lib.at(macro_pos).getWidth();
            } // else

            // cut subrow
            for(int subrow_iter = 0; subrow_iter < (int)design.Row_Info.at(row_iter).getSubRowNum(); subrow_iter++)
            {

                if((cut_left_x > design.Row_Info.at(row_iter).getSubRow(subrow_iter).get_lx()) &&
                   (cut_right_x < design.Row_Info.at(row_iter).getSubRow(subrow_iter).get_rx()))
                {
                // shape cover the middle of subrow
                    // add new sub row
                    new_subrow.setBoundary(Coor(cut_right_x, design.Row_Info.at(row_iter).getCoor()),
                                           design.Row_Info.at(row_iter).getSubRow(subrow_iter).get_rx() - cut_right_x,
                                           design.Row_Info.at(row_iter).getHeight());
                    design.Row_Info.at(row_iter).addSubRow(new_subrow);
                    // update cut subrow
                    design.Row_Info.at(row_iter).getSubRow(subrow_iter)
                        .setWidth(cut_left_x - design.Row_Info.at(row_iter).getSubRow(subrow_iter).get_lx());
                } // if
                else if((cut_left_x <= design.Row_Info.at(row_iter).getSubRow(subrow_iter).get_lx()) &&
                        (cut_right_x >= design.Row_Info.at(row_iter).getSubRow(subrow_iter).get_rx()))
                {
                // shape cover all this subrow
                    design.Row_Info.at(row_iter).eraseSubRow(subrow_iter);
                } // else if
                else if((cut_left_x <= design.Row_Info.at(row_iter).getSubRow(subrow_iter).get_lx()) &&
                        (cut_right_x >= design.Row_Info.at(row_iter).getSubRow(subrow_iter).get_lx()) &&
                        (cut_right_x < design.Row_Info.at(row_iter).getSubRow(subrow_iter).get_rx()))
                {
                // shape cover left edge of subrow
                    design.Row_Info.at(row_iter).getSubRow(subrow_iter)
                        .setBoundary(Coor(cut_right_x, design.Row_Info.at(row_iter).getCoor()),
                                     design.Row_Info.at(row_iter).getSubRow(subrow_iter).get_rx() - cut_right_x,
                                     design.Row_Info.at(row_iter).getHeight());
                } // else if
                else if((cut_right_x >= design.Row_Info.at(row_iter).getSubRow(subrow_iter).get_rx()) &&
                        (cut_left_x >= design.Row_Info.at(row_iter).getSubRow(subrow_iter).get_lx()) &&
                        (cut_left_x <= design.Row_Info.at(row_iter).getSubRow(subrow_iter).get_rx()))
                {
                // shape cover right edge of subrow
                    design.Row_Info.at(row_iter).getSubRow(subrow_iter)
                        .setWidth(cut_left_x - design.Row_Info.at(row_iter).getSubRow(subrow_iter).get_lx());
                } // else if
            } // for
        } // for
    } // for

    // cut here
    /*for(int row_iter = 0; row_iter < (int)design.Row_Info.size(); row_iter++)
    {
    	// cut subrow if width > ????
		for(int subrow_iter = 0; subrow_iter < (int)design.Row_Info.at(row_iter).getSubRowNum(); subrow_iter++)
		{
			if(design.Row_Info.at(row_iter).getSubRow(subrow_iter).getWidth() > MAXIMAL_SUBROW_LENGTH)
			{
				// add new sub row
				new_subrow.setBoundary(Coor(design.Row_Info.at(row_iter).getSubRow(subrow_iter).get_lx() + (int)(design.Row_Info.at(row_iter).getSubRow(subrow_iter).getWidth() + 1) / 2, design.Row_Info.at(row_iter).getCoor()),
									   (int)design.Row_Info.at(row_iter).getSubRow(subrow_iter).getWidth() / 2,
									   design.Row_Info.at(row_iter).getHeight());
				design.Row_Info.at(row_iter).addSubRow(new_subrow);
				// update cut subrow
				design.Row_Info.at(row_iter).getSubRow(subrow_iter)
					.setWidth((int)(design.Row_Info.at(row_iter).getSubRow(subrow_iter).getWidth() + 1) / 2);
				subrow_iter--;
			}
		}
    }*/

    // sort subrow
    for(int row_iter = 0; row_iter < (int)design.Row_Info.size(); row_iter++)
    {
        design.Row_Info.at(row_iter).sortSubrow();
    } // for


    /**** to do
    //initial cell in subrow
    */

    vector<size_t> sorted_cell;
    size_t cell_pos(0);
    size_t row_pos(0);
    for(int cell_iter = 0; cell_iter < (int)design.getNumMovable(); cell_iter++)
    {
        if(design.cell_lib.at(cell_iter).getMove() == true)
            sorted_cell.push_back(cell_iter);
    } // for

    sort(sorted_cell.begin(), sorted_cell.end(), compareCell_lx);

    for(int cell_iter = 0; cell_iter < sorted_cell.size(); cell_iter++)
    {
        // find row
        cell_pos = sorted_cell.at(cell_iter);
        row_pos = getRow(design.cell_lib.at(cell_pos).ll.y);
        if(row_pos > design.Row_Info.size())
        {
            cout << "error row_pos not fit. row_pos : " << row_pos << endl;
            cout << "cell_pos : " << cell_pos;
            cout << " (" << design.cell_lib.at(cell_pos).ll.x << "," << design.cell_lib.at(cell_pos).ll.y << ")" << endl;
        }
        // find subrow
        for(int subrow_iter = 0; subrow_iter < (int)design.Row_Info.at(row_pos).getSubRowNum(); subrow_iter++)
        {
            if(design.cell_lib.at(cell_pos).ll.x >= design.Row_Info.at(row_pos).getSubRow(subrow_iter).get_lx() &&
               design.cell_lib.at(cell_pos).ll.x <= design.Row_Info.at(row_pos).getSubRow(subrow_iter).get_rx())
            {
                design.Row_Info.at(row_pos).getSubRow(subrow_iter).addCell(cell_pos);
                //break;
            } // if
        } // for
    } // for


    // test subrow is cut correctly
    /**** to do: finish it */
    if(SUBROW_CUT_TEST_MODE)
    {
        SubRow merge_subrow;
        size_t subrow_tgt(0);
        for(int row_iter = 0; row_iter < (int)design.Row_Info.size(); row_iter++)
        {
            // merge subrow back
            merge_subrow = design.Row_Info.at(row_iter).getSubRow(0);
            subrow_tgt = 1;
            for(int macro_iter = 0; macro_iter < (int)design.Row_Info.at(row_iter).getMacroNum(); macro_iter++)
            {
                macro_pos = design.Row_Info.at(row_iter).getMacro(macro_iter);
            } // for
            cout << "row " << row_iter << " : "  << design.Row_Info.at(row_iter).getCoor() << "("
                 << design.Row_Info.at(row_iter).getSubrowOrigin() << ","
                 << design.Row_Info.at(row_iter).getSubrowOrigin() + design.Row_Info.at(row_iter).getWidth() << ")" << endl;
            for(int subrow_iter = 0; subrow_iter < (int)design.Row_Info.at(row_iter).getSubRowNum(); subrow_iter++)
            {
                cout << "(" << design.Row_Info.at(row_iter).getSubRow(subrow_iter).get_lx() << ","
                     << design.Row_Info.at(row_iter).getSubRow(subrow_iter).get_rx() << ") ";
            } // for
            cout << endl;
        } // for
    } // if

    // test subrow is sorted
    if(SUBROW_SORTED_TEST_MODE)
    {
        for(int row_iter = 0; row_iter < (int)design.Row_Info.size(); row_iter++)
        {
            for(int subrow_iter = 0; subrow_iter < (int)design.Row_Info.at(row_iter).getSubRowNum() - 1; subrow_iter++)
            {
                if(design.Row_Info.at(row_iter).getSubRow(subrow_iter).get_lx()
                   > design.Row_Info.at(row_iter).getSubRow(subrow_iter + 1).get_lx())
                {
                    cout << "Error:[pretreatment] Subrow is not sorted" << endl;
                } // if
            } // for
        } // for
    } // if

    // test all cells are in subrow
    if(ALL_CELL_IN_SUBROW)
    {
        vector<size_t> all_cell;
        for(int row_iter = 0; row_iter < (int)design.Row_Info.size(); row_iter++)
        {
            for(int subrow_iter = 0; subrow_iter < (int)design.Row_Info.at(row_iter).getSubRowNum(); subrow_iter++)
            {
                for(int cell_iter = 0; cell_iter < (int)design.Row_Info.at(row_iter).getSubRow(subrow_iter).getCellNum(); cell_iter++)
                {
                    all_cell.push_back(design.Row_Info.at(row_iter).getSubRow(subrow_iter).getCell(cell_iter));
                } // for
            } // for
        } // for

        sort(all_cell.begin(), all_cell.end());

        if(all_cell.size() != design.getNumMovable())
            cout << "Error:[pretreatment] not all cell in subrow" << endl;
        else
        {
            for(int cell_iter = 0; cell_iter < (int)design.getNumMovable(); cell_iter++)
            {
                if(all_cell.at(cell_iter) != cell_iter)
                {
                    cout << "Error:[pretreatment] not all cell in subrow" << endl;
                    break;
                } // if
            } // for
        } // else
    } // if

    int numSubRow(0);
    for(int row_iter = 0; row_iter < (int)design.Row_Info.size(); row_iter++)
    {
        numSubRow += design.Row_Info.at(row_iter).getSubRowNum();
    } // for
    cout << "Totoal subrow num : " << numSubRow << endl;

    //cout << "[pretreatment.cpp] initial subrow end" << endl;

    return;
} // initialSubRow
/* ************************************************** */
inline bool macroOverlapWithArea(size_t rowIndex, Coor macro_ll, Coor macro_ur)
{
    if(macro_ur.x <= design.Row_Info.at(rowIndex).getSubrowOrigin())
        return false;
    else if(macro_ll.x >= design.Row_Info.at(rowIndex).getSubrowOrigin() + design.Row_Info.at(rowIndex).getWidth())
        return false;
    else if(macro_ur.y <= design.Row_Info.at(rowIndex).getCoor())
        return false;
    else if(macro_ll.y >= design.Row_Info.at(rowIndex).getCoor() + design.Row_Info.at(rowIndex).getHeight())
        return false;
    else
        return true;
} // macroOverlapWithArea
/* ************************************************** */
inline bool compareCell_lx(size_t p_pos_1, size_t p_pos_2)
{
    return design.cell_lib.at(p_pos_1).ll.x < design.cell_lib.at(p_pos_2).ll.x;
} // compareCell_lx


