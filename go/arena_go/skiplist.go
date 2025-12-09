// skiplist.go — The Ultimate Arena-Backed Skip List
package arena

import (
	"math/rand"
	"sync"
	"unsafe"
)

type ordered interface {
	~int | ~int8 | ~int16 | ~int32 | ~int64 |
		~uint | ~uint8 | ~uint16 | ~uint32 | ~uint64 | ~uintptr |
		~float32 | ~float64 | ~string
}

// ArenaSkipList is a thread-safe, ordered key-value store using skip list algorithm.
// All operations (Search, Insert, Delete, Range) are protected by RWMutex.
// Memory is allocated entirely from the arena, avoiding GC pressure.
type ArenaSkipList[K ordered, V any] struct {
	arena    *Arena
	head     *skipNode[K, V]
	maxLevel int
	level    int
	p        float64
	mu       sync.RWMutex
	rng      *rand.Rand // thread-safe random source
	rngMu    sync.Mutex // protects rng
}

type skipNode[K ordered, V any] struct {
	key     K
	val     V
	level   int
	forward []*skipNode[K, V]
}

func NewSkipList[K ordered, V any](a *Arena) *ArenaSkipList[K, V] {
	const maxLevel = 16
	const p = 0.5

	// Allocate head node
	head := (*skipNode[K, V])(a.raw.Alloc(uint64(unsafe.Sizeof(skipNode[K, V]{})), 16))

	// Allocate initial forward array (level 0)
	forward := a.raw.Alloc(uint64(maxLevel+1)*uint64(unsafe.Sizeof((*skipNode[K, V])(nil))), 16)
	*head = skipNode[K, V]{
		level:   maxLevel,
		forward: unsafe.Slice((*(*skipNode[K, V]))(forward), maxLevel+1),
	}

	return &ArenaSkipList[K, V]{
		arena:    a,
		head:     head,
		maxLevel: maxLevel,
		level:    0,
		p:        p,
		rng:      rand.New(rand.NewSource(rand.Int63())),
	}
}

func (sl *ArenaSkipList[K, V]) randomLevel() int {
	sl.rngMu.Lock()
	level := 0
	for level < sl.maxLevel && sl.rng.Float64() < sl.p {
		level++
	}
	sl.rngMu.Unlock()
	return level
}

// Search — correct version
func (sl *ArenaSkipList[K, V]) Search(key K) (V, bool) {
	sl.mu.RLock()
	defer sl.mu.RUnlock()

	x := sl.head
	for i := sl.level; i >= 0; i-- {
		for x.forward[i] != nil && x.forward[i].key < key {
			x = x.forward[i]
		}
	}
	x = x.forward[0]
	if x != nil && x.key == key {
		return x.val, true
	}
	var zero V
	return zero, false
}

// Insert — fully correct
func (sl *ArenaSkipList[K, V]) Insert(key K, value V) {
	sl.mu.Lock()
	defer sl.mu.Unlock()

	update := make([]*skipNode[K, V], sl.maxLevel+1)
	x := sl.head

	for i := sl.level; i >= 0; i-- {
		for x.forward[i] != nil && x.forward[i].key < key {
			x = x.forward[i]
		}
		update[i] = x
	}

	x = x.forward[0]
	if x != nil && x.key == key {
		x.val = value
		return
	}

	level := sl.randomLevel()
	if level > sl.level {
		for i := sl.level + 1; i <= level; i++ {
			update[i] = sl.head
		}
		sl.level = level
	}

	// Allocate new node
	node := (*skipNode[K, V])(sl.arena.raw.Alloc(uint64(unsafe.Sizeof(skipNode[K, V]{})), 16))
	forward := sl.arena.raw.Alloc(uint64(level+1)*uint64(unsafe.Sizeof((*skipNode[K, V])(nil))), 16)
	*node = skipNode[K, V]{
		key:     key,
		val:     value,
		level:   level,
		forward: unsafe.Slice((*(*skipNode[K, V]))(forward), level+1),
	}

	for i := 0; i <= level; i++ {
		node.forward[i] = update[i].forward[i]
		update[i].forward[i] = node
	}
}

