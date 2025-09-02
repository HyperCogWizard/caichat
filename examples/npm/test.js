#!/usr/bin/env node

/**
 * test.js - Simple test for the AIChat NPM package
 */

const { execSync } = require('child_process');
const path = require('path');
const fs = require('fs');

function runTests() {
  console.log('🧪 Testing AIChat NPM Package');
  console.log('==============================');
  
  try {
    // Test 1: Check if binary exists
    console.log('Test 1: Binary existence...');
    const binaryName = process.platform === 'win32' ? 'aichat.exe' : 'aichat';
    const binaryPath = path.join(__dirname, '..', 'bin', binaryName);
    
    if (fs.existsSync(binaryPath)) {
      console.log('✅ Binary exists');
    } else {
      console.log('❌ Binary not found');
      return false;
    }
    
    // Test 2: Check version
    console.log('Test 2: Version check...');
    try {
      const version = execSync(`"${binaryPath}" --version`, { 
        encoding: 'utf8',
        timeout: 5000 
      });
      console.log(`✅ Version: ${version.trim()}`);
    } catch (err) {
      console.log(`❌ Version check failed: ${err.message}`);
      return false;
    }
    
    // Test 3: Check help
    console.log('Test 3: Help command...');
    try {
      const help = execSync(`"${binaryPath}" --help`, { 
        encoding: 'utf8',
        timeout: 5000 
      });
      if (help.includes('aichat') || help.includes('AIChat')) {
        console.log('✅ Help command works');
      } else {
        console.log('❌ Help output unexpected');
        return false;
      }
    } catch (err) {
      console.log(`❌ Help command failed: ${err.message}`);
      return false;
    }
    
    // Test 4: Check wrapper script
    console.log('Test 4: Wrapper script...');
    const wrapperPath = path.join(__dirname, 'aichat-wrapper.js');
    if (fs.existsSync(wrapperPath)) {
      console.log('✅ Wrapper script exists');
    } else {
      console.log('❌ Wrapper script not found');
      return false;
    }
    
    console.log('\n🎉 All tests passed!');
    return true;
    
  } catch (error) {
    console.error(`❌ Test suite failed: ${error.message}`);
    return false;
  }
}

if (require.main === module) {
  const success = runTests();
  process.exit(success ? 0 : 1);
}

module.exports = { runTests };