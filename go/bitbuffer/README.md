# BitBuffer

A high-performance Go library for bit manipulation and hex encoding/decoding operations.

## Features

- **BitBuffer**: Efficient bit-level data manipulation
- **HexString**: Ultra-fast hex encoding and decoding with SIMD-style optimizations
- **NumString**: High-performance numeric string parsing and formatting

## NumString Performance

The `DecodeNumString` and `EncodeNumString` functions provide optimized numeric string parsing and formatting, significantly outperforming the standard library for longer numeric strings.

### Benchmark Results

```
goos: linux
goarch: amd64
pkg: playground/go/bitbuffer
cpu: Intel(R) Core(TM) i5-5250U CPU @ 1.60GHz

BenchmarkDecodeNumString/0-4                            141912231    8.107 ns/op    0 B/op    0 allocs/op
BenchmarkDecodeNumString/123-4                          100000000   10.06 ns/op     0 B/op    0 allocs/op
BenchmarkDecodeNumString/123456789-4                     78523057   16.09 ns/op     0 B/op    0 allocs/op
BenchmarkDecodeNumString/-987654321-4                    73370511   18.72 ns/op     0 B/op    0 allocs/op
BenchmarkDecodeNumString/9223372036854775807-4           39023121   26.11 ns/op     0 B/op    0 allocs/op
BenchmarkDecodeNumString/-9223372036854775808-4          48094503   27.66 ns/op     0 B/op    0 allocs/op

BenchmarkDecodeNumStringStd/0-4                         149869964    8.842 ns/op    0 B/op    0 allocs/op
BenchmarkDecodeNumStringStd/123-4                       100000000   10.71 ns/op     0 B/op    0 allocs/op
BenchmarkDecodeNumStringStd/123456789-4                  57499473   21.25 ns/op     0 B/op    0 allocs/op
BenchmarkDecodeNumStringStd/-987654321-4                 54176362   21.76 ns/op     0 B/op    0 allocs/op
BenchmarkDecodeNumStringStd/9223372036854775807-4        14141086   92.74 ns/op     0 B/op    0 allocs/op
BenchmarkDecodeNumStringStd/-9223372036854775808-4       13177461   89.48 ns/op     0 B/op    0 allocs/op

BenchmarkEncodeNumString/0-4                            388383079    2.918 ns/op    0 B/op    0 allocs/op
BenchmarkEncodeNumString/123-4                           24194830   47.29 ns/op    24 B/op    1 allocs/op
BenchmarkEncodeNumString/123456789-4                     19213791   52.05 ns/op    24 B/op    1 allocs/op
BenchmarkEncodeNumString/-987654321-4                    19499756   52.89 ns/op    24 B/op    1 allocs/op
BenchmarkEncodeNumString/9223372036854775807-4           18161535   64.09 ns/op    24 B/op    1 allocs/op
BenchmarkEncodeNumString/-9223372036854775808-4          18312895   64.74 ns/op    24 B/op    1 allocs/op

BenchmarkEncodeNumStringStd/0-4                         296642630    4.012 ns/op    0 B/op    0 allocs/op
BenchmarkEncodeNumStringStd/123-4                        27732754   38.59 ns/op     3 B/op    1 allocs/op
BenchmarkEncodeNumStringStd/123456789-4                  17564618   65.20 ns/op    16 B/op    1 allocs/op
BenchmarkEncodeNumStringStd/-987654321-4                 17503028   66.47 ns/op    16 B/op    1 allocs/op
BenchmarkEncodeNumStringStd/9223372036854775807-4        12366620   99.89 ns/op    24 B/op    1 allocs/op
BenchmarkEncodeNumStringStd/-9223372036854775808-4       12665913  112.0 ns/op     24 B/op    1 allocs/op
```

### Performance Comparison

#### Decoding (String to Int64)

| Input                                   | DecodeNumString | strconv.Atoi | Speedup              |
|-----------------------------------------|-----------------|--------------|----------------------|
| `"0"`                                   | 8.107 ns/op     | 8.842 ns/op  | ~1.09x faster        |
| `"123"`                                 | 10.06 ns/op     | 10.71 ns/op  | ~1.06x faster        |
| `"123456789"`                           | 16.09 ns/op     | 21.25 ns/op  | **~1.32x faster**    |
| `"-987654321"`                          | 18.72 ns/op     | 21.76 ns/op  | **~1.16x faster**    |
| `"9223372036854775807"` (max int64)     | 26.11 ns/op     | 92.74 ns/op  | **~3.55x faster**    |
| `"-9223372036854775808"` (min int64)    | 27.66 ns/op     | 89.48 ns/op  | **~3.23x faster**    |

#### Encoding (Int64 to String)

| Input                         | EncodeNumString | strconv.FormatInt | Speedup            |
|-------------------------------|-----------------|-------------------|--------------------|
| `0`                           | 2.918 ns/op     | 4.012 ns/op       | ~1.38x faster      |
| `123`                         | 47.29 ns/op     | 38.59 ns/op       | ~0.82x (slower)    |
| `123456789`                   | 52.05 ns/op     | 65.20 ns/op       | **~1.25x faster**  |
| `-987654321`                  | 52.89 ns/op     | 66.47 ns/op       | **~1.26x faster**  |
| `9223372036854775807` (max)   | 64.09 ns/op     | 99.89 ns/op       | **~1.56x faster**  |
| `-9223372036854775808` (min)  | 64.74 ns/op     | 112.0 ns/op       | **~1.73x faster**  |

**Key Findings:**
- **Decoding**: Zero allocations for both implementations; 3.5x faster for max int64
- **Encoding**: Competitive performance, 1.56-1.73x faster for large numbers
- Batch processing (uint64/uint32/uint16) provides significant speedup for longer strings
- Digit pairs lookup table and uint32 writes optimize encoding performance

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

### DecodeNumString
- **Batch Processing**: Processes 8, 4, 2, and 1 digits at a time using uint64/uint32/uint16/uint8
- **Lookup Table**: 256-byte pre-computed digit validation table
- **Unsafe Pointers**: Zero-copy string access for maximum performance
- **Sign Handling**: Efficient handling of positive/negative numbers
- **Zero Allocations**: Direct parsing without intermediate allocations
- **Progressive Optimization**: Performance scales with string length

### EncodeNumString
- **Fixed Buffer**: Pre-allocated 22-byte array for maximum int64 representation
- **Digit Pairs Lookup**: 200-byte table for "00"-"99" pairs
- **Uint32 Writes**: Process 4 digits at once for numbers >= 10000
- **Uint16 Writes**: Process 2 digits at once using digit pairs
- **Reverse Fill**: Fill buffer from right to left, return only used portion
- **Uint64 Conversion**: Handles int64 overflow correctly for minimum value
- **Unsafe Pointers**: Direct memory access for optimal performance
- **Single Allocation**: Only for the final string result

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

### NumString

```go
import "playground/go/bitbuffer"

// Decode numeric string to int64
num, err := bitbuffer.DecodeNumString("123456789")
if err != nil {
    // handle error
}
fmt.Println(num) // 123456789

// Supports negative numbers
num, err = bitbuffer.DecodeNumString("-987654321")
fmt.Println(num) // -987654321

// Encode int64 to string
str := bitbuffer.EncodeNumString(123456789)
fmt.Println(str) // "123456789"
```

### HexString

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