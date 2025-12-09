package arena

import (
	"testing"
)

// Test ArenaString Clone methods
func TestArenaString_Clone(t *testing.T) {
	a := New(10, BUMP)
	defer a.Delete()

	tests := []struct {
		name  string
		input string
	}{
		{"empty", ""},
		{"short-sso", "hello"},
		{"long", "this is a much longer string that exceeds SSO limit"},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			as := a.MakeArenaString(tt.input)

			// Clone the string
			cloned := as.Clone()

			// Verify content
			if cloned != tt.input {
				t.Errorf("Clone() = %q, want %q", cloned, tt.input)
			}

			// Delete arena
			a.Reset()

			// Cloned string should still be valid
			if cloned != tt.input {
				t.Errorf("After arena reset: Clone() = %q, want %q", cloned, tt.input)
			}
		})
	}
}

func TestArenaString_Bytes(t *testing.T) {
	a := New(10, BUMP)
	defer a.Delete()

	as := a.MakeArenaString("hello world")
	bytes := as.Bytes()

	expected := []byte("hello world")
	if len(bytes) != len(expected) {
		t.Errorf("len(Bytes()) = %d, want %d", len(bytes), len(expected))
	}
	for i := range expected {
		if bytes[i] != expected[i] {
			t.Errorf("Bytes()[%d] = %d, want %d", i, bytes[i], expected[i])
		}
	}

	// Modify arena string
	as.AppendString("!")

	// Original bytes should be unchanged
	if string(bytes) != "hello world" {
		t.Errorf("Bytes modified when arena string changed: %q", string(bytes))
	}
}

func TestArenaString_Clone_Empty(t *testing.T) {
	a := New(10, BUMP)
	defer a.Delete()

	as := a.MakeArenaString("")
	cloned := as.Clone()

	if cloned != "" {
		t.Errorf("Clone() of empty string = %q, want empty", cloned)
	}

	bytes := as.Bytes()
	if bytes != nil {
		t.Errorf("Bytes() of empty string = %v, want nil", bytes)
	}
}

// Test ArenaSlice Clone
func TestArenaSlice_Clone(t *testing.T) {
	a := New(10, BUMP)
	defer a.Delete()

	tests := []struct {
		name string
		data []int
	}{
		{"empty", []int{}},
		{"small", []int{1, 2, 3}},
		{"large", make([]int, 100)},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			// Fill large slice
			for i := range tt.data {
				tt.data[i] = i
			}

			as := MakeArenaSlice(a, tt.data...)

			// Clone the slice
			cloned := as.Clone()

			// Verify content
			if len(cloned) != len(tt.data) {
				t.Fatalf("len(Clone()) = %d, want %d", len(cloned), len(tt.data))
			}
			for i := range tt.data {
				if cloned[i] != tt.data[i] {
					t.Errorf("Clone()[%d] = %d, want %d", i, cloned[i], tt.data[i])
				}
			}

			// Modify arena slice
			if as.Len() > 0 {
				as.Append(999)
			}

			// Cloned slice should be unchanged
			if len(cloned) != len(tt.data) {
				t.Errorf("Cloned slice length changed: %d, want %d", len(cloned), len(tt.data))
			}

			// Delete arena
			a.Reset()

			// Cloned slice should still be valid
			for i := range tt.data {
				if cloned[i] != tt.data[i] {
					t.Errorf("After arena reset: Clone()[%d] = %d, want %d", i, cloned[i], tt.data[i])
				}
			}
		})
	}
}

func TestArenaSlice_Clone_Nil(t *testing.T) {
	a := New(10, BUMP)
	defer a.Delete()

	as := MakeArenaSlice[int](a)
	cloned := as.Clone()

	if cloned != nil {
		t.Errorf("Clone() of empty slice = %v, want nil", cloned)
	}
}

