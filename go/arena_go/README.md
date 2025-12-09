# Arena: High-Performance Go Memory Allocators

Arena is a high-performance, zero-GC memory allocation library for Go that provides arena-based memory management with multiple allocation strategies.

## Features

- **Three Allocation Strategies**: Bump, Slab, and Buddy allocators
- **Thread-Safe**: All allocators and collections are fully thread-safe with RWMutex protection
- **Zero-GC**: All allocations live in arena memory, avoiding GC pressure
- **Generic API**: Type-safe allocation of any Go type
- **Specialized Collections**: ArenaString (with SSO), ArenaSlice (with inline buffers), ArenaMap (hash map), ArenaSkipList (ordered map), and Pool (object pooling)
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

### Pool[T]
- **Concurrent Alloc()**: ✅ Safe - Multiple goroutines can allocate concurrently
- **Concurrent Free()**: ✅ Safe - Multiple goroutines can free concurrently
- **Concurrent Reset()**: ✅ Safe - Can be called concurrently with Alloc/Free
- **Arena Lifecycle**: Pool memory is freed when Arena is deleted

### Best Practices
1. Create Arena instances per-goroutine or use a single Arena with thread-safe allocations
2. Never call Reset() or Delete() while other goroutines are using the arena
3. ArenaMap, ArenaSkipList, and Pool can be safely shared across goroutines
4. For maximum performance in single-threaded scenarios, use per-thread arenas
5. Use Pool for frequently allocated/freed objects of the same type

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

    // Allocate struct objects (recommended for structs)
    type Node struct { Value int; Next *Node }
    node := arena.MakeObject[Node](a)
    node.Value = 100

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
// Allocate any type T (generic)
ptr := arena.Alloc[T](a)

// Allocate struct objects (recommended for structs)
type Node struct {
    Value int
    Next  *Node
}
node := arena.MakeObject[Node](a)
node.Value = 42

// Allocate slices
slice := arena.MakeSlice[T](a, length, capacity)

// Allocate strings
str := a.MakeString("text")
```

**MakeObject vs Alloc:**
- `MakeObject[T]` - Returns `*T`, uses proper alignment for the type, recommended for struct types
- `Alloc[T]` - Returns `*T`, generic allocation with default 16-byte alignment

**Example: Building a linked list**
```go
// Create arena-backed linked list
head := arena.MakeObject[Node](a)
head.Value = 1

second := arena.MakeObject[Node](a)
second.Value = 2
head.Next = second

third := arena.MakeObject[Node](a)
third.Value = 3
second.Next = third

// Traverse (all nodes live in arena, zero heap allocations)
for n := head; n != nil; n = n.Next {
    fmt.Println(n.Value)
}
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

### Pool - Object Pooling

```go
type Node struct {
    Value int
    Left  *Node
    Right *Node
}

// Create a pool for Node objects
pool := arena.NewPool[Node](a)

// Allocate from pool (reuses freed objects)
node := pool.Alloc()
node.Value = 42

// Use the node...

// Return to pool for reuse
pool.Free(node)

// Allocate again - reuses the freed node
node2 := pool.Alloc() // Returns zeroed, reused memory

// Check free list size
fmt.Printf("Free objects: %d\n", pool.Len())

// Clear free list (memory stays in arena)
pool.Reset()
```

**Features:**
- Type-safe object pooling with zero GC allocations
- Automatic memory zeroing on reuse
- Thread-safe with mutex protection
- Perfect for frequently allocated/freed objects (AST nodes, packets, messages)
- Memory is freed when Arena is deleted

**Use Cases:**
- Parsing: AST nodes, tokens
- Networking: packet buffers, connection objects
- Game engines: entity components, events
- Database: query plans, result sets

### Escaping Arena Memory to Heap

When you need data to outlive the arena, use Clone methods to create heap-allocated copies:

