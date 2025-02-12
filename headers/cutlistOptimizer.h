#ifndef CUTLIST_OPTIMIZER_H
#define CUTLIST_OPTIMIZER_H

#include <stdlib.h>  // For malloc and free

typedef struct 
{
    int stock_length;     // Fixed length of each stock piece
    int *required_pieces; // Array of required piece lengths
    int piece_count;      // Number of required pieces
} CutlistInput;

typedef struct 
{
    int stock_used;   // Number of stock pieces used
    int *assignments; // Array mapping each piece to a stock index
    int waste;        // Total waste after cutting
} CutlistResult;

CutlistResult cutlist_optimize(CutlistInput input);

#endif // CUTLIST_OPTIMIZER_H
