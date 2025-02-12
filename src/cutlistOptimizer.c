#include "cutlistOptimizer.h"

CutlistResult cutlist_optimize(CutlistInput input) 
{
    CutlistResult result;
    result.assignments = (int *)malloc(input.piece_count * sizeof(int));
    result.waste = 0;

    if (!result.assignments) 
    {
        result.waste = -1;
        result.stock_used = -1;
        return result;
    }

    // Step 1: Sort pieces in descending order (First-Fit Decreasing)
    for (int currentPiece = 0; currentPiece < (input.piece_count - 1); currentPiece++) 
    {
        for (int nextPiece = (currentPiece + 1); nextPiece < input.piece_count; nextPiece++) 
        {
            if (input.required_pieces[currentPiece] < input.required_pieces[nextPiece]) 
            {
                int temp = input.required_pieces[currentPiece];
                input.required_pieces[currentPiece] = input.required_pieces[nextPiece];
                input.required_pieces[nextPiece] = temp;
            }
        }
    }

    // Step 2: Allocate pieces to stock pieces
    int *stock_remaining = (int *)malloc(input.piece_count * sizeof(int));
    int stockCount = 0;

    for (int pieceIndex = 0; pieceIndex < input.piece_count; pieceIndex++) 
    {
        int placed = 0;
        for (int stockIndex = 0; stockIndex < stockCount; stockIndex++) 
        {
            if (stock_remaining[stockIndex] >= input.required_pieces[pieceIndex]) 
            {
                stock_remaining[stockIndex] -= input.required_pieces[pieceIndex];
                result.assignments[pieceIndex] = stockIndex;
                placed = 1;
                break;
            }
        }
        if (!placed) 
        {
            stock_remaining[stockCount] = input.stock_length - input.required_pieces[pieceIndex];
            result.assignments[pieceIndex] = stockCount;
            stockCount++;
        }
    }

    // Calculate waste
    for (int stockIndex = 0; stockIndex < stockCount; stockIndex++) 
    {
        result.waste += stock_remaining[stockIndex];
    }

    result.stock_used = stockCount;

    free(stock_remaining);
    return result;
}
