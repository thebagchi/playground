package arena

import "testing"

func TestBumpAllocator(t *testing.T) {
	a := New(1, BUMP)
	p1 := Alloc[int](a)
	if p1 == nil {
		t.Fatal("alloc failed")
	}
	p2 := Alloc[int](a)
	if p2 == nil {
		t.Fatal("alloc failed")
	}
	if p1 == p2 {
		t.Fatal("same pointer")
	}
	a.Reset()
	p3 := Alloc[int](a)
	if p3 != p1 {
		t.Fatal("not reset")
	}
}
