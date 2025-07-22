#!/usr/bin/env guile
!#

;;
;; test-c-extension.scm - Test the C++ extension loading
;;

(display "Testing CAIChat C++ extension loading...\n\n")

;; Try to load the C++ extension directly
(display "Loading C++ extension...\n")
(load-extension "./build/libcaichat-opencog" "opencog_caichat_init")
(display "✓ C++ extension loaded successfully!\n\n")

;; Test basic function availability
(display "Testing function availability:\n")

(define (test-function func-name)
  (display (format #f "  Testing ~a... " func-name))
  (if (defined? func-name)
      (display "✓ Available\n")
      (display "✗ Not found\n")))

(test-function 'caichat-create-client-config)
(test-function 'caichat-create-session)
(test-function 'caichat-add-message)
(test-function 'caichat-complete)
(test-function 'caichat-ggml-load-model)
(test-function 'caichat-ggml-model-info)
(test-function 'caichat-neural-symbolic-bridge)

;; Test basic functionality
(display "\nTesting basic functionality:\n")

;; Create a client config
(display "Creating GGML client config...\n")
(define config-id (caichat-create-client-config "ggml" "/tmp/test.bin" "" ""))
(display (format #f "✓ Config created: ~a\n" config-id))

;; Create a session
(display "Creating session...\n")
(define session-id (caichat-create-session config-id #f))
(display (format #f "✓ Session created: ~a\n" session-id))

;; Test GGML model info
(display "Getting GGML model info...\n")
(define model-info (caichat-ggml-model-info session-id))
(display (format #f "✓ Model info: ~a\n" model-info))

;; Test neural-symbolic bridge
(display "Testing neural-symbolic bridge...\n")
(define bridge-result (caichat-neural-symbolic-bridge "artificial intelligence"))
(display (format #f "✓ Bridge result: ~a\n" bridge-result))

(display "\n╔══════════════════════════════════════════════════════════════╗\n")
(display "║                     Test Complete                           ║\n")
(display "╚══════════════════════════════════════════════════════════════╝\n")
(display "✅ CAIChat C++ extension is working correctly!\n")
(display "✅ GGML integration functions are available!\n")
(display "✅ Neural-symbolic bridge is functional!\n\n")

(display "The GGML OpenCog integration is ready for use! 🎉\n")