// arena_slice.go
package arena

import (
	"unsafe"
)

// ArenaSlice[T] – the ultimate appendable slice in arena memory
// • Small slices → inline buffer (SSO-style)
// • Large slices → growable arena memory
// • Append/Push never touches the Go heap
type ArenaSlice[T any] struct {
	arena *Arena
	ptr   unsafe.Pointer // nil when using inline buffer
	len   int
	cap   int
	data  [16]T   // inline buffer – 16 elements of T (adjustable)
	flag  bool    // false = inline, true = arena-backed
	_     [7]byte // padding to 64 bytes (cache-line friendly)
}

// Len returns current length
func (s *ArenaSlice[T]) Len() int {
	return s.len
}

// Cap returns current capacity
func (s *ArenaSlice[T]) Cap() int {
	if !s.flag {
		return len(s.data)
	}
	return s.cap
}

// Slice returns the current slice (zero-copy)
func (s *ArenaSlice[T]) Slice() []T {
	if !s.flag {
		return s.data[:s.len]
	}
	return unsafe.Slice((*T)(s.ptr), s.cap)[:s.len]
}

// Append one element
func (s *ArenaSlice[T]) Append(v T) {
	s.ensureCapacity(s.len + 1)
	if !s.flag {
		s.data[s.len] = v
	} else {
		var zero T
		elemSize := unsafe.Sizeof(zero)
		*(*T)(unsafe.Add(s.ptr, elemSize*uintptr(s.len))) = v
	}
	s.len++
}

// AppendSlice appends multiple elements
func (s *ArenaSlice[T]) AppendSlice(src []T) {
	if len(src) == 0 {
		return
	}
	s.ensureCapacity(s.len + len(src))
	if !s.flag {
		copy(s.data[s.len:], src)
	} else {
		dst := unsafe.Slice((*T)(s.ptr), s.cap)
		copy(dst[s.len:], src)
	}
	s.len += len(src)
}

// ensureCapacity grows if needed
func (s *ArenaSlice[T]) ensureCapacity(needed int) {
	if needed <= s.Cap() {
		return
	}

	// Migrate to arena if still inline
	if !s.flag {
		s.migrateToArena(needed)
		return
	}

	// Grow arena-backed buffer
	newCap := s.cap * 2
	if newCap < needed {
		newCap = needed
	}
	if newCap < 64 {
		newCap = 64
	}

	var zero T
	elemSize := unsafe.Sizeof(zero)
	if elemSize == 0 {
		elemSize = 1
	}

	newPtr := s.arena.raw.Alloc(uint64(newCap)*uint64(elemSize), 16)
	if s.ptr != nil {
		copy(unsafe.Slice((*T)(newPtr), newCap), unsafe.Slice((*T)(s.ptr), s.cap))
	}
	s.ptr = newPtr
	s.cap = newCap
}

// migrateToArena moves inline data to arena
func (s *ArenaSlice[T]) migrateToArena(needed int) {
	newCap := len(s.data) * 2
	if newCap < needed {
		newCap = needed
	}
	if newCap < 64 {
		newCap = 64
	}

	var zero T
	elemSize := unsafe.Sizeof(zero)
	if elemSize == 0 {
		elemSize = 1
	}

	s.ptr = s.arena.raw.Alloc(uint64(newCap)*uint64(elemSize), 16)
	copy(unsafe.Slice((*T)(s.ptr), newCap), s.data[:])
	s.cap = newCap
	s.flag = true
}

// Reset keeps capacity, clears length
func (s *ArenaSlice[T]) Reset() {
	s.len = 0
	// Keep ptr/cap for reuse
}

// MakeArenaSlice creates a new ArenaSlice from initial data
func MakeArenaSlice[T any](a *Arena, initial ...T) ArenaSlice[T] {
	var as ArenaSlice[T]
	as.arena = a
	if len(initial) <= len(as.data) {
		copy(as.data[:], initial)
		as.len = len(initial)
		as.flag = false
	} else {
		as.AppendSlice(initial)
	}
	return as
}
