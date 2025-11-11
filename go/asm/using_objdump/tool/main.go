package main

import (
	"bytes"
	"debug/elf"
	"flag"
	"fmt"
	"os"
	"os/exec"
	"sort"
	"strings"

	"golang.org/x/arch/x86/x86asm"

	"playground/go/asm/using_objdump/tool/parser"
)

const NEWLINE = "\n"

var (
	filename = flag.String("file", "", "input filename")
	compiler = flag.String("compiler", "gcc", "compiler to use (clang or gcc)")
)

func clangFound() bool {
	_, err := exec.LookPath("clang")
	return err == nil
}

func gccFound() bool {
	_, err := exec.LookPath("gcc")
	return err == nil
}

func objdumpFound() bool {
	_, err := exec.LookPath("objdump")
	return err == nil
}

func clangCompile(input, output string) error {
	args := []string{
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

	cmd := exec.Command("clang", args...)
	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stderr

	return cmd.Run()
}

func gccCompile(input, output string) error {
	args := []string{
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

	cmd := exec.Command("gcc", args...)
	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stderr

	return cmd.Run()
}

func objdump(filename string) (string, error) {
	args := []string{
		"-d",
		"-M", "intel",
		filename,
	}

	cmd := exec.Command("objdump", args...)
	var out bytes.Buffer
	cmd.Stdout = &out
	cmd.Stderr = os.Stderr

	err := cmd.Run()
	return out.String(), err
}

func dumpSection(base uint64, bits int, code []byte, symbols []elf.Symbol) {
	var (
		index = 0
		pc    = uint64(0)
		buf   strings.Builder
	)
	for pc < uint64(len(code)) {
		addr := base + pc
		// Check for function symbol
		if index < len(symbols) && symbols[index].Value == addr {
			buf.WriteString("TEXT " + symbols[index].Name + "(SB)")
			buf.WriteString(NEWLINE)
			index++
		}
		inst, err := x86asm.Decode(code[pc:], bits)
		if err != nil {
			// Fallback on error (e.g. data in code section)
			buf.WriteString(fmt.Sprintf("  %08x:  %02x                db 0x%02x", addr, code[pc], code[pc]))
			buf.WriteString(NEWLINE)
			pc++
			continue
		}

		// Print address, raw bytes, mnemonic
		buf.WriteString(fmt.Sprintf("  %08x:  ", addr))
		for i := 0; i < inst.Len; i++ {
			buf.WriteString(fmt.Sprintf("%02x ", code[pc+uint64(i)]))
		}
		for i := inst.Len; i < 8; i++ {
			buf.WriteString("   ")
		}
		buf.WriteString(inst.String())
		buf.WriteString(NEWLINE)

		pc += uint64(inst.Len)
	}
	fmt.Print(buf.String())
}

func x86Disassemble(name string) error {
	f, err := elf.Open(name)
	if err != nil {
		return err
	}
	defer f.Close()

	syms, err := f.Symbols()
	if err != nil {
		return err
	}
	fmt.Println("Number of symbols:", len(syms))
	var funcs []elf.Symbol
	for _, s := range syms {
		if elf.ST_TYPE(s.Info) == elf.STT_FUNC {
			funcs = append(funcs, s)
		}
	}
	fmt.Println("Number of funcs:", len(funcs))
	sort.Slice(funcs, func(i, j int) bool {
		return funcs[i].Value < funcs[j].Value
	})

	for i, sec := range f.Sections {
		// fmt.Println("Section", i, sec.Name, "Type:", sec.Type, "Flags:", sec.Flags)
		if sec.Type != elf.SHT_PROGBITS || (sec.Flags&elf.SHF_EXECINSTR) == 0 {
			continue
		}
		data, err := sec.Data()
		if err != nil {
			fmt.Println("sec.Data", sec.Name, ":", err)
			continue
		}
		var syms []elf.Symbol
		for _, s := range funcs {
			if s.Section == elf.SectionIndex(i) {
				syms = append(syms, s)
			}
		}
		fmt.Println("Number of symbols for", sec.Name, ":", len(syms))
		sort.Slice(syms, func(i, j int) bool {
			return syms[i].Value < syms[j].Value
		})
		fmt.Println("Disassembly of section :" + sec.Name)
		dumpSection(sec.Addr, 64, data, syms)
		fmt.Println()
	}
	return nil
}

func main() {
	flag.Parse()

	if *filename == "" {
		fmt.Println("Error: -file argument is required")
		flag.Usage()
		os.Exit(1)
	}

	fmt.Println("Input file:", *filename)
	fmt.Println("Compiler:", *compiler)

	// Parse the C file to extract functions using DWARF
	fmt.Printf("Parsing C file using %s...\n", *compiler)
	functions, err := parser.ListFunctions(*filename, *compiler)
	if err != nil {
		fmt.Println("Error parsing C file:", err)
		os.Exit(1)
	}

	fmt.Println("\nParsed functions:")
	for _, f := range functions {
		fmt.Println(" ", f.String())
	}
	fmt.Println("Total functions found:", len(functions))

	object := *filename + ".o"

	var compileFunc func(string, string) error

	switch *compiler {
	case "clang":
		if !clangFound() {
			fmt.Println("Error: Clang is not available")
			os.Exit(1)
		}
		compileFunc = clangCompile
		fmt.Println("using clang compiler ...")
	case "gcc":
		if !gccFound() {
			fmt.Println("Error: GCC is not available")
			os.Exit(1)
		}
		compileFunc = gccCompile
		fmt.Println("using gcc compiler ...")
	default:
		fmt.Println("Error: Unsupported compiler:", *compiler)
		fmt.Println("Use 'clang' or 'gcc'")
		os.Exit(1)
	}

	if err := compileFunc(*filename, object); err != nil {
		fmt.Println("Error compiling:", err)
		os.Exit(1)
	}

	if !objdumpFound() {
		fmt.Println("Error: objdump is not available")
		os.Exit(1)
	}

	output, err := objdump(object)
	if err != nil || output == "" {
		fmt.Println("Error: objdump failed, ", err)
	} else {
		fmt.Print(output)
	}

	if false {
		if err := x86Disassemble(object); err != nil {
			fmt.Println("Error disassembling:", err)
			os.Exit(1)
		}
	}

	fmt.Println("Compilation successful. Output:", object)
}