// Test ArenaMap Clone
func TestArenaMap_Clone(t *testing.T) {
	a := New(10, BUMP)
	defer a.Delete()

	m := NewMap[string, int](a)

	// Add entries
	entries := map[string]int{
		"one":   1,
		"two":   2,
		"three": 3,
		"four":  4,
		"five":  5,
	}

	for k, v := range entries {
		m.Set(k, v)
	}

	// Clone the map
	cloned := m.Clone()

	// Verify all entries
	if len(cloned) != len(entries) {
		t.Fatalf("len(Clone()) = %d, want %d", len(cloned), len(entries))
	}

	for k, v := range entries {
		if cv, ok := cloned[k]; !ok || cv != v {
			t.Errorf("Clone()[%q] = %d (ok=%v), want %d", k, cv, ok, v)
		}
	}

	// Modify arena map
	m.Set("six", 6)

	// Cloned map should be unchanged
	if len(cloned) != len(entries) {
		t.Errorf("Cloned map changed after arena map modification")
	}
	if _, ok := cloned["six"]; ok {
		t.Errorf("Cloned map contains new entry from arena map")
	}

	// Delete arena
	a.Reset()

	// Cloned map should still be valid
	for k, v := range entries {
		if cv, ok := cloned[k]; !ok || cv != v {
			t.Errorf("After arena reset: Clone()[%q] = %d (ok=%v), want %d", k, cv, ok, v)
		}
	}
}

func TestArenaMap_Clone_Empty(t *testing.T) {
	a := New(10, BUMP)
	defer a.Delete()

	m := NewMap[string, int](a)
	cloned := m.Clone()

	if cloned != nil {
		t.Errorf("Clone() of empty map = %v, want nil", cloned)
	}
}

// Test ArenaSkipList Clone
func TestArenaSkipList_Clone(t *testing.T) {
	a := New(10, BUMP)
	defer a.Delete()

	sl := NewSkipList[int, string](a)

	// Add entries
	entries := map[int]string{
		1: "one",
		2: "two",
		3: "three",
		4: "four",
		5: "five",
	}

	for k, v := range entries {
		sl.Insert(k, v)
	}

	// Clone the skip list
	cloned := sl.Clone()

	// Verify all entries
	if len(cloned) != len(entries) {
		t.Fatalf("len(Clone()) = %d, want %d", len(cloned), len(entries))
	}

	for k, v := range entries {
		if cv, ok := cloned[k]; !ok || cv != v {
			t.Errorf("Clone()[%d] = %q (ok=%v), want %q", k, cv, ok, v)
		}
	}

	// Modify skip list
	sl.Insert(6, "six")

	// Cloned map should be unchanged
	if len(cloned) != len(entries) {
		t.Errorf("Cloned map changed after skip list modification")
	}
	if _, ok := cloned[6]; ok {
		t.Errorf("Cloned map contains new entry from skip list")
	}

	// Delete arena
	a.Reset()

	// Cloned map should still be valid
	for k, v := range entries {
		if cv, ok := cloned[k]; !ok || cv != v {
			t.Errorf("After arena reset: Clone()[%d] = %q (ok=%v), want %q", k, cv, ok, v)
		}
	}
}

func TestArenaSkipList_Clone_Empty(t *testing.T) {
	a := New(10, BUMP)
	defer a.Delete()

	sl := NewSkipList[int, string](a)
	cloned := sl.Clone()

	if cloned != nil {
		t.Errorf("Clone() of empty skip list = %v, want nil", cloned)
	}
}

// Test ArenaSkipList CloneSlice
func TestArenaSkipList_CloneSlice(t *testing.T) {
	a := New(10, BUMP)
	defer a.Delete()

	sl := NewSkipList[int, string](a)

	// Add entries in random order
	entries := []struct {
		k int
		v string
	}{
		{5, "five"},
		{2, "two"},
		{4, "four"},
		{1, "one"},
		{3, "three"},
	}

	for _, e := range entries {
		sl.Insert(e.k, e.v)
	}

	// Clone to slice (should be sorted)
	cloned := sl.CloneSlice()

	// Verify length
	if len(cloned) != len(entries) {
		t.Fatalf("len(CloneSlice()) = %d, want %d", len(cloned), len(entries))
	}

	// Verify sorted order
	expected := []struct {
		Key int
		Val string
	}{
		{1, "one"},
		{2, "two"},
		{3, "three"},
		{4, "four"},
		{5, "five"},
	}

	for i, e := range expected {
		if cloned[i].Key != e.Key || cloned[i].Val != e.Val {
			t.Errorf("CloneSlice()[%d] = {%d, %q}, want {%d, %q}",
				i, cloned[i].Key, cloned[i].Val, e.Key, e.Val)
		}
	}

	// Delete arena
	a.Reset()

	// Cloned slice should still be valid and sorted
	for i, e := range expected {
		if cloned[i].Key != e.Key || cloned[i].Val != e.Val {
			t.Errorf("After arena reset: CloneSlice()[%d] = {%d, %q}, want {%d, %q}",
				i, cloned[i].Key, cloned[i].Val, e.Key, e.Val)
		}
	}
}

