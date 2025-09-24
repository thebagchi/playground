package exercise

import (
	"math"
	"testing"
)

func MaxSumSubarray(items []int) int {
	if len(items) == 0 {
		return math.MinInt
	}
	var (
		maxSum  = items[0]
		currSum = maxSum
	)
	for _, item := range items[1:] {
		currSum = max(item, currSum+item)
		maxSum = max(maxSum, currSum)
	}
	return maxSum

}

func TestMaxSumSubarray(t *testing.T) {
	if result := MaxSumSubarray([]int{1, 2, 3, 4, 5}); result != 15 {
		t.Errorf("Expected 15, got %d", result)
	}
	if result := MaxSumSubarray([]int{-5, -2, -8, -1}); result != -1 {
		t.Errorf("Expected -1, got %d", result)
	}
	if result := MaxSumSubarray([]int{-2, 1, -3, 4, -1, 2, 1, -5, 4}); result != 6 {
		t.Errorf("Expected 6, got %d", result)
	}
	if result := MaxSumSubarray([]int{5}); result != 5 {
		t.Errorf("Expected 5, got %d", result)
	}
	if result := MaxSumSubarray([]int{0, 0, 0, 0}); result != 0 {
		t.Errorf("Expected 0, got %d", result)
	}
}
