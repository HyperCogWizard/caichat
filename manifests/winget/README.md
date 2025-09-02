# WinGet Package Manifest

This directory contains the Windows Package Manager (WinGet) manifest files for AIChat.

## Files

- `sigoden.aichat.yaml` - Version manifest
- `sigoden.aichat.locale.en-US.yaml` - Default locale manifest
- `sigoden.aichat.installer.yaml` - Installer manifest

## Usage

### Installing AIChat via WinGet

Once this manifest is submitted to the official WinGet repository, users can install AIChat using:

```powershell
winget install sigoden.aichat
```

### Testing the Manifest Locally

Before submission, you can test the manifest locally:

```powershell
# Validate the manifest
winget validate manifests/winget/

# Test installation (replace with actual path)
winget install --manifest manifests/winget/sigoden.aichat.yaml
```

## Submission Process

1. **Update SHA256 hashes**: Replace `INSERT_HASH_HERE` placeholders in the installer manifest with actual SHA256 hashes from the GitHub releases.

2. **Fork the WinGet repository**: Fork [microsoft/winget-pkgs](https://github.com/microsoft/winget-pkgs)

3. **Copy manifests**: Copy the manifest files to:
   ```
   manifests/s/sigoden/aichat/0.29.0/
   ```

4. **Submit PR**: Create a pull request to the WinGet repository.

## Prerequisites

- The GitHub release must exist with the specified assets
- SHA256 checksums must be calculated for each binary
- All URLs must be accessible and valid

## Updating for New Versions

When releasing a new version:

1. Update the `PackageVersion` in all three files
2. Update the download URLs in the installer manifest
3. Calculate and update the SHA256 hashes
4. Update the `ReleaseNotesUrl` to point to the new release

## Notes

- This manifest follows WinGet schema version 1.4.0
- The package is configured as a portable app (zip-based installation)
- All Windows architectures are supported (x64, arm64, x86)