package exercise

import (
	"fmt"
	"math/rand"
	"testing"
	"time"
)

func Sort012(items []int) {
	var (
		low  = 0
		high = len(items) - 1
		mid  = 0
	)
	for mid <= high {
		if items[mid] == 0 {
			items[low], items[mid] = items[mid], items[low]
			low = low + 1
			mid = mid + 1
			continue
		}
		if items[mid] == 1 {
			mid = mid + 1
			continue
		}
		if items[mid] == 2 {
			items[high], items[mid] = items[mid], items[high]
			high = high - 1
			continue
		}
	}
}

func MakeArray(n int) []int {
	r := rand.New(rand.NewSource(time.Now().UnixNano()))
	items := make([]int, n)
	for i := range n {
		items[i] = r.Intn(3) // Only 0, 1, or 2
	}
	fmt.Println(items)
	return items
}

func TestSort012(t *testing.T) {
	items := MakeArray(10)
	Sort012(items)
	fmt.Println(items)
}
