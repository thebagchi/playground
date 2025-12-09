// buddy.go
package arena

import (
	"math/bits"
	"sync"
	"unsafe"
)

// ----------------------- BUDDY (fully correct) -----------------------
type BuddyAllocator struct {
	chunkSize uint64
	chunks    [][]byte
	free      [][]int // free[order] = list of block indices
	order     int
	mu        sync.Mutex
}

func NewBuddyAllocator(chunkSize, numChunks int) *BuddyAllocator {
	if chunkSize&(chunkSize-1) != 0 {
		panic("chunkSize must be power of 2")
	}
	order := bits.Len(uint(chunkSize)) - 1
	b := &BuddyAllocator{
		chunkSize: uint64(chunkSize),
		order:     order,
		free:      make([][]int, order+1),
	}
	for range numChunks {
		b.addChunk()
	}
	return b
}

func (b *BuddyAllocator) addChunk() {
	var (
		chunk = MakePages(int(b.chunkSize))
		idx   = len(b.chunks)
	)
	b.chunks = append(b.chunks, chunk)
	b.free[b.order] = append(b.free[b.order], idx<<b.order)
}

func (b *BuddyAllocator) Alloc(size, align uint64) unsafe.Pointer {
	if size == 0 {
		size = 1
	}
	// Calculate aligned size first
	alignedSize := (size + align - 1) &^ (align - 1)
	if alignedSize == 0 {
		alignedSize = 1
	}
	// Find the order (power of 2) needed for this size
	// For alignedSize <= 1, order = 0 (2^0 = 1 byte)
	// For alignedSize 2, order = 1 (2^1 = 2 bytes)
	// For alignedSize 3-4, order = 2 (2^2 = 4 bytes), etc.
	order := 0
	if alignedSize > 1 {
		order = bits.Len64(alignedSize - 1)
	}
	// Clamp to maximum order
	if order > b.order {
		order = b.order
	}

	b.mu.Lock()
	defer b.mu.Unlock()

	for o := order; o <= b.order; o++ {
		if len(b.free[o]) > 0 {
			block := b.free[o][len(b.free[o])-1]
			b.free[o] = b.free[o][:len(b.free[o])-1]

			// split down
			cur := block
			for split := o; split > order; split-- {
				var (
					left  = cur
					right = cur + (1 << (split - 1))
				)
				b.free[split-1] = append(b.free[split-1], left, right)
				cur = left
			}

			chunkIdx := cur >> b.order
			offset := cur & ((1 << b.order) - 1)
			return unsafe.Pointer(&b.chunks[chunkIdx][offset])
		}
	}
	// No suitable block found, add a new chunk (lock already held)
	b.addChunk()
	// Now the new chunk is in free[b.order], allocate from it
	if len(b.free[b.order]) > 0 {
		block := b.free[b.order][len(b.free[b.order])-1]
		b.free[b.order] = b.free[b.order][:len(b.free[b.order])-1]

		// split down
		cur := block
		for split := b.order; split > order; split-- {
			var (
				left  = cur
				right = cur + (1 << (split - 1))
			)
			b.free[split-1] = append(b.free[split-1], left, right)
			cur = left
		}

		chunkIdx := cur >> b.order
		offset := cur & ((1 << b.order) - 1)
		return unsafe.Pointer(&b.chunks[chunkIdx][offset])
	}
	// This should never happen
	panic("buddy allocator: failed to allocate after growing")
}

func (b *BuddyAllocator) Reset() {
	b.mu.Lock()
	defer b.mu.Unlock()
	for i := range b.free {
		b.free[i] = b.free[i][:0]
	}
	for i := range b.chunks {
		b.free[b.order] = append(b.free[b.order], i<<b.order)
	}
}

func (b *BuddyAllocator) Delete() {
	b.mu.Lock()
	defer b.mu.Unlock()
	for _, c := range b.chunks {
		ReleasePages(c)
	}
	b.chunks, b.free = nil, nil
}
