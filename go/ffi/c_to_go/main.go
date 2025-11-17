package main

//go:generate go build -o libgo.so -buildmode=c-shared main.go
//go:generate go build -o libgo.a -buildmode=c-archive main.go
//go:generate gcc -o main.bin main.c libgo.a

import "C"
import "fmt"

//export AddInt
func AddInt(a, b uint64) uint64 {
	return a + b
}

func main() {
	fmt.Println("inside main ...")
}
