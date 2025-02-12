#ifndef CUTLIST_OPTIMIZER_H
#define CUTLIST_OPTIMIZER_H

#include <stdlib.h>  // For malloc and free

typedef struct 
{
    int *optimal_assignments;
    int *current_assignments;
    int *remaining_stock_space;
    int optimal_stock_count;
    int optimal_waste;
    int current_stock_count;
    int stock_length;
    int total_pieces;
    int *piece_sizes;
} PackingState;

typedef struct {
    int *required_pieces;
    int piece_count;
    int stock_length;
} CutlistInput;

typedef struct {
    int *assignments;
    int stock_used;
    int waste;
} CutlistResult;

void find_best_packing(PackingState *state, int current_piece_index);
void optimize_cutlist(CutlistInput input, CutlistResult *result);

#endif // CUTLIST_OPTIMIZER_H
