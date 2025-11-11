package parser

import (
	"os"
	"path/filepath"
	"testing"
)

func TestListFunctions(t *testing.T) {
	// Create a temporary C file with multiple functions
	code := `
#include <stdint.h>

int8_t AddInt8(int8_t a, int8_t b) {
    return a + b;
}

int16_t AddInt16(int16_t a, int16_t b) {
    return a + b;
}

uint32_t AddUint32(uint32_t a, uint32_t b) {
    return a + b;
}

uint64_t MultiplyUint64(uint64_t a, uint64_t b) {
    return a * b;
}

void NoReturn(int x) {
    // Function with void return
}

int NoParams(void) {
    return 42;
}
`

	// Create temp directory and file
	tmp := t.TempDir()
	file := filepath.Join(tmp, "test.c")

	if err := os.WriteFile(file, []byte(code), 0644); err != nil {
		t.Fatalf("Failed to write C file: %v", err)
	}

	// Test with gcc
	t.Run("gcc", func(t *testing.T) {
		functions, err := ListFunctions(file, "gcc")
		if err != nil {
			t.Fatalf("ListFunctions failed: %v", err)
		}

		if len(functions) == 0 {
			t.Fatal("Expected functions, got none")
		}

		t.Logf("Found %d functions with gcc:", len(functions))
		for i, fn := range functions {
			t.Logf("  [%d] %s", i+1, fn.String())
		}
	})

	// Test with clang
	t.Run("clang", func(t *testing.T) {
		functions, err := ListFunctions(file, "clang")
		if err != nil {
			t.Fatalf("ListFunctions failed: %v", err)
		}

		if len(functions) == 0 {
			t.Fatal("Expected functions, got none")
		}

		t.Logf("Found %d functions with clang:", len(functions))
		for i, fn := range functions {
			t.Logf("  [%d] %s", i+1, fn.String())
		}
	})
}
