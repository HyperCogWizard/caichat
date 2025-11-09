# NPM Package Implementation

This directory contains the implementation for the AIChat NPM package wrapper.

## Files

- `download-binary.js` - Post-install script that downloads platform-specific binaries
- `aichat-wrapper.js` - Binary wrapper that executes the native AIChat binary
- `test.js` - Test script to verify the package works correctly

## How it Works

1. **Installation**: When `npm install @aichat/cli` is run, the `postinstall` script (`download-binary.js`) automatically downloads the appropriate binary for the user's platform from GitHub releases.

2. **Execution**: When users run `aichat` or `npx aichat`, it executes the wrapper script (`aichat-wrapper.js`) which forwards all arguments to the native binary.

3. **Platform Support**: Supports Windows (x64, arm64, x86), macOS (x64, arm64), and Linux (x64, arm64, x86).

## Usage

### For Package Maintainers

1. Copy the example `npm-package.json` to the root as `package.json`
2. Update version numbers and metadata as needed
3. Ensure GitHub releases contain the expected binary archives
4. Test locally:
   ```bash
   npm install
   npm test
   ./bin/aichat --version
   ```
5. Publish:
   ```bash
   npm publish
   ```

### For End Users

```bash
# Install globally
npm install -g @aichat/cli

# Use the CLI
aichat --help
aichat "What is the weather like?"

# Or use with npx (no global install)
npx @aichat/cli --help
```

## Features

- **Zero-dependency**: Uses only Node.js built-in modules
- **Cross-platform**: Automatically detects and downloads correct binary
- **Transparent**: Acts as a thin wrapper around the native binary
- **Robust**: Includes error handling and verification steps
- **Fast**: Direct binary execution, no JavaScript overhead during runtime

## Implementation Notes

- The download script maps Node.js platform/arch to Rust target triples
- Binary verification ensures the downloaded executable works
- The wrapper preserves all stdio streams and exit codes
- Temporary files are cleaned up after installation
- Supports both global and local npm installations

## Testing

The test suite verifies:
- Binary downloaded correctly
- Version command works
- Help command works  
- Wrapper script exists and is functional

Run tests with: `npm test`

## Troubleshooting

If installation fails:
1. Check network connectivity to GitHub
2. Verify the release exists and contains expected assets
3. Try `npm install --force` to re-download
4. Check platform/architecture support

For execution issues:
1. Verify binary permissions (Unix: should be executable)
2. Check if antivirus software is blocking execution
3. Try running the binary directly from `bin/` directory