# C Parser for Function Extraction

This parser extracts function declarations from C source files using the same approach as Go's `cgo` tool.

## Features

- **DWARF-based parsing**: Uses debug symbols (like cgo) for accurate function extraction
- **Compiler support**: Works with both GCC and Clang
- **Go AST types**: Returns function signatures as Go AST types (`*ast.FuncType`)
- **Type information**: Extracts return types and parameter types from DWARF

## How It Works

### DWARF Debug Information Method (cgo approach)

This parser uses the same approach as Go's `cgo` tool:

1. **Compilation**: Compiles the C source file with debug symbols (`-g` flag)
2. **ELF Reading**: Opens the resulting object file as an ELF binary
3. **DWARF Extraction**: Reads DWARF debug sections from the object file
4. **Function Discovery**: Iterates through DWARF entries looking for `TagSubprogram` entries (functions)
5. **Type Resolution**: Resolves return types and parameter types through DWARF type information
6. **AST Generation**: Converts DWARF types to Go AST expressions

**Advantages:**
- **Very accurate** - uses the compiler's own understanding of the code
- **Handles complex C** - macros, typedefs, and other C constructs are handled by the compiler
- **Type-safe** - extracts actual type information from DWARF
- **No parsing needed** - compiler does all the hard work
- **Same as cgo** - proven approach used by Go's toolchain
- **Go-native types** - returns Go AST structures for easy manipulation

## Usage

```go
import "playground/go/asm/using_objdump/tool/parser"

// Parse with GCC
functions, err := parser.ListFunctions("sample.c", "gcc")
if err != nil {
    log.Fatal(err)
}

// Parse with Clang
functions, err := parser.ListFunctions("sample.c", "clang")
if err != nil {
    log.Fatal(err)
}

// Process results
for _, fn := range functions {
    // Use the String() method for formatted output
    fmt.Println(fn.String())
    
    // Or use FormatFuncType for just the type signature
    fmt.Println(parser.FormatFuncType(fn.Type))
    
    // Access function name and type directly
    fmt.Printf("Name: %s\n", fn.Name)
    fmt.Printf("Type: %+v\n", fn.Type)
}
```

## Data Structures

### Function
```go
type Function struct {
    Name string        // Function name (e.g., "AddInt8")
    Type *ast.FuncType // Go AST function type with parameters and results
}
```

The `Function` struct has a `String()` method that formats the complete function signature:
```go
func (f *Function) String() string
// Example output: "func AddInt8(arg1 int8, arg2 int8) int8"
```

### Helper Functions

#### FormatFuncType
```go
func FormatFuncType(ft *ast.FuncType) string
```
Formats an `ast.FuncType` into a readable string with named parameters:
- Example: `"func(arg1 int8, arg2 int8) int8"`

## Implementation Details

The parser follows cgo's DWARF-based methodology:

1. **Compilation**: Compiles C code with `-g` flag to generate debug symbols
2. **ELF Reading**: Opens the resulting object file as an ELF binary
3. **DWARF Extraction**: Reads DWARF debug sections from the object file
4. **Function Discovery**: Iterates through DWARF entries looking for `TagSubprogram` entries
5. **Type Resolution**: Resolves return types and parameter types through DWARF type information
6. **Cleanup**: Removes temporary object file after parsing

### Why DWARF Instead of Parsing?

From cgo's documentation:
> "Cgo determines the meaning of C identifiers not by parsing C code but by feeding carefully constructed programs into the system C compiler and interpreting the generated debug information and object files."

Advantages over text parsing:
- Compiler handles all C syntax complexity
- Accurate type information from debug symbols
- Handles macros, typedefs, includes automatically
- Same approach as production Go toolchain

## Reference

This implementation is inspired by Go's `cgo` tool. Key references:

- [`cmd/cgo/gcc.go`](https://github.com/golang/go/blob/master/src/cmd/cgo/gcc.go) - cgo's C parser
- [`cmd/cgo/doc.go`](https://github.com/golang/go/blob/master/src/cmd/cgo/doc.go) - cgo documentation

## Testing

Run the tests:
```bash
go test ./parser -v
```

## Example Output

For a C file containing:
```c
int8_t AddInt8(int8_t a, int8_t b);
int16_t AddInt16(int16_t a, int16_t b);
uint32_t AddUint32(uint32_t a, uint32_t b);
uint64_t MultiplyUint64(uint64_t a, uint64_t b);
void NoReturn(int x);
int NoParams(void);
```

The parser outputs:
```
func AddInt8(arg1 int8, arg2 int8) int8
func AddInt16(arg1 int16, arg2 int16) int16
func AddUint32(arg1 uint32, arg2 uint32) uint32
func MultiplyUint64(arg1 uint64, arg2 uint64) uint64
func NoReturn(arg1 int32)
func NoParams() int32
```
