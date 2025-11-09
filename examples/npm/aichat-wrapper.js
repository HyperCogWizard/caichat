#!/usr/bin/env node

/**
 * aichat - NPM binary wrapper for AIChat
 * 
 * This script acts as a wrapper around the native AIChat binary,
 * passing through all command-line arguments and handling the execution.
 */

const { spawn } = require('child_process');
const path = require('path');
const fs = require('fs');

// Determine the binary name based on platform
const binaryName = process.platform === 'win32' ? 'aichat.exe' : 'aichat';
const binaryPath = path.join(__dirname, '..', 'bin', binaryName);

/**
 * Check if the binary exists and is executable
 */
function checkBinary() {
  if (!fs.existsSync(binaryPath)) {
    console.error(`❌ AIChat binary not found at ${binaryPath}`);
    console.error('Try running: npm install --force');
    process.exit(1);
  }
  
  try {
    // Check if binary is executable (Unix-like systems)
    if (process.platform !== 'win32') {
      fs.accessSync(binaryPath, fs.constants.X_OK);
    }
  } catch (err) {
    console.error(`❌ AIChat binary is not executable at ${binaryPath}`);
    console.error('Try running: npm install --force');
    process.exit(1);
  }
}

/**
 * Execute the AIChat binary with the provided arguments
 */
function executeAIChat() {
  checkBinary();
  
  // Pass all command-line arguments to the binary
  const args = process.argv.slice(2);
  
  // Spawn the binary process
  const child = spawn(binaryPath, args, {
    stdio: 'inherit', // Pass through stdin, stdout, stderr
    shell: false      // Direct execution, no shell wrapper
  });
  
  // Handle process events
  child.on('error', (error) => {
    console.error(`❌ Failed to execute AIChat: ${error.message}`);
    process.exit(1);
  });
  
  child.on('close', (code, signal) => {
    if (signal) {
      console.error(`❌ AIChat was killed by signal ${signal}`);
      process.exit(1);
    }
    
    // Exit with the same code as the child process
    process.exit(code || 0);
  });
  
  // Handle termination signals
  process.on('SIGINT', () => {
    child.kill('SIGINT');
  });
  
  process.on('SIGTERM', () => {
    child.kill('SIGTERM');
  });
}

// Execute AIChat if this script is run directly
if (require.main === module) {
  executeAIChat();
}

module.exports = { executeAIChat, binaryPath };