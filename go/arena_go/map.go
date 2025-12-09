// arena_map.go
package arena

import (
	"sync"
	"unsafe"
)

// ArenaMap is a high-performance, zero-GC hash map that lives entirely in arena memory.
// Thread-safe: All operations (Get, Set, Delete, Range) are protected by an RWMutex.
// Multiple goroutines can safely call Get concurrently, while Set/Delete operations are serialized.
type ArenaMap[K comparable, V any] struct {
	mu      sync.RWMutex
	arena   *Arena
	buckets unsafe.Pointer // []bucket[K, V]
	count   int
	cap     int
	mask    uint64
	// inline storage for small maps (SSO-style)
	inline [8]bucket[K, V]
	flag   bool // false = inline, true = arena-backed
}

// bucket holds one key-value pair + hash
type bucket[K comparable, V any] struct {
	hash uint64
	key  K
	val  V
}

// NewMap creates a new ArenaMap
func NewMap[K comparable, V any](a *Arena) *ArenaMap[K, V] {
	m := &ArenaMap[K, V]{
		arena: a,
		cap:   8,
		mask:  7,
		flag:  false,
	}
	// Zero-initialize inline buckets
	for i := range m.inline {
		m.inline[i] = bucket[K, V]{}
	}
	return m
}

// hash function using FNV-1a
func hashKey[K comparable](key K) uint64 {
	// FNV-1a hash offset basis
	const offset64 = 14695981039346656037
	const prime64 = 1099511628211

	// Get byte representation of the key
	var data []byte
	switch v := any(key).(type) {
	case string:
		data = unsafe.Slice(unsafe.StringData(v), len(v))
	case int:
		data = unsafe.Slice((*byte)(unsafe.Pointer(&v)), unsafe.Sizeof(v))
	case int8:
		data = unsafe.Slice((*byte)(unsafe.Pointer(&v)), unsafe.Sizeof(v))
	case int16:
		data = unsafe.Slice((*byte)(unsafe.Pointer(&v)), unsafe.Sizeof(v))
	case int32:
		data = unsafe.Slice((*byte)(unsafe.Pointer(&v)), unsafe.Sizeof(v))
	case int64:
		data = unsafe.Slice((*byte)(unsafe.Pointer(&v)), unsafe.Sizeof(v))
	case uint:
		data = unsafe.Slice((*byte)(unsafe.Pointer(&v)), unsafe.Sizeof(v))
	case uint8:
		data = unsafe.Slice((*byte)(unsafe.Pointer(&v)), unsafe.Sizeof(v))
	case uint16:
		data = unsafe.Slice((*byte)(unsafe.Pointer(&v)), unsafe.Sizeof(v))
	case uint32:
		data = unsafe.Slice((*byte)(unsafe.Pointer(&v)), unsafe.Sizeof(v))
	case uint64:
		data = unsafe.Slice((*byte)(unsafe.Pointer(&v)), unsafe.Sizeof(v))
	default:
		// For other types, use their memory representation
		data = unsafe.Slice((*byte)(unsafe.Pointer(&key)), unsafe.Sizeof(key))
	}

	// FNV-1a algorithm
	hash := uint64(offset64)
	for _, b := range data {
		hash ^= uint64(b)
		hash *= uint64(prime64)
	}
	return hash
}

// Set inserts or updates a key-value pair
func (m *ArenaMap[K, V]) Set(key K, value V) {
	m.mu.Lock()
	defer m.mu.Unlock()

	if m.count*2 > m.cap {
		m.grow()
	}

	hash := hashKey(key)
	index := hash & m.mask
	buckets := m.getBuckets()

	for {
		b := &buckets[index]

		if b.hash == 0 || b.hash == 0xFFFFFFFFFFFFFFFF {
			// Empty or tombstone → insert here
			b.hash = hash
			b.key = key
			b.val = value
			m.count++
			return
		}

		if b.hash == hash && b.key == key {
			// Update existing
			b.val = value
			return
		}

		index = (index + 1) & m.mask
	}
}

