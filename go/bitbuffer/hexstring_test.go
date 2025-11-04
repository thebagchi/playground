package bitbuffer

import (
	"encoding/hex"
	"fmt"
	"strings"
	"testing"
	"unsafe"
)

func TestDecodeHexString(t *testing.T) {
	testCases := []struct {
		name     string
		input    string
		hasError bool
	}{
		{
			name:     "valid hex string",
			input:    "48656c6c6f20576f726c64",
			hasError: false,
		},
		{
			name:     "empty string",
			input:    "",
			hasError: false,
		},
		{
			name:     "single byte",
			input:    "ff",
			hasError: false,
		},
		{
			name:     "odd length",
			input:    "f",
			hasError: true,
		},
		{
			name:     "invalid hex character",
			input:    "gg",
			hasError: true,
		},
		{
			name:     "mixed case",
			input:    "48656C6C6F",
			hasError: false,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			result, err := DecodeHexString(tc.input)
			if tc.hasError {
				if err == nil {
					t.Errorf("expected error but got none")
				}
				return
			}
			if err != nil {
				t.Errorf("unexpected error: %v", err)
				return
			}
			// Validate by encoding back to hex and comparing
			if hex.EncodeToString(result) != strings.ToLower(tc.input) {
				t.Errorf("round-trip failed: got %s, expected %s", hex.EncodeToString(result), strings.ToLower(tc.input))
			}
		})
	}
}

func BenchmarkDecodeHexString(b *testing.B) {
	testData := "4c6f72656d20697073756d20646f6c6f722073697420616d65742c20636f6e73656374657475722061646970697363696e6720656c69742e2053656420646f20656975736d6f642074656d706f7220696e6369646964756e74207574206c61626f726520657420646f6c6f7265206d61676e6120616c697175612e"
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		_, _ = DecodeHexString(testData)
	}
}

func BenchmarkDecodeHexStringStd(b *testing.B) {
	testData := "4c6f72656d20697073756d20646f6c6f722073697420616d65742c20636f6e73656374657475722061646970697363696e6720656c69742e2053656420646f20656975736d6f642074656d706f7220696e6369646964756e74207574206c61626f726520657420646f6c6f7265206d61676e6120616c697175612e"
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		_, _ = hex.DecodeString(testData)
	}
}

func TestEncodeHexString(t *testing.T) {
	testCases := []struct {
		name     string
		input    []byte
		expected string
	}{
		{
			name:     "Lorem ipsum",
			input:    []byte("Lorem ipsum"),
			expected: "4c6f72656d20697073756d",
		},
		{
			name:     "empty slice",
			input:    []byte{},
			expected: "",
		},
		{
			name:     "single byte",
			input:    []byte{255},
			expected: "ff",
		},
		{
			name:     "mixed bytes",
			input:    []byte{0, 1, 15, 16, 255},
			expected: "00010f10ff",
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			result := EncodeHexString(tc.input)
			if result != tc.expected {
				t.Errorf("expected %s, got %s", tc.expected, result)
			}
		})
	}
}

func BenchmarkEncodeHexString(b *testing.B) {
	testData := []byte("Hello World Hello World Hello World")
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		_ = EncodeHexString(testData)
	}
}

func BenchmarkEncodeHexStringStd(b *testing.B) {
	testData := []byte("Hello World Hello World Hello World")
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		_ = hex.EncodeToString(testData)
	}
}

func TestRoundTrip(t *testing.T) {
	original := []byte("Hello World Hello World Hello World")

	encoded := EncodeHexString(original)
	decoded, err := DecodeHexString(encoded)

	if err != nil {
		t.Fatalf("Decode error: %v", err)
	}

	if string(original) != string(decoded) {
		t.Errorf("Round-trip failed: original %q, decoded %q", original, decoded)
	}
}

func TestSample(t *testing.T) {
	var (
		data = []byte("Hello World")
		src  = unsafe.SliceData(data)
	)
	i, j, k := 0, 0, len(data)
	for k-i >= 4 {
		v := (*uint32)(unsafe.Add(unsafe.Pointer(src), i))
		hi := *v & 0x0F0F0F0F
		lo := (*v >> 4) & 0x0F0F0F0F

		fmt.Printf("0x%08x 0x%08x 0x%08x\n", *v, hi, lo)
		i += 4
		j += 8
	}
}
