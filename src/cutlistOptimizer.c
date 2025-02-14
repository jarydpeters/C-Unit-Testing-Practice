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
    for (int currentPiece = 0; currentPiece < input.pieceCount; currentPiece++)
    {
        printf("Piece %d: %d\n", (currentPiece + 1), input.requiredPieces[currentPiece]);
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
    char *output = getStockAssignmentsAsString(&state);
    printf("%s", output);
    free(output);

    // Free dynamically allocated memory
    free(state.optimalAssignments);
    free(state.currentAssignments);
    free(state.remainingStockSpace);
}

// Depth-First Recursive Search Traversal to find best packing of pieces onto stocks. Explores all enumerations, O(n!) runtime
void findBestPacking(PackingState *state, int currentPieceIndex) 
{
    printf("\nFIND BEST PACKING RECURSIVE CALL\n");

    // Base Case: If all pieces have been assigned to a stock, evaluate solution for amount of stock used and total waste
    if (currentPieceIndex == state->totalPieces) 
    {
        printf("ALL PIECES PLACED. EVALUATING SOLUTION\n\n");
        int total_waste = 0;
        // Add up all waste on all stocks for total value
        for (int stock_index = 0; stock_index < state->currentStockCount; stock_index++) 
        {
            total_waste += state->remainingStockSpace[stock_index];
        }

        printf("\nEvaluating solution: Stock Used = %d, Waste = %d\n\n", state->currentStockCount, total_waste);

        // Replace existing best solution with currently evaluated solution if it has less total waste
        if (total_waste < state->optimalWaste) 
        {
            state->optimalWaste = total_waste;
            state->optimalStockCount = state->currentStockCount;

            for (int piece_index = 0; piece_index < state->totalPieces; piece_index++) 
            {
                state->optimalAssignments[piece_index] = state->currentAssignments[piece_index];
            }

            // Print out best case every time one is found
            printf("New Best Found: Stock Used = %d, Waste = %d\n\n", state->optimalStockCount, state->optimalWaste);
            char *output = getStockAssignmentsAsString(state);
            printf("%s", output);
            free(output);
        }
        return;
    }
    else
    {
        printf("CURRENT PIECE BEING PLACED: %u\n\n", (currentPieceIndex + 1));
    }

    // Pruning: Stop early if currently evaluated case is already worse than best known case
    if ((state->currentStockCount) > state->optimalStockCount) 
    {
        printf("\nCurrent case being evaluated is the same or worse than best known solution. Skipping...\n\n");
        return;
    }

    // Grab size of piece currently being placed
    int current_piece_size = state->pieceSizes[currentPieceIndex];

    // Try placing piece into any existing stock
    for (int stock_index = 0; (stock_index < state->currentStockCount); stock_index++) 
    {
        // Only place piece if there's room for it in the existing stock
        if (state->remainingStockSpace[stock_index] >= current_piece_size) 
        {
            printf("%-20s | Stock #%2d | Piece #%2d (size: %3d) | Remaining Space in Stock #%2d: %3d\n",
               "Placing piece", (stock_index + 1), (currentPieceIndex + 1), current_piece_size,
               (stock_index + 1), (state->remainingStockSpace[stock_index] - current_piece_size));

            state->remainingStockSpace[stock_index] -= current_piece_size;
            state->currentAssignments[currentPieceIndex] = stock_index;

            // Print out current state of stocks and which pieces are cut from them
            char *output = getStockAssignmentsAsString(state);
            printf("%s", output);
            free(output);

            // Recursive call: find placement of next piece
            findBestPacking(state, currentPieceIndex + 1);

            // Backtrack: Remove piece to try placing it somewhere else to see if it leads to a more efficient packing
            printf("BACKTRACKING: Removing piece %d (size %d) from Stock #%d to try for a more optimal placement...\n", 
                   currentPieceIndex, current_piece_size, (stock_index + 1));

            // Restore space on stock after having backtracked piece off of it
            state->remainingStockSpace[stock_index] += current_piece_size;

            // Explicitly mark piece as unassigned
            state->currentAssignments[currentPieceIndex] = -1;

            // Print updated stock assignments AFTER removing the piece
            printf("\nUpdated Stock Assignments after Backtracking:\n");
            output = getStockAssignmentsAsString(state);
            printf("%s", output);
            free(output);
        }
    }

    // Open a new stock if needed
    printf("%-20s | Stock #%2d | Piece #%2d (size: %3d) | Remaining Space in Stock #%2d: %3d\n",
       "Starting new stock", (state->currentStockCount + 1), (currentPieceIndex + 1), current_piece_size,
       (state->currentStockCount + 1), (state->stockLength - current_piece_size));

    // Reduce available space on new stock by length of piece added to it
    state->remainingStockSpace[state->currentStockCount] = state->stockLength - current_piece_size;

    // Record current piece's stock number assignment
    state->currentAssignments[currentPieceIndex] = state->currentStockCount;

    // Increase the total number of stocks used
    state->currentStockCount++;

    char *output = getStockAssignmentsAsString(state);
    printf("%s", output);
    free(output);

    // Recursive call: find placement of next piece
    findBestPacking(state, currentPieceIndex + 1);

    // Backtrack: Remove piece to try placing it somewhere else to see if it leads to a more efficient packing. Undo new stock addition as it is now empty
    printf("BACKTRACKING: Undo addition of new stock. Closing Stock #%d (piece %d removed)...\n", 
           state->currentStockCount, (currentPieceIndex + 1));

    state->currentStockCount--;
    state->remainingStockSpace[state->currentStockCount] = 0;

    // Explicitly mark piece as unassigned
    state->currentAssignments[currentPieceIndex] = -1;

    // Print updated stock assignments AFTER removing the new stock
    printf("\nUpdated Stock Assignments after Backtracking (New Stock Removal):\n");
    output = getStockAssignmentsAsString(state);
    printf("%s", output);
    free(output);
}

// Function to generate stock assignments as a string
char* getStockAssignmentsAsString(PackingState *state) 
{
    // Allocate a sufficiently large buffer
    int buffer_size = 1024;
    char *output = (char*)malloc(buffer_size);
    if (!output) return NULL;  // Return NULL if memory allocation fails

    output[0] = '\0';  // Ensure string starts empty
    char temp[100];     // Temporary buffer for each line

    strcat(output, "Stock assignments:\n");

    for (int stock_index = 0; stock_index < state->currentStockCount; stock_index++) 
    {
        snprintf(temp, sizeof(temp), "Stock #%d: ", stock_index + 1);
        strcat(output, temp);

        int first = 1;
        for (int piece_index = 0; piece_index < state->totalPieces; piece_index++) 
        {
            if (state->currentAssignments[piece_index] == stock_index) 
            {
                if (!first) strcat(output, ", ");
                snprintf(temp, sizeof(temp), "%d", state->pieceSizes[piece_index]);
                strcat(output, temp);
                first = 0;
            }
        }
        strcat(output, "\n");
    }

    strcat(output, "\n");
    return output;  // Caller must free the returned string
}
