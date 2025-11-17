# Go to C FFI Example

This example demonstrates calling C functions from Go code using Foreign Function Interface (FFI) with cgo, showcasing both embedded C code and linked external libraries.

## Overview

- `main.go`: Go code with embedded C function + links to external C library
- `lib.c`: C implementation with `Add` function
- `lib.h`: C header file declaring the `Add` function
- `libadd.a`: Generated static C library (created via `go generate`)

## Building

```bash
go generate  # Builds the C library automatically
```

This runs the following `go:generate` directives:
- `gcc -c lib.c -o lib.o`
- `ar rcs libadd.a lib.o`

## Running

```bash
go run main.go  # Run the Go program that calls both C functions
```

## Expected Output

```
Calling C functions from Go...
Result from embedded C AddInt(42, 58): 100
Result from lib.c Add(42, 58): 100
Go program completed!
```

## How it Works

This example demonstrates **two C integration approaches**:

1. **Embedded C Code**: The `AddInt` function is defined directly in the Go file within a cgo comment block and compiled by cgo
2. **External Library**: The `Add` function from `lib.c` is compiled into `libadd.a` and linked via LDFLAGS

The cgo directives handle both:
- `#cgo CFLAGS: -I.` - Include current directory for headers
- `#cgo LDFLAGS: -L. -ladd` - Link against libadd.a
- `#include "lib.h"` - Declare external functions

This shows the flexibility of cgo in combining embedded C code with external C libraries.