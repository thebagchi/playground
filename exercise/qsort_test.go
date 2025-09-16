package exercise

import (
	"reflect"
	"testing"
)

func Partition(items []int) int {
	var (
		pos   = 0
		pivot = items[len(items)-1]
		curr  = 0
	)
	for ; curr < len(items)-1; curr = curr + 1 {
		item := items[curr]
		if item < pivot {
			items[pos], items[curr] = items[curr], items[pos]
			pos = pos + 1
		}
	}
	items[pos], items[curr] = items[curr], items[pos]
	return pos
}

func Quicksort(items []int) {
	if len(items) <= 1 {
		return
	}
	pid := Partition(items)
	Quicksort(items[:pid])
	Quicksort(items[pid+1:])
}

func TestQuicksort(t *testing.T) {
	var (
		input    = []int{10, 3, 5, 1, 9, 2, 8, 4, 7, 6}
		expected = []int{1, 2, 3, 4, 5, 6, 7, 8, 9, 10}
	)
	Quicksort(input)
	if !reflect.DeepEqual(input, expected) {
		t.Errorf("Quicksort failed: got %v, want %v", input, expected)
	}
}
