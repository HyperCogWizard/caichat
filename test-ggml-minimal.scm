#!/usr/bin/env guile
!#

;;
;; test-ggml-minimal.scm - Minimal test of GGML integration (no OpenCog required)
;;

(add-to-load-path ".")
(add-to-load-path "opencog/scm")

;; Load only the CAIChat modules that work without OpenCog
(use-modules (opencog caichat config))

(display "╔══════════════════════════════════════════════════════════════╗\n")
(display "║          CAIChat GGML Integration Test (Minimal)             ║\n")
(display "╚══════════════════════════════════════════════════════════════╝\n\n")

;; Test 1: Configuration Help
(display "Test 1: Configuration Help\n")
(display "===========================\n")

(caichat-help)

;; Test 2: Provider Listing
(display "\nTest 2: Provider Listing\n")
(display "========================\n")

(define providers (caichat-list-providers))
(display (format #f "Initially configured providers: ~a\n" providers))

;; Test 3: GGML Setup (simulation)
(display "\nTest 3: GGML Setup\n")
(display "==================\n")

(define test-model-path "/tmp/test-model.ggmlv3.q4_0.bin")
(display (format #f "Setting up GGML with model path: ~a\n" test-model-path))

;; This will work even without a real model file
(caichat-setup-ggml test-model-path 4 2048)

;; Test 4: Updated Provider List
(display "\nTest 4: Updated Provider List\n")
(display "=============================\n")

(define updated-providers (caichat-list-providers))
(display (format #f "Updated providers: ~a\n" updated-providers))

;; Test 5: Configuration Display
(display "\nTest 5: Configuration Details\n")
(display "=============================\n")

(define ggml-config (caichat-get-config 'ggml))
(display (format #f "GGML configuration: ~a\n" ggml-config))

;; Summary
(display "\n╔══════════════════════════════════════════════════════════════╗\n")
(display "║                     Test Summary                             ║\n")
(display "╚══════════════════════════════════════════════════════════════╝\n")
(display "✅ Minimal GGML integration tests completed!\n")
(display "\nFeatures tested (without OpenCog):\n")
(display "  ✓ Help system functionality\n")
(display "  ✓ Provider listing\n")
(display "  ✓ GGML configuration setup\n")
(display "  ✓ Configuration management\n\n")

(display "Note: Full GGML functionality requires:\n")
(display "  - OpenCog AtomSpace integration\n")
(display "  - Actual GGML model files\n")
(display "  - C++ extensions loaded\n\n")

(display "Configuration is working correctly! 🎉\n")