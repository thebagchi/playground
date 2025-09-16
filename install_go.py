#!/usr/bin/env python3

import subprocess
import platform
import sys
import re
import urllib.request
import urllib.error
import json
import os
import tarfile
import shutil
import tempfile
import argparse


def get_latest_version_github():
    """Get latest Go version from GitHub API"""
    try:
        with urllib.request.urlopen(
            "https://api.github.com/repos/golang/go/releases/latest", timeout=10
        ) as response:
            data = json.loads(response.read().decode())
            return data.get("tag_name")
    except (urllib.error.URLError, json.JSONDecodeError) as e:
        print(f"Failed to fetch from GitHub API: {e}")
        return None


def get_latest_version_golang_org():
    """Get latest Go version from golang.org"""
    try:
        with urllib.request.urlopen(
            "https://golang.org/VERSION?m=text", timeout=10
        ) as response:
            return response.read().decode().strip()
    except urllib.error.URLError as e:
        print(f"Failed to fetch from golang.org: {e}")
        return None


def get_current_version():
    """Get currently installed Go version"""
    try:
        result = subprocess.run(
            ["go", "version"], capture_output=True, text=True, timeout=5, check=False
        )
        if result.returncode == 0:
            # Extract version from output like "go version go1.21.0 linux/amd64"
            match = re.search(r"go(\d+\.\d+\.\d+)", result.stdout)
            if match:
                return f"go{match.group(1)}"
        return None
    except (subprocess.TimeoutExpired, FileNotFoundError):
        return None


def get_go_mod_version():
    """Get Go version from go.mod file in current directory"""
    try:
        with open("go.mod", "r", encoding="utf-8") as f:
            content = f.read()
            match = re.search(r"go\s+(\d+\.\d+(?:\.\d+)?)", content)
            if match:
                version = match.group(1)
                # Add .0 if only major.minor is specified
                if len(version.split(".")) == 2:
                    version += ".0"
                return f"go{version}"
        return None
    except FileNotFoundError:
        return None


def get_download_info(version):
    """Generate download information for the latest version"""
    if not version:
        return None

    # Detect architecture
    machine = platform.machine().lower()
    if machine in ["x86_64", "amd64"]:
        go_arch = "amd64"
    elif machine in ["aarch64", "arm64"]:
        go_arch = "arm64"
    elif machine.startswith("armv6"):
        go_arch = "armv6l"
    else:
        go_arch = "amd64"

    # Detect OS
    os_name = platform.system().lower()

    download_url = f"https://golang.org/dl/{version}.{os_name}-{go_arch}.tar.gz"
    return {
        "arch": go_arch,
        "os": os_name,
        "url": download_url,
        "filename": f"{version}.{os_name}-{go_arch}.tar.gz",
    }


def download_file(url, filename, progress_callback=None):
    """Download a file with progress indication"""
    try:
        with urllib.request.urlopen(url) as response:
            total_size = int(response.headers.get("Content-Length", 0))
            downloaded = 0
            with open(filename, "wb") as f:
                while True:
                    chunk = response.read(8192)
                    if not chunk:
                        break
                    f.write(chunk)
                    downloaded += len(chunk)
                    if progress_callback and total_size > 0:
                        progress = (downloaded / total_size) * 100
                        progress_callback(progress, downloaded, total_size)
            return True
    except urllib.error.URLError as e:
        print(f"Download failed: {e}")
        return False


def show_progress(progress, downloaded, total):
    """Show download progress"""
    bar_length = 50
    filled_length = int(bar_length * progress / 100)
    bar = "█" * filled_length + "-" * (bar_length - filled_length)
    downloaded_mb = downloaded / (1024 * 1024)
    total_mb = total / (1024 * 1024)
    print(
        f"\r[{bar}] {progress:.1f}% ({downloaded_mb:.1f}/{total_mb:.1f} MB)",
        end="",
        flush=True,
    )


def backup_existing_go():
    """Backup existing Go installation"""
    go_root = os.environ.get("GOROOT", "/usr/local/go")
    if os.path.exists(go_root):
        backup_path = f"{go_root}.backup"
        if os.path.exists(backup_path):
            shutil.rmtree(backup_path)
        print(f"Backing up existing Go installation to {backup_path}")
        shutil.move(go_root, backup_path)
        return backup_path
    return None


def install_go(download_info, install_path="/usr/local"):
    """Download and install Go"""
    print(f"\n🚀 Installing Go {download_info['url'].split('/')[-1].split('.')[0]}...")

    # Check if running as root for system-wide installation
    if install_path.startswith("/usr") and os.geteuid() != 0:
        print("⚠️  System-wide installation requires root privileges.")
        print("Run with sudo or choose a different installation path.")
        return False

    # Create temporary directory for download
    with tempfile.TemporaryDirectory() as temp_dir:
        temp_file = os.path.join(temp_dir, download_info["filename"])

        print(f"Downloading {download_info['url']}...")
        success = download_file(download_info["url"], temp_file, show_progress)
        print()  # New line after progress bar

        if not success:
            return False

        print("✅ Download completed!")
        # Backup existing installation
        backup_path = backup_existing_go()
        try:
            # Extract the tarball
            print("📦 Extracting Go...")
            with tarfile.open(temp_file, "r:gz") as tar:
                tar.extractall(path=install_path)
            print(f"✅ Go installed to {install_path}/go")
            # Update PATH instructions
            go_bin_path = f"{install_path}/go/bin"
            print(
                "\n📝 Add the following to your shell profile (~/.bashrc, ~/.zshrc, etc.):"
            )
            print(f"export PATH={go_bin_path}:$PATH")
            print(f"export GOROOT={install_path}/go")

            # Check if PATH needs to be updated
            current_path = os.environ.get("PATH", "")
            if go_bin_path not in current_path:
                print(f"\n⚠️  {go_bin_path} is not in your PATH.")
                print("Run the following command to update your current session:")
                print(f"export PATH={go_bin_path}:$PATH")

            return True

        except Exception as e:
            print(f"❌ Installation failed: {e}")

            # Restore backup if installation failed
            if backup_path and os.path.exists(backup_path):
                print("🔄 Restoring previous Go installation...")
                go_root = os.environ.get("GOROOT", "/usr/local/go")
                if os.path.exists(go_root):
                    shutil.rmtree(go_root)
                shutil.move(backup_path, go_root)
                print("✅ Previous installation restored")

            return False


