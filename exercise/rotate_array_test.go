package exercise

import (
	"testing"
)

func ReverseArray(items []int) {
	last := len(items) - 1
	for i := 0; i < len(items)/2; i++ {
		items[i], items[last-i] = items[last-i], items[i]
	}
}

func RotateArray(items []int, k int) {
	if len(items) <= k || k < 0 {
		return
	}
	ReverseArray(items)
	var (
		first  = items[:k]
		second = items[k:]
	)
	ReverseArray(first)
	ReverseArray(second)
}

func TestRotateArray(t *testing.T) {
	var (
		eitems = []int{4, 5, 1, 2, 3}
		oitems = []int{1, 2, 3, 4, 5}
	)
	RotateArray(oitems, 2)
	for i := range eitems {
		if oitems[i] != eitems[i] {
			t.Errorf("Expected %d, but got %d", eitems[i], oitems[i])
		}
	}
}
