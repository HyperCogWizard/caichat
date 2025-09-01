#!/bin/bash
# analyze-packaging.sh - Analyze current packaging state of the repository

echo "🔍 AIChat Packaging Analysis"
echo "============================"

# Check current directory structure
echo "📁 Repository Structure:"
echo "- Source: $(find src -name "*.rs" | wc -l) Rust files"
echo "- C++ Integration: $(test -f CMakeLists.txt && echo "✅ Present" || echo "❌ Missing")"
echo "- Web Assets: $(test -d assets && echo "✅ Present" || echo "❌ Missing")"
echo "- Scripts: $(test -d scripts && echo "✅ Present" || echo "❌ Missing")"
echo ""

# Analyze Cargo.toml
echo "📦 Cargo Package Info:"
if [[ -f Cargo.toml ]]; then
    echo "- Name: $(grep '^name' Cargo.toml | head -1 | cut -d'"' -f2)"
    echo "- Version: $(grep '^version' Cargo.toml | head -1 | cut -d'"' -f2)"
    echo "- License: $(grep '^license' Cargo.toml | head -1 | cut -d'"' -f2)"
    deps_count=$(awk '/^\[dependencies\]/{flag=1;next}/^\[/{flag=0}flag && /^[a-z]/{count++}END{print count+0}' Cargo.toml)
    echo "- Dependencies: $deps_count"
else
    echo "❌ No Cargo.toml found"
fi
echo ""

# Check build systems
echo "🔧 Build Systems:"
echo "- Cargo: $(test -f Cargo.toml && echo "✅" || echo "❌")"
echo "- CMake: $(test -f CMakeLists.txt && echo "✅" || echo "❌")"
echo "- GitHub Actions: $(test -d .github/workflows && echo "✅" || echo "❌")"
echo ""

# Check existing package managers
echo "📋 Current Distribution Channels:"
current_channels=0

# Check for package manager files
if [[ -f README.md ]] && grep -q "cargo install" README.md 2>/dev/null; then
    echo "- ✅ Cargo/crates.io"
    current_channels=$((current_channels + 1))
fi

if [[ -f README.md ]] && grep -q "brew install" README.md 2>/dev/null; then
    echo "- ✅ Homebrew"
    current_channels=$((current_channels + 1))
fi

if [[ -f README.md ]] && grep -q "pacman -S" README.md 2>/dev/null; then
    echo "- ✅ Pacman (Arch)"
    current_channels=$((current_channels + 1))
fi

if [[ -f README.md ]] && grep -q "scoop install" README.md 2>/dev/null; then
    echo "- ✅ Scoop (Windows)"
    current_channels=$((current_channels + 1))
fi

if [[ -f .github/workflows/release.yaml ]]; then
    echo "- ✅ GitHub Releases"
    current_channels=$((current_channels + 1))
fi

echo "- Total active channels: $current_channels"
echo ""

# Analyze potential new packages
echo "🎯 Packaging Opportunities:"
echo ""

echo "NPM Package Analysis:"
echo "- Target: Node.js developers, web integration"
echo "- Approach: Binary wrapper + optional WASM"
echo "- Effort: Medium (need wrapper scripts)"
echo "- Impact: High (large Node.js ecosystem)"
echo ""

echo "PyPI Package Analysis:"
echo "- Target: Python developers, data science"
echo "- Approach: PyO3 bindings or binary wrapper"
echo "- Effort: Medium (need Python bindings)"
echo "- Impact: High (large Python ecosystem)"
echo ""

echo "Guix Package Analysis:"
echo "- Target: GNU/Linux users, reproducible builds"
echo "- Approach: Scheme package definition"
echo "- Effort: Low (just package definition)"
echo "- Impact: Medium (growing but niche)"
echo ""

echo "ESM/WASM Analysis:"
echo "- Target: Web developers, Deno users"
echo "- Approach: Compile to WebAssembly"
echo "- Effort: High (need web-compatible features)"
echo "- Impact: Medium (modern web ecosystem)"
echo ""

# Check for missing standard files
echo "📝 Packaging Prerequisites:"
missing_files=()

[[ ! -f LICENSE ]] && [[ ! -f LICENSE-MIT ]] && [[ ! -f LICENSE-APACHE ]] && missing_files+=("LICENSE file")
[[ ! -f README.md ]] && missing_files+=("README.md")

if [[ ${#missing_files[@]} -eq 0 ]]; then
    echo "- ✅ All standard files present"
else
    echo "- ❌ Missing: ${missing_files[*]}"
fi

# Check release workflow
if [[ -f .github/workflows/release.yaml ]]; then
    echo "- ✅ Release automation configured"
    platforms=$(grep -c "target:" .github/workflows/release.yaml 2>/dev/null || echo 0)
    echo "- ✅ Building for $platforms platforms"
else
    echo "- ❌ No release automation found"
fi

echo ""
echo "🚀 Recommended Priority:"
echo "1. 🟢 NPM binary wrapper (high impact, medium effort)"
echo "2. 🟢 PyPI Python bindings (high impact, medium effort)" 
echo "3. 🟡 Guix package definition (medium impact, low effort)"
echo "4. 🟡 ESM/WASM module (medium impact, high effort)"
echo "5. 🟡 Additional Linux distros (Nix, Flatpak, Snap)"
echo ""

echo "✅ Analysis complete! Use this data to plan packaging strategy."