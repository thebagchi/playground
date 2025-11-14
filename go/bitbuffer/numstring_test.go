package bitbuffer

import (
	"strconv"
	"testing"
)

var nums = []int64{
	0, 1, -1,
	12, -12,
	123, -123,
	1234, -1234,
	12345, -12345,
	123456, -123456,
	1234567, -1234567,
	12345678, -12345678,
	123456789, -123456789,
	1234567890, -1234567890,
	12345678901, -12345678901,
	123456789012, -123456789012,
	1234567890123, -1234567890123,
	12345678901234, -12345678901234,
	123456789012345, -123456789012345,
	1234567890123456, -1234567890123456,
	12345678901234567, -12345678901234567,
	123456789012345678, -123456789012345678,
	1234567890123456789, -1234567890123456789,
	9223372036854775807, -9223372036854775808,
}

var strs = []string{
	"0", "1", "-1",
	"12", "-12",
	"123", "-123",
	"1234", "-1234",
	"12345", "-12345",
	"123456", "-123456",
	"1234567", "-1234567",
	"12345678", "-12345678",
	"123456789", "-123456789",
	"1234567890", "-1234567890",
	"12345678901", "-12345678901",
	"123456789012", "-123456789012",
	"1234567890123", "-1234567890123",
	"12345678901234", "-12345678901234",
	"123456789012345", "-123456789012345",
	"1234567890123456", "-1234567890123456",
	"12345678901234567", "-12345678901234567",
	"123456789012345678", "-123456789012345678",
	"1234567890123456789", "-1234567890123456789",
	"9223372036854775807", "-9223372036854775808",
}

func TestDecodeNumString(t *testing.T) {
	for _, input := range strs {
		expected, _ := strconv.Atoi(input)
		t.Run(input, func(t *testing.T) {
			result, err := DecodeNumString(input)
			if err != nil {
				t.Errorf("unexpected error for input %q: %v", input, err)
			}
			if result != int64(expected) {
				t.Errorf("DecodeNumString(%q) = %d, want %d", input, result, expected)
			}
		})
	}

	// Test error cases
	errorTests := []string{
		"abc",
		"12a34",
		"-",
		"+",
	}

	for _, input := range errorTests {
		t.Run(input, func(t *testing.T) {
			_, err := DecodeNumString(input)
			if err == nil {
				t.Errorf("expected error for input %q, got none", input)
			}
		})
	}
}

func TestEncodeNumString(t *testing.T) {
	for _, input := range nums {
		expected := strconv.FormatInt(input, 10)
		t.Run(expected, func(t *testing.T) {
			result := EncodeNumString(input)
			if result != expected {
				t.Errorf("EncodeNumString(%d) = %q, want %q", input, result, expected)
			}
		})
	}
}

func TestNumStringRoundTrip(t *testing.T) {
	for _, tt := range nums {
		encoded := EncodeNumString(tt)
		decoded, err := DecodeNumString(encoded)
		if err != nil {
			t.Errorf("round trip failed for %d: %v", tt, err)
		}
		if decoded != tt {
			t.Errorf("round trip failed: %d -> %q -> %d", tt, encoded, decoded)
		}
	}
}

// Benchmark tests
func BenchmarkDecodeNumString(b *testing.B) {
	for _, input := range strs {
		b.Run(input, func(b *testing.B) {
			b.ReportAllocs()
			for i := 0; i < b.N; i++ {
				_, _ = DecodeNumString(input)
			}
		})
	}
}

func BenchmarkDecodeNumStringStd(b *testing.B) {
	for _, input := range strs {
		b.Run(input, func(b *testing.B) {
			b.ReportAllocs()
			for i := 0; i < b.N; i++ {
				_, _ = strconv.Atoi(input)
			}
		})
	}
}

func BenchmarkEncodeNumString(b *testing.B) {
	for _, tc := range nums {
		b.Run(strconv.FormatInt(tc, 10), func(b *testing.B) {
			b.ReportAllocs()
			for i := 0; i < b.N; i++ {
				_ = EncodeNumString(tc)
			}
		})
	}
}

func BenchmarkEncodeNumStringStd(b *testing.B) {
	for _, tc := range nums {
		b.Run(strconv.FormatInt(tc, 10), func(b *testing.B) {
			b.ReportAllocs()
			for i := 0; i < b.N; i++ {
				_ = strconv.FormatInt(tc, 10)
			}
		})
	}
}
