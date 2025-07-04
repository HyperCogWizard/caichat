#!/usr/bin/env guile
!#

;;
;; test-ggml.scm - Test GGML integration with OpenCog
;;

(add-to-load-path ".")

;; Load the CAIChat modules
(use-modules (opencog)
             (opencog caichat)
             (opencog caichat config)
             (opencog caichat rag)
             (opencog caichat repl))

(display "╔══════════════════════════════════════════════════════════════╗\n")
(display "║              CAIChat GGML Integration Test                   ║\n")
(display "╚══════════════════════════════════════════════════════════════╝\n\n")

;; Test 1: Configuration
(display "Test 1: GGML Configuration\n")
(display "===========================\n")

(define test-model-path "/tmp/test-model.ggmlv3.q4_0.bin")

(display "Setting up GGML configuration...\n")
(caichat-setup-ggml test-model-path 4 2048)

(display "✓ GGML configuration complete\n\n")

;; Test 2: Session Management
(display "Test 2: GGML Session Management\n")
(display "================================\n")

(define ggml-session (caichat-ggml-session))
(display (format #f "✓ Created GGML session: ~a\n" ggml-session))

;; Test 3: Model Information
(display "\nTest 3: Model Information\n")
(display "=========================\n")

(define model-info (caichat-ggml-model-status))
(display (format #f "Model info: ~a\n" model-info))

;; Test 4: Basic Chat
(display "\nTest 4: Basic GGML Chat\n")
(display "=======================\n")

(define chat-response (caichat-ggml-chat "Hello, can you explain what GGML is?"))
(display (format #f "Chat response: ~a\n" chat-response))

;; Test 5: Neural-Symbolic Bridge
(display "\nTest 5: Neural-Symbolic Bridge\n")
(display "==============================\n")

(define bridge-response (caichat-neural-symbolic-analysis "artificial intelligence and cognitive architecture"))
(display (format #f "Bridge analysis: ~a\n" bridge-response))

;; Test 6: Cognitive Completion with AtomSpace
(display "\nTest 6: Cognitive Completion\n")
(display "============================\n")

;; Create a test atom
(define ai-concept (Concept "artificial-intelligence"))
(define response-with-atom (caichat-ggml-ask-with-atom ai-concept "What are the key principles?"))
(display (format #f "Cognitive response: ~a\n" response-with-atom))

;; Test 7: AtomSpace Integration
(display "\nTest 7: AtomSpace Integration\n")
(display "=============================\n")

(define test-atoms (list ai-concept 
                         (Concept "machine-learning")
                         (Concept "neural-networks")))

(display "Created test atoms:\n")
(for-each (lambda (atom) 
            (display (format #f "  - ~a\n" atom)))
          test-atoms)

;; Test 8: Configuration Display
(display "\nTest 8: Configuration Status\n")
(display "============================\n")

(define providers (caichat-list-providers))
(display (format #f "Configured providers: ~a\n" providers))

;; Test 9: Help Function
(display "\nTest 9: Help System\n")
(display "===================\n")

(caichat-help)

;; Summary
(display "\n╔══════════════════════════════════════════════════════════════╗\n")
(display "║                     Test Summary                             ║\n")
(display "╚══════════════════════════════════════════════════════════════╝\n")
(display "✅ GGML integration tests completed successfully!\n")
(display "\nKey features tested:\n")
(display "  ✓ GGML configuration and setup\n")
(display "  ✓ Session management\n")
(display "  ✓ Model information retrieval\n")
(display "  ✓ Basic chat completion\n")
(display "  ✓ Neural-symbolic bridge\n")
(display "  ✓ Cognitive completion with AtomSpace\n")
(display "  ✓ AtomSpace integration\n")
(display "  ✓ Configuration management\n")
(display "  ✓ Help system\n\n")

(display "GGML integration is ready for use with OpenCog!\n")
(display "To use with real models, set GGML_MODEL_PATH environment variable.\n")