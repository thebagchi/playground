package main

import (
	"encoding/json"
	"fmt"
	"io"
	"log"
	"net/http"
	"os"
	"os/exec"
	"path/filepath"
	"sort"
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

const (
	KIND_NULL   = "NULL"
	KIND_NUMBER = "NUMBER"
	KIND_STRING = "STRING"
	KIND_BOOL   = "BOOL"
	KIND_STRUCT = "STRUCT"
	KIND_LIST   = "LIST"
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

func pullLLVM() {
	run("sh", "-c", join("cd", LLVM_DIR, "&&", "git", "pull"))
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

func printKind(x *structpb.Value) string {
	if x == nil {
		return ""
	}
	switch x.GetKind().(type) {
	case *structpb.Value_NullValue:
		return KIND_NULL
	case *structpb.Value_NumberValue:
		return KIND_NUMBER
	case *structpb.Value_StringValue:
		return KIND_STRING
	case *structpb.Value_BoolValue:
		return KIND_BOOL
	case *structpb.Value_StructValue:
		return KIND_STRUCT
	case *structpb.Value_ListValue:
		return KIND_LIST
	default:
		return ""
	}
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

func getKeyString(val *structpb.Value, key string) string {
	if v := getKey(val, key); v != nil {
		if printKind(v) == KIND_STRING {
			return v.GetStringValue()
		} else {
			if data, err := v.MarshalJSON(); err == nil {
				return string(data)
			}
		}
	}
	return ""
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
	kind := printKind(value)
	if kind == "" {
		log.Fatalln("Failed to get value kind: unknown or nil")
	}
	log.Println("Kind:", kind)

	if dict := value.GetStructValue(); nil != dict {
		filtered := make(map[string]*structpb.Value)
		for key, val := range dict.GetFields() {
			kopc := printKind(getKey(val, "Opcode"))
			kasm := printKind(getKey(val, "AsmString"))
			if kopc != KIND_LIST || kasm != KIND_STRING {
				continue
			}
			opcode := getKey(val, "Opcode")
			if opcode == nil {
				continue
			}
			list := opcode.GetListValue()
			if list == nil || len(list.Values) == 0 {
				continue
			}
			asm := getKey(val, "AsmString")
			if asm == nil || len(asm.GetStringValue()) == 0 {
				continue
			}
			filtered[key] = val
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

func formatOpcode(opc string) string {
	var bits []int
	if err := json.Unmarshal([]byte(opc), &bits); err != nil {
		return ""
	}
	// Convert bits to integer (LSB first - LLVM convention)
	// Index 0 = bit position 0 (LSB), Index N = bit position N
	var num int
	for i, bit := range bits {
		num = num + (bit << i)
	}
	// fmt.Println("Len: ", len(bits), num)
	return fmt.Sprintf("%d", num)
}

func processASM(asm string) {
	//fmt.Println(asm)
}

func makeCSV() {
	data, err := os.ReadFile(X86_JSON)
	if err != nil {
		log.Fatalln("Failed to read JSON:", err)
	}
	value := new(structpb.Value)
	if err := value.UnmarshalJSON(data); err != nil {
		log.Fatalln("Failed to parse JSON to structpb.Value:", err)
	}
	dict := value.GetStructValue()
	if dict == nil || dict.Fields == nil {
		log.Fatalln("Parsed JSON is not a dictionary")
	}
	keys := make([]string, 0, len(dict.Fields))
	for k := range dict.Fields {
		keys = append(keys, k)
	}
	sort.Strings(keys)
	for _, key := range keys {
		value := dict.Fields[key]
		opc := formatOpcode(getKeyString(value, "Opcode"))
		asm := getKeyString(value, "AsmString")
		if len(opc) == 0 {
			opc = getKeyString(value, "Opcode")
			log.Fatalln("Failed to process opcode: ", key, "|", opc, "|", asm)
		}
		if len(asm) == 0 {
			var bits []int
			if err := json.Unmarshal([]byte(getKeyString(value, "Opcode")), &bits); err != nil {
				// Do Nothing ...
			}
			log.Println(key, "|", opc, "|", asm, "|", len(bits), "|", getKeyString(value, "Opcode"))
		}
		processASM(asm)
	}
}

func main() {
	if err := downloadCSV(); err != nil {
		log.Fatalln("Failed to download CSVs:", err)
	}
	// Check if .llvm exists
	if _, err := os.Stat(LLVM_DIR); os.IsNotExist(err) {
		cloneLLVM()
	} else {
		pullLLVM()
	}
	findTD()
	buildLLVM()
	generateX86JSON()
	parseJSON()
	makeCSV()
}
