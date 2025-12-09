package arena

import (
	"testing"
	"unsafe"
)

// Test types for MakeObject
type SimpleStruct struct {
	Value int
}

type ComplexStruct struct {
	ID      int
	Name    string
	Active  bool
	Payload [64]byte
}

type Node struct {
	Value int
	Next  *Node
	Prev  *Node
}

type EmptyStruct struct{}

func TestMakeObject_Simple(t *testing.T) {
	a := New(1, BUMP)
	defer a.Delete()

	obj := MakeObject[SimpleStruct](a)
	if obj == nil {
		t.Fatal("MakeObject returned nil")
	}

	// Should be zero-initialized
	if obj.Value != 0 {
		t.Errorf("Expected zero-initialized Value, got %d", obj.Value)
	}

	// Modify and verify
	obj.Value = 42
	if obj.Value != 42 {
		t.Errorf("Expected Value=42, got %d", obj.Value)
	}
}

func TestMakeObject_Complex(t *testing.T) {
	a := New(1, BUDDY)
	defer a.Delete()

	obj := MakeObject[ComplexStruct](a)
	if obj == nil {
		t.Fatal("MakeObject returned nil")
	}

	// Verify zero-initialization
	if obj.ID != 0 {
		t.Errorf("Expected zero-initialized ID, got %d", obj.ID)
	}
	if obj.Name != "" {
		t.Errorf("Expected empty Name, got %q", obj.Name)
	}
	if obj.Active {
		t.Errorf("Expected Active=false, got true")
	}

	// Modify fields
	obj.ID = 123
	obj.Name = "test"
	obj.Active = true
	obj.Payload[0] = 0xFF

	// Verify modifications
	if obj.ID != 123 || obj.Name != "test" || !obj.Active || obj.Payload[0] != 0xFF {
		t.Errorf("Field modifications not preserved")
	}
}

func TestMakeObject_LinkedList(t *testing.T) {
	a := New(10, BUMP)
	defer a.Delete()

	// Create a linked list
	head := MakeObject[Node](a)
	head.Value = 1

	second := MakeObject[Node](a)
	second.Value = 2
	head.Next = second
	second.Prev = head

	third := MakeObject[Node](a)
	third.Value = 3
	second.Next = third
	third.Prev = second

	// Verify structure
	if head.Next != second {
		t.Error("head.Next doesn't point to second")
	}
	if second.Prev != head {
		t.Error("second.Prev doesn't point to head")
	}
	if second.Next != third {
		t.Error("second.Next doesn't point to third")
	}
	if third.Prev != second {
		t.Error("third.Prev doesn't point to second")
	}

	// Traverse forward
	values := []int{}
	for n := head; n != nil; n = n.Next {
		values = append(values, n.Value)
	}
	if len(values) != 3 || values[0] != 1 || values[1] != 2 || values[2] != 3 {
		t.Errorf("Forward traversal failed: got %v, want [1 2 3]", values)
	}

	// Traverse backward
	values = []int{}
	for n := third; n != nil; n = n.Prev {
		values = append(values, n.Value)
	}
	if len(values) != 3 || values[0] != 3 || values[1] != 2 || values[2] != 1 {
		t.Errorf("Backward traversal failed: got %v, want [3 2 1]", values)
	}
}

func TestMakeObject_EmptyStruct(t *testing.T) {
	a := New(1, BUMP)
	defer a.Delete()

	obj := MakeObject[EmptyStruct](a)
	if obj == nil {
		t.Fatal("MakeObject returned nil for empty struct")
	}

	// Empty struct should have size 0, but we allocate 1 byte minimum
	size := unsafe.Sizeof(*obj)
	if size != 0 {
		t.Logf("Empty struct has size %d (expected 0, arena allocates 1 byte minimum)", size)
	}
}

func TestMakeObject_MultipleObjects(t *testing.T) {
	a := New(10, SLAB)
	defer a.Delete()

	// Allocate multiple objects
	objs := make([]*SimpleStruct, 100)
	for i := 0; i < 100; i++ {
		objs[i] = MakeObject[SimpleStruct](a)
		objs[i].Value = i
	}

	// Verify all objects
	for i := 0; i < 100; i++ {
		if objs[i].Value != i {
			t.Errorf("Object %d has Value=%d, want %d", i, objs[i].Value, i)
		}
	}

	// Verify objects are distinct
	objs[0].Value = 999
	if objs[1].Value == 999 {
		t.Error("Objects are not distinct")
	}
}