```go
a := arena.New(1024, arena.BUMP)
defer a.Delete()

// For collections (ArenaMap, ArenaSkipList, ArenaString, ArenaSlice)
m := arena.NewMap[string, int](a, 10)
m.Set("key", 42)
heapMap := m.Clone()  // Returns heap-allocated map[string]int

sl := arena.NewSkipList[int, string](a)
sl.Insert(1, "one")
heapMap2 := sl.Clone()              // Returns map[int]string
heapSlice := sl.CloneSlice()        // Returns sorted []struct{Key int; Val string}

str := arena.NewString(a)
str.WriteString("hello")
heapString := str.Clone()           // Returns heap-allocated string
heapBytes := str.Bytes()            // Returns heap-allocated []byte

arr := arena.NewSlice[int](a, 0, 10)
arr.Append(1, 2, 3)
heapSlice2 := arr.Clone()           // Returns heap-allocated []int

// For arena-backed primitives (MakeString, MakeSlice, MakeObject)
arenaStr := a.MakeString("arena string")
heapStr := arena.CloneString(arenaStr)  // Survives arena deletion

arenaSlice := arena.MakeSlice[int](a, 0, 5)
arenaSlice = append(arenaSlice, 1, 2, 3)
heapSlice3 := arena.CloneSlice(arenaSlice)  // Survives arena deletion

type Person struct { Name string; Age int }
arenaObj := arena.MakeObject[Person](a)
arenaObj.Name = "Alice"
arenaObj.Age = 30
heapObj := arena.CloneObject(arenaObj)  // Survives arena deletion

// Now safe to delete arena
a.Delete()

// All heap-allocated clones remain valid
fmt.Println(heapMap["key"])    // 42
fmt.Println(heapString)         // "hello"
fmt.Println(heapStr)            // "arena string"
fmt.Println(heapSlice3)         // [1 2 3]
fmt.Println(heapObj.Name)       // "Alice"
```

**Clone Methods:**
- `ArenaMap.Clone()` → `map[K]V`
- `ArenaSkipList.Clone()` → `map[K]V`
- `ArenaSkipList.CloneSlice()` → `[]struct{Key K; Val V}` (sorted)
- `ArenaString.Clone()` → `string`
- `ArenaString.Bytes()` → `[]byte`
- `ArenaSlice.Clone()` → `[]T`
- `arena.CloneString(string)` → `string` (for MakeString results)
- `arena.CloneSlice[T]([]T)` → `[]T` (for MakeSlice results)
- `arena.CloneObject[T](*T)` → `*T` (for MakeObject results)

**Important Notes:**
- `CloneObject` performs a **shallow copy** - pointer fields will still point to arena memory
- For deep copying of complex structures, implement custom clone logic
- Clone methods are optimized for speed - minimal overhead for copying data

**Use Cases:**
- Returning data from request handlers after arena cleanup
- Caching arena-computed results in long-lived structures
- Migrating partial results between processing stages
- Preserving data across arena resets

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
| Pool | Alloc/Free | O(1) | Low | Object reuse, frequent alloc/free |
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

### MakeObject Performance
```
BenchmarkMakeObject_Simple           29M ops/sec    37 ns/op    0 allocs
BenchmarkMakeObject_Complex          14M ops/sec    85 ns/op    0 allocs
BenchmarkMakeObject_LinkedList        2M ops/sec   544 ns/op    0 allocs (10 nodes)
BenchmarkMakeObject_vs_HeapAlloc:
  - MakeObject                       29M ops/sec    37 ns/op    0 allocs
  - HeapAlloc (new(T))              2.4B ops/sec   0.4 ns/op    0 allocs*
```
*Note: Heap allocations show 0 allocs in microbenchmarks due to compiler optimizations, but produce GC pressure in real workloads.

### ArenaMap vs Standard Map
```
BenchmarkArenaMap_Set                 2M ops/sec   357 ns/op    0 allocs
BenchmarkStdMap_Set                   1M ops/sec   272 ns/op    0 allocs (74B)
BenchmarkArenaMap_Get                 9M ops/sec    42 ns/op    0 allocs
BenchmarkStdMap_Get                  15M ops/sec    19 ns/op    0 allocs
```

