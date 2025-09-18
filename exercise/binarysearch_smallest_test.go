package exercise

import (
	"fmt"
	"testing"
)

func BinarySearchSmallest(items []int) int {
	var (
		start = 0
		end   = len(items) - 1
	)
	if len(items) == 0 {
		return -1
	}
	if items[start] <= items[end] {
		return 0
	}
	for start < end {
		mid := (start + end) / 2
		if items[mid] > items[end] {
			// 3 4 5 0 1 2
			// value at mid is greater than value at end
			// value is available in right
			start = mid + 1
		} else {
			// 5 0 1 2 3 4
			// value at mid is less than value at start
			// value is available in left or mid itself
			end = mid
		}
	}
	return start
}

func TestBinarySearchSmallest(t *testing.T) {
	items := []int{7, 8, 9, 0, 1, 2, 3, 4, 5, 6}
	pos := BinarySearchSmallest(items)
	fmt.Println("smallest found at:", pos)
}
