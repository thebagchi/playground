package bitbuffer

import (
	"encoding/hex"
	"strings"
	"testing"
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
	testData := "48656c6c6f20576f726c642048656c6c6f20576f726c642048656c6c6f20576f726c64"
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		_, _ = DecodeHexString(testData)
	}
}

func TestEncodeHexString(t *testing.T) {
	testCases := []struct {
		name     string
		input    []byte
		expected string
	}{
		{
			name:     "Hello World",
			input:    []byte("Hello World"),
			expected: "48656c6c6f20576f726c64",
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

func TestRoundTrip(t *testing.T) {
	original := []byte("Hello World! This is a test of hex encoding/decoding.")

	encoded := EncodeHexString(original)
	decoded, err := DecodeHexString(encoded)

	if err != nil {
		t.Fatalf("Decode error: %v", err)
	}

	if string(original) != string(decoded) {
		t.Errorf("Round-trip failed: original %q, decoded %q", original, decoded)
	}
}
