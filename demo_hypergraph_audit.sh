#!/usr/bin/env bash
#
# Hypergraph Audit Demo Script
# 
# This script demonstrates the hypergraph audit and reinforcement functionality
# in the CAIChat system.

set -e

echo "╔══════════════════════════════════════════════════════════════╗"
echo "║              CAIChat Hypergraph Audit Demo                  ║"
echo "╚══════════════════════════════════════════════════════════════╝"
echo

# Check if aichat binary exists
if ! command -v cargo &> /dev/null; then
    echo "❌ Cargo not found. Please install Rust and Cargo first."
    exit 1
fi

echo "🔨 Building CAIChat with hypergraph audit functionality..."
cargo build --release --quiet

if [ $? -eq 0 ]; then
    echo "✅ Build successful!"
else
    echo "❌ Build failed. Please check the compilation errors above."
    exit 1
fi

echo
echo "📊 Running hypergraph audit..."
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

# Run the hypergraph audit
cargo run --release -- --audit-hypergraph

echo
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "🎉 Hypergraph audit completed!"
echo
echo "The audit shows:"
echo "  • Module health status and synergy scores"
echo "  • Inter-module connection strengths"
echo "  • Performance metrics and efficiency ratings"
echo "  • Recommendations for system improvements"
echo
echo "This audit helps ensure optimal cognitive coherence across"
echo "all CAIChat core modules for enhanced hypergraph synergy."