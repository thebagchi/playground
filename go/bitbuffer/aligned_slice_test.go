package bitbuffer

import (
	"fmt"
	"testing"
	"unsafe"
)

// =============================================================================
// FUNCTIONAL TESTS
// =============================================================================

func TestFloat32Align32Original(t *testing.T) {
	sizes := []int{1, 8, 16, 32, 100, 1000}

	for _, size := range sizes {
		slice := Float32Align32(size)
		if len(slice) != size {
			t.Errorf("Expected length %d, got %d", size, len(slice))
		}

		// Check alignment
		ptr := uintptr(unsafe.Pointer(&slice[0]))
		if ptr%32 != 0 {
			t.Errorf("Slice not 32-byte aligned for size %d: ptr=%v", size, ptr)
		}
	}
}

func TestFloat32Align32EdgeCases(t *testing.T) {
	// Test zero size
	slice := Float32Align32(0)
	if slice != nil {
		t.Error("Expected nil slice for size 0")
	}

	// Test negative size
	slice = Float32Align32(-1)
	if slice != nil {
		t.Error("Expected nil slice for negative size")
	}
}

func TestAlignment(t *testing.T) {
	slice := Float32Align32(100)
	ptr := uintptr(unsafe.Pointer(&slice[0]))

	t.Logf("Slice address: 0x%x", ptr)
	t.Logf("Alignment check (32-byte): %v", ptr%32 == 0)

	// Verify we can write to the slice
	for i := range slice {
		slice[i] = float32(i)
	}

	// Verify values
	for i := range slice {
		if slice[i] != float32(i) {
			t.Errorf("Value mismatch at index %d: got %f, want %f", i, slice[i], float32(i))
		}
	}
}

func TestEdgeCases(t *testing.T) {
	t.Run("ZERO-SIZE", func(t *testing.T) {
		slice := AlignedSlice[float32](0, 32)
		if slice != nil {
			t.Error("Expected nil slice for size 0")
		}
	})

	t.Run("NEGATIVE-SIZE", func(t *testing.T) {
		slice := AlignedSlice[float32](-1, 32)
		if slice != nil {
			t.Error("Expected nil slice for negative size")
		}
	})

	t.Run("INVALID-ALIGNMENT-ZERO", func(t *testing.T) {
		slice := AlignedSlice[float32](10, 0)
		if slice != nil {
			t.Error("Expected nil slice for zero alignment")
		}
	})

	t.Run("INVALID-ALIGNMENT-NOT-POWER-OF-2", func(t *testing.T) {
		slice := AlignedSlice[float32](10, 12) // 12 is not a power of 2
		if slice != nil {
			t.Error("Expected nil slice for non-power-of-2 alignment")
		}
	})

	t.Run("ALIGNMENT-SMALLER-THAN-TYPE-SIZE", func(t *testing.T) {
		// For uint64 (8 bytes), alignment of 4 should be adjusted to 8
		slice := AlignedSlice[uint64](10, 4)
		if len(slice) != 10 {
			t.Errorf("Expected length 10, got %d", len(slice))
		}
		ptr := uintptr(unsafe.Pointer(unsafe.SliceData(slice)))
		if ptr%8 != 0 { // Should be aligned to at least 8 bytes (size of uint64)
			t.Errorf("Slice not properly aligned: ptr=0x%x", ptr)
		}
	})
}

func TestDataIntegrity(t *testing.T) {
	t.Run("FLOAT32-WRITE-READ", func(t *testing.T) {
		slice := AlignedSlice[float32](100, 32)
		for i := range slice {
			slice[i] = float32(i) * 1.5
		}
		for i, val := range slice {
			expected := float32(i) * 1.5
			if val != expected {
				t.Errorf("Value mismatch at index %d: got %f, want %f", i, val, expected)
			}
		}
	})

	t.Run("UINT64-WRITE-READ", func(t *testing.T) {
		slice := AlignedSlice[uint64](50, 64)
		for i := range slice {
			slice[i] = uint64(i) * 1000
		}
		for i, val := range slice {
			expected := uint64(i) * 1000
			if val != expected {
				t.Errorf("Value mismatch at index %d: got %d, want %d", i, val, expected)
			}
		}
	})

	t.Run("INT32-WRITE-READ", func(t *testing.T) {
		slice := AlignedSlice[int32](40, 32)
		for i := range slice {
			slice[i] = int32(i-20) * 100 // Include negative values
		}
		for i, val := range slice {
			expected := int32(i-20) * 100
			if val != expected {
				t.Errorf("Value mismatch at index %d: got %d, want %d", i, val, expected)
			}
		}
	})

	t.Run("INT64-WRITE-READ", func(t *testing.T) {
		slice := AlignedSlice[int64](30, 16)
		for i := range slice {
			slice[i] = int64(i-15) * 2000 // Include negative values
		}
		for i, val := range slice {
			expected := int64(i-15) * 2000
			if val != expected {
				t.Errorf("Value mismatch at index %d: got %d, want %d", i, val, expected)
			}
		}
	})
}

