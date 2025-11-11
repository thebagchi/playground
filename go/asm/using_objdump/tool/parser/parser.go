package parser

import (
	"bytes"
	"debug/dwarf"
	"debug/elf"
	"fmt"
	"go/ast"
	"os"
	"os/exec"
	"strings"
)

// Function represents a parsed C function with its name and type signature
type Function struct {
	Name string
	Type *ast.FuncType
}

// formatExpr converts an ast.Expr to a string representation
func formatExpr(expr ast.Expr) string {
	switch e := expr.(type) {
	case *ast.Ident:
		return e.Name
	case *ast.StarExpr:
		return "*" + formatExpr(e.X)
	case *ast.ArrayType:
		return "[]" + formatExpr(e.Elt)
	case *ast.FuncType:
		var b strings.Builder
		b.WriteString("func(")
		if e.Params != nil {
			for i, field := range e.Params.List {
				if i > 0 {
					b.WriteString(", ")
				}
				b.WriteString(formatExpr(field.Type))
			}
		}
		b.WriteString(")")
		if e.Results != nil && len(e.Results.List) > 0 {
			b.WriteString(" ")
			if len(e.Results.List) == 1 {
				b.WriteString(formatExpr(e.Results.List[0].Type))
			} else {
				b.WriteString("(")
				for i, field := range e.Results.List {
					if i > 0 {
						b.WriteString(", ")
					}
					b.WriteString(formatExpr(field.Type))
				}
				b.WriteString(")")
			}
		}
		return b.String()
	default:
		return fmt.Sprintf("%v", expr)
	}
}

// String returns a string representation of the function with its name
// Example: "func AddInt8(arg1 int8, arg2 int8) int8"
func (f *Function) String() string {
	if f == nil || f.Type == nil {
		return ""
	}

	var b strings.Builder
	b.WriteString("func ")
	b.WriteString(f.Name)
	b.WriteString("(")

	// Format parameters with names
	if f.Type.Params != nil && len(f.Type.Params.List) > 0 {
		for i, field := range f.Type.Params.List {
			if i > 0 {
				b.WriteString(", ")
			}
			b.WriteString(fmt.Sprintf("arg%d ", i+1))
			b.WriteString(formatExpr(field.Type))
		}
	}

	b.WriteString(")")

	// Format return type (skip if void - Go doesn't specify return type for void functions)
	if f.Type.Results != nil && len(f.Type.Results.List) > 0 {
		if len(f.Type.Results.List) == 1 {
			// Check if single return type is void
			ret := formatExpr(f.Type.Results.List[0].Type)
			if ret != "void" {
				b.WriteString(" ")
				b.WriteString(ret)
			}
		} else {
			// Multiple return values - format as tuple
			b.WriteString(" (")
			for i, field := range f.Type.Results.List {
				if i > 0 {
					b.WriteString(", ")
				}
				b.WriteString(formatExpr(field.Type))
			}
			b.WriteString(")")
		}
	}

	return b.String()
}

// FormatFuncType returns a string representation of the function signature
// Example: "func(arg1 int8, arg2 int8) int8"
func FormatFuncType(ft *ast.FuncType) string {
	if ft == nil {
		return ""
	}

	var b strings.Builder
	b.WriteString("func(")

	// Format parameters with names
	if ft.Params != nil && len(ft.Params.List) > 0 {
		for i, field := range ft.Params.List {
			if i > 0 {
				b.WriteString(", ")
			}
			b.WriteString(fmt.Sprintf("arg%d ", i+1))
			b.WriteString(formatExpr(field.Type))
		}
	}

	b.WriteString(")")

	// Format return type (skip if void - Go doesn't specify return type for void functions)
	if ft.Results != nil && len(ft.Results.List) > 0 {
		if len(ft.Results.List) == 1 {
			// Check if single return type is void
			retType := formatExpr(ft.Results.List[0].Type)
			if retType != "void" {
				b.WriteString(" ")
				b.WriteString(retType)
			}
		} else {
			// Multiple return values - format as tuple
			b.WriteString(" (")
			for i, field := range ft.Results.List {
				if i > 0 {
					b.WriteString(", ")
				}
				b.WriteString(formatExpr(field.Type))
			}
			b.WriteString(")")
		}
	}

	return b.String()
}

