// pool.go — Type-safe, zero-GC object pool using Arena allocator
package arena

import (
	"sync"
	"unsafe"
)

// Pool[T] is a high-performance, type-safe object pool that allocates from an Arena.
// It reuses freed objects via an internal free list, reducing allocation pressure.
// Perfect for AST nodes, query plans, protobuf messages, and other frequently allocated objects.
//
// Thread Safety:
//   - All operations (Alloc, Free, Reset) are thread-safe
//   - Multiple goroutines can safely allocate and free concurrently
//   - Pool shares the Arena's lifecycle - when Arena is deleted, all Pool memory is freed
type Pool[T any] struct {
	arena    *Arena
	size     uintptr
	freeList []unsafe.Pointer
	mu       sync.Mutex
}

// NewPool creates a new object pool for type T that allocates from the given Arena.
// All allocations are 16-byte aligned for optimal performance.
func NewPool[T any](a *Arena) *Pool[T] {
	var zero T
	size := unsafe.Sizeof(zero)
	if size == 0 {
		size = 1
	}
	// Align to 16 bytes (cache line friendly)
	size = (size + 15) &^ 15

	return &Pool[T]{
		arena:    a,
		size:     size,
		freeList: make([]unsafe.Pointer, 0, 256),
	}
}

// Alloc returns a freshly zeroed T from the pool.
// If the free list is not empty, reuses a previously freed object.
// Otherwise, allocates new memory from the Arena.
func (p *Pool[T]) Alloc() *T {
	p.mu.Lock()
	defer p.mu.Unlock()

	if len(p.freeList) > 0 {
		ptr := p.freeList[len(p.freeList)-1]
		p.freeList = p.freeList[:len(p.freeList)-1]
		// Zero the memory for safety
		var zero T
		*(*T)(ptr) = zero
		return (*T)(ptr)
	}

	// Allocate from the Arena
	ptr := p.arena.raw.Alloc(uint64(p.size), 16)
	return (*T)(ptr)
}

// Free returns an object to the pool's free list for reuse.
// The object must have been allocated by this Pool's Alloc() method.
// It's safe to call Free(nil).
//
// Note: Freed objects are not returned to the Arena - they're held in the
// Pool's free list until Reset() is called or the Arena is deleted.
func (p *Pool[T]) Free(obj *T) {
	if obj == nil {
		return
	}
	p.mu.Lock()
	defer p.mu.Unlock()
	p.freeList = append(p.freeList, unsafe.Pointer(obj))
}

// Reset clears the free list, making all freed objects eligible for reuse.
// This does not free memory back to the Arena - use Arena.Reset() for that.
func (p *Pool[T]) Reset() {
	p.mu.Lock()
	defer p.mu.Unlock()
	p.freeList = p.freeList[:0]
}

// Len returns the number of objects currently in the free list.
func (p *Pool[T]) Len() int {
	p.mu.Lock()
	defer p.mu.Unlock()
	return len(p.freeList)
}