func TestArenaSkipList_CloneSlice_Empty(t *testing.T) {
	a := New(10, BUMP)
	defer a.Delete()

	sl := NewSkipList[int, string](a)
	cloned := sl.CloneSlice()

	if cloned != nil {
		t.Errorf("CloneSlice() of empty skip list = %v, want nil", cloned)
	}
}

// Benchmark Clone operations
func BenchmarkArenaString_Clone(b *testing.B) {
	a := New(1024, BUMP)
	defer a.Delete()

	as := a.MakeArenaString("this is a test string that is long enough to not use SSO")

	b.ResetTimer()
	b.ReportAllocs()
	for i := 0; i < b.N; i++ {
		_ = as.Clone()
	}
}

func BenchmarkArenaSlice_Clone(b *testing.B) {
	a := New(1024, BUMP)
	defer a.Delete()

	data := make([]int, 100)
	for i := range data {
		data[i] = i
	}
	as := MakeArenaSlice(a, data...)

	b.ResetTimer()
	b.ReportAllocs()
	for i := 0; i < b.N; i++ {
		_ = as.Clone()
	}
}

func BenchmarkArenaMap_Clone(b *testing.B) {
	a := New(1024, BUMP)
	defer a.Delete()

	m := NewMap[int, int](a)
	for i := 0; i < 100; i++ {
		m.Set(i, i*2)
	}

	b.ResetTimer()
	b.ReportAllocs()
	for i := 0; i < b.N; i++ {
		_ = m.Clone()
	}
}

func BenchmarkArenaSkipList_Clone(b *testing.B) {
	a := New(1024, BUMP)
	defer a.Delete()

	sl := NewSkipList[int, int](a)
	for i := 0; i < 100; i++ {
		sl.Insert(i, i*2)
	}

	b.ResetTimer()
	b.ReportAllocs()
	for i := 0; i < b.N; i++ {
		_ = sl.Clone()
	}
}

func BenchmarkArenaSkipList_CloneSlice(b *testing.B) {
	a := New(1024, BUMP)
	defer a.Delete()

	sl := NewSkipList[int, int](a)
	for i := 0; i < 100; i++ {
		sl.Insert(i, i*2)
	}

	b.ResetTimer()
	b.ReportAllocs()
	for i := 0; i < b.N; i++ {
		_ = sl.CloneSlice()
	}
}

// Test arena.CloneSlice (for MakeSlice)
func TestCloneSlice(t *testing.T) {
	a := New(10, BUMP)
	defer a.Delete()

	// Create arena-backed slice
	data := []int{1, 2, 3, 4, 5}
	arenaSlice := MakeSlice[int](a, 0, len(data))
	arenaSlice = append(arenaSlice, data...)

	// Clone it
	cloned := CloneSlice(arenaSlice)

	// Verify content
	if len(cloned) != len(arenaSlice) {
		t.Fatalf("len(CloneSlice()) = %d, want %d", len(cloned), len(arenaSlice))
	}
	for i := range arenaSlice {
		if cloned[i] != arenaSlice[i] {
			t.Errorf("CloneSlice()[%d] = %d, want %d", i, cloned[i], arenaSlice[i])
		}
	}

	// Modify arena slice
	arenaSlice[0] = 999

	// Cloned slice should be unchanged
	if cloned[0] == 999 {
		t.Errorf("Cloned slice was modified when arena slice changed")
	}

	// Delete arena
	a.Delete()

	// Cloned slice should still be valid
	for i, v := range data {
		if cloned[i] != v {
			t.Errorf("After arena delete: CloneSlice()[%d] = %d, want %d", i, cloned[i], v)
		}
	}
}

func TestCloneSlice_Empty(t *testing.T) {
	a := New(10, BUMP)
	defer a.Delete()

	arenaSlice := MakeSlice[int](a, 0, 0)
	cloned := CloneSlice(arenaSlice)

	if cloned != nil {
		t.Errorf("CloneSlice() of empty slice = %v, want nil", cloned)
	}
}

