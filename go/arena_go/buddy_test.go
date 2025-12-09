package arena

import "testing"

func TestBuddyAllocator(t *testing.T) {
	a := New(1, BUDDY)
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

func TestArenaMap(t *testing.T) {
	a := New(1, BUMP)
	m := NewMap[string, int](a)

	// Test Set and Get
	m.Set("one", 1)
	m.Set("two", 2)
	m.Set("three", 3)

	if v, ok := m.Get("one"); !ok || v != 1 {
		t.Errorf("expected 1, got %v, %v", v, ok)
	}
	if v, ok := m.Get("two"); !ok || v != 2 {
		t.Errorf("expected 2, got %v, %v", v, ok)
	}
	if v, ok := m.Get("three"); !ok || v != 3 {
		t.Errorf("expected 3, got %v, %v", v, ok)
	}

	// Test update
	m.Set("one", 10)
	if v, ok := m.Get("one"); !ok || v != 10 {
		t.Errorf("expected 10, got %v, %v", v, ok)
	}

	// Test not found
	if _, ok := m.Get("notfound"); ok {
		t.Error("expected not found")
	}

	// Test Len
	if m.Len() != 3 {
		t.Errorf("expected len 3, got %d", m.Len())
	}

	// Test Delete
	m.Delete("two")
	if _, ok := m.Get("two"); ok {
		t.Error("expected deleted")
	}
	if m.Len() != 2 {
		t.Errorf("expected len 2, got %d", m.Len())
	}

	// Test Range
	count := 0
	m.Range(func(k string, v int) bool {
		count++
		return true
	})
	if count != 2 {
		t.Errorf("expected range count 2, got %d", count)
	}

	// Test growth with unique keys
	for i := 0; i < 20; i++ {
		key := "test_key_" + string(rune('A'+i))
		m.Set(key, i+100)
		// Verify immediately
		if v, ok := m.Get(key); !ok || v != i+100 {
			t.Errorf("immediate check: expected %d for key %s, got %v, %v", i+100, key, v, ok)
		}
	}
	expectedLen := 2 + 20 // "one", "three" after delete, + 20 new
	if m.Len() != expectedLen {
		t.Errorf("expected len %d, got %d", expectedLen, m.Len())
	}

	// Verify all values
	for i := 0; i < 20; i++ {
		key := "test_key_" + string(rune('A'+i))
		if v, ok := m.Get(key); !ok || v != i+100 {
			t.Errorf("expected %d for key %s, got %v, %v", i+100, key, v, ok)
		}
	}

	// Test Reset
	m.Reset()
	if m.Len() != 0 {
		t.Errorf("expected len 0 after reset, got %d", m.Len())
	}
}

func TestArenaSlice(t *testing.T) {
	a := New(1, BUMP)
	s := MakeArenaSlice[int](a)

	// Test append
	for i := 0; i < 100; i++ {
		s.Append(i)
	}

	if s.Len() != 100 {
		t.Errorf("expected len 100, got %d", s.Len())
	}

	// Verify values
	slice := s.Slice()
	for i := 0; i < 100; i++ {
		if slice[i] != i {
			t.Errorf("expected %d, got %d", i, slice[i])
		}
	}
}
