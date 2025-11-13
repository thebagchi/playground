package bitbuffer

import (
	"strconv"
	"testing"
)

func TestDecodeNumString(t *testing.T) {
	tests := []struct {
		input    string
		expected int64
		wantErr  bool
	}{
		{"0", 0, false},
		{"123", 123, false},
		{"456789", 456789, false},
		{"-123", -123, false},
		{"+456", 456, false},
		{"9223372036854775807", 9223372036854775807, false},   // max int64
		{"-9223372036854775808", -9223372036854775808, false}, // min int64
		{"", 0, false},
		{"abc", 0, true},
		{"12a34", 0, true},
		{"-", 0, true},
		{"+", 0, true},
	}

	for _, tt := range tests {
		t.Run(tt.input, func(t *testing.T) {
			result, err := DecodeNumString(tt.input)
			if tt.wantErr {
				if err == nil {
					t.Errorf("expected error for input %q, got none", tt.input)
				}
			} else {
				if err != nil {
					t.Errorf("unexpected error for input %q: %v", tt.input, err)
				}
				if result != tt.expected {
					t.Errorf("DecodeNumString(%q) = %d, want %d", tt.input, result, tt.expected)
				}
			}
		})
	}
}

func TestEncodeNumString(t *testing.T) {
	tests := []struct {
		input    int64
		expected string
	}{
		{0, "0"},
		{123, "123"},
		{456789, "456789"},
		{-123, "-123"},
		{9223372036854775807, "9223372036854775807"},
		{-9223372036854775808, "-9223372036854775808"},
	}

	for _, tt := range tests {
		t.Run(tt.expected, func(t *testing.T) {
			result := EncodeNumString(tt.input)
			if result != tt.expected {
				t.Errorf("EncodeNumString(%d) = %q, want %q", tt.input, result, tt.expected)
			}
		})
	}
}

func TestNumStringRoundTrip(t *testing.T) {
	tests := []int64{
		0, 1, -1, 123, -456, 999999, -999999,
		9223372036854775807, -9223372036854775808,
	}

	for _, tt := range tests {
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
	testCases := []string{
		"0",
		"123",
		"123456789",
		"-987654321",
		"9223372036854775807",
		"-9223372036854775808",
	}

	for _, tc := range testCases {
		b.Run(tc, func(b *testing.B) {
			b.ReportAllocs()
			for i := 0; i < b.N; i++ {
				_, _ = DecodeNumString(tc)
			}
		})
	}
}

func BenchmarkDecodeNumStringStd(b *testing.B) {
	testCases := []string{
		"0",
		"123",
		"123456789",
		"-987654321",
		"9223372036854775807",
		"-9223372036854775808",
	}

	for _, tc := range testCases {
		b.Run(tc, func(b *testing.B) {
			b.ReportAllocs()
			for i := 0; i < b.N; i++ {
				_, _ = strconv.Atoi(tc)
			}
		})
	}
}

func BenchmarkEncodeNumString(b *testing.B) {
	testCases := []int64{
		0,
		123,
		123456789,
		-987654321,
		9223372036854775807,
		-9223372036854775808,
	}

	for _, tc := range testCases {
		b.Run(strconv.FormatInt(tc, 10), func(b *testing.B) {
			b.ReportAllocs()
			for i := 0; i < b.N; i++ {
				_ = EncodeNumString(tc)
			}
		})
	}
}

func BenchmarkEncodeNumStringStd(b *testing.B) {
	testCases := []int64{
		0,
		123,
		123456789,
		-987654321,
		9223372036854775807,
		-9223372036854775808,
	}

	for _, tc := range testCases {
		b.Run(strconv.FormatInt(tc, 10), func(b *testing.B) {
			b.ReportAllocs()
			for i := 0; i < b.N; i++ {
				_ = strconv.FormatInt(tc, 10)
			}
		})
	}
}
