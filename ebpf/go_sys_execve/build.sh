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

# Clear build files if --clear option is provided
if [ "$CLEAR_BUILD" = true ]; then
    echo "Removing build files..."
    rm -f bpf.o main.bin
    exit 0
fi

# Generate vmlinux.h if --gen option is provided
if [ "$GEN_VMLINUX" = true ]; then
    echo "Generating vmlinux.h..."
    bpftool btf dump file /sys/kernel/btf/vmlinux format c > vmlinux.h
fi

# Compile eBPF program
echo "Compiling eBPF program..."
clang -O2 -target bpf -g -c bpf.c -I . -o bpf.o

# Compile Go application
echo "Compiling go application..."
go build -o main.bin main.go