func TestAlignmentVerification(t *testing.T) {
	t.Run("VERIFY-16BYTE-ALIGNMENT", func(t *testing.T) {
		slice := AlignedSlice16[float32](100)
		ptr := uintptr(unsafe.Pointer(unsafe.SliceData(slice)))

		t.Logf("16-byte aligned slice address: 0x%x", ptr)

		if ptr%16 != 0 {
			t.Errorf("Expected 16-byte alignment, but got address 0x%x", ptr)
		}

		// Verify it's also aligned for smaller alignments
		if ptr%8 != 0 {
			t.Errorf("16-byte aligned slice should also be 8-byte aligned")
		}
		if ptr%4 != 0 {
			t.Errorf("16-byte aligned slice should also be 4-byte aligned")
		}
	})

	t.Run("VERIFY-32BYTE-ALIGNMENT", func(t *testing.T) {
		slice := AlignedSlice32[float64](50)
		ptr := uintptr(unsafe.Pointer(unsafe.SliceData(slice)))

		t.Logf("32-byte aligned slice address: 0x%x", ptr)

		if ptr%32 != 0 {
			t.Errorf("Expected 32-byte alignment, but got address 0x%x", ptr)
		}

		// Verify it's also aligned for smaller alignments
		if ptr%16 != 0 {
			t.Errorf("32-byte aligned slice should also be 16-byte aligned")
		}
	})

	t.Run("VERIFY-64BYTE-ALIGNMENT", func(t *testing.T) {
		slice := AlignedSlice64[uint64](25)
		ptr := uintptr(unsafe.Pointer(unsafe.SliceData(slice)))

		t.Logf("64-byte aligned slice address: 0x%x", ptr)

		if ptr%64 != 0 {
			t.Errorf("Expected 64-byte alignment, but got address 0x%x", ptr)
		}

		// Verify it's also aligned for smaller alignments
		if ptr%32 != 0 {
			t.Errorf("64-byte aligned slice should also be 32-byte aligned")
		}
		if ptr%16 != 0 {
			t.Errorf("64-byte aligned slice should also be 16-byte aligned")
		}
	})
}

// =============================================================================
// GENERIC TESTS
// =============================================================================

