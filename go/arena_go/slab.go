// slab.go
package arena

import (
	"sync"
	"syscall"
	"unsafe"
)

// ----------------------- SLAB (fixed-size reuse) -----------------------
type SlabAllocator struct {
	blockSize uintptr
	free      []unsafe.Pointer
	blocks    [][]byte
	mu        sync.Mutex
}

func NewSlabAllocator(blockSize, totalBytes int) *SlabAllocator {
	if blockSize < 16 {
		blockSize = 16
	}
	blockSize = (blockSize + 15) &^ 15
	s := &SlabAllocator{blockSize: uintptr(blockSize)}
	var (
		pageSize      = syscall.Getpagesize()
		blocksPerPage = pageSize / int(blockSize)
		pages         = (totalBytes + pageSize - 1) / pageSize
	)
	for i := 0; i < pages; i++ {
		page := MakePages(pageSize)
		s.blocks = append(s.blocks, page)
		for j := range blocksPerPage {
			s.free = append(s.free, unsafe.Pointer(&page[j*int(blockSize)]))
		}
	}
	return s
}

func (s *SlabAllocator) Alloc(size, _ uint64) unsafe.Pointer {
	if size > uint64(s.blockSize) {
		panic("size exceeds slab block size")
	}
	s.mu.Lock()
	defer s.mu.Unlock()
	if len(s.free) == 0 {
		// grow one page
		page := MakePages(syscall.Getpagesize())
		s.blocks = append(s.blocks, page)
		for j := range syscall.Getpagesize() / int(s.blockSize) {
			s.free = append(s.free, unsafe.Pointer(&page[j*int(s.blockSize)]))
		}
	}
	ptr := s.free[len(s.free)-1]
	s.free = s.free[:len(s.free)-1]
	return ptr
}

func (s *SlabAllocator) Reset() {
	s.mu.Lock()
	defer s.mu.Unlock()
	s.free = s.free[:0]
	for _, b := range s.blocks {
		for i := range len(b) / int(s.blockSize) {
			s.free = append(s.free, unsafe.Pointer(&b[i*int(s.blockSize)]))
		}
	}
}

func (s *SlabAllocator) Delete() {
	s.mu.Lock()
	defer s.mu.Unlock()
	for _, b := range s.blocks {
		ReleasePages(b)
	}
	s.blocks, s.free = nil, nil
}