### Pool vs Standard Allocation
```
BenchmarkPool_Alloc                  19M ops/sec    53 ns/op    0 allocs
BenchmarkPool_AllocFree              19M ops/sec    51 ns/op    0 allocs
BenchmarkPool_AllocFreeReuse         19M ops/sec    51 ns/op    0 allocs
BenchmarkStdAlloc_AllocFree           2B ops/sec   0.5 ns/op    0 allocs
BenchmarkPool_Struct                 19M ops/sec    52 ns/op    0 allocs
BenchmarkStdAlloc_Struct              2B ops/sec   0.4 ns/op    0 allocs
```

**Key Insights:**
- Arena allocations produce **zero GC allocations**, reducing GC pressure
- Bump allocator is fastest for sequential allocations (~28ns/op)
- Pool provides consistent performance for object reuse (~52ns/op)
- Standard allocation is faster for single objects but triggers GC
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

### Building Data Structures with MakeObject

See `examples/makeobject_example.go` for complete examples of:
- **Binary Search Tree**: Arena-allocated BST with insertBST and traversal
- **Graph**: Arena-allocated graph nodes with edge connections
- **Doubly Linked List**: Arena-allocated linked list with forward/backward traversal

Run the example:
```bash
cd examples
go run makeobject_example.go
```

### Test Files

See the `*_test.go` files for comprehensive usage examples:

- `makeobject_test.go` - MakeObject usage patterns, linked lists, alignment
- `bump_test.go` - Basic allocation patterns
- `slab_test.go` - Slab allocator usage
- `buddy_test.go` - Buddy allocator and collections
- `bench_test.go` - Performance benchmarks and concurrent usage patterns
- `clone_test.go` - Clone methods for escaping arena memory

## Benchmarks

Run benchmarks with:
```bash
go test -bench=. -benchmem
```

## Use Cases

**Perfect for:**
- **Parsers & Compilers**: AST nodes, tokens, symbol tables (use MakeObject for nodes)
- **Request Handlers**: Per-request arena, cleanup after response (use Clone to persist data)
- **Game Engines**: Per-frame allocations, entity components (use Pool for frequent alloc/free)
- **Database Query Processing**: Query plans, intermediate results (use ArenaMap for lookups)
- **Network Packet Processing**: Packet buffers, connection state (use Pool for packet objects)
- **JSON/XML Parsing**: DOM nodes, attribute maps (use MakeObject for nodes, ArenaMap for attributes)
- **Graph Algorithms**: Nodes, edges, visited sets (use MakeObject for nodes, ArenaMap for tracking)
- **String Processing**: Concatenation, tokenization (use ArenaString for building)

**When to use each allocation method:**
- `MakeObject[T]` - **Struct instances** (nodes, objects, records)
- `MakeSlice[T]` - **Dynamic arrays** (lists, buffers, collections)
- `MakeString` - **String data** (text, identifiers, paths)
- `Alloc[T]` - **Generic allocation** (primitive types, when alignment doesn't matter)
- `NewMap` - **Hash tables** (lookups, caches, symbol tables)
- `NewSkipList` - **Ordered data** (sorted lists, priority queues)
- `NewPool` - **Object reuse** (frequent alloc/free of same type)

## Implementation Details

- **Memory Backend**: Uses `mmap` for page-aligned allocations
- **Hash Function**: FNV-1a for ArenaMap
- **Load Factor**: 50% for ArenaMap to reduce collisions
- **SSO Thresholds**: 23 bytes for strings, 16 elements for slices
- **Alignment**: Type-specific alignment for MakeObject, 16-byte for Alloc

## Contributing

1. Ensure all tests pass: `go test -race .`
2. Format code: `go fmt .`
3. Add tests for new features
4. Update documentation

## License

See LICENSE file for details.