func TestAlignedSliceGeneric(t *testing.T) {
	testCases := []struct {
		name      string
		size      int
		alignment uintptr
	}{
		{"SMALL-16", 8, 16},
		{"MEDIUM-16", 64, 16},
		{"LARGE-16", 256, 16},
		{"SMALL-32", 8, 32},
		{"MEDIUM-32", 64, 32},
		{"LARGE-32", 256, 32},
		{"SMALL-64", 8, 64},
		{"MEDIUM-64", 64, 64},
		{"LARGE-64", 256, 64},
	}

	t.Run("float32", func(t *testing.T) {
		for _, tc := range testCases {
			t.Run(tc.name, func(t *testing.T) {
				slice := AlignedSlice[float32](tc.size, tc.alignment)
				if len(slice) != tc.size {
					t.Errorf("Expected length %d, got %d", tc.size, len(slice))
				}
				ptr := uintptr(unsafe.Pointer(unsafe.SliceData(slice)))
				if ptr%tc.alignment != 0 {
					t.Errorf("Slice not %d-byte aligned: ptr=0x%x", tc.alignment, ptr)
				}
			})
		}
	})

	t.Run("float64", func(t *testing.T) {
		for _, tc := range testCases {
			t.Run(tc.name, func(t *testing.T) {
				slice := AlignedSlice[float64](tc.size, tc.alignment)
				if len(slice) != tc.size {
					t.Errorf("Expected length %d, got %d", tc.size, len(slice))
				}
				ptr := uintptr(unsafe.Pointer(unsafe.SliceData(slice)))
				if ptr%tc.alignment != 0 {
					t.Errorf("Slice not %d-byte aligned: ptr=0x%x", tc.alignment, ptr)
				}
			})
		}
	})

	t.Run("uint64", func(t *testing.T) {
		for _, tc := range testCases {
			t.Run(tc.name, func(t *testing.T) {
				slice := AlignedSlice[uint64](tc.size, tc.alignment)
				if len(slice) != tc.size {
					t.Errorf("Expected length %d, got %d", tc.size, len(slice))
				}
				ptr := uintptr(unsafe.Pointer(unsafe.SliceData(slice)))
				if ptr%tc.alignment != 0 {
					t.Errorf("Slice not %d-byte aligned: ptr=0x%x", tc.alignment, ptr)
				}
			})
		}
	})

	t.Run("uint32", func(t *testing.T) {
		for _, tc := range testCases {
			t.Run(tc.name, func(t *testing.T) {
				slice := AlignedSlice[uint32](tc.size, tc.alignment)
				if len(slice) != tc.size {
					t.Errorf("Expected length %d, got %d", tc.size, len(slice))
				}
				ptr := uintptr(unsafe.Pointer(unsafe.SliceData(slice)))
				if ptr%tc.alignment != 0 {
					t.Errorf("Slice not %d-byte aligned: ptr=0x%x", tc.alignment, ptr)
				}
			})
		}
	})

	t.Run("uint16", func(t *testing.T) {
		for _, tc := range testCases {
			t.Run(tc.name, func(t *testing.T) {
				slice := AlignedSlice[uint16](tc.size, tc.alignment)
				if len(slice) != tc.size {
					t.Errorf("Expected length %d, got %d", tc.size, len(slice))
				}
				ptr := uintptr(unsafe.Pointer(unsafe.SliceData(slice)))
				if ptr%tc.alignment != 0 {
					t.Errorf("Slice not %d-byte aligned: ptr=0x%x", tc.alignment, ptr)
				}
			})
		}
	})

	t.Run("uint8", func(t *testing.T) {
		for _, tc := range testCases {
			t.Run(tc.name, func(t *testing.T) {
				slice := AlignedSlice[uint8](tc.size, tc.alignment)
				if len(slice) != tc.size {
					t.Errorf("Expected length %d, got %d", tc.size, len(slice))
				}
				ptr := uintptr(unsafe.Pointer(unsafe.SliceData(slice)))
				if ptr%tc.alignment != 0 {
					t.Errorf("Slice not %d-byte aligned: ptr=0x%x", tc.alignment, ptr)
				}
			})
		}
	})

	t.Run("int32", func(t *testing.T) {
		for _, tc := range testCases {
			t.Run(tc.name, func(t *testing.T) {
				slice := AlignedSlice[int32](tc.size, tc.alignment)
				if len(slice) != tc.size {
					t.Errorf("Expected length %d, got %d", tc.size, len(slice))
				}
				ptr := uintptr(unsafe.Pointer(unsafe.SliceData(slice)))
				if ptr%tc.alignment != 0 {
					t.Errorf("Slice not %d-byte aligned: ptr=0x%x", tc.alignment, ptr)
				}
			})
		}
	})

	t.Run("int64", func(t *testing.T) {
		for _, tc := range testCases {
			t.Run(tc.name, func(t *testing.T) {
				slice := AlignedSlice[int64](tc.size, tc.alignment)
				if len(slice) != tc.size {
					t.Errorf("Expected length %d, got %d", tc.size, len(slice))
				}
				ptr := uintptr(unsafe.Pointer(unsafe.SliceData(slice)))
				if ptr%tc.alignment != 0 {
					t.Errorf("Slice not %d-byte aligned: ptr=0x%x", tc.alignment, ptr)
				}
			})
		}
	})
}

