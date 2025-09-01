---
name: Packager Analysis & Strategy Template
about: Analyze current packaging state and generate optimal strategies for multiple platforms
title: 'Packaging Strategy Analysis for [PLATFORM]'
labels: packaging, enhancement, infrastructure
assignees: ''

---

<!-- This template analyzes the current repository state and provides optimal packaging strategies -->

## Repository Analysis

### Current Packaging State ✅

**Build Systems Detected:**
- ✅ **Cargo/Rust**: Primary build system (`Cargo.toml` v0.29.0)
- ✅ **CMake/C++**: OpenCog integration (`CMakeLists.txt`)
- ✅ **GitHub Actions**: CI/CD with multi-platform releases
- ✅ **Multi-arch Support**: 8+ target architectures in release workflow

**Existing Distribution Channels:**
- ✅ **Cargo/crates.io**: `cargo install aichat`
- ✅ **Homebrew**: `brew install aichat`
- ✅ **Pacman (Arch)**: `pacman -S aichat`
- ✅ **Scoop (Windows)**: `scoop install aichat`
- ✅ **Termux (Android)**: `pkg install aichat`
- ✅ **GitHub Releases**: Pre-built binaries for macOS, Linux, Windows

**Project Characteristics:**
- **Type**: CLI tool with optional web interface
- **Language**: Rust (primary) + C++ (OpenCog integration)
- **Dependencies**: 70+ Rust crates, OpenCog libraries (optional)
- **Assets**: Web playground, shell completions, config examples
- **License**: MIT OR Apache-2.0 (dual license)

---

## Quick Analysis Tool

To automatically analyze your repository's current packaging state, run:

```bash
# Download and run the packaging analysis script
curl -sSL https://raw.githubusercontent.com/HyperCogWizard/caichat/main/scripts/analyze-packaging.sh | bash

# Or if you have the repository locally:
./scripts/analyze-packaging.sh
```

This script will provide:
- Current build system analysis
- Existing distribution channels
- Dependency information  
- Platform-specific recommendations
- Priority matrix for new packages

---

## Platform-Specific Packaging Strategies

### 🐙 GitHub (Current: ✅ Optimal)

**Current Implementation:**
- Multi-platform automated releases (8 targets)
- Binary artifacts with GitHub Releases
- CI/CD pipeline with cross-compilation

**Optimization Recommendations:**
- [x] ✅ Already optimal
- [ ] **Enhancement**: Add checksums and GPG signatures
- [ ] **Enhancement**: Container images (Docker/Podman)
- [ ] **Enhancement**: GitHub Packages for Rust crates

**Action Items:**
```yaml
# Add to .github/workflows/release.yaml
- name: Generate checksums
  run: |
    cd target/release
    sha256sum aichat > aichat.sha256
    gpg --detach-sign --armor aichat
```

### 📦 NPM (Current: ❌ Not Available)

**Strategy**: Binary wrapper package for Node.js ecosystems

**Implementation Approach:**
1. **Binary Wrapper**: Package pre-built binaries with Node.js wrapper
2. **Native Module**: Create N-API bindings for core functionality
3. **Web Assembly**: Compile core to WASM for browser/Node.js

**Priority Implementation**: Binary Wrapper
```json
{
  "name": "@aichat/cli",
  "version": "0.29.0",
  "bin": {
    "aichat": "./bin/aichat"
  },
  "scripts": {
    "postinstall": "node scripts/download-binary.js"
  },
  "os": ["darwin", "linux", "win32"],
  "cpu": ["x64", "arm64"]
}
```

**Action Items:**
- [ ] Create `npm/` directory structure
- [ ] Implement platform detection and binary download
- [ ] Add npm publishing to release workflow
- [ ] Consider @types/aichat for TypeScript users

### 🐍 PyPI (Current: ❌ Not Available)

**Strategy**: Python bindings with PyO3 or binary wrapper

**Implementation Options:**

1. **PyO3 Bindings** (Recommended):
```rust
// src/python.rs
use pyo3::prelude::*;

#[pyfunction]
fn aichat_complete(prompt: String, model: Option<String>) -> PyResult<String> {
    // Rust implementation
}

#[pymodule]
fn aichat(_py: Python, m: &PyModule) -> PyResult<()> {
    m.add_function(wrap_pyfunction!(aichat_complete, m)?)?;
    Ok(())
}
```

2. **Binary Wrapper**:
```python
# pyproject.toml
[project]
name = "aichat-cli"
version = "0.29.0"
scripts = {aichat = "aichat_cli.main:main"}
```

**Action Items:**
- [ ] Add PyO3 to Cargo.toml
- [ ] Create Python module structure
- [ ] Implement core bindings
- [ ] Add maturin for Python packaging
- [ ] Setup PyPI publishing in CI

### 🐧 Guix (Current: ❌ Not Available)

**Strategy**: GNU Guix package definition

**Implementation**:
```scheme
;; gnu/packages/aichat.scm
(define-public aichat
  (package
    (name "aichat")
    (version "0.29.0")
    (source (origin
              (method git-fetch)
              (uri (git-reference
                    (url "https://github.com/sigoden/aichat")
                    (commit (string-append "v" version))))
              (file-name (git-file-name name version))
              (sha256
               (base32 "..."))))
    (build-system cargo-build-system)
    (arguments
     `(#:cargo-inputs
       (("rust-anyhow" ,rust-anyhow-1)
        ("rust-clap" ,rust-clap-4)
        ;; ... other dependencies
        )))
    (native-inputs
     (list pkg-config))
    (inputs
     (list openssl curl))
    (synopsis "All-in-one LLM CLI tool")
    (description "AIChat is a feature-rich CLI tool for interacting with 
multiple LLM providers with support for shell assistance, REPL mode, 
RAG, and more.")
    (home-page "https://github.com/sigoden/aichat")
    (license (list license:expat license:asl2.0))))