// ListFunctions parses a C source file and extracts function declarations
// It uses gcc/clang to compile with debug symbols and reads DWARF info (cgo approach)
// Returns a slice of Functions with names and type signatures
func ListFunctions(filename string, compiler string) ([]Function, error) {
	return compileFunctions(filename, compiler)
}

// compile compiles a C source file to an object file with the same options as main.go
func compile(compiler, input, output string) error {
	var args []string

	if compiler == "clang" {
		args = []string{
			"-g",
			"-O3",
			"-fomit-frame-pointer",
			"-mno-red-zone",
			"-masm=intel",
			"-fno-asynchronous-unwind-tables",
			"-mllvm", "-inline-threshold=1000",
			"-fno-stack-protector",
			"-fno-pie",
			"-fno-ident",
			"-fno-common",
			"-fno-plt",
			// "-mstackrealign",
			"-fno-jump-tables",
			"-c",
			input,
			"-o",
			output,
		}
	} else {
		// gcc or other compiler
		args = []string{
			"-g",
			"-O3",
			"-fomit-frame-pointer",
			"-mno-red-zone",
			"-masm=intel",
			"-fno-asynchronous-unwind-tables",
			"-fno-stack-protector",
			"-fno-pie",
			"-fno-ident",
			"-fno-common",
			"-fno-plt",
			// "-mstackrealign",
			"-fno-jump-tables",
			"-c",
			input,
			"-o",
			output,
		}
	}

	cmd := exec.Command(compiler, args...)
	var stderr bytes.Buffer
	cmd.Stderr = &stderr

	if err := cmd.Run(); err != nil {
		return fmt.Errorf("compilation failed: %w\nstderr: %s", err, stderr.String())
	}

	return nil
}

// processELF opens an ELF file and reads DWARF data from it
func processELF(object string) (*dwarf.Data, error) {
	// Open ELF file and read DWARF
	ef, err := elf.Open(object)
	if err != nil {
		return nil, fmt.Errorf("failed to open object file: %w", err)
	}
	defer ef.Close()

	data, err := ef.DWARF()
	if err != nil {
		return nil, fmt.Errorf("failed to read DWARF data: %w", err)
	}

	return data, nil
}

// compileFunctions compiles C code with debug symbols and extracts function info from DWARF
// This is the approach used by cgo
func compileFunctions(filename string, compiler string) ([]Function, error) {
	// Compile to object file with debug symbols
	object := filename + ".d.o"
	defer os.Remove(object)

	if err := compile(compiler, filename, object); err != nil {
		return nil, err
	}

	data, err := processELF(object)
	if err != nil {
		return nil, err
	}

	return extractFunctions(data)
}

// extractFunctions extracts function declarations from DWARF debug information
func extractFunctions(d *dwarf.Data) ([]Function, error) {
	var functions []Function
	reader := d.Reader()

	for {
		entry, err := reader.Next()
		if err != nil {
			return nil, fmt.Errorf("reading DWARF entry: %w", err)
		}
		if entry == nil {
			break
		}

		// Look for function subprogram entries
		if entry.Tag == dwarf.TagSubprogram {
			fn := extractFunction(d, entry)
			if fn != nil {
				functions = append(functions, *fn)
			}
		}
	}

	return functions, nil
}

