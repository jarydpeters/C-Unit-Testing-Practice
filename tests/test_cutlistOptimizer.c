#include "unity.h"
#include "cutlistOptimizer.h"

#include <stdlib.h>
#include <time.h>

void setUp(void) {}
void tearDown(void) {}

void testWhenGreedyAlgorithmIsSufficient(void) 
{
    int required[] = {50, 75, 100, 25, 80};  // Pieces to cut
    CutlistInput input = {required, 5, 200};  // Stock length = 200

    CutlistResult result;
    result.assignments = (int *)malloc(input.piece_count * sizeof(int)); // Ensure assignment memory allocation
    optimize_cutlist(input, &result);

    // Verify stock count
    TEST_ASSERT_EQUAL_INT(2, result.stock_used); // Expected stock pieces needed

    // Expected assignments: Pieces fit into stocks as follows
    // Stock #1: [100, 80]    (20 leftover)
    // Stock #2: [75, 50, 25] (50 leftover)
    
    // rearranged required list = 100, 80, 75, 50, 25
    int expected_assignments[]  =  {0,  0,  1,  1,  1};  // Expected stock indices per piece

    for (int i = 0; i < input.piece_count; i++) 
    {
        if (expected_assignments[i] != result.assignments[i]) 
        {
            printf("Mismatch at index %d: expected %d, got %d\n", 
                i, expected_assignments[i], result.assignments[i]);
        }
        TEST_ASSERT_EQUAL_INT(expected_assignments[i], result.assignments[i]);
    }

    // Verify waste calculation
    int expected_waste = (200 - (100 + 80)) + (200 - (75 + 50 + 25)); // 20 + 50 = 70
    TEST_ASSERT_EQUAL_INT(expected_waste, result.waste);

    free(result.assignments);
}

void testWhenGreedyAlgorithmIsNotSufficient(void) 
{
    int required[] = {60, 40, 40, 70, 70, 120};  // Pieces to cut
    CutlistInput input = {required, 6, 200};  // Stock length = 200

    CutlistResult result;
    result.assignments = (int *)malloc(input.piece_count * sizeof(int)); // Ensure assignment memory allocation
    optimize_cutlist(input, &result);

    // Verify stock count
    TEST_ASSERT_EQUAL_INT(2, result.stock_used); // Expected stock pieces needed

    // Expected assignments: Pieces fit into stocks as follows
    // Stock #1: [120, 40, 40] (0 leftover)
    // Stock #2: [70, 70, 60]  (0 leftover)

    // Note: a purely greedy agorithm with arrive at a less-optimized result of:
    // Stock #1: [120, 70]    (10 leftover)
    // Stock #2: [70, 60, 40] (30 leftover)
    // Stock #3: [40]         (160 leftover)
    
    // rearranged required list = 120, 70, 70, 60, 40, 40
    int expected_assignments[]  =  {0,  1,  1,  1,  0,  0};  // Expected stock indices per piece

    for (int i = 0; i < input.piece_count; i++) 
    {
        if (expected_assignments[i] != result.assignments[i]) 
        {
            printf("Mismatch at index %d: expected %d, got %d\n", 
                i, expected_assignments[i], result.assignments[i]);
        }
        TEST_ASSERT_EQUAL_INT(expected_assignments[i], result.assignments[i]);
    }

    // Verify waste calculation
    int expected_waste = 0;
    TEST_ASSERT_EQUAL_INT(expected_waste, result.waste);

    free(result.assignments);
}

void testSinglePieceFitsExactly(void) 
{
    int required[] = {100};  
    CutlistInput input = {required, 1, 100};  

    CutlistResult result;
    result.assignments = (int *)malloc(input.piece_count * sizeof(int)); 
    optimize_cutlist(input, &result);

    TEST_ASSERT_EQUAL_INT(1, result.stock_used); 
    TEST_ASSERT_EQUAL_INT(0, result.waste);

    int expected_assignments[] = {0}; 
    for (int i = 0; i < input.piece_count; i++) 
    {
        TEST_ASSERT_EQUAL_INT(expected_assignments[i], result.assignments[i]);
    }

    free(result.assignments);
}

void testMultiplePiecesFitExactly(void) 
{
    int required[] = {50, 50};  
    CutlistInput input = {required, 2, 100};  

    CutlistResult result;
    result.assignments = (int *)malloc(input.piece_count * sizeof(int)); 
    optimize_cutlist(input, &result);

    TEST_ASSERT_EQUAL_INT(1, result.stock_used); 
    TEST_ASSERT_EQUAL_INT(0, result.waste);

    int expected_assignments[] = {0, 0}; 
    for (int i = 0; i < input.piece_count; i++) 
    {
        TEST_ASSERT_EQUAL_INT(expected_assignments[i], result.assignments[i]);
    }

    free(result.assignments);
}