func TestConvenienceFunctions(t *testing.T) {
	t.Run("AlignedSlice16", func(t *testing.T) {
		slice := AlignedSlice16[float32](128)
		if len(slice) != 128 {
			t.Errorf("Expected length 128, got %d", len(slice))
		}
		ptr := uintptr(unsafe.Pointer(unsafe.SliceData(slice)))
		if ptr%16 != 0 {
			t.Errorf("Slice not 16-byte aligned: ptr=0x%x", ptr)
		}
	})

	t.Run("AlignedSlice32", func(t *testing.T) {
		slice := AlignedSlice32[float64](100)
		if len(slice) != 100 {
			t.Errorf("Expected length 100, got %d", len(slice))
		}
		ptr := uintptr(unsafe.Pointer(unsafe.SliceData(slice)))
		if ptr%32 != 0 {
			t.Errorf("Slice not 32-byte aligned: ptr=0x%x", ptr)
		}
	})

	t.Run("AlignedSlice64", func(t *testing.T) {
		slice := AlignedSlice64[uint64](50)
		if len(slice) != 50 {
			t.Errorf("Expected length 50, got %d", len(slice))
		}
		ptr := uintptr(unsafe.Pointer(unsafe.SliceData(slice)))
		if ptr%64 != 0 {
			t.Errorf("Slice not 64-byte aligned: ptr=0x%x", ptr)
		}
	})

	t.Run("Float32Align16", func(t *testing.T) {
		slice := Float32Align16(64)
		if len(slice) != 64 {
			t.Errorf("Expected length 64, got %d", len(slice))
		}
		ptr := uintptr(unsafe.Pointer(unsafe.SliceData(slice)))
		if ptr%16 != 0 {
			t.Errorf("Slice not 16-byte aligned: ptr=0x%x", ptr)
		}
	})

	t.Run("Float64Align16", func(t *testing.T) {
		slice := Float64Align16(48)
		if len(slice) != 48 {
			t.Errorf("Expected length 48, got %d", len(slice))
		}
		ptr := uintptr(unsafe.Pointer(unsafe.SliceData(slice)))
		if ptr%16 != 0 {
			t.Errorf("Slice not 16-byte aligned: ptr=0x%x", ptr)
		}
	})

	t.Run("Float64Align32", func(t *testing.T) {
		slice := Float64Align32(75)
		if len(slice) != 75 {
			t.Errorf("Expected length 75, got %d", len(slice))
		}
		ptr := uintptr(unsafe.Pointer(unsafe.SliceData(slice)))
		if ptr%32 != 0 {
			t.Errorf("Slice not 32-byte aligned: ptr=0x%x", ptr)
		}
	})

	t.Run("Uint32Align16", func(t *testing.T) {
		slice := Uint32Align16(32)
		if len(slice) != 32 {
			t.Errorf("Expected length 32, got %d", len(slice))
		}
		ptr := uintptr(unsafe.Pointer(unsafe.SliceData(slice)))
		if ptr%16 != 0 {
			t.Errorf("Slice not 16-byte aligned: ptr=0x%x", ptr)
		}
	})

	t.Run("Uint64Align32", func(t *testing.T) {
		slice := Uint64Align32(40)
		if len(slice) != 40 {
			t.Errorf("Expected length 40, got %d", len(slice))
		}
		ptr := uintptr(unsafe.Pointer(unsafe.SliceData(slice)))
		if ptr%32 != 0 {
			t.Errorf("Slice not 32-byte aligned: ptr=0x%x", ptr)
		}
	})

	t.Run("Int32Align16", func(t *testing.T) {
		slice := Int32Align16(24)
		if len(slice) != 24 {
			t.Errorf("Expected length 24, got %d", len(slice))
		}
		ptr := uintptr(unsafe.Pointer(unsafe.SliceData(slice)))
		if ptr%16 != 0 {
			t.Errorf("Slice not 16-byte aligned: ptr=0x%x", ptr)
		}
	})

	t.Run("Int64Align32", func(t *testing.T) {
		slice := Int64Align32(35)
		if len(slice) != 35 {
			t.Errorf("Expected length 35, got %d", len(slice))
		}
		ptr := uintptr(unsafe.Pointer(unsafe.SliceData(slice)))
		if ptr%32 != 0 {
			t.Errorf("Slice not 32-byte aligned: ptr=0x%x", ptr)
		}
	})
}

// =============================================================================
// BENCHMARK TESTS
// =============================================================================

func BenchmarkFloat32Align32Original(b *testing.B) {
	sizes := []int{8, 64, 256, 1024}

	for _, size := range sizes {
		b.Run(fmt.Sprintf("size_%d", size), func(b *testing.B) {
			b.ReportAllocs()
			for i := 0; i < b.N; i++ {
				slice := Float32Align32(size)
				_ = slice
			}
		})
	}
}

