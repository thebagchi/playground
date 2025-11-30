#!/bin/bash

# Parse command line arguments
GEN_VMLINUX=false
CLEAR_BUILD=false

while [ $# -gt 0 ]; do
    case $1 in
        --gen)
            GEN_VMLINUX=true
            shift
            ;;
        --clear)
            CLEAR_BUILD=true
            shift
            ;;
        *)
            echo "Unknown option: $1"
            echo "Usage: $0 [--gen] [--clear]"
            exit 1
            ;;
    esac
done

# Check if BPF is supported
check_bpf_support() {
    if clang-18 -print-targets 2>&1 | grep -q "bpf"; then
        return 0
    fi
    echo "Error: clang-18 does not support BPF target"
    echo "Please install clang with BPF support: sudo apt-get install clang llvm llvm-dev"
    exit 1
}

# Clear build files if --clear option is provided
if [ "$CLEAR_BUILD" = true ]; then
    echo "Removing build files..."
    rm -f bpf.o main.bin
    exit 0
fi

# Check BPF support
echo "Checking BPF support..."
check_bpf_support
echo "✓ BPF support verified"

# Generate vmlinux.h if --gen option is provided
if [ "$GEN_VMLINUX" = true ]; then
    echo "Generating vmlinux.h..."
    bpftool btf dump file /sys/kernel/btf/vmlinux format c > vmlinux.h
fi

# Compile eBPF program
echo "Compiling eBPF program..."
clang-18 -O2 -target bpf -g -c bpf.c -I . -o bpf.o

# Compile Go application
echo "Compiling Go application..."
go build -o main.bin main.go