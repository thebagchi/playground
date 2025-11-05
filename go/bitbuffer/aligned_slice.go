package bitbuffer

/*
Alignment Guide for SIMD Optimization

Different alignment requirements for optimal performance:

8-byte alignment:
  - Natural alignment for uint64, float64
  - Basic requirement for 64-bit operations
  - Not typically needed explicitly (Go handles this)

16-byte alignment (128-bit):
  - SSE (Streaming SIMD Extensions) instructions
  - NEON instructions on ARM (mobile/Apple Silicon)
  - Process 4 float32 or 2 float64 values simultaneously
  - Essential for: math operations, signal processing, graphics
  - Performance gain: 2-4x for suitable algorithms

32-byte alignment (256-bit):
  - AVX/AVX2 (Advanced Vector Extensions) instructions
  - Process 8 float32 or 4 float64 values simultaneously
  - Modern Intel/AMD CPUs (2011+)
  - Performance gain: 4-8x for suitable algorithms
  - Best for: large array operations, matrix math

64-byte alignment (512-bit):
  - AVX-512 instructions (high-end Intel CPUs)
  - Cache line alignment (typically 64 bytes)
  - Process 16 float32 or 8 float64 values simultaneously
  - Prevents false sharing in multi-threaded applications
  - Performance gain: 8-16x for suitable algorithms

Recommendation by use case:
  - Graphics/Games: 16-byte (SSE/NEON)
  - Scientific computing: 32-byte (AVX2)
  - High-performance computing: 64-byte (AVX-512/Cache)
  - Multi-threaded: 64-byte (cache line)
*/

import "unsafe"

// Number interface defines all numeric types
type Number interface {
	float32 | float64 | uint8 | uint16 | uint32 | uint64 | int8 | int16 | int32 | int64
}

// sizeOf returns the size of type T at compile time
// This is the idiomatic way to get the size of a generic type parameter in Go
// since unsafe.Sizeof requires a value, not a type
func sizeOf[T any]() uintptr {
	var dummy T
	return unsafe.Sizeof(dummy)
}

// Float32Align32 creates a 32-byte aligned float32 slice (convenience function)
func Float32Align32(n int) []float32 {
	return AlignedSlice32[float32](n)
}

// AlignedSlice creates an aligned slice for any numeric type with specified alignment
func AlignedSlice[T Number](n int, alignment uintptr) []T {
	if n <= 0 {
		return nil
	}

	// Validate alignment is power of 2
	if alignment == 0 || (alignment&(alignment-1)) != 0 {
		return nil // Invalid alignment
	}

	size := sizeOf[T]()

	// Ensure alignment is at least as large as type size
	if alignment < size {
		alignment = size
	}

	var (
		pad        = int(alignment/size - 1) // Calculate padding from alignment and size
		slice      = make([]T, n+pad)
		basePtr    = unsafe.Pointer(unsafe.SliceData(slice))
		alignedPtr = (uintptr(basePtr) + alignment - 1) &^ (alignment - 1) // Use bit manipulation for alignment
		offset     = int((alignedPtr - uintptr(basePtr)) / size)           // Calculate offset in elements
	)

	// Return aligned slice using unsafe.Add and unsafe.Slice for better bounds checking
	// Previously: return slice[offset : offset+n : offset+n]
	// Triple colon syntax [start:end:cap] sets explicit capacity to prevent reallocation
	return unsafe.Slice((*T)(unsafe.Add(basePtr, offset*int(size))), n)
}

// Convenience functions for common alignments

// AlignedSlice16 creates a 16-byte aligned slice for any supported type (SSE/NEON SIMD)
func AlignedSlice16[T Number](n int) []T {
	return AlignedSlice[T](n, 16)
}

// AlignedSlice32 creates a 32-byte aligned slice for any supported type (AVX/AVX2 SIMD)
func AlignedSlice32[T Number](n int) []T {
	return AlignedSlice[T](n, 32)
}

// AlignedSlice64 creates a 64-byte aligned slice for any supported type (AVX-512 SIMD / Cache line)
func AlignedSlice64[T Number](n int) []T {
	return AlignedSlice[T](n, 64)
}

// Specific type convenience functions for common use cases

// 16-byte alignment (SSE/NEON SIMD optimized)

// Example: vertices := Float32Align16(1000)
func Float32Align16(n int) []float32 {
	return AlignedSlice16[float32](n)
}

// Example: doubles := Float64Align16(500)
func Float64Align16(n int) []float64 {
	return AlignedSlice16[float64](n)
}

// Example: pixels := Uint32Align16(1920*1080)
func Uint32Align16(n int) []uint32 {
	return AlignedSlice16[uint32](n)
}

// Example: timestamps := Uint64Align16(10000)
func Uint64Align16(n int) []uint64 {
	return AlignedSlice16[uint64](n)
}

// Example: deltas := Int32Align16(2048)
func Int32Align16(n int) []int32 {
	return AlignedSlice16[int32](n)
}

// Example: samples := Int64Align16(48000)
func Int64Align16(n int) []int64 {
	return AlignedSlice16[int64](n)
}

// 32-byte alignment (AVX/AVX2 SIMD optimized)

// Example: matrix := Float64Align32(1024*1024)
func Float64Align32(n int) []float64 {
	return AlignedSlice32[float64](n)
}

// Example: indices := Uint32Align32(50000)
func Uint32Align32(n int) []uint32 {
	return AlignedSlice32[uint32](n)
}

// Example: hashes := Uint64Align32(8192)
func Uint64Align32(n int) []uint64 {
	return AlignedSlice32[uint64](n)
}

// Example: weights := Int32Align32(10000)
func Int32Align32(n int) []int32 {
	return AlignedSlice32[int32](n)
}

// Example: gradients := Int64Align32(5000)
func Int64Align32(n int) []int64 {
	return AlignedSlice32[int64](n)
}

// 64-byte alignment (AVX-512 SIMD / Cache line optimized)

// Example: dataset := Float64Align64(10000000)
func Float64Align64(n int) []float64 {
	return AlignedSlice64[float64](n)
}
