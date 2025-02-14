#ifndef CUTLIST_OPTIMIZER_H
#define CUTLIST_OPTIMIZER_H

#include <stdio.h>
#include <stdlib.h>  // For malloc and free
#include <string.h>

typedef struct 
{
    int *optimalAssignments;
    int *currentAssignments;
    int *remainingStockSpace;
    int optimalStockCount;
    int optimalWaste;
    int currentStockCount;
    int stockLength;
    int totalPieces;
    int *pieceSizes;
} PackingState;

typedef struct {
    int *requiredPieces;
    int pieceCount;
    int stockLength;
} CutlistInput;

typedef struct {
    int *assignments;
    int stockUsed;
    int waste;
} CutlistResult;

void optimizeCutlist(CutlistInput input, CutlistResult *result);
void findBestPacking(PackingState *state, int currentPieceIndex);
char* getStockAssignmentsAsString(PackingState *state);

#endif // CUTLIST_OPTIMIZER_H
