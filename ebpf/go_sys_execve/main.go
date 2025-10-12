package main

import (
	"fmt"
	"log"
	"os"
	"os/signal"
	"syscall"
	"time"

	"github.com/cilium/ebpf"
	"github.com/cilium/ebpf/link"
)

func main() {
	// Check if bpf.o exists
	if _, err := os.Stat("bpf.o"); os.IsNotExist(err) {
		log.Fatal("bpf.o not found. Please run build.sh first to compile the eBPF program.")
	}

	// Load the eBPF program
	spec, err := ebpf.LoadCollectionSpec("bpf.o")
	if err != nil {
		log.Fatalf("Failed to load eBPF collection spec: %v", err)
	}

	// Create a new collection
	coll, err := ebpf.NewCollection(spec)
	if err != nil {
		log.Fatalf("Failed to create eBPF collection: %v", err)
	}
	defer coll.Close()

	// Get the syscall_count map
	syscallCountMap := coll.Maps["syscall_count"]
	if syscallCountMap == nil {
		log.Fatal("syscall_count map not found in eBPF program")
	}

	// Get the kprobe program
	prog := coll.Programs["kprobe_execve"]
	if prog == nil {
		log.Fatal("kprobe_execve program not found in eBPF program")
	}

	// Attach the kprobe
	kp, err := link.Kprobe("__x64_sys_execve", prog, nil)
	if err != nil {
		log.Fatalf("Failed to attach kprobe: %v", err)
	}
	defer kp.Close()

	fmt.Println("eBPF program loaded and kprobe attached successfully!")
	fmt.Println("Monitoring sys_execve calls...")

	// Set up signal handling for graceful shutdown
	sigChan := make(chan os.Signal, 1)
	signal.Notify(sigChan, syscall.SIGINT, syscall.SIGTERM)

	// Periodically read and display the syscall count
	ticker := time.NewTicker(1 * time.Second)
	defer ticker.Stop()

	for {
		select {
		case <-sigChan:
			fmt.Println("\nShutting down...")
			return
		case <-ticker.C:
			var key uint32 = 0
			var value uint64

			err := syscallCountMap.Lookup(&key, &value)
			if err != nil {
				log.Printf("Failed to lookup map value: %v", err)
				continue
			}

			fmt.Printf("\rSyscall execve count: %d", value)
		}
	}
}