```

**Action Items:**
- [ ] Create Guix package definition
- [ ] Submit to gnu/packages repository
- [ ] Add OpenCog optional dependencies
- [ ] Test on Guix System

### 🌐 ESM (ECMAScript Modules) (Current: ❌ Not Available)

**Strategy**: WebAssembly compilation for web/Deno usage

**Implementation Approach:**
1. **Core WASM Module**: Compile Rust to WebAssembly
2. **ESM Wrapper**: JavaScript/TypeScript wrapper
3. **Streaming Support**: For real-time chat

```javascript
// dist/aichat.mjs
import init, { AiChat } from './aichat_bg.wasm';

export class AiChatClient {
  constructor() {
    this.inner = null;
  }
  
  async init() {
    await init();
    this.inner = new AiChat();
  }
  
  async complete(prompt, options = {}) {
    return this.inner.complete(prompt, JSON.stringify(options));
  }
}

export default AiChatClient;
```

**Build Configuration:**
```toml
[lib]
crate-type = ["cdylib"]

[dependencies]
wasm-bindgen = "0.2"
js-sys = "0.3"
web-sys = "0.3"
```

**Action Items:**
- [ ] Add wasm-pack configuration
- [ ] Create web-compatible feature set
- [ ] Implement streaming APIs for web
- [ ] Setup ESM publishing pipeline

### 📦 Additional Platform Recommendations

#### 1. **Nix Packages**
```nix
{ lib, rustPlatform, fetchFromGitHub, pkg-config, openssl }:

rustPlatform.buildRustPackage rec {
  pname = "aichat";
  version = "0.29.0";
  
  src = fetchFromGitHub {
    owner = "sigoden";
    repo = "aichat";
    rev = "v${version}";
    sha256 = "...";
  };
  
  cargoSha256 = "...";
  
  nativeBuildInputs = [ pkg-config ];
  buildInputs = [ openssl ];
}
```

#### 2. **Flatpak**
```yaml
# org.aichat.AIChat.yaml
app-id: org.aichat.AIChat
runtime: org.freedesktop.Platform
runtime-version: '23.08'
sdk: org.freedesktop.Sdk
sdk-extensions:
  - org.freedesktop.Sdk.Extension.rust-stable
command: aichat
```

#### 3. **Snap Package**
```yaml
# snap/snapcraft.yaml
name: aichat
version: '0.29.0'
summary: All-in-one LLM CLI tool
description: |
  AIChat is a comprehensive CLI tool for interacting with multiple 
  LLM providers, featuring shell assistance, REPL mode, and more.

grade: stable
confinement: strict
base: core22

parts:
  aichat:
    plugin: rust
    source: .
```

#### 4. **Homebrew Formula Enhancement**
```ruby
# Formula/aichat.rb (enhancement suggestions)
class Aichat < Formula
  desc "All-in-one LLM CLI tool"
  homepage "https://github.com/sigoden/aichat"
  
  # Add shell completion installation
  def install
    bin.install "aichat"
    generate_completions_from_executable(bin/"aichat", "--generate-completions")
  end
end
```

#### 5. **Windows Package Manager (winget)**
```yaml
# manifests/a/aichat/aichat/0.29.0/aichat.aichat.yaml
PackageIdentifier: aichat.aichat
PackageVersion: 0.29.0
PackageLocale: en-US
Publisher: sigoden
PackageName: AIChat
License: MIT OR Apache-2.0
ShortDescription: All-in-one LLM CLI tool
```

---

## Implementation Priority Matrix

| Platform | Effort | Impact | Users | Priority |
|----------|--------|--------|-------|----------|
| NPM | Medium | High | Large | 🟢 High |
| PyPI | Medium | High | Large | 🟢 High |
| Guix | Low | Medium | Small | 🟡 Medium |
| ESM/WASM | High | Medium | Medium | 🟡 Medium |
| Nix | Low | Medium | Medium | 🟡 Medium |
| Flatpak | Medium | Low | Small | 🔴 Low |
| Snap | Low | Low | Small | 🔴 Low |
| winget | Low | High | Large | 🟢 High |

---

## Next Steps

### Immediate Actions (Week 1-2)
- [ ] Setup NPM binary wrapper package
- [ ] Create winget manifest
- [ ] Add release artifacts checksums

### Short Term (Month 1)
- [ ] Implement PyO3 Python bindings
- [ ] Submit Guix package definition
- [ ] Create Nix package

### Medium Term (Month 2-3)
- [ ] WebAssembly/ESM implementation
- [ ] Flatpak and Snap packages
- [ ] Enhanced container images

### Validation Checklist
- [ ] Each package installs correctly
- [ ] All core features work in packaged version
- [ ] Documentation updated for new install methods
- [ ] CI/CD pipeline includes new packages
- [ ] License compliance verified

---

**Assessment Complete** ✅
*This analysis provides a comprehensive roadmap for expanding AIChat's packaging ecosystem across multiple platforms while maintaining the existing robust GitHub-based distribution.*