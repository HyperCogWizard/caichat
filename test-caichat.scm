#!/usr/bin/env guile
!#

;;
;; test-caichat.scm - Basic test for CAIChat OpenCog module
;;

(add-to-load-path ".")

;; Try to load our modules
(display "Testing CAIChat OpenCog module...\n")

(define (test-module-loading)
  "Test basic module loading"
  (display "Testing module loading...\n")
  
  ;; Test configuration module
  (use-modules (opencog caichat config))
  (display "✓ Config module loaded\n")
  
  ;; Test RAG module  
  (use-modules (opencog caichat rag))
  (display "✓ RAG module loaded\n")
  
  ;; Test REPL module
  (use-modules (opencog caichat repl))
  (display "✓ REPL module loaded\n")
  
  ;; Test main module
  (use-modules (opencog caichat))
  (display "✓ Main module loaded\n")
  
  #t)

(define (test-basic-functions)
  "Test basic function availability"
  (display "Testing basic functions...\n")
  
  ;; Test help function
  (caichat-help)
  (display "✓ Help function works\n")
  
  ;; Test configuration functions
  (let ((providers (caichat-list-providers)))
    (display (format #f "✓ Provider list: ~a\n" providers)))
  
  #t)

(define (test-embedding-simulation)
  "Test the embedding simulation"
  (display "Testing embedding functions...\n")
  
  (let ((embedding1 (caichat-embed-text "artificial intelligence"))
        (embedding2 (caichat-embed-text "machine learning")))
    (display (format #f "✓ Embedding 1: ~a\n" embedding1))
    (display (format #f "✓ Embedding 2: ~a\n" embedding2))
    
    ;; Test similarity calculation
    (let ((similarity (calculate-cosine-similarity embedding1 embedding2)))
      (display (format #f "✓ Similarity: ~a\n" similarity))))
  
  #t)

(define (run-tests)
  "Run all tests"
  (display "=== CAIChat OpenCog Module Tests ===\n\n")
  
  (and (test-module-loading)
       (test-basic-functions)
       (test-embedding-simulation)
       (begin
         (display "\n✅ All tests passed!\n")
         #t)))

;; Run the tests
(run-tests)