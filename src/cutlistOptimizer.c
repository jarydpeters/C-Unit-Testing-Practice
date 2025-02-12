#include "cutlistOptimizer.h"

// Function to optimize the cutlist and minimize waste
void optimize_cutlist(CutlistInput input, CutlistResult *result) 
{
    // Check if any piece is too large to fit into stock
    for (int currentPiece = 0; currentPiece < input.piece_count; currentPiece++)
    {
        if (input.required_pieces[currentPiece] > input.stock_length)
        {
            // Indicate failure with special flag values
            result->stock_used = -1;
            result->waste = -1;
            printf("\nRequested piece is longer than stock length!\n\n");
            return;
        }
    }

    PackingState state;

    // Initialize the state structure with input data
    state.total_pieces = input.piece_count;
    state.stock_length = input.stock_length;
    state.piece_sizes = input.required_pieces;
    state.optimal_waste = INT_MAX; // Start with the worst possible waste
    state.optimal_stock_count = input.piece_count; // Start with an upper bound on stock usage
    state.current_stock_count = 0; // No stock pieces used at the start

    // Allocate memory for tracking assignments and stock space
    state.optimal_assignments = (int *)malloc(input.piece_count * sizeof(int));
    state.current_assignments = (int *)malloc(input.piece_count * sizeof(int));
    state.remaining_stock_space = (int *)malloc(input.piece_count * sizeof(int));

    // Sort pieces in descending order (Largest First) to improve efficiency
    printf("\nSorting pieces in descending order:\n");
    for (int first_piece = 0; first_piece < input.piece_count - 1; first_piece++) 
    {
        for (int second_piece = first_piece + 1; second_piece < input.piece_count; second_piece++) 
        {
            if (input.required_pieces[first_piece] < input.required_pieces[second_piece]) 
            {
                // Swap elements to ensure descending order
                int temp_size = input.required_pieces[first_piece];
                input.required_pieces[first_piece] = input.required_pieces[second_piece];
                input.required_pieces[second_piece] = temp_size;
            }
        }
    }

    // Print sorted pieces
    for (int i = 0; i < input.piece_count; i++)
    {
        printf("Piece %d: %d\n", i, input.required_pieces[i]);
    }

    // Start searching for the best packing configuration
    find_best_packing(&state, 0);

    // Store the best result in the output structure
    result->stock_used = state.optimal_stock_count;
    result->waste = state.optimal_waste;
    
    // Copy the best piece assignments
    for (int piece_index = 0; piece_index < input.piece_count; piece_index++) 
    {
        result->assignments[piece_index] = state.optimal_assignments[piece_index];
    }

    printf("\nBest Packing Found: Stock Used = %d, Waste = %d\n\n", result->stock_used, result->waste);
    print_stock_assignments(result, input);

    // Free dynamically allocated memory
    free(state.optimal_assignments);
    free(state.current_assignments);
    free(state.remaining_stock_space);
}

// Recursive function to find the best packing configuration
void find_best_packing(PackingState *state, int current_piece_index) 
{
    // Base case: If all pieces have been assigned, evaluate the packing
    if (current_piece_index == state->total_pieces) 
    {
        // Calculate total waste for the current configuration
        int total_waste = 0;
        for (int stock_index = 0; stock_index < state->current_stock_count; stock_index++) 
        {
            total_waste += state->remaining_stock_space[stock_index];
        }

        // If this configuration has less waste, store it as the best found so far
        if (total_waste < state->optimal_waste) 
        {
            state->optimal_waste = total_waste;
            state->optimal_stock_count = state->current_stock_count;

            // Save the best assignment configuration
            for (int piece_index = 0; piece_index < state->total_pieces; piece_index++) 
            {
                state->optimal_assignments[piece_index] = state->current_assignments[piece_index];
            }

            // Debug output for new best configuration
            printf("\nNew Best Found: Stock Used = %d, Waste = %d\n", state->optimal_stock_count, state->optimal_waste);
            print_stock_assignments_from_state(state);
        }
        return; // End this recursion branch
    }

    // Pruning: Stop early if we are already worse than the best found**
    if (state->current_stock_count >= state->optimal_stock_count) 
    {
        return;
    }

    // Get the size of the current piece to be placed
    int current_piece_size = state->piece_sizes[current_piece_index];

    // Try placing this piece into an existing stock piece
    for (int stock_index = 0; stock_index < state->current_stock_count; stock_index++) 
    {
        // Check if the piece fits in this stock
        if (state->remaining_stock_space[stock_index] >= current_piece_size) 
        {
            // Reduce the available space in this stock
            state->remaining_stock_space[stock_index] -= current_piece_size;
            // Assign this piece to the current stock index
            state->current_assignments[current_piece_index] = stock_index;

            // Recursively attempt to place the next piece
            find_best_packing(state, current_piece_index + 1);

            // Backtrack: Restore the stock space for the next attempt
            state->remaining_stock_space[stock_index] += current_piece_size;
        }
    }

    // Try placing this piece into a new stock piece
    state->remaining_stock_space[state->current_stock_count] = state->stock_length - current_piece_size;
    state->current_assignments[current_piece_index] = state->current_stock_count;

    // Increase the count of used stock pieces
    state->current_stock_count++;

    // Recursively attempt to place the next piece
    find_best_packing(state, current_piece_index + 1);

    // Backtrack: Undo the addition of a new stock piece
    state->current_stock_count--;
    state->remaining_stock_space[state->current_stock_count] = 0;
}

// Function to print stock assignments from the current best state
void print_stock_assignments_from_state(PackingState *state)
{
    int stock_index;
    printf("Stock assignments:\n");
    for (stock_index = 0; stock_index < state->optimal_stock_count; stock_index++) 
    {
        printf("Stock #%d: ", stock_index + 1);
        int first = 1;
        for (int piece_index = 0; piece_index < state->total_pieces; piece_index++) 
        {
            if (state->optimal_assignments[piece_index] == stock_index) 
            {
                if (!first) 
                    printf(", ");
                printf("%d", state->piece_sizes[piece_index]);
                first = 0;
            }
        }
        printf("\n");
    }
}

// Function to print final stock assignments from the result
void print_stock_assignments(CutlistResult *result, CutlistInput input)
{
    int stock_index;
    printf("Final stock assignments:\n");
    for (stock_index = 0; stock_index < result->stock_used; stock_index++) 
    {
        printf("Stock #%d: ", stock_index + 1);
        int first = 1;
        for (int piece_index = 0; piece_index < input.piece_count; piece_index++) 
        {
            if (result->assignments[piece_index] == stock_index) 
            {
                if (!first) 
                    printf(", ");
                printf("%d", input.required_pieces[piece_index]);
                first = 0;
            }
        }
        printf("\n");
    }
}
