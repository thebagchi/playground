package arena

import "syscall"

// MakePages allocates memory pages using mmap.
func MakePages(size int) []byte {
	data, err := syscall.Mmap(-1, 0, size, syscall.PROT_READ|syscall.PROT_WRITE, syscall.MAP_PRIVATE|syscall.MAP_ANONYMOUS)
	if err != nil {
		panic(err)
	}
	return data
}

// ReleasePages frees memory pages allocated with MakePages.
func ReleasePages(data []byte) {
	syscall.Munmap(data)
}
