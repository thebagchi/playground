package main

import (
	"archive/tar"
	"bufio"
	"compress/gzip"
	"flag"
	"fmt"
	"io"
	"net/http"
	"os"
	"os/exec"
	"path/filepath"
	"regexp"
	"runtime"
	"strings"
	"time"

	"github.com/google/uuid"
	"google.golang.org/protobuf/types/known/structpb"
)

const (
	VERSION_URL                  = "https://go.dev/VERSION?m=text"
	DOWNLOAD_BASE_URL            = "https://golang.org/dl/"
	DEFAULT_GO_ROOT              = "/usr/local/go"
	BACKUP_SUFFIX                = ".backup"
	TAR_GZ_EXT                   = ".tar.gz"
	GO_PREFIX                    = "go"
	ARCH_AMD64                   = "amd64"
	ARCH_ARM64                   = "arm64"
	ARCH_ARMv6L                  = "armv6l"
	VERSION_REGEX_PATTERN        = `go(\d+\.\d+\.\d+)`
	GO_MOD_VERSION_REGEX_PATTERN = `go\s+(\d+\.\d+(?:\.\d+)?)`
	FIELD_ARCH                   = "arch"
	FIELD_OS                     = "os"
	FIELD_URL                    = "url"
	FIELD_FILENAME               = "filename"
	FIELD_LATEST_VERSION         = "latest_version"
	FIELD_CURRENT_VERSION        = "current_version"
	FIELD_GO_MOD_VERSION         = "go_mod_version"
	FIELD_DOWNLOAD_INFO          = "download_info"
)

var (
	install  = flag.Bool("i", false, "Download and install the latest Go version")
	path     = flag.String("p", "/usr/local", "Installation path (default: /usr/local)")
	yes      = flag.Bool("y", false, "Skip confirmation prompts (auto-confirm)")
	download = flag.Bool("d", false, "Download the Go archive without installing")
	output   = flag.String("o", "", "Output path for downloaded file (default: current directory)")
	suggest  = flag.Bool("s", false, "Show suggestion for updating go.mod file")
)

func GenerateUUID() string {
	return uuid.New().String()
}

func GetStringField(value *structpb.Value, field string) string {
	return value.GetStructValue().Fields[field].GetStringValue()
}

func GetStructField(value *structpb.Value, field string) *structpb.Value {
	return value.GetStructValue().Fields[field]
}

func GetLatestVersionGolangOrg() (string, error) {
	client := &http.Client{Timeout: 10 * time.Second}
	resp, err := client.Get(VERSION_URL)
	if err != nil {
		return "", fmt.Errorf("failed to fetch from golang.org: %v", err)
	}
	defer resp.Body.Close()

	scanner := bufio.NewScanner(resp.Body)
	if scanner.Scan() {
		return strings.TrimSpace(scanner.Text()), nil
	}
	return "", fmt.Errorf("no version found in response")
}

func GetCurrentVersion() (string, error) {
	cmd := exec.Command("go", "version")
	output, err := cmd.Output()
	if err != nil {
		return "", err
	}

	re := regexp.MustCompile(VERSION_REGEX_PATTERN)
	matches := re.FindStringSubmatch(string(output))
	if len(matches) > 1 {
		return GO_PREFIX + matches[1], nil
	}
	return "", fmt.Errorf("could not parse go version")
}

func GetGoModVersion() (string, error) {
	file, err := os.Open("go.mod")
	if err != nil {
		return "", err
	}
	defer file.Close()

	scanner := bufio.NewScanner(file)
	re := regexp.MustCompile(GO_MOD_VERSION_REGEX_PATTERN)
	for scanner.Scan() {
		line := scanner.Text()
		matches := re.FindStringSubmatch(line)
		if len(matches) > 1 {
			version := matches[1]
			parts := strings.Split(version, ".")
			if len(parts) == 2 {
				version += ".0"
			}
			return GO_PREFIX + version, nil
		}
	}
	return "", fmt.Errorf("no go version found in go.mod")
}

func GetDownloadInfo(version string) *structpb.Value {
	if version == "" {
		return nil
	}

	// Detect architecture
	var goArch string
	switch runtime.GOARCH {
	case "amd64", "x86_64":
		goArch = ARCH_AMD64
	case "arm64", "aarch64":
		goArch = ARCH_ARM64
	default:
		if strings.HasPrefix(runtime.GOARCH, "armv6") {
			goArch = ARCH_ARMv6L
		} else {
			goArch = ARCH_AMD64
		}
	}

	// Detect OS
	var (
		osName      = runtime.GOOS
		downloadURL = fmt.Sprintf("%s%s.%s-%s%s", DOWNLOAD_BASE_URL, version, osName, goArch, TAR_GZ_EXT)
		filename    = fmt.Sprintf("%s.%s-%s%s", version, osName, goArch, TAR_GZ_EXT)
	)

	s := &structpb.Struct{
		Fields: map[string]*structpb.Value{
			FIELD_ARCH:     structpb.NewStringValue(goArch),
			FIELD_OS:       structpb.NewStringValue(osName),
			FIELD_URL:      structpb.NewStringValue(downloadURL),
			FIELD_FILENAME: structpb.NewStringValue(filename),
		},
	}

	return structpb.NewStructValue(s)
}

