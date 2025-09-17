package exercise

import (
	"fmt"
	"testing"
)

func RecurrsiveBinarySearch(items []int, elem int) int {
	var (
		start = 0
		end   = len(items) - 1
		mid   = (start + end) / 2
		tmp   []int
	)
	for {
		if items[mid] == elem {
			return mid
		}
		if elem > items[mid] {
			if items[start] <= items[mid] {
				mid = mid + 1
				tmp = items[mid:]
			} else {
				if elem <= items[mid] {
					mid = mid + 1
					tmp = items[mid:]
				} else {
					tmp = items[:mid]
					mid = 0
				}
			}
			break
		}
		if elem < items[mid] {
			if items[end] >= items[mid] {
				tmp = items[:mid]
				mid = 0
			} else {
				if elem >= items[mid] {
					tmp = items[:mid]
					mid = 0
				} else {
					mid = mid + 1
					tmp = items[mid:]
				}
			}
			break
		}
	}
	pos := RecurrsiveBinarySearch(tmp, elem)
	if pos != -1 {
		pos = mid + pos
	}
	return pos
}

func BinarySearch(items []int, elem int) int {
	var (
		start = 0
		end   = len(items) - 1
		mid   = -1
		pos   = -1
	)
	for start <= end {
		mid = (start + end) / 2
		if items[mid] == elem {
			pos = mid
			break
		}
		if elem > items[mid] {
			if items[start] <= items[mid] {
				// Left half is sorted, but elem > items[mid]
				// So elem must be in right half
				start = mid + 1
			} else {
				// Right half is sorted
				// Check if elem is in the sorted right half
				if elem <= items[end] {
					// elem is in right half
					start = mid + 1
				} else {
					// elem is in left half
					end = mid - 1
				}
			}
		}
		if elem < items[mid] {
			if items[end] >= items[mid] {
				// Right half is sorted, but elem < items[mid]
				// So elem must be in left half
				end = mid - 1
			} else {
				// Left half is sorted
				// Check if elem is in the sorted left half
				if elem >= items[start] {
					// elem is in left half
					end = mid - 1
				} else {
					// elem is in right half
					start = mid + 1
				}
			}
		}
	}
	return pos
}

func TestBinarySearch(t *testing.T) {
	items := []int{7, 8, 9, 0, 1, 2, 3, 4, 5, 6}
	for i := range 10 {
		fmt.Println("searching:", i)
		pos := BinarySearch(items, i)
		fmt.Println("found at:", pos)
	}
}

func TestRecurrsiveBinarySearch(t *testing.T) {
	items := []int{7, 8, 9, 0, 1, 2, 3, 4, 5, 6}
	for i := range 10 {
		fmt.Println("searching:", i)
		pos := BinarySearch(items, i)
		fmt.Println("found at:", pos)
	}
}
