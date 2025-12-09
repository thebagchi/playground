package arena

import (
	"testing"
)

func TestArenaSkipList(t *testing.T) {
	a := New(4, BUMP)
	defer a.Delete()

	sl := NewSkipList[int, string](a)

	// Test empty skiplist
	if sl.Len() != 0 {
		t.Errorf("expected length 0, got %d", sl.Len())
	}

	if _, ok := sl.Search(1); ok {
		t.Error("expected not found for empty skiplist")
	}

	// Test insertion
	sl.Insert(5, "five")
	sl.Insert(3, "three")
	sl.Insert(7, "seven")
	sl.Insert(1, "one")
	sl.Insert(9, "nine")

	if sl.Len() != 5 {
		t.Errorf("expected length 5, got %d", sl.Len())
	}

	// Test search
	if val, ok := sl.Search(5); !ok || val != "five" {
		t.Errorf("expected 'five', got %v, %v", val, ok)
	}

	if val, ok := sl.Search(3); !ok || val != "three" {
		t.Errorf("expected 'three', got %v, %v", val, ok)
	}

	if val, ok := sl.Search(1); !ok || val != "one" {
		t.Errorf("expected 'one', got %v, %v", val, ok)
	}

	if val, ok := sl.Search(9); !ok || val != "nine" {
		t.Errorf("expected 'nine', got %v, %v", val, ok)
	}

	if _, ok := sl.Search(4); ok {
		t.Error("expected not found for key 4")
	}

	// Test Contains
	if !sl.Contains(5) {
		t.Error("expected Contains(5) to be true")
	}

	if sl.Contains(4) {
		t.Error("expected Contains(4) to be false")
	}

	// Test update
	sl.Insert(5, "FIVE")
	if val, ok := sl.Search(5); !ok || val != "FIVE" {
		t.Errorf("expected 'FIVE' after update, got %v, %v", val, ok)
	}

	// Test Min/Max
	if key, val, ok := sl.Min(); !ok || key != 1 || val != "one" {
		t.Errorf("expected Min (1, 'one'), got (%v, %v, %v)", key, val, ok)
	}

	if key, val, ok := sl.Max(); !ok || key != 9 || val != "nine" {
		t.Errorf("expected Max (9, 'nine'), got (%v, %v, %v)", key, val, ok)
	}

	// Test Range
	keys := []int{}
	values := []string{}
	sl.Range(func(k int, v string) bool {
		keys = append(keys, k)
		values = append(values, v)
		return true
	})

	expectedKeys := []int{1, 3, 5, 7, 9}
	expectedValues := []string{"one", "three", "FIVE", "seven", "nine"}

	if len(keys) != len(expectedKeys) {
		t.Errorf("expected %d keys, got %d", len(expectedKeys), len(keys))
	}

	for i, expected := range expectedKeys {
		if i >= len(keys) || keys[i] != expected {
			t.Errorf("expected key[%d] = %d, got %v", i, expected, keys)
		}
	}

	for i, expected := range expectedValues {
		if i >= len(values) || values[i] != expected {
			t.Errorf("expected value[%d] = %s, got %v", i, expected, values)
		}
	}

	// Test deletion
	if !sl.Delete(3) {
		t.Error("expected Delete(3) to succeed")
	}

	if sl.Len() != 4 {
		t.Errorf("expected length 4 after delete, got %d", sl.Len())
	}

	if _, ok := sl.Search(3); ok {
		t.Error("expected key 3 to be deleted")
	}

	if sl.Delete(10) {
		t.Error("expected Delete(10) to fail")
	}

	// Test Reset
	sl.Reset()
	if sl.Len() != 0 {
		t.Errorf("expected length 0 after reset, got %d", sl.Len())
	}

	if _, ok := sl.Search(5); ok {
		t.Error("expected not found after reset")
	}
}

func TestArenaSkipListStringKeys(t *testing.T) {
	a := New(4, BUMP)
	defer a.Delete()

	sl := NewSkipList[string, int](a)

	// Test with string keys
	sl.Insert("apple", 1)
	sl.Insert("banana", 2)
	sl.Insert("cherry", 3)

	if val, ok := sl.Search("banana"); !ok || val != 2 {
		t.Errorf("expected 2, got %v, %v", val, ok)
	}

	if key, val, ok := sl.Min(); !ok || key != "apple" || val != 1 {
		t.Errorf("expected Min ('apple', 1), got (%v, %v, %v)", key, val, ok)
	}

	if key, val, ok := sl.Max(); !ok || key != "cherry" || val != 3 {
		t.Errorf("expected Max ('cherry', 3), got (%v, %v, %v)", key, val, ok)
	}
}

func TestArenaSkipListLarge(t *testing.T) {
	a := New(64, BUMP) // Use BUMP allocator for large test
	defer a.Delete()

	sl := NewSkipList[int, int](a)

	// Insert many elements
	for i := 0; i < 1000; i++ {
		sl.Insert(i, i*10)
	}

	if sl.Len() != 1000 {
		t.Errorf("expected length 1000, got %d", sl.Len())
	}

	// Test some random lookups
	testCases := []int{0, 99, 500, 999}
	for _, key := range testCases {
		if val, ok := sl.Search(key); !ok || val != key*10 {
			t.Errorf("expected %d for key %d, got %v, %v", key*10, key, val, ok)
		}
	}

	// Test Min/Max
	if key, val, ok := sl.Min(); !ok || key != 0 || val != 0 {
		t.Errorf("expected Min (0, 0), got (%v, %v, %v)", key, val, ok)
	}

	if key, val, ok := sl.Max(); !ok || key != 999 || val != 9990 {
		t.Errorf("expected Max (999, 9990), got (%v, %v, %v)", key, val, ok)
	}
}
