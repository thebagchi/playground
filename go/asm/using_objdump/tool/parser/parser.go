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
			fmt.Fprintf(&b, "arg%d %v", i+1, field.Type)
		}
	}

	b.WriteString(")")

	// Format return type
	if f.Type.Results != nil && len(f.Type.Results.List) > 0 {
		b.WriteString(" ")
		if len(f.Type.Results.List) == 1 {
			fmt.Fprintf(&b, "%v", f.Type.Results.List[0].Type)
		} else {
			b.WriteString("(")
			for i, field := range f.Type.Results.List {
				if i > 0 {
					b.WriteString(", ")
				}
				fmt.Fprintf(&b, "%v", field.Type)
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
			fmt.Fprintf(&b, "arg%d %v", i+1, field.Type)
		}
	}

	b.WriteString(")")

	// Format return type
	if ft.Results != nil && len(ft.Results.List) > 0 {
		b.WriteString(" ")
		if len(ft.Results.List) == 1 {
			fmt.Fprintf(&b, "%v", ft.Results.List[0].Type)
		} else {
			b.WriteString("(")
			for i, field := range ft.Results.List {
				if i > 0 {
					b.WriteString(", ")
				}
				fmt.Fprintf(&b, "%v", field.Type)
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
			"-g", // Add debug symbols for DWARF
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
			"-mstackrealign",
			"-fno-jump-tables",
			"-c",
			input,
			"-o",
			output,
		}
	} else {
		// gcc or other compiler
		args = []string{
			"-g", // Add debug symbols for DWARF
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
			"-mstackrealign",
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
	// Remove qualifiers first
	t = removeQualifier(t)

	switch v := t.(type) {
	case *dwarf.BasicType:
		return ast.NewIdent(v.Name)
	case *dwarf.PtrType:
		return &ast.StarExpr{X: makeType(v.Type)}
	case *dwarf.ArrayType:
		return &ast.ArrayType{Elt: makeType(v.Type)}
	case *dwarf.StructType:
		if v.StructName != "" {
			return ast.NewIdent(v.StructName)
		}
		return ast.NewIdent("struct")
	case *dwarf.TypedefType:
		return ast.NewIdent(v.Name)
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
