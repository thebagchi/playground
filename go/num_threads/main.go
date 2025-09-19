package main

import (
	"fmt"
	"runtime"
	"time"
)

const ONE_MINUTE = 60 * time.Second

func setMaxProcs(n int) {
	fmt.Println("==> updating max procs to: ", n)
	runtime.GOMAXPROCS(n)
	fmt.Println("==> max procs: ", runtime.GOMAXPROCS(0))
	time.Sleep(ONE_MINUTE)
}

func resetMaxProcs() {
	fmt.Println("==> updating max procs to: default")
	runtime.SetDefaultGOMAXPROCS()
	fmt.Println("==> max procs: ", runtime.GOMAXPROCS(0))
	time.Sleep(ONE_MINUTE)
}

func main() {

	fmt.Println("num cpus: ", runtime.NumCPU())
	fmt.Println("==> max procs: ", runtime.GOMAXPROCS(0))

	setMaxProcs(4)
	setMaxProcs(2)
	resetMaxProcs()
	setMaxProcs(1)
}
