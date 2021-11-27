#pragma once

#include "region.h"
#include "swap_target.h"
#include "basic_types.h"


using namespace std;

void cascade(void);

SwapTarget cascadeMoveCellToThisRegion(Region r, int cell_index, map<int, bool> was_used);
SwapTarget emergencyFindCascateVictim(int cell_index, map<int, bool> was_used);

SwapTarget findCascadeVictim(Region r, int cell_index, float minimal_heuristic_value, map<int, bool> was_used);

float cascadeTargetHeuristics(int cell_index, SwapTarget target);





