#!/usr/bin/env node

/**
 * download-binary.js - NPM postinstall script to download platform-specific binaries
 * 
 * This script downloads the appropriate AIChat binary for the current platform
 * from GitHub releases and places it in the correct location for the NPM package.
 */

const fs = require('fs');
const path = require('path');
const https = require('https');
const { execSync } = require('child_process');

// Package configuration
const PACKAGE_NAME = 'aichat';
const GITHUB_REPO = 'sigoden/aichat';
const VERSION = process.env.npm_package_version || '0.29.0';

// Platform mappings
const PLATFORM_MAP = {
  'darwin': 'apple-darwin',
  'linux': 'unknown-linux-musl',
  'win32': 'pc-windows-msvc'
};

const ARCH_MAP = {
  'x64': 'x86_64',
  'arm64': 'aarch64',
  'ia32': 'i686'
};

/**
 * Get the platform-specific download information
 */
function getPlatformInfo() {
  const platform = process.platform;
  const arch = process.arch;
  
  if (!PLATFORM_MAP[platform]) {
    throw new Error(`Unsupported platform: ${platform}`);
  }
  
  if (!ARCH_MAP[arch]) {
    throw new Error(`Unsupported architecture: ${arch}`);
  }
  
  const targetTriple = `${ARCH_MAP[arch]}-${PLATFORM_MAP[platform]}`;
  const extension = platform === 'win32' ? 'zip' : 'tar.gz';
  const filename = `${PACKAGE_NAME}-v${VERSION}-${targetTriple}.${extension}`;
  const url = `https://github.com/${GITHUB_REPO}/releases/download/v${VERSION}/${filename}`;
  
  return {
    platform,
    arch,
    targetTriple,
    filename,
    url,
    extension,
    binaryName: platform === 'win32' ? `${PACKAGE_NAME}.exe` : PACKAGE_NAME
  };
}

/**
 * Download file from URL
 */
function downloadFile(url, destPath) {
  return new Promise((resolve, reject) => {
    console.log(`Downloading ${url}...`);
    
    const file = fs.createWriteStream(destPath);
    
    https.get(url, (response) => {
      if (response.statusCode === 302 || response.statusCode === 301) {
        // Follow redirect
        file.close();
        fs.unlinkSync(destPath);
        downloadFile(response.headers.location, destPath).then(resolve).catch(reject);
        return;
      }
      
      if (response.statusCode !== 200) {
        file.close();
        fs.unlinkSync(destPath);
        reject(new Error(`Download failed with status ${response.statusCode}`));
        return;
      }
      
      response.pipe(file);
      
      file.on('finish', () => {
        file.close();
        resolve();
      });
      
      file.on('error', (err) => {
        file.close();
        fs.unlinkSync(destPath);
        reject(err);
      });
    }).on('error', (err) => {
      file.close();
      if (fs.existsSync(destPath)) {
        fs.unlinkSync(destPath);
      }
      reject(err);
    });
  });
}

/**
 * Extract archive
 */
function extractArchive(archivePath, extractDir, platformInfo) {
  console.log(`Extracting ${archivePath}...`);
  
  if (platformInfo.extension === 'zip') {
    // Use built-in unzipping for Windows
    if (process.platform === 'win32') {
      execSync(`powershell Expand-Archive -Path "${archivePath}" -DestinationPath "${extractDir}" -Force`);
    } else {
      execSync(`unzip -o "${archivePath}" -d "${extractDir}"`);
    }
  } else {
    // tar.gz
    execSync(`tar -xzf "${archivePath}" -C "${extractDir}"`);
  }
}

/**
 * Main installation function
 */
async function install() {
  try {
    const platformInfo = getPlatformInfo();
    
    console.log(`Installing AIChat v${VERSION} for ${platformInfo.platform}-${platformInfo.arch}`);
    
    // Create directories
    const binDir = path.join(__dirname, '..', 'bin');
    const tempDir = path.join(__dirname, '..', 'temp');
    
    if (!fs.existsSync(binDir)) {
      fs.mkdirSync(binDir, { recursive: true });
    }
    
    if (!fs.existsSync(tempDir)) {
      fs.mkdirSync(tempDir, { recursive: true });
    }
    
    // Download the archive
    const archivePath = path.join(tempDir, platformInfo.filename);
    await downloadFile(platformInfo.url, archivePath);
    
    // Extract the archive
    extractArchive(archivePath, tempDir, platformInfo);
    
    // Find and move the binary
    const extractedBinaryPath = path.join(tempDir, platformInfo.binaryName);
    const finalBinaryPath = path.join(binDir, platformInfo.binaryName);
    
    if (!fs.existsSync(extractedBinaryPath)) {
      throw new Error(`Binary not found at ${extractedBinaryPath}`);
    }
    
    // Copy binary to bin directory
    fs.copyFileSync(extractedBinaryPath, finalBinaryPath);
    
    // Make binary executable (Unix-like systems)
    if (process.platform !== 'win32') {
      fs.chmodSync(finalBinaryPath, '755');
    }
    
    // Clean up
    fs.rmSync(tempDir, { recursive: true, force: true });
    
    console.log(`✅ AIChat installed successfully to ${finalBinaryPath}`);
    
    // Verify installation
    try {
      const version = execSync(`"${finalBinaryPath}" --version`, { encoding: 'utf8' });
      console.log(`✅ Version check: ${version.trim()}`);
    } catch (err) {
      console.warn(`⚠️  Could not verify installation: ${err.message}`);
    }
    
  } catch (error) {
    console.error(`❌ Installation failed: ${error.message}`);
    process.exit(1);
  }
}

// Run the installer
if (require.main === module) {
  install();
}

module.exports = { install, getPlatformInfo };