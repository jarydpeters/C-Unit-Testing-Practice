#ifndef CUTLIST_OPTIMIZER_H
#define CUTLIST_OPTIMIZER_H

#include <stdlib.h>  // For malloc and free

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

void find_best_packing(int *pieces, int piece_count, int stock_length, int *best_assignments, int *best_stock_used, int *best_waste, int *current_assignments, int *stock_remaining, int stock_used, int index);
void optimize_cutlist(CutlistInput input, CutlistResult *result);

#endif // CUTLIST_OPTIMIZER_H
