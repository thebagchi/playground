// bump.go
package arena

import (
	"sync"
	"unsafe"
)

// ----------------------- BUMP (fastest) -----------------------
type BumpAllocator struct {
	chunks [][]byte
	cur    int
	off    int
	mu     sync.Mutex
}

func NewBumpAllocator(size int) *BumpAllocator {
	return &BumpAllocator{chunks: [][]byte{MakePages(size)}}
}

func (b *BumpAllocator) Alloc(size, align uint64) unsafe.Pointer {
	b.mu.Lock()
	defer b.mu.Unlock()

	aligned := (b.off + int(align-1)) &^ int(align-1)
	if aligned+int(size) > len(b.chunks[b.cur]) {
		// grow
		if b.cur+1 >= len(b.chunks) {
			b.chunks = append(b.chunks, MakePages(len(b.chunks[0])))
		}
		b.cur++
		b.off = 0
		aligned = 0
	}
	ptr := unsafe.Pointer(&b.chunks[b.cur][aligned])
	b.off = aligned + int(size)
	return ptr
}

func (b *BumpAllocator) Reset() {
	b.mu.Lock()
	b.cur, b.off = 0, 0
	b.mu.Unlock()
}

func (b *BumpAllocator) Delete() {
	b.mu.Lock()
	for _, c := range b.chunks {
		ReleasePages(c)
	}
	b.chunks = nil
	b.mu.Unlock()
}