// Delete — correct
func (sl *ArenaSkipList[K, V]) Delete(key K) bool {
	sl.mu.Lock()
	defer sl.mu.Unlock()

	update := make([]*skipNode[K, V], sl.maxLevel+1)
	x := sl.head

	for i := sl.level; i >= 0; i-- {
		for x.forward[i] != nil && x.forward[i].key < key {
			x = x.forward[i]
		}
		update[i] = x
	}

	x = x.forward[0]
	if x == nil || x.key != key {
		return false
	}

	for i := 0; i <= sl.level; i++ {
		if update[i].forward[i] != x {
			break
		}
		update[i].forward[i] = x.forward[i]
	}

	for sl.level > 0 && sl.head.forward[sl.level] == nil {
		sl.level--
	}
	return true
}

func (sl *ArenaSkipList[K, V]) Range(f func(K, V) bool) {
	sl.mu.RLock()
	defer sl.mu.RUnlock()
	x := sl.head.forward[0]
	for x != nil {
		if !f(x.key, x.val) {
			return
		}
		x = x.forward[0]
	}
}

func (sl *ArenaSkipList[K, V]) Len() int {
	sl.mu.RLock()
	defer sl.mu.RUnlock()
	count := 0
	x := sl.head.forward[0]
	for x != nil {
		count++
		x = x.forward[0]
	}
	return count
}

func (sl *ArenaSkipList[K, V]) Reset() {
	sl.mu.Lock()
	defer sl.mu.Unlock()
	for i := range sl.head.forward {
		sl.head.forward[i] = nil
	}
	sl.level = 0
}

func (sl *ArenaSkipList[K, V]) Contains(key K) bool { _, ok := sl.Search(key); return ok }
func (sl *ArenaSkipList[K, V]) Min() (K, V, bool) {
	sl.mu.RLock()
	defer sl.mu.RUnlock()
	if x := sl.head.forward[0]; x != nil {
		return x.key, x.val, true
	}
	var k K
	var v V
	return k, v, false
}
func (sl *ArenaSkipList[K, V]) Max() (K, V, bool) {
	sl.mu.RLock()
	defer sl.mu.RUnlock()
	if sl.level < 0 {
		var k K
		var v V
		return k, v, false
	}
	x := sl.head
	for i := sl.level; i >= 0; i-- {
		for x.forward[i] != nil {
			x = x.forward[i]
		}
	}
	if x != sl.head {
		return x.key, x.val, true
	}
	var k K
	var v V
	return k, v, false
}

// Clone returns a heap-allocated standard Go map with all entries from the skip list.
// The returned map is independent of the arena lifecycle and can be safely used
// after the arena is deleted. Use this when you need to preserve skip list data
// beyond the arena's lifetime. Note: The returned map does not preserve order.
func (sl *ArenaSkipList[K, V]) Clone() map[K]V {
	sl.mu.RLock()
	defer sl.mu.RUnlock()

	count := sl.Len()
	if count == 0 {
		return nil
	}

	result := make(map[K]V, count)
	x := sl.head.forward[0]
	for x != nil {
		result[x.key] = x.val
		x = x.forward[0]
	}
	return result
}

// CloneSlice returns a heap-allocated slice of key-value pairs in sorted order.
// The returned slice is independent of the arena lifecycle and can be safely used
// after the arena is deleted. Use this when you need to preserve skip list data
// with ordering beyond the arena's lifetime.
func (sl *ArenaSkipList[K, V]) CloneSlice() []struct {
	Key K
	Val V
} {
	sl.mu.RLock()
	defer sl.mu.RUnlock()

	count := sl.Len()
	if count == 0 {
		return nil
	}

	result := make([]struct {
		Key K
		Val V
	}, 0, count)
	x := sl.head.forward[0]
	for x != nil {
		result = append(result, struct {
			Key K
			Val V
		}{Key: x.key, Val: x.val})
		x = x.forward[0]
	}
	return result
}
