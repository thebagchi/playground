# BitBuffer

A high-performance Go library for bit manipulation and hex encoding/decoding operations.

## Features

- **BitBuffer**: Efficient bit-level data manipulation
- **HexString**: Ultra-fast hex encoding and decoding with SIMD-style optimizations

## HexString Performance

The `DecodeHexString` and `EncodeHexString` functions provide high-performance hex encoding and decoding with advanced optimizations including SIMD-style bit manipulation, bulk memory operations, and zero-allocation designs.

### Benchmark Results

```
goos: linux
goarch: amd64
pkg: playground/go/bitbuffer
cpu: Intel(R) Core(TM) i5-5250U CPU @ 1.60GHz

BenchmarkDecodeHexString-4              35301658    330.9 ns/op    128 B/op    1 allocs/op
BenchmarkDecodeHexStringStd-4           33854869    358.1 ns/op    128 B/op    1 allocs/op
BenchmarkEncodeHexString-4              81653338    149.5 ns/op     80 B/op    1 allocs/op
BenchmarkEncodeHexStringStd-4           54347139    214.0 ns/op    160 B/op    2 allocs/op
```

### Performance Characteristics

| Operation        | Implementation | Performance | Memory Usage | Allocations |
|------------------|----------------|-------------|--------------|-------------|
| **Hex Decoding** | Custom         | 330.9 ns/op | 128 B/op     | 1           |
| **Hex Decoding** | Stdlib         | 358.1 ns/op | 128 B/op     | 1           |
| **Hex Encoding** | Custom         | 149.5 ns/op | 80 B/op      | 1           |
| **Hex Encoding** | Stdlib         | 214.0 ns/op | 160 B/op     | 2           |

### Performance Comparison

- **Decoding**: Custom implementation is ~7.6% faster (330.9 vs 358.1 ns/op)
- **Encoding**: Custom implementation is ~30.1% faster (149.5 vs 214.0 ns/op) with 50% less memory

## Optimizations

### DecodeHexString
- **SIMD-style Nibble Processing**: Efficiently extracts and validates hex characters
- **Batch Processing**: Handles 8, 4, and 2 hex characters at a time
- **Memory-Efficient**: Uses only 256-byte lookup table instead of large precomputed tables
- **Unsafe Pointers**: Zero-copy string access
- **Little-Endian Optimized**: Memory layout aware for optimal performance
- **Single-Pass Validation**: Creates bytes first, then validates nibbles in the result
- **Case Insensitive**: Supports both uppercase and lowercase hex digits

### EncodeHexString
- **Bulk Memory Writes**: Type-appropriate `uint64`/`uint32`/`uint16`/`uint8` operations for optimal alignment
- **Nibble Reuse**: Efficient variable reuse for final value construction
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

Run benchmarks with memory stats:
```bash
go test -bench=. -benchmem
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