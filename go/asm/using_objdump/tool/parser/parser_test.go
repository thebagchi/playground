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
#include <stdbool.h>
#include <complex.h>

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

char test_char(char a) { 
	return a; 
}

unsigned char test_uchar(unsigned char a) { 
	return a; 
}

int8_t test_int8(int8_t a) { 
	return a; 
}

uint8_t test_uint8(uint8_t a) { 
	return a; 
}

int16_t test_int16(int16_t a) { 
	return a; 
}

uint16_t test_uint16(uint16_t a) { 
	return a; 
}

int32_t test_int32(int32_t a) { 
	return a; 
}

uint32_t test_uint32(uint32_t a) { 
	return a; 
}

int64_t test_int64(int64_t a) { 
	return a; 
}

uint64_t test_uint64(uint64_t a) { 
	return a; 
}

float test_float(float a) { 
	return a; 
}

double test_double(double a) { 
	return a; 
}

bool test_bool(bool a) { 
	return a; 
}

int8_t* test_int8_ptr(int8_t* a) { 
	return a; 
}

uint8_t* test_uint8_ptr(uint8_t* a) { 
	return a; 
}

int16_t* test_int16_ptr(int16_t* a) { 
	return a; 
}

uint16_t* test_uint16_ptr(uint16_t* a) { 
	return a; 
}

int32_t* test_int32_ptr(int32_t* a) { 
	return a; 
}

uint32_t* test_uint32_ptr(uint32_t* a) { 
	return a; 
}

int64_t* test_int64_ptr(int64_t* a) { 
	return a; 
}

uint64_t* test_uint64_ptr(uint64_t* a) { 
	return a; 
}

float* test_float_ptr(float* a) { 
	return a; 
}

double* test_double_ptr(double* a) { 
	return a; 
}

bool* test_bool_ptr(bool* a) { 
	return a; 
}

char* test_char_ptr(char* a) { 
	return a; 
}

unsigned char* test_uchar_ptr(unsigned char* a) { 
	return a; 
}

int* test_int_ptr(int* a) { 
	return a; 
}

void* test_void_ptr(void* a) { 
	return a; 
}

float complex* test_complex_float_ptr(float complex* a) { 
	return a; 
}

double complex* test_complex_double_ptr(double complex* a) { 
	return a; 
}

void test_void(void) {
	return;
}

float complex test_complex_float(float complex a) { 
	return a; 
}

double complex test_complex_double(double complex a) { 
	return a; 
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
