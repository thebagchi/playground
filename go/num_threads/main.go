package main

import (
	"bufio"
	"fmt"
	"os"
	"runtime"
	"strconv"
	"strings"
	"time"
)

const TEN_SECOND = 10 * time.Second

func getThreads() int {
	file, err := os.Open("/proc/self/status")
	if err != nil {
		fmt.Println("Error: ", err)
		return 0
	}
	defer file.Close()
	scanner := bufio.NewScanner(file)
	for scanner.Scan() {
		line := scanner.Text()
		if strings.HasPrefix(line, "Threads:") {
			fields := strings.Fields(line)
			if len(fields) == 2 {
				num, err := strconv.Atoi(fields[1])
				if nil != err {
					fmt.Println("Error: ", err)
					return 0
				}
				return num
			}
		}
	}
	return 0
}

//go:generate podman build -f dockerfile -t alpine-golang
//go:generate podman run --cpus=0.5 --rm -it alpine-golang
//go:generate podman run --cpus=2.0 --rm -it alpine-golang
func setMaxProcs(n int) {
	fmt.Println("==> updating max procs to: ", n)
	runtime.GOMAXPROCS(n)
	fmt.Println("==> max procs: ", runtime.GOMAXPROCS(0))
	time.Sleep(TEN_SECOND)
	fmt.Println("==> num threads: ", getThreads())
}

func resetMaxProcs() {
	fmt.Println("==> updating max procs to: default")
	runtime.SetDefaultGOMAXPROCS()
	fmt.Println("==> max procs: ", runtime.GOMAXPROCS(0))
	time.Sleep(TEN_SECOND)
	fmt.Println("==> num threads: ", getThreads())
}

func main() {

	fmt.Println("num cpus: ", runtime.NumCPU())
	fmt.Println("==> max procs: ", runtime.GOMAXPROCS(0))
	fmt.Println("==> num threads: ", getThreads())

	for i := 256; i > 0; i = i >> 1 {
		setMaxProcs(i)
	}
	resetMaxProcs()
}
