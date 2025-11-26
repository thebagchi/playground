//go:build ignore

package main

import (
	"errors"
	"fmt"
	"io"
	"log"
	"net/http"
	"os"
	"os/exec"
	"path/filepath"
	"strings"

	"google.golang.org/protobuf/encoding/protojson"
	"google.golang.org/protobuf/types/known/structpb"
)

const (
	LLVM_DIR         = ".llvm"
	BUILD_DIR        = ".llvm/build"
	TBLGEN_BIN       = ".llvm/build/bin/llvm-tblgen"
	X86_TD           = ".llvm/llvm/lib/Target/X86/X86.td"
	INCLUDE_DIR      = ".llvm/llvm/include"
	X86_INCLUDE_DIR  = ".llvm/llvm/lib/Target/X86"
	FILES_TXT        = "files.txt"
	X86_JSON         = "x86.json"
	OPCODES_JSON     = "opcodes.json"
	GO_DIR           = ".go"
	X86_CSV_URL      = "https://raw.githubusercontent.com/golang/arch/refs/heads/master/x86/x86.csv"
	X86_V02_CSV_URL  = "https://raw.githubusercontent.com/golang/arch/refs/heads/master/x86/x86.v0.2.csv"
	X86_CSV_FILE     = "x86.csv"
	X86_V02_CSV_FILE = "x86.v0.2.csv"
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
		if dict.Fields != nil {
			if v, ok := dict.Fields[key]; ok {
				return v
			}
		}
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
		filtered := make(map[string]*structpb.Value)
		for key, val := range dict.GetFields() {
			if opcode := getKey(val, "Opcode"); opcode != nil {
				filtered[key] = val
			} else {
				// fmt.Println("Key: ", key)
			}
		}
		opcodes := structpb.NewStructValue(&structpb.Struct{
			Fields: filtered,
		})
		content := protojson.Format(opcodes)
		if err := os.WriteFile(X86_JSON, []byte(content), 0644); err != nil {
			log.Fatalln("Failed to write opcodes.txt:", err)
		}
		sizeMB := float64(len(content)) / (1024 * 1024)
		log.Println("Success: Extracted", len(filtered), fmt.Sprintf("(%.1f MB)", sizeMB))
	}
	log.Println("Parsed JSON to structpb.Value successfully")
}

func httpDownload(url, filename string) error {
	resp, err := http.Get(url)
	if err != nil {
		return err
	}
	defer resp.Body.Close()
	if resp.StatusCode != http.StatusOK {
		return fmt.Errorf("bad status: %s", resp.Status)
	}
	out, err := os.Create(filename)
	if err != nil {
		return err
	}
	defer out.Close()
	_, err = io.Copy(out, resp.Body)
	return err
}

func downloadCSV() error {
	dir := GO_DIR
	if err := os.MkdirAll(dir, 0755); err != nil {
		return err
	}
	urls := []string{
		X86_CSV_URL,
		X86_V02_CSV_URL,
	}
	filenames := []string{
		filepath.Join(dir, X86_CSV_FILE),
		filepath.Join(dir, X86_V02_CSV_FILE),
	}
	for i, url := range urls {
		if err := httpDownload(url, filenames[i]); err != nil {
			return fmt.Errorf("failed to download %s: %v", url, err)
		}
	}
	return nil
}

func main() {
	if err := downloadCSV(); err != nil {
		log.Fatalln("Failed to download CSVs:", err)
	}
	// Check if .llvm exists
	if _, err := os.Stat(LLVM_DIR); os.IsNotExist(err) {
		cloneLLVM()
	}
	findTD()
	buildLLVM()
	generateX86JSON()
	parseJSON()
}
