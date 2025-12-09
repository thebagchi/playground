# Arena: High-Performance Go Memory Allocators

Arena is a high-performance, zero-GC memory allocation library for Go that provides arena-based memory management with multiple allocation strategies.

## Features

- **Three Allocation Strategies**: Bump, Slab, and Buddy allocators
- **Thread-Safe**: All allocators and collections are fully thread-safe with RWMutex protection
- **Zero-GC**: All allocations live in arena memory, avoiding GC pressure
- **Generic API**: Type-safe allocation of any Go type
- **Specialized Collections**: ArenaString (with SSO), ArenaSlice (with inline buffers), ArenaMap (hash map), and ArenaSkipList (ordered map)
- **Automatic Growth**: Allocators grow automatically when needed
- **Memory Efficient**: Uses mmap for large allocations

## Thread Safety Guarantees

### Allocators (Bump, Slab, Buddy)
- **Concurrent Alloc()**: ✅ Safe - Multiple goroutines can allocate concurrently
- **Concurrent Reset()**: ❌ Not safe with Alloc() - Must synchronize externally
- **Concurrent Delete()**: ❌ Not safe with Alloc() - Must synchronize externally
- **Multiple Arenas**: ✅ Independent - No synchronization needed between different Arena instances

### ArenaMap
- **Concurrent Get()**: ✅ Safe - Multiple readers can access concurrently
- **Concurrent Set()**: ✅ Safe - Writes are serialized, readers don't block each other
- **Concurrent Delete()**: ✅ Safe - Writes are serialized
- **Concurrent Range()**: ✅ Safe - Iteration holds read lock

### ArenaSkipList
- **Concurrent Search()**: ✅ Safe - Multiple readers can search concurrently
- **Concurrent Insert()**: ✅ Safe - Writes are serialized, readers don't block each other
- **Concurrent Delete()**: ✅ Safe - Writes are serialized
- **Concurrent Range()**: ✅ Safe - Iteration holds read lock

### Best Practices
1. Create Arena instances per-goroutine or use a single Arena with thread-safe allocations
2. Never call Reset() or Delete() while other goroutines are using the arena
3. ArenaMap and ArenaSkipList can be safely shared across goroutines
4. For maximum performance in single-threaded scenarios, use per-thread arenas

## Quick Start

```go
package main

import (
    "fmt"
    "arena"
)

func main() {
    // Create an arena with 4 pages (16 KiB) using bump allocator
    a := arena.New(4, arena.BUMP)
    defer a.Delete()

    // Allocate individual values
    x := arena.Alloc[int](a)
    *x = 42

    // Allocate slices
    slice := arena.MakeSlice[int](a, 0, 10)
    slice = append(slice, 1, 2, 3)

    // Allocate strings
    str := a.MakeString("Hello, Arena!")

    // Use specialized collections
    s := a.MakeArenaString("initial")
    s.AppendString(" more text")

    arr := arena.MakeArenaSlice[int](a, 1, 2, 3)
    arr.Append(4)

    m := arena.NewMap[string, int](a)
    m.Set("key", 123)

    fmt.Printf("x=%d, slice=%v, str=%s\n", *x, slice, str)
    fmt.Printf("arena string: %s\n", s.String())
    fmt.Printf("arena slice: %v\n", arr.Slice())
    if val, ok := m.Get("key"); ok {
        fmt.Printf("map value: %d\n", val)
    }
}
```

## Allocators

### Bump Allocator (`arena.BUMP`)
- **Fastest**: Simple pointer bumping with minimal overhead
- **Best for**: Short-lived allocations, sequential allocation patterns
- **Growth**: Adds new memory pages as needed

### Slab Allocator (`arena.SLAB`)
- **Fixed-size blocks**: Allocates in 256-byte blocks (configurable)
- **Best for**: Many small allocations of similar sizes
- **Reuse**: Freed blocks are reused automatically

### Buddy Allocator (`arena.BUDDY`)
- **Power-of-two sizes**: Allocates in power-of-two sized blocks
- **Best for**: Variable-sized allocations, general-purpose use
- **Efficient**: Minimizes fragmentation through buddy system

## API Reference

### Arena Creation

```go
// Create arena with specified pages and allocator type
a := arena.New(pages int, typ arena.Allocator)

// Available allocator types
arena.BUMP   // Fast bump pointer
arena.SLAB   // Fixed-size block reuse
arena.BUDDY  // Power-of-two buddy system

// Cleanup
defer a.Delete()
```

### Basic Allocation

```go
// Allocate any type T
ptr := arena.Alloc[T](a)

// Allocate slices
slice := arena.MakeSlice[T](a, length, capacity)

// Allocate strings
str := a.MakeString("text")
```

### Specialized Collections

#### ArenaString - String Builder with SSO

```go
// Create string builder
s := a.MakeArenaString("initial")

// Append operations
s.AppendString(" more")
s.Append([]byte(" bytes"))

// Get result
result := s.String()
length := s.Len()
capacity := s.Cap()

// Reset for reuse
s.Reset()
```

**Features:**
- ≤ 23 bytes: Stored inline (zero allocation)
- \> 23 bytes: Arena-backed, grows automatically
- Append never touches Go heap

#### ArenaSlice - Dynamic Arrays with Inline Buffers

