# Register Usage (System V ABI – x86-64)

## Register Sizes
All general-purpose registers in x86-64 are **64-bit (8 bytes)** wide:
- RDI, RSI, RDX, RCX, R8, R9: Parameter registers (64-bit)
- RAX: Return register (64-bit)

## Data Type Byte Widths (stdint.h Fixed-Width Types)

| Type          | Size (bytes) |
| --------------|--------------|
| int8_t        | 1            |
| uint8_t       | 1            |
| int16_t       | 2            |
| uint16_t      | 2            |
| int32_t       | 4            |
| uint32_t      | 4            |
| int64_t       | 8            |
| uint64_t      | 8            |
| float         | 4            |
| double        | 8            |
| uintptr_t     | 8            |

## Parameter Passing

| Arg # | Register | Byte Width   |
|-------|----------|--------------|
| 1     | RDI      | 8 bytes      |
| 2     | RSI      | 8 bytes      |
| 3     | RDX      | 8 bytes      |
| 4     | RCX      | 8 bytes      |
| 5     | R8       | 8 bytes      |
| 6     | R9       | 8 bytes      |
| 7+    | Stack    | 8 bytes each |

### Parameter Size Handling
- **Smaller types** (< 8 bytes): Zero-extended or sign-extended to 64-bit register
- **8-byte types**: Use full 64-bit register
- **Larger types** (structs, arrays): Passed by reference (pointer in register)

## Return Values

**Return:** RAX (64-bit register) or XMM0 (floating point)

| Return Type | Register | Handling                 |
|-------------|----------|--------------------------|
| void        | —        | None                     |
| int8_t      | RAX      | Lower 8 bits (AL)        |
| uint8_t     | RAX      | Lower 8 bits (AL)        |
| int16_t     | RAX      | Lower 16 bits (AX)       |
| uint16_t    | RAX      | Lower 16 bits (AX)       |
| int32_t     | RAX      | Lower 32 bits (EAX)      |
| uint32_t    | RAX      | Lower 32 bits (EAX)      |
| int64_t     | RAX      | Full register            |
| uint64_t    | RAX      | Full register            |
| float       | XMM0     | IEEE 754 single precision|
| double      | XMM0     | IEEE 754 double precision|
| uintptr_t   | RAX      | Pointer value            |
| struct      | RAX      | Pointer to struct        |

# Registers in Go Assembly

Go uses Plan 9 assembly syntax, which has its own register names that map to x86-64 registers. Here's the mapping:

## General Purpose Registers

| Go Register | x86-64 Register | Purpose                        |
|-------------|-----------------|--------------------------------|
| AX          | RAX             | Accumulator, return value      |
| BX          | RBX             | Base register                  |
| CX          | RCX             | Counter, loop control          |
| DX          | RDX             | Data register, division        |
| SI          | RSI             | Source index                   |
| DI          | RDI             | Destination index              |
| BP          | RBP             | Base pointer (frame pointer)   |
| SP          | RSP             | Stack pointer                  |
| R8          | R8              | General purpose                |
| R9          | R9              | General purpose                |
| R10         | R10             | General purpose                |
| R11         | R11             | General purpose                |
| R12         | R12             | General purpose (preserved)    |
| R13         | R13             | General purpose (preserved)    |
| R14         | R14             | General purpose (preserved)    |
| R15         | R15             | General purpose (preserved)    |

## Floating Point and SIMD Registers

| Go Register | x86-64 Register | Purpose                            |
|-------------|-----------------|------------------------------------|
| X0          | XMM0            | Floating point return, parameters  |
| X1          | XMM1            | Floating point parameters          |
| X2          | XMM2            | Floating point parameters          |
| X3          | XMM3            | Floating point parameters          |
| X4-X15      | XMM4-XMM15      | Additional floating point/SIMD     |

## SIMD Register Sizes and Instruction Sets

Modern x86-64 processors support different register sizes for SIMD (Single Instruction, Multiple Data) operations:

