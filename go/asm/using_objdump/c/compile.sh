#!/bin/bash

# Compile sample.c to object file with GCC
gcc -O3 -fno-omit-frame-pointer -mno-red-zone -masm=intel              \
    -fno-asynchronous-unwind-tables -mstackrealign                     \
    -fno-stack-protector -fno-pie -fno-ident -fno-common -fno-plt      \
    -c sample.c -o sample_gcc.o

# Generate assembly file with GCC
gcc -O3 -fno-omit-frame-pointer -mno-red-zone -masm=intel              \
    -fno-asynchronous-unwind-tables -mstackrealign                     \
    -fno-stack-protector -fno-pie -fno-ident -fno-common -fno-plt      \
    -S sample.c -o sample_gcc.s

# Compile sample.c to object file with Clang
clang -O3 -fno-omit-frame-pointer -mno-red-zone -masm=intel            \
    -fno-asynchronous-unwind-tables -mllvm -inline-threshold=1000      \
    -mstackrealign -fno-stack-protector -fno-pie -fno-ident            \
    -fno-common -fno-plt                                               \
    -c sample.c -o sample_clang.o

# Generate assembly file with Clang
clang -O3 -fno-omit-frame-pointer -mno-red-zone -masm=intel            \
    -fno-asynchronous-unwind-tables -mllvm -inline-threshold=1000      \
    -mstackrealign -fno-stack-protector -fno-pie -fno-ident            \
    -fno-common -fno-plt                                               \
    -S sample.c -o sample_clang.s

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
go tool objdump sample_gcc.o

# Disassemble Clang object file with go tool objdump
echo "Go objdump on Clang .o:"
go tool objdump sample_clang.o