func TestMakeObject_ResetArena(t *testing.T) {
	a := New(10, BUMP)
	defer a.Delete()

	obj1 := MakeObject[SimpleStruct](a)
	obj1.Value = 42
	ptr1 := uintptr(unsafe.Pointer(obj1))

	// Reset arena
	a.Reset()

	// Allocate new object (should reuse memory)
	obj2 := MakeObject[SimpleStruct](a)
	ptr2 := uintptr(unsafe.Pointer(obj2))

	// After Reset, memory is reused but may not be zeroed
	// (this is arena semantics - raw memory reuse)
	if ptr1 == ptr2 {
		t.Logf("Memory reused after Reset at address 0x%x (expected behavior)", ptr1)
		// Note: Reset doesn't guarantee zero-initialization in all allocators
		// The memory contains whatever was there before
	}

	// Explicitly set to verify we can use the memory
	obj2.Value = 100
	if obj2.Value != 100 {
		t.Errorf("Cannot modify object after Reset")
	}
}

func TestMakeObject_DifferentAllocators(t *testing.T) {
	tests := []struct {
		name      string
		allocator Allocator
	}{
		{"BUMP", BUMP},
		{"SLAB", SLAB},
		{"BUDDY", BUDDY},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			a := New(10, tt.allocator)
			defer a.Delete()

			obj := MakeObject[ComplexStruct](a)
			if obj == nil {
				t.Fatal("MakeObject returned nil")
			}

			obj.ID = 100
			obj.Name = tt.name
			obj.Active = true

			if obj.ID != 100 || obj.Name != tt.name || !obj.Active {
				t.Errorf("Object fields not preserved with %s allocator", tt.name)
			}
		})
	}
}

func TestMakeObject_Alignment(t *testing.T) {
	a := New(10, BUMP)
	defer a.Delete()

	// Test different types with different alignment requirements
	type Aligned8 struct {
		_ uint64
	}
	type Aligned4 struct {
		_ uint32
	}
	type Aligned2 struct {
		_ uint16
	}
	type Aligned1 struct {
		_ uint8
	}

	obj8 := MakeObject[Aligned8](a)
	obj4 := MakeObject[Aligned4](a)
	obj2 := MakeObject[Aligned2](a)
	obj1 := MakeObject[Aligned1](a)

	// Check alignment (pointer should be divisible by alignment)
	addr8 := uintptr(unsafe.Pointer(obj8))
	addr4 := uintptr(unsafe.Pointer(obj4))
	addr2 := uintptr(unsafe.Pointer(obj2))
	addr1 := uintptr(unsafe.Pointer(obj1))

	if addr8%8 != 0 {
		t.Errorf("8-byte aligned object has address 0x%x (not 8-byte aligned)", addr8)
	}
	if addr4%4 != 0 {
		t.Errorf("4-byte aligned object has address 0x%x (not 4-byte aligned)", addr4)
	}
	if addr2%2 != 0 {
		t.Errorf("2-byte aligned object has address 0x%x (not 2-byte aligned)", addr2)
	}
	// addr1 always aligned (any address works)
	_ = addr1
}

// Benchmarks
func BenchmarkMakeObject_Simple(b *testing.B) {
	a := New(1024, BUMP)
	defer a.Delete()

	b.ResetTimer()
	b.ReportAllocs()
	for i := 0; i < b.N; i++ {
		obj := MakeObject[SimpleStruct](a)
		obj.Value = i
	}
}

func BenchmarkMakeObject_Complex(b *testing.B) {
	a := New(1024, BUDDY)
	defer a.Delete()

	b.ResetTimer()
	b.ReportAllocs()
	for i := 0; i < b.N; i++ {
		obj := MakeObject[ComplexStruct](a)
		obj.ID = i
	}
}

func BenchmarkMakeObject_vs_HeapAlloc(b *testing.B) {
	b.Run("MakeObject", func(b *testing.B) {
		a := New(1024, BUMP)
		defer a.Delete()

		b.ResetTimer()
		b.ReportAllocs()
		for i := 0; i < b.N; i++ {
			obj := MakeObject[SimpleStruct](a)
			obj.Value = i
		}
	})

	b.Run("HeapAlloc", func(b *testing.B) {
		b.ResetTimer()
		b.ReportAllocs()
		for i := 0; i < b.N; i++ {
			obj := &SimpleStruct{}
			obj.Value = i
		}
	})
}

func BenchmarkMakeObject_LinkedList(b *testing.B) {
	a := New(1024*10, BUMP)
	defer a.Delete()

	b.ResetTimer()
	b.ReportAllocs()
	for i := 0; i < b.N; i++ {
		// Create a small linked list
		head := MakeObject[Node](a)
		current := head
		for j := 0; j < 10; j++ {
			next := MakeObject[Node](a)
			next.Value = j
			current.Next = next
			next.Prev = current
			current = next
		}
	}
}