// Optimized version for comparison
func float32Align32Optimized(n int) []float32 {
	if n <= 0 {
		return nil
	}

	const alignment = 32
	const padding = 7 // (32/4 - 1)

	totalSize := n + padding
	s := make([]float32, totalSize)

	// Use bit manipulation instead of division
	basePtr := uintptr(unsafe.Pointer(&s[0]))
	alignedPtr := (basePtr + alignment - 1) &^ (alignment - 1)
	offset := int((alignedPtr - basePtr) >> 2) // >> 2 is /4

	return s[offset : offset+n : offset+n]
}

func BenchmarkFloat32Align32Optimized(b *testing.B) {
	sizes := []int{8, 64, 256, 1024}

	for _, size := range sizes {
		b.Run(fmt.Sprintf("size_%d", size), func(b *testing.B) {
			b.ReportAllocs()
			for i := 0; i < b.N; i++ {
				slice := float32Align32Optimized(size)
				_ = slice
			}
		})
	}
}

func BenchmarkAlignedSliceGeneric(b *testing.B) {
	sizes := []int{8, 64, 256, 1024}

	b.Run("float32", func(b *testing.B) {
		for _, size := range sizes {
			b.Run(fmt.Sprintf("size_%d", size), func(b *testing.B) {
				b.ReportAllocs()
				for i := 0; i < b.N; i++ {
					slice := AlignedSlice[float32](size, 32)
					_ = slice
				}
			})
		}
	})

	b.Run("float64", func(b *testing.B) {
		for _, size := range sizes {
			b.Run(fmt.Sprintf("size_%d", size), func(b *testing.B) {
				b.ReportAllocs()
				for i := 0; i < b.N; i++ {
					slice := AlignedSlice[float64](size, 32)
					_ = slice
				}
			})
		}
	})

	b.Run("uint64", func(b *testing.B) {
		for _, size := range sizes {
			b.Run(fmt.Sprintf("size_%d", size), func(b *testing.B) {
				b.ReportAllocs()
				for i := 0; i < b.N; i++ {
					slice := AlignedSlice[uint64](size, 32)
					_ = slice
				}
			})
		}
	})
}

// Benchmark different alignments to show performance characteristics
func BenchmarkAlignmentComparison(b *testing.B) {
	const size = 1024

	b.Run("16BYTE-ALIGNED", func(b *testing.B) {
		b.ReportAllocs()
		for i := 0; i < b.N; i++ {
			slice := AlignedSlice16[float32](size)
			// Simulate SIMD-friendly operation
			for j := 0; j < len(slice); j += 4 { // Process 4 elements at a time (SSE)
				if j+3 < len(slice) {
					slice[j] = 1.0
					slice[j+1] = 2.0
					slice[j+2] = 3.0
					slice[j+3] = 4.0
				}
			}
			_ = slice
		}
	})

	b.Run("32BYTE-ALIGNED", func(b *testing.B) {
		b.ReportAllocs()
		for i := 0; i < b.N; i++ {
			slice := AlignedSlice32[float32](size)
			// Simulate SIMD-friendly operation
			for j := 0; j < len(slice); j += 8 { // Process 8 elements at a time (AVX)
				if j+7 < len(slice) {
					for k := 0; k < 8; k++ {
						slice[j+k] = float32(k + 1)
					}
				}
			}
			_ = slice
		}
	})

	b.Run("64BYTE-ALIGNED", func(b *testing.B) {
		b.ReportAllocs()
		for i := 0; i < b.N; i++ {
			slice := AlignedSlice64[float32](size)
			// Simulate SIMD-friendly operation
			for j := 0; j < len(slice); j += 16 { // Process 16 elements at a time (AVX-512)
				if j+15 < len(slice) {
					for k := 0; k < 16; k++ {
						slice[j+k] = float32(k + 1)
					}
				}
			}
			_ = slice
		}
	})

	b.Run("UNALIGNED", func(b *testing.B) {
		b.ReportAllocs()
		for i := 0; i < b.N; i++ {
			slice := make([]float32, size) // Standard Go slice (no alignment guarantee)
			// Same operation on unaligned data
			for j := 0; j < len(slice); j += 4 {
				if j+3 < len(slice) {
					slice[j] = 1.0
					slice[j+1] = 2.0
					slice[j+2] = 3.0
					slice[j+3] = 4.0
				}
			}
			_ = slice
		}
	})
}
