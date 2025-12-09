package arena

import (
	"fmt"
	"math/rand"
	"sync"
	"testing"
)

// ========================================================================
// Allocator Benchmarks
// ========================================================================

func BenchmarkBumpAllocator(b *testing.B) {
	sizes := []int{8, 64, 256, 1024, 4096}
	for _, size := range sizes {
		b.Run(fmt.Sprintf("size-%d", size), func(b *testing.B) {
			a := New(1024, BUMP)
			defer a.Delete()
			b.ResetTimer()
			b.ReportAllocs()
			for i := 0; i < b.N; i++ {
				_ = a.raw.Alloc(uint64(size), 16)
			}
		})
	}
}

func BenchmarkSlabAllocator(b *testing.B) {
	sizes := []int{8, 64, 256, 1024}
	for _, size := range sizes {
		b.Run(fmt.Sprintf("size-%d", size), func(b *testing.B) {
			a := NewSlabAllocator(size, 1024*1024)
			defer a.Delete()
			b.ResetTimer()
			b.ReportAllocs()
			for i := 0; i < b.N; i++ {
				_ = a.Alloc(uint64(size), 16)
			}
		})
	}
}

func BenchmarkBuddyAllocator(b *testing.B) {
	sizes := []int{8, 64, 256, 1024, 4096}
	for _, size := range sizes {
		b.Run(fmt.Sprintf("size-%d", size), func(b *testing.B) {
			a := NewBuddyAllocator(4096, 256)
			defer a.Delete()
			b.ResetTimer()
			b.ReportAllocs()
			for i := 0; i < b.N; i++ {
				_ = a.Alloc(uint64(size), 16)
			}
		})
	}
}

// Comparison with standard Go allocation
func BenchmarkStdAlloc(b *testing.B) {
	sizes := []int{8, 64, 256, 1024, 4096}
	for _, size := range sizes {
		b.Run(fmt.Sprintf("size-%d", size), func(b *testing.B) {
			b.ReportAllocs()
			for i := 0; i < b.N; i++ {
				_ = make([]byte, size)
			}
		})
	}
}

// ========================================================================
// ArenaMap Benchmarks
// ========================================================================

func BenchmarkArenaMap_Set(b *testing.B) {
	a := New(10240, BUMP) // Use larger arena for benchmarks
	defer a.Delete()
	m := NewMap[int, int](a)

	b.ResetTimer()
	b.ReportAllocs()
	for i := 0; i < b.N; i++ {
		m.Set(i, i*2)
	}
}

func BenchmarkArenaMap_Get(b *testing.B) {
	a := New(10240, BUMP)
	defer a.Delete()
	m := NewMap[int, int](a)

	// Prepopulate
	for i := 0; i < 1000; i++ {
		m.Set(i, i*2)
	}

	b.ResetTimer()
	b.ReportAllocs()
	for i := 0; i < b.N; i++ {
		_, _ = m.Get(i % 1000)
	}
}

func BenchmarkArenaMap_SetParallel(b *testing.B) {
	a := New(10240, BUMP)
	defer a.Delete()
	m := NewMap[int, int](a)

	b.ResetTimer()
	b.ReportAllocs()
	b.RunParallel(func(pb *testing.PB) {
		i := 0
		for pb.Next() {
			m.Set(i, i*2)
			i++
		}
	})
}

func BenchmarkArenaMap_GetParallel(b *testing.B) {
	a := New(10240, BUMP)
	defer a.Delete()
	m := NewMap[int, int](a)

	// Prepopulate
	for i := 0; i < 10000; i++ {
		m.Set(i, i*2)
	}

	b.ResetTimer()
	b.ReportAllocs()
	b.RunParallel(func(pb *testing.PB) {
		i := 0
		for pb.Next() {
			_, _ = m.Get(i % 10000)
			i++
		}
	})
}

func BenchmarkStdMap_Set(b *testing.B) {
	m := make(map[int]int)
	b.ResetTimer()
	b.ReportAllocs()
	for i := 0; i < b.N; i++ {
		m[i] = i * 2
	}
}

func BenchmarkStdMap_Get(b *testing.B) {
	m := make(map[int]int)
	for i := 0; i < 1000; i++ {
		m[i] = i * 2
	}

	b.ResetTimer()
	b.ReportAllocs()
	for i := 0; i < b.N; i++ {
		_ = m[i%1000]
	}
}

// ========================================================================
// ArenaSkipList Benchmarks
// ========================================================================

