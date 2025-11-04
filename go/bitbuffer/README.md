# BitBuffer

A high-performance Go library for bit manipulation and hex encoding/decoding operations.

## Features

- **BitBuffer**: Efficient bit-level data manipulation
- **HexString**: Ultra-fast hex encoding and decoding with SIMD-style optimizations

## HexString Performance

The `DecodeHexString` and `EncodeHexString` functions provide significant performance improvements over Go's standard library implementations.

### Benchmark Results

```
goos: linux
goarch: amd64
pkg: playground/go/bitbuffer

BenchmarkDecodeHexString-4       9189892    125.6 ns/op    48 B/op    1 allocs/op
BenchmarkHexDecodeString-4       9293634    135.6 ns/op    48 B/op    1 allocs/op
BenchmarkEncodeHexString-4       7657364    142.6 ns/op    80 B/op    1 allocs/op
BenchmarkHexEncodeToString-4     5133838    264.6 ns/op   160 B/op    2 allocs/op
```

### Performance Comparison

| Operation | Our Implementation | Go Standard Library | Improvement |
|-----------|-------------------|-------------------|-------------|
| **Hex Decoding** | 125.6 ns/op | 135.6 ns/op | **7% faster** |
| **Hex Encoding** | 142.6 ns/op | 264.6 ns/op | **46% faster** |
| **Memory Usage** | 48-80 B/op | 48-160 B/op | **Up to 50% less** |
| **Allocations** | 1 | 1-2 | **Up to 50% fewer** |

## Optimizations

### DecodeHexString
- **Batch Processing**: Handles 8, 4, and 2 hex characters at a time
- **Lookup Table**: O(1) hex digit to byte conversion
- **Unsafe Pointers**: Zero-copy string access
- **SIMD-style Operations**: Bit manipulation for efficient nibble packing
- **Little-Endian Aware**: Proper byte ordering for cross-platform compatibility

### EncodeHexString
- **Bulk Memory Writes**: Single `uint64`/`uint32`/`uint16` operations instead of individual bytes
- **Lookup Table**: Fast hex character generation
- **Unsafe Operations**: Direct memory access for performance
- **Zero-Allocation Return**: Efficient string construction

## Usage

```go
import "playground/go/bitbuffer"

// Decode hex string to bytes
data, err := bitbuffer.DecodeHexString("48656c6c6f20576f726c64")
if err != nil {
    // handle error
}
fmt.Println(string(data)) // "Hello World"

// Encode bytes to hex string
hex := bitbuffer.EncodeHexString([]byte("Hello World"))
fmt.Println(hex) // "48656c6c6f20576f726c64"
```

## Testing

Run the test suite:
```bash
go test
```

Run benchmarks:
```bash
go test -bench=.
```

Compare with standard library:
```bash
go test -bench=BenchmarkDecodeHexString -bench=BenchmarkHexDecodeString
go test -bench=BenchmarkEncodeHexString -bench=BenchmarkHexEncodeToString
```

## Architecture

The library uses advanced Go techniques for maximum performance:

- **Unsafe Pointers**: Direct memory access without bounds checking
- **Batch Processing**: Process multiple bytes/characters simultaneously
- **Lookup Tables**: Pre-computed conversion tables
- **Bit Manipulation**: Efficient nibble packing/unpacking
- **Memory Layout Awareness**: Little-endian optimized operations

## Compatibility

- Go 1.18+
- Linux, macOS, Windows
- Little-endian architectures (x86, AMD64, ARM64)

## License

MIT License