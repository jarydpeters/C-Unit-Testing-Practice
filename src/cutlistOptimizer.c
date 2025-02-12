#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "cutlistOptimizer.h"

void find_best_packing(PackingState *state, int current_piece_index) 
{
    if (current_piece_index == state->total_pieces) 
    {
        // Calculate total waste
        int total_waste = 0;
        for (int stock_index = 0; stock_index < state->current_stock_count; stock_index++) 
        {
            total_waste += state->remaining_stock_space[stock_index];
        }

        // If this configuration is better, store it
        if (total_waste < state->optimal_waste) 
        {
            state->optimal_waste = total_waste;
            state->optimal_stock_count = state->current_stock_count;
            for (int piece_index = 0; piece_index < state->total_pieces; piece_index++) 
            {
                state->optimal_assignments[piece_index] = state->current_assignments[piece_index];
            }
        }
        return;
    }

    int current_piece_size = state->piece_sizes[current_piece_index];

    // Try placing this piece in an existing stock piece
    for (int stock_index = 0; stock_index < state->current_stock_count; stock_index++) 
    {
        if (state->remaining_stock_space[stock_index] >= current_piece_size) 
        {
            state->remaining_stock_space[stock_index] -= current_piece_size;
            state->current_assignments[current_piece_index] = stock_index;
            find_best_packing(state, current_piece_index + 1);
            state->remaining_stock_space[stock_index] += current_piece_size;  // Backtrack
        }
    }

    // Try placing this piece in a new stock piece
    state->remaining_stock_space[state->current_stock_count] = state->stock_length - current_piece_size;
    state->current_assignments[current_piece_index] = state->current_stock_count;
    state->current_stock_count++;
    find_best_packing(state, current_piece_index + 1);
    state->current_stock_count--;
    state->remaining_stock_space[state->current_stock_count] = 0;  // Backtrack
}

void optimize_cutlist(CutlistInput input, CutlistResult *result) 
{
    PackingState state;

    state.total_pieces = input.piece_count;
    state.stock_length = input.stock_length;
    state.piece_sizes = input.required_pieces;
    state.optimal_waste = INT_MAX;
    state.optimal_stock_count = input.piece_count;
    state.current_stock_count = 0;

    state.optimal_assignments = (int *)malloc(input.piece_count * sizeof(int));
    state.current_assignments = (int *)malloc(input.piece_count * sizeof(int));
    state.remaining_stock_space = (int *)malloc(input.piece_count * sizeof(int));

    // Sort pieces in descending order (Largest First)
    for (int first_piece = 0; first_piece < input.piece_count - 1; first_piece++) 
    {
        for (int second_piece = first_piece + 1; second_piece < input.piece_count; second_piece++) 
        {
            if (input.required_pieces[first_piece] < input.required_pieces[second_piece]) 
            {
                int temp_size = input.required_pieces[first_piece];
                input.required_pieces[first_piece] = input.required_pieces[second_piece];
                input.required_pieces[second_piece] = temp_size;
            }
        }
    }

    // Start searching for the best packing
    find_best_packing(&state, 0);

    // Store the best result
    result->stock_used = state.optimal_stock_count;
    result->waste = state.optimal_waste;
    for (int piece_index = 0; piece_index < input.piece_count; piece_index++) 
    {
        result->assignments[piece_index] = state.optimal_assignments[piece_index];
    }

    free(state.optimal_assignments);
    free(state.current_assignments);
    free(state.remaining_stock_space);
}