func DownloadFile(url, filename string, progressCallback func(int, int64, int64)) error {
	resp, err := http.Get(url)
	if err != nil {
		return fmt.Errorf("download failed: %v", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return fmt.Errorf("download failed with status: %s", resp.Status)
	}

	totalSize := resp.ContentLength
	file, err := os.Create(filename)
	if err != nil {
		return err
	}
	defer file.Close()

	var (
		buffer     = make([]byte, 8192)
		downloaded int64
	)

	for {
		n, err := resp.Body.Read(buffer)
		if n > 0 {
			_, writeErr := file.Write(buffer[:n])
			if writeErr != nil {
				return writeErr
			}
			downloaded += int64(n)
			if progressCallback != nil && totalSize > 0 {
				progress := int((downloaded * 100) / totalSize)
				progressCallback(progress, downloaded, totalSize)
			}
		}
		if err == io.EOF {
			break
		}
		if err != nil {
			return err
		}
	}
	return nil
}

func ShowProgress(progress int, downloaded, total int64) {
	var (
		barLength    = 50
		filledLength = int((barLength * progress) / 100)
		bar          = strings.Repeat("█", filledLength) + strings.Repeat("-", barLength-filledLength)
		downloadedMB = float64(downloaded) / (1024 * 1024)
		totalMB      = float64(total) / (1024 * 1024)
	)

	fmt.Printf("\r[%s] %.1f%% (%.1f/%.1f MB)", bar, float64(progress), downloadedMB, totalMB)
}

func DownloadOnly(downloadInfo *structpb.Value, outputPath string) (string, error) {
	version := strings.Split(strings.Split(GetStringField(downloadInfo, FIELD_URL), "/")[len(strings.Split(GetStringField(downloadInfo, FIELD_URL), "/"))-1], ".")[0]
	fmt.Println("\n📥 Downloading Go", version+"...")

	var filename string
	if outputPath != "" {
		expandedPath := os.ExpandEnv(outputPath)
		if strings.HasPrefix(expandedPath, "~") {
			home, _ := os.UserHomeDir()
			expandedPath = strings.Replace(expandedPath, "~", home, 1)
		}

		if info, err := os.Stat(expandedPath); err == nil && info.IsDir() {
			filename = filepath.Join(expandedPath, GetStringField(downloadInfo, FIELD_FILENAME))
		} else {
			filename = expandedPath
		}
	} else {
		filename = GetStringField(downloadInfo, FIELD_FILENAME)
	}

	fmt.Println("Downloading", GetStringField(downloadInfo, FIELD_URL), "to", filename+"...")
	err := DownloadFile(GetStringField(downloadInfo, FIELD_URL), filename, func(p int, d, t int64) {
		ShowProgress(p, d, t)
	})
	fmt.Println() // New line after progress bar

	if err != nil {
		fmt.Println("❌ Download failed")
		// Clean up partial download file
		if _, statErr := os.Stat(filename); statErr == nil {
			os.Remove(filename)
		}
		return "", err
	}

	fmt.Println("✅ Download completed:", filename)
	return filename, nil
}

func BackupExistingGo() string {
	goRoot := os.Getenv("GOROOT")
	if goRoot == "" {
		goRoot = DEFAULT_GO_ROOT
	}

	if _, err := os.Stat(goRoot); err == nil {
		backupPath := goRoot + "-" + time.Now().Format("2006-01-02") + BACKUP_SUFFIX
		if _, err := os.Stat(backupPath); err == nil {
			os.RemoveAll(backupPath)
		}
		fmt.Println("Backing up existing Go installation to", backupPath)
		err := os.Rename(goRoot, backupPath)
		if err != nil {
			fmt.Println("Warning: Could not backup existing installation:", err)
			return ""
		}
		return backupPath
	}
	return ""
}

func InstallGo(downloadInfo *structpb.Value, installPath string) error {
	version := strings.Split(strings.Split(GetStringField(downloadInfo, FIELD_URL), "/")[len(strings.Split(GetStringField(downloadInfo, FIELD_URL), "/"))-1], ".")[0]
	fmt.Println("\n🚀 Installing Go", version+"...")

	// Check if running as root for system-wide installation
	if strings.HasPrefix(installPath, "/usr") {
		if os.Geteuid() != 0 {
			ShowPrivilegeError()
			return fmt.Errorf("insufficient privileges")
		}
	}

	// Create temporary directory for download
	var (
		uuid    = GenerateUUID()
		tempDir = filepath.Join("/tmp", uuid)
	)
	err := os.MkdirAll(tempDir, 0755)
	if err != nil {
		return err
	}
	defer os.RemoveAll(tempDir)

	tempFile := filepath.Join(tempDir, GetStringField(downloadInfo, FIELD_FILENAME))

	fmt.Println("Downloading", GetStringField(downloadInfo, FIELD_URL)+"...")
	err = DownloadFile(GetStringField(downloadInfo, FIELD_URL), tempFile, func(p int, d, t int64) {
		ShowProgress(p, d, t)
	})
	fmt.Println() // New line after progress bar

	if err != nil {
		return err
	}

	fmt.Println("✅ Download completed!")

	// Backup existing installation
	backupPath := BackupExistingGo()

	// Extract the tarball
	fmt.Println("📦 Extracting Go...")
	err = ExtractTarGz(tempFile, installPath)
	if err != nil {
		fmt.Println("❌ Installation failed:", err)

		// Restore backup if installation failed
		if backupPath != "" {
			if _, err := os.Stat(backupPath); err == nil {
				fmt.Println("🔄 Restoring previous Go installation...")
				goRoot := os.Getenv("GOROOT")
				if goRoot == "" {
					goRoot = DEFAULT_GO_ROOT
				}
				if _, err := os.Stat(goRoot); err == nil {
					os.RemoveAll(goRoot)
				}
				os.Rename(backupPath, goRoot)
				fmt.Println("✅ Previous installation restored")
			}
		}
		return err
	}

	fmt.Println("✅ Go installed to", filepath.Join(installPath, "go"))

	ShowHelp(installPath)

	return nil
}

func ExtractTarGz(tarPath, destPath string) error {
	file, err := os.Open(tarPath)
	if err != nil {
		return err
	}
	defer file.Close()

	gzr, err := gzip.NewReader(file)
	if err != nil {
		return err
	}
	defer gzr.Close()

	tr := tar.NewReader(gzr)

	for {
		header, err := tr.Next()
		if err == io.EOF {
			break
		}
		if err != nil {
			return err
		}

		target := filepath.Join(destPath, header.Name)

		switch header.Typeflag {
		case tar.TypeDir:
			if err := os.MkdirAll(target, os.FileMode(header.Mode)); err != nil {
				return err
			}
		case tar.TypeReg:
			dir := filepath.Dir(target)
			if err := os.MkdirAll(dir, 0755); err != nil {
				return err
			}

			file, err := os.OpenFile(target, os.O_CREATE|os.O_RDWR, os.FileMode(header.Mode))
			if err != nil {
				return err
			}

			if _, err := io.Copy(file, tr); err != nil {
				file.Close()
				return err
			}
			file.Close()
		}
	}
	return nil
}

func ShowPrivilegeError() {
	fmt.Println("⚠️  System-wide installation requires root privileges.")
	fmt.Println("Run with sudo or choose a different installation path.")
}

func ShowHelp(installPath string) {
	binPath := filepath.Join(installPath, "go", "bin")
	fmt.Println("\n📝 Add the following to your shell profile (~/.bashrc, ~/.zshrc, etc.):")
	fmt.Println("export PATH=" + binPath + ":$PATH")
	fmt.Println("export GOROOT=" + installPath + "/go")

	// Check if PATH needs to be updated
	currentPath := os.Getenv("PATH")
	if !strings.Contains(currentPath, binPath) {
		fmt.Println("\n⚠️ ", binPath, "is not in your PATH.")
		fmt.Println("Run the following command to update your current session:")
		fmt.Println("export PATH=" + binPath + ":$PATH")
	}
}

func SuggestGoModUpdate(currentVersion, latestVersion string) {
	if currentVersion == "" || latestVersion == "" {
		return
	}

	var (
		latestNum  = strings.TrimPrefix(latestVersion, "go")
		currentNum = strings.TrimPrefix(currentVersion, "go")
	)

	fmt.Println("\n💡 To update your go.mod file, change:")
	fmt.Println("   FROM: go", currentNum)
	fmt.Println("   TO:   go", latestNum)
	fmt.Println("\nOr run this command:")
	cmd := fmt.Sprintf("   sed -i 's/go %s/go %s/' go.mod", currentNum, latestNum)
	fmt.Println(cmd)
}

func CheckVersionInfo() (*structpb.Value, error) {
	fmt.Println("🔍 Fetching latest Go version...")
	fmt.Println(strings.Repeat("=", 50))

	goVersion, err := GetLatestVersionGolangOrg()
	if err != nil {
		fmt.Println("❌ Could not fetch latest Go version:", err)
		return nil, err
	}

	fmt.Println("Latest Go version:", goVersion)

	fmt.Println("\n" + strings.Repeat("=", 50))

	// Check currently installed version
	currentVersion, err := GetCurrentVersion()
	if err != nil {
		fmt.Println("Go is not currently installed on this system")
	} else {
		fmt.Println("Currently installed Go version:", currentVersion)
		if goVersion == currentVersion {
			fmt.Println("✅ You have the latest version!")
		} else {
			fmt.Println("⚠️  Update available:", currentVersion, "→", goVersion)
		}
	}

	// Check go.mod version
	gomodVersion, err := GetGoModVersion()
	if err == nil {
		fmt.Println("Go version in go.mod:", gomodVersion)
		if gomodVersion != goVersion {
			fmt.Println("💡 Consider updating go.mod:", gomodVersion, "→", goVersion)
		}
	}

	// Show download information
	fmt.Println("\n" + strings.Repeat("=", 50))
	fmt.Println("📥 Download information:")
	fmt.Println("Official download page: " + DOWNLOAD_BASE_URL)

	downloadInfo := GetDownloadInfo(goVersion)
	if downloadInfo != nil {
		fmt.Println("Architecture:", GetStringField(downloadInfo, FIELD_ARCH))
		fmt.Println("Operating System:", GetStringField(downloadInfo, FIELD_OS))
		fmt.Println("Direct download URL:", GetStringField(downloadInfo, FIELD_URL))
	}

	var (
		fields = map[string]*structpb.Value{
			FIELD_LATEST_VERSION:  structpb.NewStringValue(goVersion),
			FIELD_CURRENT_VERSION: structpb.NewStringValue(currentVersion),
			FIELD_GO_MOD_VERSION:  structpb.NewStringValue(gomodVersion),
		}
		s = &structpb.Struct{Fields: fields}
	)

	if downloadInfo != nil {
		fields[FIELD_DOWNLOAD_INFO] = downloadInfo
	}

	return structpb.NewStructValue(s), nil
}

func main() {
	flag.Parse()

	// Expand user path (~) to absolute path
	if strings.HasPrefix(*path, "~") {
		home, _ := os.UserHomeDir()
		*path = strings.Replace(*path, "~", home, 1)
	}

	// Check version information
	versionInfo, err := CheckVersionInfo()
	if err != nil {
		os.Exit(1)
	}

	var (
		latestVersion  = GetStringField(versionInfo, FIELD_LATEST_VERSION)
		currentVersion = GetStringField(versionInfo, FIELD_CURRENT_VERSION)
		gomodVersion   = GetStringField(versionInfo, FIELD_GO_MOD_VERSION)
		downloadInfo   = GetStructField(versionInfo, FIELD_DOWNLOAD_INFO)
	)

	// Handle --suggest-gomod flag
	if *suggest {
		if gomodVersion != "" && gomodVersion != latestVersion {
			fmt.Println("\n" + strings.Repeat("=", 50))
			SuggestGoModUpdate(gomodVersion, latestVersion)
		} else {
			fmt.Println("\n⚠️  go.mod is already up to date or not found")
		}
		return
	}

	// Handle --download-only flag
	if *download {
		if downloadInfo == nil {
			fmt.Println("❌ Cannot determine download information")
			os.Exit(1)
		}

		result, err := DownloadOnly(downloadInfo, *output)
		if err != nil {
			os.Exit(1)
		}
		fmt.Println("\n📦 Go archive downloaded to:", result)
		return
	}

	// Handle --install flag
	if *install {
		// Check if installation is needed
		if currentVersion == latestVersion && !*yes {
			fmt.Println("\n✅ Go", latestVersion, "is already installed.")
			fmt.Print("Do you want to reinstall? (y/N): ")
			var response string
			fmt.Scanln(&response)
			response = strings.ToLower(strings.TrimSpace(response))
			if response != "y" && response != "yes" {
				fmt.Println("Installation cancelled.")
				return
			}
		}

		// Confirm installation if not using --yes
		if !*yes {
			fmt.Println("\n🤔 This will install Go", latestVersion, "to", *path)
			fmt.Print("Continue? (y/N): ")
			var confirm string
			fmt.Scanln(&confirm)
			confirm = strings.ToLower(strings.TrimSpace(confirm))
			if confirm != "y" && confirm != "yes" {
				fmt.Println("Installation cancelled.")
				return
			}
		}

		// Perform installation
		fmt.Println("\n🚀 Starting installation to", *path+"...")
		err := InstallGo(downloadInfo, *path)
		if err != nil {
			fmt.Println("❌ Installation failed")
			os.Exit(1)
		}

		// Show go.mod update suggestion if present and different
		if gomodVersion != "" && gomodVersion != latestVersion {
			SuggestGoModUpdate(gomodVersion, latestVersion)
		}
	}
}