def suggest_go_mod_update(current_version, latest_version):
    """Suggest go.mod update without actually updating it"""
    if not current_version or not latest_version:
        return

    # Remove 'go' prefix for go.mod format
    latest_version_number = latest_version.replace("go", "")
    current_version_number = current_version.replace("go", "")

    print("\n💡 To update your go.mod file, change:")
    print(f"   FROM: go {current_version_number}")
    print(f"   TO:   go {latest_version_number}")
    print("\nOr run this command:")
    print(
        f"   sed -i 's/go {current_version_number}/go {latest_version_number}/' go.mod"
    )


def check_version_info():
    """Check and display version information"""
    print("🔍 Fetching latest Go version...")
    print("=" * 50)

    # Get latest version from different sources
    github_version = get_latest_version_github()
    if github_version:
        print(f"Latest Go version (GitHub): {github_version}")

    golang_org_version = get_latest_version_golang_org()
    if golang_org_version:
        print(f"Latest Go version (golang.org): {golang_org_version}")

    # Use golang.org version as primary source
    latest_version = golang_org_version or github_version

    if not latest_version:
        print("❌ Could not fetch latest Go version")
        return None

    print("\n" + "=" * 50)

    # Check currently installed version
    current_version = get_current_version()
    if current_version:
        print(f"Currently installed Go version: {current_version}")

        if latest_version == current_version:
            print("✅ You have the latest version!")
        else:
            print(f"⚠️  Update available: {current_version} → {latest_version}")
    else:
        print("Go is not currently installed on this system")

    # Check go.mod version
    go_mod_version = get_go_mod_version()
    if go_mod_version:
        print(f"Go version in go.mod: {go_mod_version}")

        if go_mod_version != latest_version:
            print(f"💡 Consider updating go.mod: {go_mod_version} → {latest_version}")

    # Show download information
    print("\n" + "=" * 50)
    print("📥 Download information:")
    print("Official download page: https://golang.org/dl/")

    download_info = get_download_info(latest_version)
    if download_info:
        print(f"Architecture: {download_info['arch']}")
        print(f"Operating System: {download_info['os']}")
        print(f"Direct download URL: {download_info['url']}")

    return {
        "latest_version": latest_version,
        "current_version": current_version,
        "go_mod_version": go_mod_version,
        "download_info": download_info,
    }


def main():
    """
    Main function to handle command line arguments and execute appropriate actions.
    """
    parser = argparse.ArgumentParser(
        description="Check, download, and install the latest Go version",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python install_go.py                          # Check version info only
  python install_go.py --install                # Install with prompts
  python install_go.py --install --yes          # Install without prompts
  python install_go.py --install --path ~/go    # Install to custom path
  python install_go.py --suggest-gomod          # Show go.mod update suggestion
        """,
    )

    parser.add_argument(
        "--install",
        "-i",
        action="store_true",
        help="Download and install the latest Go version",
    )

    parser.add_argument(
        "--path",
        "-p",
        default="/usr/local",
        help="Installation path (default: /usr/local)",
    )

    parser.add_argument(
        "--yes",
        "-y",
        action="store_true",
        help="Skip confirmation prompts (auto-confirm)",
    )

    parser.add_argument(
        "--suggest-gomod",
        "-s",
        action="store_true",
        help="Show suggestion for updating go.mod file",
    )

    args = parser.parse_args()

    # Check version information
    version_info = check_version_info()
    if not version_info:
        sys.exit(1)

    latest_version = version_info["latest_version"]
    current_version = version_info["current_version"]
    go_mod_version = version_info["go_mod_version"]
    download_info = version_info["download_info"]

    # Handle --suggest-gomod flag
    if args.suggest_gomod:
        if go_mod_version and go_mod_version != latest_version:
            print("\n" + "=" * 50)
            suggest_go_mod_update(go_mod_version, latest_version)
        else:
            print("\n⚠️  go.mod is already up to date or not found")
        return

    # Handle --install flag
    if args.install:
        # Check if installation is needed
        if current_version == latest_version and not args.yes:
            print(f"\n✅ Go {latest_version} is already installed.")
            proceed = input("Do you want to reinstall? (y/N): ").strip().lower()
            if proceed not in ["y", "yes"]:
                print("Installation cancelled.")
                return

        # Confirm installation if not using --yes
        if not args.yes:
            print(f"\n🤔 This will install Go {latest_version} to {args.path}")
            confirm = input("Continue? (y/N): ").strip().lower()
            if confirm not in ["y", "yes"]:
                print("Installation cancelled.")
                return

        # Perform installation
        print(f"\n🚀 Starting installation to {args.path}...")
        success = install_go(download_info, args.path)

        if success:
            # Show go.mod update suggestion if present and different
            if go_mod_version and go_mod_version != latest_version:
                suggest_go_mod_update(go_mod_version, latest_version)
        else:
            print("❌ Installation failed")
            sys.exit(1)


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("\n\nOperation cancelled by user")
        sys.exit(1)
    except (ValueError, RuntimeError, OSError) as e:
        print(f"\nError: {e}")
        sys.exit(1)
    pass    
