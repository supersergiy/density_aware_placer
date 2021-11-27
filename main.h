#pragma once

using namespace std;

#include <string>
#include <vector>
#include <map>
#include "region.h"
#include "basic_types.h"
#include "cell.h"
#include "cell_library.h"
#include "swap_target.h"
#include "design_info.h"
#include "utilities.h"
#include "bin.h"
#include "bin_library.h"
#include "net_pl.h"
#include "initialize.h"
#include "parameters.h"
#include "cascade.h"
#include "RowInfo.h"
#include "SubRow.h"
#include "pretreatment.h"
#include "legalizer.h"
#include "global_swap.h"
#include "debug_output_functions.h"
#include "dynamic_swap.h"
#include "plot_result.h"

/* ************************************************** */
// fileIO.cpp
bool readFile(string p_fileName);
void WritePlaceResult(string fileName, DesignInfo &design);

// [Test]fileIO.cpp :For fileIO.cpp test part
void test_nodesFile(string test_nodes_file);
void test_netsFile(string test_nets_file);
void test_plFile(string test_pl_file);
void test_sclFile(string test_scl_file);
void test_shapesFile(string test_shapes_file);
void test_routeFile(string test_route_file);

// global_swap.cpp
void global_swap(void);
void vertical_swap();
void horizontal_swap();
void local_reordering();
void displayInputStatistics(void);


//???