| Register Size | Go Register | x86-64 Register | Count | Instruction Set                           |
|---------------|-------------|-----------------|-------|-------------------------------------------|
| 128-bit       | X0-X15      | XMM0-XMM15      | 16    | SSE, SSE2, SSE3, SSSE3, SSE4.1, SSE4.2    |
| 256-bit       | Y0-Y15      | YMM0-YMM15      | 16    | AVX, AVX2, FMA3, FMA4                     |
| 512-bit       | Z0-Z31      | ZMM0-ZMM31      | 32    | AVX-512F, AVX-512BW, AVX-512DQ, AVX-512VL |

### 128-bit Registers (SSE Instruction Set)
- **Registers**: XMM0-XMM15 (16 registers total)
- **Size**: 128 bits (16 bytes)
- **Go Assembly**: Referred to as X0-X15
- **Instructions**: SSE, SSE2, SSE3, SSSE3, SSE4.1, SSE4.2
- **Use Cases**: 
  - 4 single-precision floats (32-bit each)
  - 2 double-precision floats (64-bit each)
  - 16 bytes, 8 words, 4 doublewords, or 2 quadwords
  - Legacy floating-point operations

### 256-bit Registers (AVX/AVX2 Instruction Set)
- **Registers**: YMM0-YMM15 (16 registers total)
- **Size**: 256 bits (32 bytes)
- **Go Assembly**: YMM registers are accessed through XMM names (lower 128 bits)
- **Instructions**: AVX, AVX2, FMA3, FMA4
- **Use Cases**:
  - 8 single-precision floats (32-bit each)
  - 4 double-precision floats (64-bit each)
  - 32 bytes, 16 words, 8 doublewords, or 4 quadwords
  - Enhanced vector processing with 3-operand instructions

### 512-bit Registers (AVX-512 Instruction Set)
- **Registers**: ZMM0-ZMM31 (32 registers total)
- **Size**: 512 bits (64 bytes)
- **Go Assembly**: Limited or no direct support (check Go version and architecture)
- **Instructions**: AVX-512F, AVX-512BW, AVX-512DQ, AVX-512VL, etc.
- **Use Cases**:
  - 16 single-precision floats (32-bit each)
  - 8 double-precision floats (64-bit each)
  - 64 bytes, 32 words, 16 doublewords, or 8 quadwords
  - Advanced vector processing with masking and scatter/gather operations

### Register Hierarchy
The larger registers overlap with the smaller ones:
- **ZMM registers** contain the full 512 bits
- **YMM registers** contain the lower 256 bits of ZMM registers
- **XMM registers** contain the lower 128 bits of YMM/ZMM registers

When you write to an XMM register, the upper bits of the corresponding YMM/ZMM register are zeroed out (depending on the instruction set).

### Go Assembly SIMD Support
Go's assembler has varying levels of support for SIMD instructions:
- **SSE/SSE2**: Well supported through X0-X15 registers
- **AVX/AVX2**: Supported but may require assembly directives
- **AVX-512**: Limited support, depends on target architecture and Go version

For advanced SIMD operations, consider using Go's `golang.org/x/sys/cpu` package to detect CPU capabilities at runtime.

## Special Registers

| Go Register | x86-64 Register | Purpose                               |
|-------------|-----------------|---------------------------------------|
| PC          | RIP             | Program counter (instruction pointer) |
| CS          | CS              | Code segment                          |
| FS          | FS              | Thread-local storage segment          |
| GS          | GS              | OS-specific segment                   |

## Register Usage Conventions in Go

- **AX**: Primary accumulator, function return value
- **BX, R12-R15**: Callee-saved registers (preserved across function calls)
- **CX, DX, SI, DI, R8-R11**: Caller-saved registers (not preserved)
- **SP**: Stack pointer, must be 16-byte aligned
- **BP**: Frame pointer (optional, used for debugging)
- **X0-X7**: Used for floating point parameters and return values

## Pseudo-Registers

Go assembly also defines pseudo-registers for special purposes:

| Pseudo-Register      | Purpose                               |
|----------------------|---------------------------------------|
| SB (Static Base)     | Address of the program's static data  |
| FP (Frame Pointer)   | Offset from current function's frame  |
| SP (Stack Pointer)   | Hardware stack pointer                |
| PC (Program Counter) | Current instruction address           |

Note: These pseudo-registers are resolved by the Go assembler and don't correspond to actual hardware registers.