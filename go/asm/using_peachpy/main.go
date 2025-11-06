package main

import (
	"fmt"
)

// Function from PeachPy-generated assembly
func Add(x uint64, y uint64) uint64

func main() {
	result := Add(5, 3)
	fmt.Println("Add(5, 3) =", result)

	result = Add(10, 20)
	fmt.Println("Add(10, 20) =", result)
}
