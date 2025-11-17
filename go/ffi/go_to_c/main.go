//go:generate gcc -c lib.c -o lib.o
//go:generate ar rcs libadd.a lib.o

package main

/*
#cgo CFLAGS: -I.
#cgo LDFLAGS: -L. -ladd
#include <stdint.h>
#include "lib.h"

// Embedded C function to add two integers
uint64_t AddInt(uint64_t a, uint64_t b) {
    return a + b;
}
*/
import "C"
import "fmt"

func main() {
	fmt.Println("Calling C functions from Go...")

	{
		// Call the embedded C function AddInt
		res := C.AddInt(42, 58)
		fmt.Println("Result from embedded C AddInt(42, 58):", int(res))
	}

	{
		// Call the C function Add from lib.c (compiled into libadd.a)
		res := C.Add(42, 58)
		fmt.Println("Result from lib.c Add(42, 58):", int(res))
	}

	fmt.Println("Go program completed!")
}
