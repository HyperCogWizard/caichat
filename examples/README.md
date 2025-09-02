# Packaging Examples

This directory contains example configuration files for packaging AIChat across different platforms and package managers.

## Files

### `npm-package.json`
Example `package.json` for creating an NPM wrapper package that downloads platform-specific binaries.

**Usage:**
```bash
# In your npm package directory
cp examples/npm-package.json package.json
# Edit version, dependencies as needed
npm install
npm publish
```

### `pyproject.toml`
Example PyPI package configuration using maturin for Python bindings via PyO3.

**Usage:**
```bash
# Add PyO3 to Cargo.toml first
cp examples/pyproject.toml .
maturin develop  # For development
maturin build    # For building wheels
```

### `guix-package.scm`
GNU Guix package definition for inclusion in the Guix package repository.

**Usage:**
```bash
# Test locally
guix build -f examples/guix-package.scm

# For submission to Guix
# Copy to gnu/packages/aichat.scm in Guix repository
```

## Implementation Notes

- **NPM**: Requires creating platform-specific sub-packages or a download script
- **PyPI**: Needs PyO3 dependencies added to Cargo.toml and Python wrapper code
- **Guix**: Requires computing actual SHA256 hashes for the source

## Related Scripts

- `scripts/analyze-packaging.sh`: Analyzes current packaging state
- `.github/ISSUE_TEMPLATE/packager_template.md`: Comprehensive packaging strategy template

For detailed implementation strategies, see the packager template in `.github/ISSUE_TEMPLATE/`.