// extractFunction extracts function information from a DWARF subprogram entry
// This follows cgo's approach of reading function signatures from DWARF
func extractFunction(d *dwarf.Data, entry *dwarf.Entry) *Function {
	// Get function name
	name, ok := entry.Val(dwarf.AttrName).(string)
	if !ok || name == "" {
		return nil
	}

	// Get return type from the type attribute
	var result ast.Expr
	if offset, ok := entry.Val(dwarf.AttrType).(dwarf.Offset); ok {
		if dt, err := d.Type(offset); err == nil {
			result = makeType(dt)
		}
	}
	if result == nil {
		result = ast.NewIdent("void")
	}

	// Extract parameters by reading child entries (like cgo does)
	fields := extractParameters(d, entry)

	return &Function{
		Name: name,
		Type: &ast.FuncType{
			Params:  &ast.FieldList{List: fields},
			Results: &ast.FieldList{List: []*ast.Field{{Type: result}}},
		},
	}
}

// extractParameters reads child entries of a subprogram to get formal parameters
// This mimics how cgo processes DWARF formal parameter entries
func extractParameters(d *dwarf.Data, entry *dwarf.Entry) []*ast.Field {
	var fields = make([]*ast.Field, 0)

	// We need a new reader positioned after the subprogram entry to read children
	// Note: This is a simplified approach. A full implementation would need to
	// properly track entry depth and handle nested scopes
	reader := d.Reader()
	reader.Seek(entry.Offset)

	// Skip the subprogram entry itself
	reader.Next()

	// Read children until we hit the end or a sibling
	for {
		entry, err := reader.Next()
		if err != nil || entry == nil {
			break
		}

		// If we hit a null entry, we've reached the end of children
		if entry.Tag == 0 {
			break
		}

		// Check if this is a formal parameter
		if entry.Tag == dwarf.TagFormalParameter {
			// Get the parameter type
			if offset, ok := entry.Val(dwarf.AttrType).(dwarf.Offset); ok {
				if dt, err := d.Type(offset); err == nil {
					fields = append(fields, &ast.Field{Type: makeType(dt)})
				}
			}
		} else if entry.Tag == dwarf.TagUnspecifiedParameters {
			// This is a variadic function (...) - like cgo, we'll note it but not add a param
			continue
		} else {
			// Hit a non-parameter child (like a lexical block), stop reading parameters
			break
		}
	}

	return fields
}

// makeType converts a DWARF type to a Go AST expression
func makeType(t dwarf.Type) ast.Expr {
	if t == nil {
		return ast.NewIdent("unknown")
	}

	// Remove qualifiers first
	t = removeQualifier(t)

	switch v := t.(type) {
	case *dwarf.BasicType:
		return ast.NewIdent(namedType(v.Name))
	case *dwarf.CharType:
		// CharType represents a signed character (like int8_t)
		return ast.NewIdent("int8")
	case *dwarf.UcharType:
		// UcharType represents an unsigned character (like uint8_t)
		return ast.NewIdent("uint8")
	case *dwarf.IntType:
		// IntType represents signed integer types
		return ast.NewIdent(sintType(v.ByteSize))
	case *dwarf.UintType:
		// UintType represents unsigned integer types
		return ast.NewIdent(uintType(v.ByteSize))
	case *dwarf.FloatType:
		// FloatType represents floating-point types
		return ast.NewIdent(floatType(v.ByteSize))
	case *dwarf.ComplexType:
		// ComplexType represents complex floating-point types
		return ast.NewIdent(complexType(v.ByteSize))
	case *dwarf.BoolType:
		// BoolType represents a boolean type
		return ast.NewIdent("bool")
	case *dwarf.AddrType:
		// AddrType represents a machine address type
		return ast.NewIdent("uintptr")
	case *dwarf.EnumType:
		// EnumType - map based on size, check if it has negative values for signed
		signed := false
		for _, ev := range v.Val {
			if ev.Val < 0 {
				signed = true
				break
			}
		}
		if signed {
			return ast.NewIdent(sintType(v.ByteSize))
		}
		return ast.NewIdent(uintType(v.ByteSize))
	case *dwarf.FuncType:
		// FuncType represents function pointers - use uintptr
		return ast.NewIdent("uintptr")
	case *dwarf.UnspecifiedType:
		// UnspecifiedType represents implicit/unknown/ambiguous types
		return ast.NewIdent("void")
	case *dwarf.PtrType:
		// Special case: void* maps to uintptr (like cgo does)
		if _, ok := removeQualifier(v.Type).(*dwarf.VoidType); ok {
			return ast.NewIdent("uintptr")
		}
		return &ast.StarExpr{X: makeType(v.Type)}
	case *dwarf.ArrayType:
		return &ast.ArrayType{Elt: makeType(v.Type)}
	case *dwarf.StructType:
		if v.StructName != "" {
			return ast.NewIdent(v.StructName)
		}
		return ast.NewIdent("struct")
	case *dwarf.TypedefType:
		// Resolve typedef to underlying type
		return makeType(v.Type)
	case *dwarf.VoidType:
		return ast.NewIdent("void")
	default:
		return ast.NewIdent("unknown")
	}
}