// Get returns value and true if found
func (m *ArenaMap[K, V]) Get(key K) (V, bool) {
	m.mu.RLock()
	defer m.mu.RUnlock()

	if m.cap == 0 {
		var zero V
		return zero, false
	}

	hash := hashKey(key)
	index := hash & m.mask
	buckets := m.getBuckets()

	for {
		b := &buckets[index]

		if b.hash == 0 {
			var zero V
			return zero, false // not found
		}
		if b.hash == 0xFFFFFFFFFFFFFFFF {
			// tombstone → continue probing
			index = (index + 1) & m.mask
			continue
		}
		if b.hash == hash && b.key == key {
			return b.val, true
		}
		index = (index + 1) & m.mask
		if index == (hash & m.mask) {
			break // full cycle
		}
	}

	var zero V
	return zero, false
}

// Delete removes a key
func (m *ArenaMap[K, V]) Delete(key K) {
	m.mu.Lock()
	defer m.mu.Unlock()

	if m.cap == 0 {
		return
	}

	hash := hashKey(key)
	index := hash & m.mask
	buckets := m.getBuckets()

	for {
		b := &buckets[index]

		if b.hash == 0 {
			return // not found
		}
		if b.hash == 0xFFFFFFFFFFFFFFFF {
			index = (index + 1) & m.mask
			continue
		}
		if b.hash == hash && b.key == key {
			b.hash = 0xFFFFFFFFFFFFFFFF // tombstone
			var zeroK K
			var zeroV V
			b.key = zeroK
			b.val = zeroV
			m.count--
			return
		}
		index = (index + 1) & m.mask
		if index == (hash & m.mask) {
			return
		}
	}
}

// Range calls f for each entry
func (m *ArenaMap[K, V]) Range(f func(K, V) bool) {
	m.mu.RLock()
	defer m.mu.RUnlock()

	buckets := m.getBuckets()
	for i := 0; i < m.cap; i++ {
		b := &buckets[i]
		if b.hash != 0 && b.hash != 0xFFFFFFFFFFFFFFFF {
			if !f(b.key, b.val) {
				return
			}
		}
	}
}

// Len returns number of entries
func (m *ArenaMap[K, V]) Len() int {
	m.mu.RLock()
	defer m.mu.RUnlock()
	return m.count
}

// grow doubles the map
func (m *ArenaMap[K, V]) grow() {
	oldBuckets := m.getBuckets()
	oldCap := m.cap

	newCap := oldCap * 2
	if newCap < 64 {
		newCap = 64
	}

	// Allocate new bucket array in arena
	var zero bucket[K, V]
	bucketSize := unsafe.Sizeof(zero)
	if bucketSize == 0 {
		bucketSize = 1
	}
	// Check for overflow
	if uint64(newCap) > (1<<63)/uint64(bucketSize) {
		panic("arena map: allocation size overflow")
	}
	newPtr := m.arena.raw.Alloc(uint64(newCap)*uint64(bucketSize), 16)
	newBuckets := unsafe.Slice((*bucket[K, V])(newPtr), newCap)

	// Zero-initialize new buckets
	for i := range newBuckets {
		newBuckets[i] = bucket[K, V]{}
	}

	// Update map metadata
	if !m.flag {
		m.flag = true
	}
	m.buckets = newPtr
	m.cap = newCap
	m.mask = uint64(newCap - 1)
	m.count = 0

	// Rehash all entries from old buckets
	for i := 0; i < oldCap; i++ {
		b := &oldBuckets[i]
		if b.hash != 0 && b.hash != 0xFFFFFFFFFFFFFFFF {
			// Re-insert into new buckets
			index := b.hash & m.mask
			for {
				nb := &newBuckets[index]
				if nb.hash == 0 {
					*nb = *b
					m.count++
					break
				}
				index = (index + 1) & m.mask
			}
		}
	}
}

// getBuckets returns current bucket slice
func (m *ArenaMap[K, V]) getBuckets() []bucket[K, V] {
	if !m.flag {
		return m.inline[:]
	}
	return unsafe.Slice((*bucket[K, V])(m.buckets), m.cap)
}

// Reset keeps capacity, clears all entries
func (m *ArenaMap[K, V]) Reset() {
	m.mu.Lock()
	defer m.mu.Unlock()

	buckets := m.getBuckets()
	for i := range buckets {
		buckets[i] = bucket[K, V]{}
	}
	m.count = 0
}
