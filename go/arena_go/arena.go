// arena/arena.go
//
// Package arena provides high-performance, zero-GC memory allocators with multiple strategies.
//
// Thread Safety:
//   - All allocators (Bump, Slab, Buddy) are thread-safe and can be used concurrently
//   - Alloc() operations are serialized with mutexes to prevent data races
//   - Reset() and Delete() should NOT be called concurrently with Alloc() or with each other
//   - Multiple Arena instances are completely independent and require no synchronization
//
// Memory Model:
//   - All memory is allocated via mmap and lives outside Go's garbage collector
//   - Memory is never returned to the OS until Delete() is called
//   - Reset() clears allocations but retains underlying memory pages
//
// Allocator Strategies:
//   - BUMP: Fastest, best for batch allocations or when arena is reset frequently
//   - SLAB: Best for fixed-size objects with high allocation/free turnover
//   - BUDDY: Most flexible, good for varied-size allocations with power-of-2 sizes
package arena

import (
	"syscall"
	"unsafe"
)

// ---------------------------------------------------------------
// Public API – one arena for all types
// ---------------------------------------------------------------

type Allocator int

const (
	BUMP Allocator = iota
	SLAB
	BUDDY
)

// Arena is the beautiful multi-type facade.
// Thread-safe: Multiple goroutines can safely call Alloc concurrently.
// The underlying allocator handles synchronization internally.
type Arena struct {
	raw RawArena // internal low-level allocator
}

// New creates an arena. pages == 0 → 1 page (4 KiB default)
func New(pages int, typ Allocator) *Arena {
	if pages <= 0 {
		pages = 1 // ← your request: treat 0 as 1
	}
	totalBytes := pages * syscall.Getpagesize()

	var raw RawArena
	switch typ {
	case BUMP:
		raw = NewBumpAllocator(totalBytes)
	case SLAB:
		raw = NewSlabAllocator(256, totalBytes) // configurable block size
	case BUDDY:
		raw = NewBuddyAllocator(syscall.Getpagesize(), pages)
	default:
		raw = NewBumpAllocator(totalBytes)
	}
	return &Arena{raw: raw}
}

// Alloc any type T
func Alloc[T any](a *Arena) *T {
	var zero T
	size := unsafe.Sizeof(zero)
	if size == 0 {
		size = 1
	}
	ptr := a.raw.Alloc(uint64(size), 16)
	return (*T)(ptr)
}

// MakeSlice – slice backed by arena memory
func MakeSlice[T any](a *Arena, length, capacity int) []T {
	if capacity == 0 {
		return nil
	}
	var zero T
	elemSize := unsafe.Sizeof(zero)
	if elemSize == 0 {
		elemSize = 1
	}
	// Check for overflow
	if uint64(capacity) > (1<<63)/uint64(elemSize) {
		panic("arena: slice allocation size overflow")
	}
	ptr := a.raw.Alloc(uint64(capacity)*uint64(elemSize), 16)
	slice := unsafe.Slice((*T)(ptr), capacity)
	return slice[:length]
}

// MakeString – zero-copy string that lives in the arena
func (a *Arena) MakeString(s string) string {
	if len(s) == 0 {
		return ""
	}
	ptr := a.raw.Alloc(uint64(len(s)), 1)
	copy((*[1 << 30]byte)(ptr)[:len(s):len(s)], s)
	return unsafe.String((*byte)(ptr), len(s))
}

func (a *Arena) Reset() {
	a.raw.Reset()
}
func (a *Arena) Delete() {
	a.raw.Delete()
}

// ---------------------------------------------------------------
// Internal raw allocators (all support growing)
// ---------------------------------------------------------------

type RawArena interface {
	Alloc(size, align uint64) unsafe.Pointer
	Reset()
	Delete()
}
