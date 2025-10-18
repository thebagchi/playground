package exercise

import "testing"

type UnorderedSet struct {
	values  []int // Store Value in postion provided by index
	indexes []int // Store position of value in values
	size    int
}

func MakeUnorderedSet(size int) *UnorderedSet {
	return &UnorderedSet{
		values:  make([]int, size+1),
		indexes: make([]int, size+1),
		size:    0,
	}
}

func (us *UnorderedSet) Insert(x int) {
	if !us.Get(x) {
		us.values[us.size] = x
		us.indexes[x] = us.size
		us.size = us.size + 1
	}
}

func (us *UnorderedSet) Delete(x int) {
	if us.Get(x) {
		var (
			lastP = us.size - 1
			lastV = us.values[lastP]
			currP = us.indexes[x]
		)
		us.indexes[lastV] = currP
		us.values[currP] = lastV
		us.size = us.size - 1
	}
}

func (us *UnorderedSet) Get(x int) bool {
	var (
		pos    = us.indexes[x]
		cur    = us.size
		exists = false
	)
	if pos < cur && us.values[pos] == x {
		exists = true
	}
	return exists
}

func (us *UnorderedSet) Clear() {
	us.size = 0
}

func TestUnorderedSetBasic(t *testing.T) {
	us := MakeUnorderedSet(200)

	items := []int{0, 1, 2, 50, 199}
	for _, v := range items {
		us.Insert(v)
	}
	for _, v := range items {
		if !us.Get(v) {
			t.Fatalf("expected Get(%d)=true after Insert", v)
		}
	}

	us.Insert(2)
	if !us.Get(2) {
		t.Fatalf("expected Get(2)=true after reinserting duplicate")
	}

	us.Delete(2)
	if us.Get(2) {
		t.Fatalf("expected Get(2)=false after Delete")
	}

	for _, v := range []int{0, 1, 50, 199} {
		if !us.Get(v) {
			t.Fatalf("expected Get(%d)=true after deleting different element", v)
		}
	}

	us.Delete(2)

	us.Insert(2)
	if !us.Get(2) {
		t.Fatalf("expected Get(2)=true after re-insert")
	}

	us.Clear()
	for _, v := range items {
		if us.Get(v) {
			t.Fatalf("expected Get(%d)=false after Clear", v)
		}
	}
}
