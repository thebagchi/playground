//go:build ignore

package main

import (
	"errors"
	"log"
	"os"
	"os/exec"
	"strings"

	"google.golang.org/protobuf/types/known/structpb"
)

const (
	LLVM_DIR        = ".llvm"
	BUILD_DIR       = ".llvm/build"
	TBLGEN_BIN      = ".llvm/build/bin/llvm-tblgen"
	X86_TD          = ".llvm/llvm/lib/Target/X86/X86.td"
	INCLUDE_DIR     = ".llvm/llvm/include"
	X86_INCLUDE_DIR = ".llvm/llvm/lib/Target/X86"
	FILES_TXT       = "files.txt"
	X86_JSON        = "x86.json"
	OPCODES_TXT     = "opcodes.txt"
)

func join(parts ...string) string {
	return strings.Join(parts, " ")
}

func run(name string, args ...string) {
	log.Println("RUN:", name, strings.Join(args, " "))
	cmd := exec.Command(name, args...)
	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stderr
	if err := cmd.Run(); err != nil {
		log.Fatalln("FAILED:", err)
	}
}

func cloneLLVM() {
	run("git", "clone", "https://github.com/llvm/llvm-project.git", LLVM_DIR)
}

func findTD() {
	run("sh", "-c", join("find", LLVM_DIR, "-name", "*.td", ">", FILES_TXT))
}

func buildLLVM() {
	run("sh", "-c", join("mkdir", "-p", BUILD_DIR))
	run("sh", "-c", join("cd", BUILD_DIR, "&&", "cmake", "../llvm", "-DCMAKE_BUILD_TYPE=Release", "-DLLVM_ENABLE_PROJECTS=llvm"))
	run("sh", "-c", join("cd", BUILD_DIR, "&&", "make", "llvm-tblgen"))
}

func generateX86JSON() {
	run("sh", "-c", join(TBLGEN_BIN, "-dump-json", X86_TD, "-I", INCLUDE_DIR, "-I", X86_INCLUDE_DIR, ">", X86_JSON))
}

func printKind(x *structpb.Value) error {
	switch x.GetKind().(type) {
	case *structpb.Value_NullValue:
		log.Println("Kind: Null")
	case *structpb.Value_NumberValue:
		log.Println("Kind: Number")
	case *structpb.Value_StringValue:
		log.Println("Kind: String")
	case *structpb.Value_BoolValue:
		log.Println("Kind: Bool")
	case *structpb.Value_StructValue:
		log.Println("Kind: Struct")
	case *structpb.Value_ListValue:
		log.Println("Kind: List")
	default:
		return errors.New("unknown value kind")
	}
	return nil
}

func getKey(val *structpb.Value, key string) *structpb.Value {
	if dict := val.GetStructValue(); dict != nil {
		return dict.GetFields()[key]
	}
	return nil
}

func parseJSON() {
	data, err := os.ReadFile(X86_JSON)
	if err != nil {
		log.Fatalln("Failed to read JSON:", err)
	}
	value := new(structpb.Value)
	if err := value.UnmarshalJSON(data); err != nil {
		log.Fatalln("Failed to parse JSON to structpb.Value:", err)
	}
	if err := printKind(value); err != nil {
		log.Fatalln("Failed to print value kind:", err)
	}
	if dict := value.GetStructValue(); nil != dict {
		sb := new(strings.Builder)
		for key, value := range dict.GetFields() {
			if strings.HasPrefix(key, "V") {
				opcode := getKey(value, "Opcode").GetStringValue()
				sb.WriteString(opcode)
				sb.WriteString("\n")
			}
			// process val in next edits
			_ = value
		}
		err := os.WriteFile(OPCODES_TXT, []byte(sb.String()), 0644)
		if err != nil {
			log.Fatalln("Failed to write opcodes.txt:", err)
		}
	}
	log.Println("Parsed JSON to structpb.Value successfully")
}

func main() {
	// Check if .llvm exists
	if _, err := os.Stat(LLVM_DIR); os.IsNotExist(err) {
		cloneLLVM()
	}
	findTD()
	buildLLVM()
	generateX86JSON()
	parseJSON()
}
