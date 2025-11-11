#!/bin/bash

# Compile sample.c to object file with GCC
gcc -O3 -fomit-frame-pointer -mno-red-zone -masm=intel                         \
    -fno-asynchronous-unwind-tables -fno-stack-protector -fno-pie              \
    -fno-ident -fno-common -fno-plt -mstackrealign                             \
    -fno-jump-tables -c sample.c -o sample_gcc.o

# Generate assembly file with GCC
gcc -O3 -fomit-frame-pointer -mno-red-zone -masm=intel                         \
    -fno-asynchronous-unwind-tables -fno-stack-protector -fno-pie              \
    -fno-ident -fno-common -fno-plt -mstackrealign                             \
    -fno-jump-tables -S sample.c -o sample_gcc.s

# Compile sample.c to object file with Clang
clang -O3 -fomit-frame-pointer -mno-red-zone -masm=intel                       \
    -fno-asynchronous-unwind-tables -mllvm -inline-threshold=1000              \
    -fno-stack-protector -fno-pie -fno-ident -fno-common -fno-plt              \
    -mstackrealign -fno-jump-tables -c sample.c -o sample_clang.o

# Generate assembly file with Clang
clang -O3 -fomit-frame-pointer -mno-red-zone -masm=intel                       \
    -fno-asynchronous-unwind-tables -mllvm -inline-threshold=1000              \
    -fno-stack-protector -fno-pie -fno-ident -fno-common -fno-plt              \
    -mstackrealign -fno-jump-tables -S sample.c -o sample_clang.s

# Print object file sizes
echo "Object file sizes:"
ls -lh sample_gcc.o sample_clang.o | awk '{print $9 ": " $5}'

# Disassemble GCC object file
echo "GCC disassembly:"
objdump -d sample_gcc.o

# Disassemble Clang object file
echo "Clang disassembly:"
objdump -d sample_clang.o

# Disassemble GCC object file with go tool objdump
echo "Go objdump on GCC .o:"
go tool objdump -gnu sample_gcc.o

# Disassemble Clang object file with go tool objdump
echo "Go objdump on Clang .o:"
go tool objdump -gnu sample_clang.o

# Build the Go tool
(cd ../tool && go build -o tool.bin main.go)

# Compile sample.c to object file with Clang (via Go tool)
../tool/tool.bin -file sample.c
