#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "cutlistOptimizer.h"

void find_best_packing(int *pieces, int piece_count, int stock_length, int *best_assignments, int *best_stock_used, int *best_waste, int *current_assignments, int *stock_remaining, int stock_used, int index) {
    if (index == piece_count) {
        // Calculate total waste
        int total_waste = 0;
        for (int i = 0; i < stock_used; i++) {
            total_waste += stock_remaining[i];
        }

        // If this configuration is better, store it
        if (total_waste < *best_waste) {
            *best_waste = total_waste;
            *best_stock_used = stock_used;
            for (int i = 0; i < piece_count; i++) {
                best_assignments[i] = current_assignments[i];
            }
        }
        return;
    }

    int piece_size = pieces[index];

    // Try placing this piece in an existing stock piece
    for (int i = 0; i < stock_used; i++) {
        if (stock_remaining[i] >= piece_size) {
            stock_remaining[i] -= piece_size;
            current_assignments[index] = i;
            find_best_packing(pieces, piece_count, stock_length, best_assignments, best_stock_used, best_waste, current_assignments, stock_remaining, stock_used, index + 1);
            stock_remaining[i] += piece_size;  // Backtrack
        }
    }

    // Try placing this piece in a new stock piece
    stock_remaining[stock_used] = stock_length - piece_size;
    current_assignments[index] = stock_used;
    find_best_packing(pieces, piece_count, stock_length, best_assignments, best_stock_used, best_waste, current_assignments, stock_remaining, stock_used + 1, index + 1);
    stock_remaining[stock_used] = 0;  // Backtrack
}

void optimize_cutlist(CutlistInput input, CutlistResult *result) {
    int *best_assignments = (int *)malloc(input.piece_count * sizeof(int));
    int *current_assignments = (int *)malloc(input.piece_count * sizeof(int));
    int *stock_remaining = (int *)malloc(input.piece_count * sizeof(int));
    int best_stock_used = input.piece_count;
    int best_waste = INT_MAX;

    // Sort pieces in descending order (Largest First)
    for (int i = 0; i < input.piece_count - 1; i++) {
        for (int j = i + 1; j < input.piece_count; j++) {
            if (input.required_pieces[i] < input.required_pieces[j]) {
                int temp = input.required_pieces[i];
                input.required_pieces[i] = input.required_pieces[j];
                input.required_pieces[j] = temp;
            }
        }
    }

    // Start searching for the best packing
    find_best_packing(input.required_pieces, input.piece_count, input.stock_length, best_assignments, &best_stock_used, &best_waste, current_assignments, stock_remaining, 0, 0);

    // Store the best result
    result->stock_used = best_stock_used;
    result->waste = best_waste;
    for (int i = 0; i < input.piece_count; i++) {
        result->assignments[i] = best_assignments[i];
    }

    free(best_assignments);
    free(current_assignments);
    free(stock_remaining);
}
