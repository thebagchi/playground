#!/bin/bash
bpftool btf dump file /sys/kernel/btf/vmlinux format c > vmlinux.h
clang -O2 -target bpf -g -c bpf.c -I . -o bpf.o
g++ -std=c++17 main.cc -lbpf -lelf -o main.bin