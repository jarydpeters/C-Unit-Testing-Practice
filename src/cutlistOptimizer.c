#include "cutlistOptimizer.h"

// Function to optimize the cutlist and minimize waste
void optimizeCutlist(CutlistInput input, CutlistResult *result) 
{    
    // Check if any piece is too large to fit into stock
    for (int currentPiece = 0; currentPiece < input.pieceCount; currentPiece++)
    {
        if (input.requiredPieces[currentPiece] > input.stockLength)
        {
            // Indicate failure with special flag values
            result->stockUsed = -1;
            result->waste = -1;
            printf("\nRequested piece is longer than stock length!\n\n");
            return;
        }
    }

    PackingState state;

    // Initialize the state structure with input data
    state.totalPieces = input.pieceCount;
    state.stockLength = input.stockLength;
    state.pieceSizes = input.requiredPieces;
    state.optimalWaste = INT_MAX; // Start with the worst possible waste
    state.optimalStockCount = input.pieceCount; // Start with an upper bound on stock usage
    state.currentStockCount = 0; // No stock pieces used at the start

    // Allocate memory for tracking assignments and stock space
    state.optimalAssignments = (int *)malloc(input.pieceCount * sizeof(int));
    state.currentAssignments = (int *)malloc(input.pieceCount * sizeof(int));
    state.remainingStockSpace = (int *)malloc(input.pieceCount * sizeof(int));

    // Sort pieces in descending order (Largest First) to improve efficiency
    printf("\nSorting pieces in descending order:\n");
    for (int first_piece = 0; first_piece < input.pieceCount - 1; first_piece++) 
    {
        for (int second_piece = first_piece + 1; second_piece < input.pieceCount; second_piece++) 
        {
            if (input.requiredPieces[first_piece] < input.requiredPieces[second_piece]) 
            {
                // Swap elements to ensure descending order
                int temp_size = input.requiredPieces[first_piece];
                input.requiredPieces[first_piece] = input.requiredPieces[second_piece];
                input.requiredPieces[second_piece] = temp_size;
            }
        }
    }

    // Print sorted pieces
    for (int i = 0; i < input.pieceCount; i++)
    {
        printf("Piece %d: %d\n", i, input.requiredPieces[i]);
    }
    printf("\n");

    // Start searching for the best packing configuration
    findBestPacking(&state, 0);

    // Store the best result in the output structure
    result->stockUsed = state.optimalStockCount;
    result->waste = state.optimalWaste;
    
    // Copy the best piece assignments
    for (int piece_index = 0; piece_index < input.pieceCount; piece_index++) 
    {
        result->assignments[piece_index] = state.optimalAssignments[piece_index];
    }

    printf("\nBest Packing Found: Stock Used = %d, Waste = %d\n\n\n\n\n", result->stockUsed, result->waste);
    printStockAssignmentsFromState(&state);

    currentIteration = 0;

    // Free dynamically allocated memory
    free(state.optimalAssignments);
    free(state.currentAssignments);
    free(state.remainingStockSpace);
}

void findBestPacking(PackingState *state, int currentPieceIndex) 
{
    currentIteration++;

    // Base Case: If all pieces are assigned, evaluate solution
    if (currentPieceIndex == state->totalPieces) 
    {
        int total_waste = 0;
        for (int stock_index = 0; stock_index < state->currentStockCount; stock_index++) 
        {
            total_waste += state->remainingStockSpace[stock_index];
        }

        printf("\nEvaluating solution: Stock Used = %d, Waste = %d\n\n", state->currentStockCount, total_waste);

        // Update best solution if this one is better
        if (total_waste < state->optimalWaste) 
        {
            state->optimalWaste = total_waste;
            state->optimalStockCount = state->currentStockCount;

            for (int piece_index = 0; piece_index < state->totalPieces; piece_index++) 
            {
                state->optimalAssignments[piece_index] = state->currentAssignments[piece_index];
            }

            printf("New Best Found: Stock Used = %d, Waste = %d\n\n", state->optimalStockCount, state->optimalWaste);
            printStockAssignmentsFromState(state);
        }
        return;
    }

    //Pruning: Stop early if worse than best found
    if ((state->currentStockCount) > state->optimalStockCount) 
    {
        printf("\nCurrent case being evaluated is the same or worse than best known solution. Skipping...\n\n");
        return;
    }

    int current_piece_size = state->pieceSizes[currentPieceIndex];

    // Try placing piece into an existing stock
    for (int stock_index = 0; stock_index < state->currentStockCount; stock_index++) 
    {
        if (state->remainingStockSpace[stock_index] >= current_piece_size) 
        {
            printf("%-20s | Stock #%2d | Piece #%2d (size: %3d) | Remaining Space in Stock #%2d: %3d\n",
               "Placing piece", (stock_index + 1), (currentPieceIndex + 1), current_piece_size,
               (stock_index + 1), (state->remainingStockSpace[stock_index] - current_piece_size));

            state->remainingStockSpace[stock_index] -= current_piece_size;
            state->currentAssignments[currentPieceIndex] = stock_index;

            printStockAssignmentsFromState(state);

            findBestPacking(state, currentPieceIndex + 1);

            // Backtrack
            printf("Backtracking: Removing piece %d (size %d) from Stock #%d to try for a more optimal placement...\n", 
                   currentPieceIndex, current_piece_size, (stock_index + 1));

            state->remainingStockSpace[stock_index] += current_piece_size;
        }
    }

    // Open a new stock if needed
    printf("%-20s | Stock #%2d | Piece #%2d (size: %3d) | Remaining Space in Stock #%2d: %3d\n",
       "Starting new stock", (state->currentStockCount + 1), (currentPieceIndex + 1), current_piece_size,
       (state->currentStockCount + 1), (state->stockLength - current_piece_size));

    state->remainingStockSpace[state->currentStockCount] = state->stockLength - current_piece_size;
    state->currentAssignments[currentPieceIndex] = state->currentStockCount;
    state->currentStockCount++;

    findBestPacking(state, currentPieceIndex + 1);

    // Backtrack: Undo new stock addition
    printf("Backtracking: Undo addition of new stock. Closing Stock #%d (piece %d removed)...\n", 
           state->currentStockCount, (currentPieceIndex + 1));

    state->currentStockCount--;
    state->remainingStockSpace[state->currentStockCount] = 0;
}

// Function to print stock assignments from the current best state
void printStockAssignmentsFromState(PackingState *state)
{
    int stock_index;
    printf("Stock assignments:\n");
    for (stock_index = 0; stock_index < state->optimalStockCount; stock_index++) 
    {
        printf("Stock #%d: ", stock_index + 1);
        int first = 1;
        for (int piece_index = 0; piece_index < state->totalPieces; piece_index++) 
        {
            if (state->currentAssignments[piece_index] == stock_index) 
            {
                if (!first) 
                    printf(", ");
                printf("%d", state->pieceSizes[piece_index]);
                first = 0;
            }
        }
        printf("\n");
    }
    printf("\n");
}