// Test arena.CloneString (for MakeString)
func TestCloneString(t *testing.T) {
	a := New(10, BUMP)
	defer a.Delete()

	// Create arena-backed string
	original := "hello world"
	arenaString := a.MakeString(original)

	// Clone it
	cloned := CloneString(arenaString)

	// Verify content
	if cloned != original {
		t.Errorf("CloneString() = %q, want %q", cloned, original)
	}

	// Delete arena
	a.Delete()

	// Cloned string should still be valid
	if cloned != original {
		t.Errorf("After arena delete: CloneString() = %q, want %q", cloned, original)
	}
}

func TestCloneString_Empty(t *testing.T) {
	a := New(10, BUMP)
	defer a.Delete()

	arenaString := a.MakeString("")
	cloned := CloneString(arenaString)

	if cloned != "" {
		t.Errorf("CloneString() of empty string = %q, want empty", cloned)
	}
}

// Benchmarks for arena Clone functions
func BenchmarkCloneSlice(b *testing.B) {
	a := New(1024, BUMP)
	defer a.Delete()

	data := make([]int, 100)
	for i := range data {
		data[i] = i
	}
	arenaSlice := MakeSlice[int](a, 0, len(data))
	arenaSlice = append(arenaSlice, data...)

	b.ResetTimer()
	b.ReportAllocs()
	for i := 0; i < b.N; i++ {
		_ = CloneSlice(arenaSlice)
	}
}

func BenchmarkCloneString(b *testing.B) {
	a := New(1024, BUMP)
	defer a.Delete()

	arenaString := a.MakeString("this is a test string that is long enough to demonstrate performance")

	b.ResetTimer()
	b.ReportAllocs()
	for i := 0; i < b.N; i++ {
		_ = CloneString(arenaString)
	}
}

// Test arena.CloneObject (for MakeObject)
func TestCloneObject(t *testing.T) {
	a := New(10, BUMP)
	defer a.Delete()

	type Person struct {
		ID   int
		Name string
		Age  int
	}

	// Create arena-backed object
	arenaObj := MakeObject[Person](a)
	arenaObj.ID = 123
	arenaObj.Name = "Alice"
	arenaObj.Age = 30

	// Clone it
	cloned := CloneObject(arenaObj)

	// Verify content
	if cloned.ID != arenaObj.ID || cloned.Name != arenaObj.Name || cloned.Age != arenaObj.Age {
		t.Errorf("CloneObject() content mismatch: got %+v, want %+v", cloned, arenaObj)
	}

	// Verify independence - modify arena object
	arenaObj.ID = 999
	arenaObj.Name = "Modified"
	arenaObj.Age = 99

	// Cloned object should be unchanged
	if cloned.ID == 999 || cloned.Name == "Modified" || cloned.Age == 99 {
		t.Errorf("Cloned object was modified when arena object changed")
	}
	if cloned.ID != 123 || cloned.Name != "Alice" || cloned.Age != 30 {
		t.Errorf("Cloned object data corrupted: got %+v", cloned)
	}

	// Delete arena
	a.Delete()

	// Cloned object should still be valid
	if cloned.ID != 123 || cloned.Name != "Alice" || cloned.Age != 30 {
		t.Errorf("After arena delete: CloneObject() = %+v, want {123 Alice 30}", cloned)
	}
}

func TestCloneObject_Nil(t *testing.T) {
	type Simple struct {
		Value int
	}

	var nilPtr *Simple
	cloned := CloneObject(nilPtr)

	if cloned != nil {
		t.Errorf("CloneObject(nil) = %v, want nil", cloned)
	}
}

func TestCloneObject_NestedStruct(t *testing.T) {
	a := New(10, BUMP)
	defer a.Delete()

	type Address struct {
		Street string
		City   string
	}

	type Person struct {
		Name    string
		Age     int
		Address Address
	}

	// Create nested struct in arena
	arenaObj := MakeObject[Person](a)
	arenaObj.Name = "Bob"
	arenaObj.Age = 25
	arenaObj.Address.Street = "123 Main St"
	arenaObj.Address.City = "Boston"

	// Clone it
	cloned := CloneObject(arenaObj)

	// Verify nested content
	if cloned.Name != "Bob" || cloned.Age != 25 {
		t.Errorf("Cloned person data incorrect: %+v", cloned)
	}
	if cloned.Address.Street != "123 Main St" || cloned.Address.City != "Boston" {
		t.Errorf("Cloned address data incorrect: %+v", cloned.Address)
	}

	// Modify arena object
	arenaObj.Address.City = "Modified"

	// Cloned should be unchanged
	if cloned.Address.City != "Boston" {
		t.Errorf("Cloned nested field was modified")
	}

	// Delete arena and verify
	a.Delete()
	if cloned.Address.City != "Boston" {
		t.Errorf("Cloned nested data corrupted after arena delete")
	}
}