// removeQualifier removes type qualifiers (const, volatile, restrict)
// but preserves typedef types (like int8_t, uint32_t, etc.)
func removeQualifier(t dwarf.Type) dwarf.Type {
	for {
		if qt, ok := t.(*dwarf.QualType); ok {
			t = qt.Type
		} else {
			return t
		}
	}
}

// sintType converts signed integer types based on byte size to Go type names
func sintType(byteSize int64) string {
	switch byteSize {
	case 1:
		return "int8"
	case 2:
		return "int16"
	case 4:
		return "int32"
	case 8:
		return "int64"
	default:
		return "int"
	}
}

// uintType converts unsigned integer types based on byte size to Go type names
func uintType(byteSize int64) string {
	switch byteSize {
	case 1:
		return "uint8"
	case 2:
		return "uint16"
	case 4:
		return "uint32"
	case 8:
		return "uint64"
	default:
		return "uint"
	}
}

// floatType converts floating-point types based on byte size to Go type names
func floatType(byteSize int64) string {
	switch byteSize {
	case 4:
		return "float32"
	case 8:
		return "float64"
	default:
		return "float"
	}
}

// complexType converts complex floating-point types based on byte size to Go type names
func complexType(byteSize int64) string {
	switch byteSize {
	case 8:
		return "complex64"
	case 16:
		return "complex128"
	default:
		return "complex"
	}
}

// namedType converts C type names to Go type names
func namedType(ctype string) string {
	switch ctype {
	// Signed integers
	case "char":
		fallthrough
	case "signed char":
		return "int8"
	case "short":
		fallthrough
	case "short int":
		fallthrough
	case "signed short":
		fallthrough
	case "signed short int":
		return "int16"
	case "int":
		fallthrough
	case "signed int":
		return "int32"
	case "long":
		fallthrough
	case "long int":
		fallthrough
	case "signed long":
		fallthrough
	case "signed long int":
		return "int64"
	case "long long":
		fallthrough
	case "long long int":
		fallthrough
	case "signed long long":
		fallthrough
	case "signed long long int":
		return "int64"

	// Unsigned integers
	case "unsigned char":
		return "uint8"
	case "unsigned short":
		fallthrough
	case "unsigned short int":
		return "uint16"
	case "unsigned int":
		fallthrough
	case "unsigned":
		return "uint32"
	case "unsigned long":
		fallthrough
	case "unsigned long int":
		return "uint64"
	case "unsigned long long":
		fallthrough
	case "unsigned long long int":
		return "uint64"

	// Floating point
	case "float":
		return "float32"
	case "double":
		return "float64"

	// Other types
	case "void":
		return "void"
	case "bool":
		fallthrough
	case "_Bool":
		return "bool"

	default:
		// Return as-is if not a known type
		return ctype
	}
}
