// arena_string.go
package arena

import (
	"unsafe"
)

const ssoMax = 23

// ArenaString – the ultimate string builder for arena allocators
// • ≤ 23 bytes → SSO (zero allocation)
// • > 23 bytes → arena-backed, growable
// • append() never allocates from heap
type ArenaString struct {
	arena *Arena
	// Layout when SSO (flag == false):
	//   data[0:len] = string bytes
	//   data[len]   = 0 (null terminator, optional)
	// Layout when arena-backed (flag == true):
	//   ptr = pointer to arena memory
	//   len, cap = current length and capacity
	ptr  unsafe.Pointer // only used when flag == true
	len  int
	cap  int
	data [24]byte // inline buffer for SSO
	flag bool     // false = SSO, true = arena-backed
}

// String returns the current string value
func (s *ArenaString) String() string {
	if !s.flag {
		// SSO path – zero-copy
		return string(s.data[:s.len])
	}
	return unsafe.String((*byte)(s.ptr), s.len)
}

// Len returns current length
func (s *ArenaString) Len() int {
	return s.len
}

// Cap returns current capacity
func (s *ArenaString) Cap() int {
	if !s.flag {
		return len(s.data)
	}
	return s.cap
}

// append bytes – never touches the Go heap
func (s *ArenaString) Append(bytes []byte) {
	if len(bytes) == 0 {
		return
	}

	if !s.flag {
		// Currently SSO
		avail := len(s.data) - s.len
		if avail >= len(bytes) {
			// Fits in SSO buffer
			copy(s.data[s.len:], bytes)
			s.len += len(bytes)
			return
		}

		// SSO overflow → migrate to arena
		s.migrateToArena()
	}

	// Arena-backed path
	s.growIfNeeded(len(bytes))
	copy(unsafe.Slice((*byte)(s.ptr), s.cap)[s.len:], bytes)
	s.len += len(bytes)
}

// AppendString – convenience
func (s *ArenaString) AppendString(str string) {
	s.Append(unsafe.Slice(unsafe.StringData(str), len(str)))
}

// growIfNeeded ensures capacity >= len + needed
func (s *ArenaString) growIfNeeded(needed int) {
	if s.len+needed <= s.cap {
		return
	}
	newCap := s.cap * 2
	if newCap < s.len+needed {
		newCap = s.len + needed
	}
	if newCap < 64 {
		newCap = 64
	}

	newPtr := s.arena.raw.Alloc(uint64(newCap), 8)
	if s.ptr != nil {
		copy(unsafe.Slice((*byte)(newPtr), newCap), unsafe.Slice((*byte)(s.ptr), s.len))
	}
	s.ptr = newPtr
	s.cap = newCap
}

// migrateToArena moves SSO content to arena memory
func (s *ArenaString) migrateToArena() {
	s.growIfNeeded(0) // allocates at least 64 bytes
	copy(unsafe.Slice((*byte)(s.ptr), s.cap), s.data[:s.len])
	s.flag = true
}

// Reset clears the string (keeps capacity if arena-backed)
func (s *ArenaString) Reset() {
	s.len = 0
	// Keep arena memory – will be reused
}

// Clone returns a heap-allocated copy of the string that escapes the arena.
// The returned string is independent of the arena lifecycle and can be safely
// used after the arena is deleted. Use this when you need to preserve string
// data beyond the arena's lifetime.
func (s *ArenaString) Clone() string {
	if s.len == 0 {
		return ""
	}
	// Create a new heap-allocated string
	return string(s.Bytes())
}

// Bytes returns a copy of the string content as a heap-allocated byte slice.
// The returned slice is independent of the arena and safe to use after arena deletion.
func (s *ArenaString) Bytes() []byte {
	if s.len == 0 {
		return nil
	}
	if !s.flag {
		// SSO path - copy from inline buffer
		b := make([]byte, s.len)
		copy(b, s.data[:s.len])
		return b
	}
	// Arena path - copy from arena memory
	b := make([]byte, s.len)
	copy(b, unsafe.Slice((*byte)(s.ptr), s.len))
	return b
}

// MakeArenaString now returns ArenaString builder
func (a *Arena) MakeArenaString(s string) ArenaString {
	var as ArenaString
	as.arena = a
	if len(s) <= ssoMax {
		copy(as.data[:], s)
		as.len = len(s)
		as.flag = false
	} else {
		as.AppendString(s) // uses arena path
	}
	return as
}