func TestCloneObject_WithPointers(t *testing.T) {
	a := New(10, BUMP)
	defer a.Delete()

	type Node struct {
		Value int
		Next  *Node
	}

	// Create linked list in arena
	node1 := MakeObject[Node](a)
	node1.Value = 1

	node2 := MakeObject[Node](a)
	node2.Value = 2
	node1.Next = node2

	// Clone first node (shallow copy - Next still points to arena)
	cloned := CloneObject(node1)

	// Cloned value should match
	if cloned.Value != 1 {
		t.Errorf("CloneObject() Value = %d, want 1", cloned.Value)
	}

	// Note: This is a shallow copy, so Next still points to arena memory
	// This is expected behavior - deep copying would require custom logic
	if cloned.Next != node2 {
		t.Errorf("CloneObject() is not a shallow copy (Next pointer changed)")
	}

	// Verify independence of Value field
	node1.Value = 999
	if cloned.Value == 999 {
		t.Errorf("Cloned object Value was modified")
	}
}

func TestCloneObject_Array(t *testing.T) {
	a := New(10, BUMP)
	defer a.Delete()

	type Data struct {
		Values [5]int
		Count  int
	}

	arenaObj := MakeObject[Data](a)
	arenaObj.Values = [5]int{1, 2, 3, 4, 5}
	arenaObj.Count = 5

	cloned := CloneObject(arenaObj)

	// Verify array was copied
	for i := 0; i < 5; i++ {
		if cloned.Values[i] != i+1 {
			t.Errorf("CloneObject() Values[%d] = %d, want %d", i, cloned.Values[i], i+1)
		}
	}

	// Modify arena array
	arenaObj.Values[0] = 999

	// Cloned should be unchanged
	if cloned.Values[0] == 999 {
		t.Errorf("Cloned array was modified")
	}

	a.Delete()
	if cloned.Values[0] != 1 {
		t.Errorf("Cloned array corrupted after arena delete")
	}
}

// Benchmarks for CloneObject
func BenchmarkCloneObject_Simple(b *testing.B) {
	a := New(1024, BUMP)
	defer a.Delete()

	type Simple struct {
		Value int
	}

	obj := MakeObject[Simple](a)
	obj.Value = 42

	b.ResetTimer()
	b.ReportAllocs()
	for i := 0; i < b.N; i++ {
		_ = CloneObject(obj)
	}
}

func BenchmarkCloneObject_Complex(b *testing.B) {
	a := New(1024, BUMP)
	defer a.Delete()

	type Complex struct {
		ID      int
		Name    string
		Active  bool
		Payload [64]byte
	}

	obj := MakeObject[Complex](a)
	obj.ID = 123
	obj.Name = "test"
	obj.Active = true

	b.ResetTimer()
	b.ReportAllocs()
	for i := 0; i < b.N; i++ {
		_ = CloneObject(obj)
	}
}

func BenchmarkCloneObject_vs_HeapCopy(b *testing.B) {
	type Data struct {
		Values [10]int
		Count  int
	}

	b.Run("CloneObject", func(b *testing.B) {
		a := New(1024, BUMP)
		defer a.Delete()

		obj := MakeObject[Data](a)
		for i := 0; i < 10; i++ {
			obj.Values[i] = i
		}

		b.ResetTimer()
		b.ReportAllocs()
		for i := 0; i < b.N; i++ {
			_ = CloneObject(obj)
		}
	})

	b.Run("HeapCopy", func(b *testing.B) {
		obj := &Data{}
		for i := 0; i < 10; i++ {
			obj.Values[i] = i
		}

		b.ResetTimer()
		b.ReportAllocs()
		for i := 0; i < b.N; i++ {
			result := new(Data)
			*result = *obj
			_ = result
		}
	})
}
