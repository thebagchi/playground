# C to Go FFI Example

This example demonstrates calling Go functions from C code using Foreign Function Interface (FFI).

## Overview

- `main.go`: Go code with exported functions that can be called from C
- `main.c`: C code that calls the Go functions and sleeps for 100 seconds
- `libgo.a` / `libgo.so`: Generated Go libraries (created via `go generate`)
- `libgo.h`: Generated C header file with function declarations
- `main.bin`: Compiled C executable
- `run.sh`: Script to run the executable and check thread count

## Threading Behavior

**Important Note**: Although the C code itself does not create any threads explicitly, the program shows 7 threads when running. This is because:

1. **Go Runtime Threads**: Even when calling Go functions from C via FFI, Go's runtime creates multiple threads for:
   - Garbage collection
   - System calls
   - Runtime management
   - Scheduler operations

2. **No C Threads**: The C code (`main.c`) contains no threading code - no `pthread_create()`, no OpenMP, etc.

3. **FFI Overhead**: The thread count comes from Go's runtime being initialized when the first Go function is called from C.

## Building

```bash
go generate  # Builds Go libraries and C executable automatically
```

This runs the following `go:generate` directives:
- `go build -o libgo.so -buildmode=c-shared main.go`
- `go build -o libgo.a -buildmode=c-archive main.go`
- `gcc -o main.bin main.c libgo.a`

## Running

```bash
./main.bin                # Run the C executable (sleeps 100 seconds)
./run.sh                  # Run and check thread count (waits for completion)
```

## Expected Output

```
inside main ...
Result from Go AddInt(42, 58): 100
Sleeping for 100 seconds...
Woke up after sleep!
```

The program will show ~7 threads despite having no explicit threading in C code.