// pool.go — Type-safe, zero-GC object pool using SlabAllocator
package arena

import (
	"sync"
	"syscall"
	"unsafe"
)

// Pool[T] is a high-performance, type-safe object pool.
// It reuses fixed-size memory blocks from the arena — perfect for AST nodes,
// query plans, protobuf messages, etc.
type Pool[T any] struct {
	arena    *Arena
	slab     *SlabAllocator // internal slab (one per T)
	freeList []unsafe.Pointer
	mu       sync.Mutex
}

// NewPool creates a new object pool for type T.
// Uses the arena's slab allocator under the hood.
func NewPool[T any](a *Arena) *Pool[T] {
	var zero T
	size := unsafe.Sizeof(zero)
	if size == 0 {
		size = 1
	}
	// Align to 16 bytes (cache line friendly)
	size = (size + 15) &^ 15

	// Reuse existing slab if possible, or create new
	// For simplicity and performance, we create one slab per Pool[T]
	slab := NewSlabAllocator(int(size), syscall.Getpagesize()*16) // initial 64KB

	return &Pool[T]{
		arena:    a,
		slab:     slab,
		freeList: make([]unsafe.Pointer, 0, 256),
	}
}

// Alloc returns a freshly zeroed T from the pool
func (p *Pool[T]) Alloc() *T {
	p.mu.Lock()
	defer p.mu.Unlock()

	if len(p.freeList) > 0 {
		ptr := p.freeList[len(p.freeList)-1]
		p.freeList = p.freeList[:len(p.freeList)-1]
		// Zero the memory (optional but safe)
		var zero T
		*(*T)(ptr) = zero
		return (*T)(ptr)
	}

	// Allocate from slab
	ptr := p.slab.Alloc(uint64(unsafe.Sizeof(*new(T))), 16)
	return (*T)(ptr)
}

// Free returns an object to the pool (optional — Reset() clears all)
func (p *Pool[T]) Free(obj *T) {
	if obj == nil {
		return
	}
	p.mu.Lock()
	defer p.mu.Unlock()
	p.freeList = append(p.freeList, unsafe.Pointer(obj))
}

// Reset clears the free list (all objects become reusable via Alloc)
func (p *Pool[T]) Reset() {
	p.mu.Lock()
	defer p.mu.Unlock()
	p.freeList = p.freeList[:0]
}

// Delete frees all underlying memory
func (p *Pool[T]) Delete() {
	p.mu.Lock()
	defer p.mu.Unlock()
	p.slab.Delete()
	p.freeList = nil
}