void testAllPiecesFitMultipleStocksNoWaste(void) 
{
    int required[] = {60, 60, 60, 60};  
    CutlistInput input = {required, 4, 120};  

    CutlistResult result;
    result.assignments = (int *)malloc(input.piece_count * sizeof(int)); 
    optimize_cutlist(input, &result);

    TEST_ASSERT_EQUAL_INT(2, result.stock_used); 
    TEST_ASSERT_EQUAL_INT(0, result.waste);

    int expected_assignments[] = {0, 0, 1, 1};  
    for (int i = 0; i < input.piece_count; i++) 
    {
        TEST_ASSERT_EQUAL_INT(expected_assignments[i], result.assignments[i]);
    }

    free(result.assignments);
}

void testStockWasteForced(void) 
{
    int required[] = {40, 40, 30};  
    CutlistInput input = {required, 3, 100};  

    CutlistResult result;
    result.assignments = (int *)malloc(input.piece_count * sizeof(int)); 
    optimize_cutlist(input, &result);

    TEST_ASSERT_EQUAL_INT(2, result.stock_used); 
    TEST_ASSERT_EQUAL_INT(90, result.waste);

    int expected_assignments[] = {0, 0, 1};  
    for (int i = 0; i < input.piece_count; i++) 
    {
        TEST_ASSERT_EQUAL_INT(expected_assignments[i], result.assignments[i]);
    }

    free(result.assignments);
}

void testPieceTooLargeCannotFit(void) 
{
    int required[] = {120, 30, 30};  
    CutlistInput input = {required, 3, 100};  

    CutlistResult result;
    result.assignments = (int *)malloc(input.piece_count * sizeof(int)); 
    optimize_cutlist(input, &result);

    TEST_ASSERT_EQUAL_INT(-1, result.waste); 

    free(result.assignments);
}

void testCutlistOptimizationWithLargeDataset(void) 
{
    int required[20];
    int stock_length = 1000;

    // Generate 20 pseudo-random piece sizes between 10 and 250
    for (int i = 0; i < 20; i++) 
    {
        required[i] = (i % 10) * 25 + 10; // Generates a repeating pattern of values between 10 and 250
    }

    CutlistInput input = {required, 20, stock_length};  

    CutlistResult result;
    result.assignments = (int *)malloc(input.piece_count * sizeof(int)); 

    // Time the execution
    clock_t start = clock();
    optimize_cutlist(input, &result);
    clock_t end = clock();

    double elapsed_time = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Moderate dataset test executed in: %.4f seconds\n", elapsed_time);

    // Ensure the solution is valid (each piece is assigned to some stock)
    for (int i = 0; i < input.piece_count; i++) 
    {
        TEST_ASSERT_TRUE(result.assignments[i] >= 0);
    }

    // Free memory
    free(result.assignments);
}


void testCutlistOptimizationWithVeryLargeDataset(void) 
{
    int required[50];
    int stock_length = 1000;

    // Generate 50 random piece sizes between 10 and 250
    for (int i = 0; i < 50; i++) 
    {
        required[i] = (i % 10) * 25 + 10; // Generates a repeating pattern of values between 10 and 250
    }

    CutlistInput input = {required, 50, stock_length};  

    CutlistResult result;
    result.assignments = (int *)malloc(input.piece_count * sizeof(int)); 

    // Time the execution
    clock_t start = clock();
    optimize_cutlist(input, &result);
    clock_t end = clock();

    double elapsed_time = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Large dataset test executed in: %.4f seconds\n", elapsed_time);

    // Ensure the solution is valid (each piece is assigned to some stock)
    for (int i = 0; i < input.piece_count; i++) 
    {
        TEST_ASSERT_TRUE(result.assignments[i] >= 0);
    }

    // Free memory
    free(result.assignments);
}

int main(void) 
{
    UNITY_BEGIN();

    RUN_TEST(testWhenGreedyAlgorithmIsSufficient);
    RUN_TEST(testWhenGreedyAlgorithmIsNotSufficient);
    RUN_TEST(testSinglePieceFitsExactly);
    RUN_TEST(testMultiplePiecesFitExactly);
    RUN_TEST(testAllPiecesFitMultipleStocksNoWaste);
    RUN_TEST(testStockWasteForced);
    RUN_TEST(testPieceTooLargeCannotFit);
    RUN_TEST(testCutlistOptimizationWithLargeDataset);
    RUN_TEST(testCutlistOptimizationWithVeryLargeDataset);

    return UNITY_END();
}