```go
// Create slice with initial values
arr := arena.MakeArenaSlice[int](a, 1, 2, 3)

// Append operations
arr.Append(4)
arr.AppendSlice([]int{5, 6})

// Access data
data := arr.Slice()  // zero-copy
length := arr.Len()
capacity := arr.Cap()

// Reset for reuse
arr.Reset()
```

**Features:**
- ≤ 16 elements: Stored inline (zero allocation)
- \> 16 elements: Arena-backed, grows automatically
- Append never touches Go heap

#### ArenaSkipList - Ordered Map with O(log n) Operations

```go
// Create ordered map (skip list)
m := arena.NewSkipList[string, int](a)

// Insert operations maintain sorted order
m.Insert("apple", 1)
m.Insert("banana", 2)
m.Insert("cherry", 3)

// Search in O(log n) time
if val, ok := m.Search("banana"); ok {
    // found
}

// Iterate in sorted order
m.Range(func(k string, v int) bool {
    fmt.Printf("%s: %d\n", k, v)
    return true // continue iteration
})

// Min/Max operations
if key, val, ok := m.Min(); ok {
    // smallest key
}
if key, val, ok := m.Max(); ok {
    // largest key
}

m.Delete("banana")
m.Reset() // clear all entries
```

**Features:**
- Ordered key-value storage with O(log n) operations
- Probabilistic skip list with configurable maximum level (16)
- Thread-safe with RWMutex
- Arena-backed - zero GC pressure

### Memory Management

```go
// Reset arena (keep capacity, clear contents)
a.Reset()

// Free all memory
a.Delete()
```

## Performance Characteristics

| Data Structure | Operations | Time Complexity | Space Usage | Best For |
|----------------|------------|-----------------|-------------|----------|
| ArenaMap | Get/Set/Delete | O(1) average | Medium | Fast lookups, unordered data |
| ArenaSkipList | Search/Insert/Delete | O(log n) | Medium-High | Ordered data, range queries |
| ArenaString | Append | O(1) amortized | Low | String building |
| ArenaSlice | Append | O(1) amortized | Low | Dynamic arrays |
| Bump Allocator | Alloc | O(1) | High (no reuse) | Sequential allocations |
| Slab Allocator | Alloc | O(1) | Medium | Many small objects |
| Buddy Allocator | Alloc | O(log n) | Low | Variable sizes |

## Performance Benchmarks

Comparison of arena allocations vs standard Go allocations (on Intel Core i5-5250U @ 1.60GHz):

### Allocator Performance
```
BenchmarkBumpAllocator/size-8        14M ops/sec    28 ns/op    0 allocs
BenchmarkBumpAllocator/size-64       14M ops/sec    28 ns/op    0 allocs
BenchmarkSlabAllocator/size-8         4M ops/sec    53 ns/op    0 allocs
BenchmarkBuddyAllocator/size-8        9M ops/sec    35 ns/op    0 allocs
BenchmarkStdAlloc/size-64             1M ops/sec    79 ns/op    1 allocs
```

### ArenaMap vs Standard Map
```
BenchmarkArenaMap_Set                 2M ops/sec   357 ns/op    0 allocs
BenchmarkStdMap_Set                   1M ops/sec   272 ns/op    0 allocs (74B)
BenchmarkArenaMap_Get                 9M ops/sec    42 ns/op    0 allocs
BenchmarkStdMap_Get                  15M ops/sec    19 ns/op    0 allocs
```

**Key Insights:**
- Arena allocations produce **zero GC allocations**, reducing GC pressure
- Bump allocator is fastest for sequential allocations (~28ns/op)
- ArenaMap trades some read speed for zero-allocation writes
- Best for: batch processing, short-lived request contexts, parsers, compilers

### Memory Alignment

All allocations are 16-byte aligned for optimal performance and compatibility with ARM architectures.

## Error Handling

- Allocation size overflow: Panics with descriptive message
- Invalid allocator type: Falls back to Bump allocator
- Memory allocation failure: Panics (mmap failure)

## Testing

The package includes comprehensive tests including:
- Unit tests for all allocators and collections
- Thread-safety validation tests
- Race condition detection (run with `-race`)
- Comprehensive benchmarks

Run tests:
```bash
go test -v .                    # Basic tests
go test -race .                 # With race detector
go test -bench=. -benchmem      # Performance benchmarks
```

## Examples

See the `*_test.go` files for comprehensive usage examples:

- `bump_test.go` - Basic allocation patterns
- `slab_test.go` - Slab allocator usage
- `buddy_test.go` - Buddy allocator and collections
- `bench_test.go` - Performance benchmarks and concurrent usage patterns

## Benchmarks

Run benchmarks with:
```bash
go test -bench=. -benchmem
```

## Implementation Details

- **Memory Backend**: Uses `mmap` for page-aligned allocations
- **Hash Function**: FNV-1a for ArenaMap
- **Load Factor**: 50% for ArenaMap to reduce collisions
- **SSO Thresholds**: 23 bytes for strings, 16 elements for slices
- **Alignment**: 16-byte minimum alignment

## Contributing

1. Ensure all tests pass: `go test -race .`
2. Format code: `go fmt .`
3. Add tests for new features
4. Update documentation

## License

See LICENSE file for details.