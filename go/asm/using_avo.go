//go:build ignore

package main

import (
	"github.com/mmcloughlin/avo/build"
)

func main() {
	build.TEXT("Add", build.NOSPLIT, "func(x, y uint64) uint64")
	build.Doc("Add adds two uint64 numbers using assembly.")

	// Load parameters
	x := build.Load(build.Param("x"), build.GP64())
	y := build.Load(build.Param("y"), build.GP64())

	// Add them
	build.ADDQ(x, y)

	// Store result
	build.Store(y, build.ReturnIndex(0))

	// Return
	build.RET()

	build.Generate()
}
