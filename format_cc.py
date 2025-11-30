#!/usr/bin/env python3
"""Format and lint C/C++ files using clang-format and clang-tidy."""

import json
import os
import subprocess
from pathlib import Path

# Configuration constants
VSCODE_DIR = ".vscode"
SETTINGS_FILE = "settings.json"
GIT_FILE_PATTERN = "git ls-files | grep -E '\\.(h|cc|cpp|c)$'"
DB_PATHS = ["compile_commands.json", "build/compile_commands.json"]

# Log messages
LOG_FINDING_FILES = "🔍 Finding C/C++ files..."
LOG_FOUND_FILES = "📁 Found {count} files"
LOG_FORMATTING = "🎨 Formatting with clang-format..."
LOG_FORMAT_COMPLETE = "✅ Formatting complete"
LOG_CONFIGURING_VSCODE = "🔧 Configuring VS Code..."
LOG_VSCODE_COMPLETE = "✅ VS Code configured"
LOG_CHECKING_CLANG_TIDY = "🔧 Checking clang-tidy..."
LOG_CHECKS_COMPLETE = "✅ Checks complete"
LOG_DONE = "🎉 Done! Run 'git diff' to see changes"
LOG_VSCODE_EXISTS = "✓ VS Code settings already configured"
LOG_NO_COMPILE_DB = "⚠ No compilation database found"
LOG_GENERATE_DB = (
    "  Generate: mkdir build && cd build && cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON .."
)


def run(cmd):
    """Execute a shell command and return the result."""
    return subprocess.run(cmd, shell=True, capture_output=True, text=True)


def find_cpp_files():
    """Discover all tracked C/C++ files."""
    result = run(GIT_FILE_PATTERN)
    return result.stdout.strip().split("\n") if result.returncode == 0 else []


def format_files(files):
    """Apply clang-format to all C/C++ files."""
    for file in files:
        run(f"clang-format -i '{file}'")


def configure_vscode():
    """Set up VS Code with clang-format and clang-tidy integration."""
    os.makedirs(VSCODE_DIR, exist_ok=True)
    settings_file = os.path.join(VSCODE_DIR, SETTINGS_FILE)

    if os.path.exists(settings_file):
        print(LOG_VSCODE_EXISTS)
        return

    settings = {
        "C_Cpp.clang_format_style": "file:${workspaceFolder}/.clang-format",
        "C_Cpp.clang_format_fallbackStyle": "Google",
        "C_Cpp.codeAnalysis.clangTidy.config": "${workspaceFolder}/.clang-tidy",
        "C_Cpp.formatting": "clangFormat",
        "editor.formatOnSave": True,
        "files.associations": {
            "*.h": "cpp",
            "*.cc": "cpp",
            "*.cpp": "cpp",
            "*.c": "c",
        },
    }

    with open(settings_file, "w") as f:
        json.dump(settings, f, indent=2)


def check_clang_tidy():
    """Verify clang-tidy compilation database availability."""
    if not any(os.path.exists(p) for p in DB_PATHS):
        print(LOG_NO_COMPILE_DB)
        print(LOG_GENERATE_DB)


def main():
    """Execute the formatting workflow."""
    print(LOG_FINDING_FILES)
    files = find_cpp_files()
    print(LOG_FOUND_FILES.format(count=len(files)))

    print(f"\n{LOG_FORMATTING}")
    format_files(files)
    print(f"{LOG_FORMAT_COMPLETE}\n")

    print(LOG_CONFIGURING_VSCODE)
    configure_vscode()
    print(f"{LOG_VSCODE_COMPLETE}\n")

    print(LOG_CHECKING_CLANG_TIDY)
    check_clang_tidy()
    print(f"{LOG_CHECKS_COMPLETE}\n")

    print(LOG_DONE)


if __name__ == "__main__":
    main()