func BenchmarkArenaSkipList_Insert(b *testing.B) {
	a := New(10240, BUMP)
	defer a.Delete()
	sl := NewSkipList[int, int](a)

	b.ResetTimer()
	b.ReportAllocs()
	for i := 0; i < b.N; i++ {
		sl.Insert(i, i*2)
	}
}

func BenchmarkArenaSkipList_Search(b *testing.B) {
	a := New(10240, BUMP)
	defer a.Delete()
	sl := NewSkipList[int, int](a)

	// Prepopulate
	for i := 0; i < 1000; i++ {
		sl.Insert(i, i*2)
	}

	b.ResetTimer()
	b.ReportAllocs()
	for i := 0; i < b.N; i++ {
		_, _ = sl.Search(i % 1000)
	}
}

func BenchmarkArenaSkipList_InsertParallel(b *testing.B) {
	a := New(10240, BUMP)
	defer a.Delete()
	sl := NewSkipList[int, int](a)

	b.ResetTimer()
	b.ReportAllocs()
	b.RunParallel(func(pb *testing.PB) {
		i := 0
		for pb.Next() {
			sl.Insert(i, i*2)
			i++
		}
	})
}

func BenchmarkArenaSkipList_SearchParallel(b *testing.B) {
	a := New(10240, BUMP)
	defer a.Delete()
	sl := NewSkipList[int, int](a)

	// Prepopulate
	for i := 0; i < 10000; i++ {
		sl.Insert(i, i*2)
	}

	b.ResetTimer()
	b.ReportAllocs()
	b.RunParallel(func(pb *testing.PB) {
		i := 0
		for pb.Next() {
			_, _ = sl.Search(i % 10000)
			i++
		}
	})
}

// ========================================================================
// ArenaString Benchmarks
// ========================================================================

func BenchmarkArenaString_Append_Small(b *testing.B) {
	a := New(10240, BUMP)
	defer a.Delete()

	b.ResetTimer()
	b.ReportAllocs()
	for i := 0; i < b.N; i++ {
		s := &ArenaString{arena: a}
		s.AppendString("hello")
		s.AppendString("world")
	}
}

func BenchmarkArenaString_Append_Large(b *testing.B) {
	a := New(10240, BUMP)
	defer a.Delete()
	longStr := string(make([]byte, 1000))

	b.ResetTimer()
	b.ReportAllocs()
	for i := 0; i < b.N; i++ {
		s := &ArenaString{arena: a}
		s.AppendString(longStr)
	}
}

func BenchmarkStdString_Append_Small(b *testing.B) {
	b.ReportAllocs()
	for i := 0; i < b.N; i++ {
		s := ""
		s += "hello"
		s += "world"
		_ = s
	}
}

func BenchmarkStdString_Append_Large(b *testing.B) {
	longStr := string(make([]byte, 1000))

	b.ReportAllocs()
	for i := 0; i < b.N; i++ {
		s := ""
		s += longStr
		_ = s
	}
}

// ========================================================================
// ArenaSlice Benchmarks
// ========================================================================

func BenchmarkArenaSlice_Append(b *testing.B) {
	a := New(10240, BUMP)
	defer a.Delete()

	b.ResetTimer()
	b.ReportAllocs()
	for i := 0; i < b.N; i++ {
		s := &ArenaSlice[int]{arena: a}
		for j := 0; j < 100; j++ {
			s.Append(j)
		}
	}
}

func BenchmarkStdSlice_Append(b *testing.B) {
	b.ReportAllocs()
	for i := 0; i < b.N; i++ {
		s := make([]int, 0)
		for j := 0; j < 100; j++ {
			s = append(s, j)
		}
	}
}

// ========================================================================
// Concurrent Stress Tests
// ========================================================================

func BenchmarkConcurrent_Mixed_ArenaMap(b *testing.B) {
	a := New(10240, BUMP)
	defer a.Delete()
	m := NewMap[int, int](a)

	// Prepopulate
	for i := 0; i < 1000; i++ {
		m.Set(i, i*2)
	}

	b.ResetTimer()
	b.ReportAllocs()
	b.RunParallel(func(pb *testing.PB) {
		i := 0
		for pb.Next() {
			if i%3 == 0 {
				m.Set(rand.Intn(10000), i)
			} else {
				_, _ = m.Get(rand.Intn(10000))
			}
			i++
		}
	})
}

