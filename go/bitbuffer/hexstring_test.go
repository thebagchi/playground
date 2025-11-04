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

func BenchmarkHexDecodeString(b *testing.B) {
	testData := "48656c6c6f20576f726c642048656c6c6f20576f726c642048656c6c6f20576f726c64"
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		_, _ = hex.DecodeString(testData)
	}
}
