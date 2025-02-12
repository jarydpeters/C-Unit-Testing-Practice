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

    printf("\nBest Packing Found: Stock Used = %d, Waste = %d\n\n", result->stockUsed, result->waste);
    printStockAssignments(result, input);

    // Free dynamically allocated memory
    free(state.optimalAssignments);
    free(state.currentAssignments);
    free(state.remainingStockSpace);
}

// Recursive function to find the best packing configuration
void findBestPacking(PackingState *state, int currentPieceIndex) 
{
    currentIteration++;

    // Base case: If all pieces have been assigned, evaluate the packing
    if (currentPieceIndex == state->totalPieces) 
    {
        // Calculate total waste for the current configuration
        int total_waste = 0;
        for (int stock_index = 0; stock_index < state->currentStockCount; stock_index++) 
        {
            total_waste += state->remainingStockSpace[stock_index];
        }

        // If this configuration has less waste, store it as the best found so far
        if (total_waste < state->optimalWaste) 
        {
            state->optimalWaste = total_waste;
            state->optimalStockCount = state->currentStockCount;

            // Save the best assignment configuration
            for (int piece_index = 0; piece_index < state->totalPieces; piece_index++) 
            {
                state->optimalAssignments[piece_index] = state->currentAssignments[piece_index];
            }

            // Debug output for new best configuration
            printf("\nNew Best Found: Stock Used = %d, Waste = %d\n", state->optimalStockCount, state->optimalWaste);
            printStockAssignmentsFromState(state);
        }
        return; // End this recursion branch
    }

    // Pruning: Stop early if we are already worse than the best found
    if (state->currentStockCount >= state->optimalStockCount) 
    {
        printf("\ncurrent case #%u being evaluated is already worse than best known solution. Ending calculation early...", currentIteration);
        return;
    }

    // Get the size of the current piece to be placed
    int current_piece_size = state->pieceSizes[currentPieceIndex];

    // Try placing this piece into an existing stock piece
    for (int stock_index = 0; stock_index < state->currentStockCount; stock_index++) 
    {
        // Check if the piece fits in this stock
        if (state->remainingStockSpace[stock_index] >= current_piece_size) 
        {
            // Reduce the available space in this stock
            state->remainingStockSpace[stock_index] -= current_piece_size;
            // Assign this piece to the current stock index
            state->currentAssignments[currentPieceIndex] = stock_index;

            // Recursively attempt to place the next piece
            findBestPacking(state, currentPieceIndex + 1);

            // Backtrack: Restore the stock space for the next attempt
            state->remainingStockSpace[stock_index] += current_piece_size;
        }
    }

    // Try placing this piece into a new stock piece
    state->remainingStockSpace[state->currentStockCount] = state->stockLength - current_piece_size;
    state->currentAssignments[currentPieceIndex] = state->currentStockCount;

    // Increase the count of used stock pieces
    state->currentStockCount++;

    // Recursively attempt to place the next piece
    findBestPacking(state, currentPieceIndex + 1);

    // Backtrack: Undo the addition of a new stock piece
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
            if (state->optimalAssignments[piece_index] == stock_index) 
            {
                if (!first) 
                    printf(", ");
                printf("%d", state->pieceSizes[piece_index]);
                first = 0;
            }
        }
        printf("\n");
    }
}

// Function to print final stock assignments from the result
void printStockAssignments(CutlistResult *result, CutlistInput input)
{
    int stock_index;
    printf("Final stock assignments:\n");
    for (stock_index = 0; stock_index < result->stockUsed; stock_index++) 
    {
        printf("Stock #%d: ", stock_index + 1);
        int first = 1;
        for (int piece_index = 0; piece_index < input.pieceCount; piece_index++) 
        {
            if (result->assignments[piece_index] == stock_index) 
            {
                if (!first) 
                    printf(", ");
                printf("%d", input.requiredPieces[piece_index]);
                first = 0;
            }
        }
        printf("\n");
    }
}