func BenchmarkConcurrent_Mixed_SkipList(b *testing.B) {
	a := New(10240, BUMP)
	defer a.Delete()
	sl := NewSkipList[int, int](a)

	// Prepopulate
	for i := 0; i < 1000; i++ {
		sl.Insert(i, i*2)
	}

	b.ResetTimer()
	b.ReportAllocs()
	b.RunParallel(func(pb *testing.PB) {
		i := 0
		for pb.Next() {
			if i%3 == 0 {
				sl.Insert(rand.Intn(10000), i)
			} else {
				_, _ = sl.Search(rand.Intn(10000))
			}
			i++
		}
	})
}

func BenchmarkConcurrent_Allocators(b *testing.B) {
	allocators := []struct {
		name string
		typ  Allocator
	}{
		{"Bump", BUMP},
		{"Slab", SLAB},
		{"Buddy", BUDDY},
	}

	for _, alloc := range allocators {
		b.Run(alloc.name, func(b *testing.B) {
			a := New(1024, alloc.typ)
			defer a.Delete()

			b.ResetTimer()
			b.ReportAllocs()
			b.RunParallel(func(pb *testing.PB) {
				for pb.Next() {
					size := uint64(64 + rand.Intn(192)) // 64-256 bytes
					_ = a.raw.Alloc(size, 16)
				}
			})
		})
	}
}

// ========================================================================
// Memory Usage Benchmarks
// ========================================================================

func BenchmarkMemoryEfficiency_ArenaVsStd(b *testing.B) {
	b.Run("Arena", func(b *testing.B) {
		a := New(1024, BUMP)
		defer a.Delete()

		b.ResetTimer()
		b.ReportAllocs()
		for i := 0; i < b.N; i++ {
			for j := 0; j < 100; j++ {
				_ = Alloc[int](a)
			}
			a.Reset()
		}
	})

	b.Run("Standard", func(b *testing.B) {
		b.ReportAllocs()
		for i := 0; i < b.N; i++ {
			for j := 0; j < 100; j++ {
				_ = new(int)
			}
		}
	})
}

// ========================================================================
// Thread Safety Validation Test
// ========================================================================

func TestThreadSafety_ArenaMap(t *testing.T) {
	a := New(1024, BUMP)
	defer a.Delete()
	m := NewMap[int, int](a)

	var wg sync.WaitGroup
	workers := 10
	iterations := 1000

	wg.Add(workers)
	for w := 0; w < workers; w++ {
		go func(workerID int) {
			defer wg.Done()
			for i := 0; i < iterations; i++ {
				key := workerID*iterations + i
				m.Set(key, key*2)
				val, ok := m.Get(key)
				if !ok || val != key*2 {
					t.Errorf("Worker %d: expected %d, got %d, ok=%v", workerID, key*2, val, ok)
				}
			}
		}(w)
	}
	wg.Wait()

	// Verify count
	if m.Len() != workers*iterations {
		t.Errorf("Expected %d entries, got %d", workers*iterations, m.Len())
	}
}

func TestThreadSafety_SkipList(t *testing.T) {
	a := New(1024, BUMP)
	defer a.Delete()
	sl := NewSkipList[int, int](a)

	var wg sync.WaitGroup
	workers := 10
	iterations := 1000

	wg.Add(workers)
	for w := 0; w < workers; w++ {
		go func(workerID int) {
			defer wg.Done()
			for i := 0; i < iterations; i++ {
				key := workerID*iterations + i
				sl.Insert(key, key*2)
				val, ok := sl.Search(key)
				if !ok || val != key*2 {
					t.Errorf("Worker %d: expected %d, got %d, ok=%v", workerID, key*2, val, ok)
				}
			}
		}(w)
	}
	wg.Wait()

	// Verify count
	if sl.Len() != workers*iterations {
		t.Errorf("Expected %d entries, got %d", workers*iterations, sl.Len())
	}
}

func TestThreadSafety_Allocators(t *testing.T) {
	allocators := []struct {
		name string
		typ  Allocator
	}{
		{"Bump", BUMP},
		{"Slab", SLAB},
		{"Buddy", BUDDY},
	}

	for _, alloc := range allocators {
		t.Run(alloc.name, func(t *testing.T) {
			a := New(1024, alloc.typ)
			defer a.Delete()

			var wg sync.WaitGroup
			workers := 10
			iterations := 1000

			wg.Add(workers)
			for w := 0; w < workers; w++ {
				go func() {
					defer wg.Done()
					for i := 0; i < iterations; i++ {
						ptr := Alloc[int](a)
						if ptr == nil {
							t.Errorf("Allocation failed")
						}
						*ptr = i
					}
				}()
			}
			wg.Wait()
		})
	}
}
