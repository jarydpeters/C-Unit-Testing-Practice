#include "unity.h"
#include "cutlistOptimizer.h"
#include <stdlib.h>

void setUp(void) {}
void tearDown(void) {}

void testCutlistOptimizationWhenGreedyAlgorithmIsSufficient(void) 
{
    int required[] = {50, 75, 100, 25, 80};  // Pieces to cut
    CutlistInput input = {200, required, 5}; // Stock length = 200

    CutlistResult result = cutlist_optimize(input);

    // Verify stock count
    TEST_ASSERT_EQUAL_INT(2, result.stock_used); // Expected stock pieces needed

    // Expected assignments: Pieces fit into stocks as follows
    // Stock #1: [100, 80]    (fits with in 200 with 20 leftover)
    // Stock #2: [75, 50, 25] (fits with in 200 with 50 leftover)
    
    // rearranged required list = 100, 80, 75, 50, 25
    int expected_assignments[]  =  {0,  0,  1,  1,  1};  // Expected stock indices per piece

    for (int i = 0; i < input.piece_count; i++) 
    {
        TEST_ASSERT_EQUAL_INT(expected_assignments[i], result.assignments[i]);
    }

    // Verify waste calculation
    int expected_waste = (200 - (100 + 80)) + (200 - (75 + 50 + 25)); // 20 + 50 = 70
    TEST_ASSERT_EQUAL_INT(expected_waste, result.waste);

    free(result.assignments);
}

void testCutlistOptimizationWhenGreedyAlgorithmIsNotSufficient(void) 
{
    int required[] = {60, 40, 40, 70, 70, 120};  // Pieces to cut
    CutlistInput input = {200, required, 6}; // Stock length = 200

    CutlistResult result = cutlist_optimize(input);

    // Verify stock count
    TEST_ASSERT_EQUAL_INT(2, result.stock_used); // Expected stock pieces needed

    // Expected assignments: Pieces fit into stocks as follows
    // Stock #1: [120, 40, 40] (fits with in 200 with 0 leftover)
    // Stock #2: [70, 70, 60]  (fits with in 200 with 0 leftover)

    // Note: a purely greedy agorithm with arrive at a less-optimized result of:
    // Stock #1: [120, 70]
    // Stock #2: [70, 60, 40]
    // Stock #3: [40]
    
    // rearranged required list = 120, 70, 70, 60, 40, 40
    int expected_assignments[]  =  {0,  1,  1,  0,  1,  1};  // Expected stock indices per piece

    for (int i = 0; i < input.piece_count; i++) 
    {
        TEST_ASSERT_EQUAL_INT(expected_assignments[i], result.assignments[i]);
    }

    // Verify waste calculation
    int expected_waste = 0;
    TEST_ASSERT_EQUAL_INT(expected_waste, result.waste);

    free(result.assignments);
}

int main(void) 
{
    UNITY_BEGIN();
    RUN_TEST(testCutlistOptimizationWhenGreedyAlgorithmIsSufficient);
    RUN_TEST(testCutlistOptimizationWhenGreedyAlgorithmIsNotSufficient);
    return UNITY_END();
}
