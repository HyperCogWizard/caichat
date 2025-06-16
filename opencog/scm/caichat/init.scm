;;
;; init.scm - CAIChat OpenCog module initialization
;;
;; This file loads all CAIChat modules and sets up the environment
;;

(define-module (opencog caichat init))

(use-modules (opencog)
             (opencog caichat)
             (opencog caichat config)
             (opencog caichat repl)
             (opencog caichat rag))

;; Re-export main functions for convenience
(re-export caichat-configure
           caichat-start-session
           caichat-chat
           caichat-ask
           caichat-repl
           caichat-help
           caichat-rag-query
           caichat-add-document
           caichat-create-knowledge-base)

;; Module initialization
(define (caichat-init)
  "Initialize the CAIChat OpenCog module"
  (cog-logger-info "Initializing CAIChat OpenCog module...")
  
  ;; Try to load configuration
  (let ((config-file (string-append (getenv "HOME") "/.caichat/config.scm")))
    (when (file-exists? config-file)
      (caichat-load-config config-file)
      (cog-logger-info "Loaded configuration from " config-file)))
  
  ;; Auto-configure from environment if no config found
  (when (null? (caichat-list-providers))
    (caichat-auto-configure))
  
  ;; Set up default system prompt
  (caichat-set-system-prompt 
   "You are a helpful AI assistant integrated with OpenCog. You can reason about atoms, patterns, and knowledge representations in the AtomSpace.")
  
  (cog-logger-info "CAIChat OpenCog module initialized successfully"))

;; Display welcome message
(define (caichat-welcome)
  "Display welcome message and basic usage"
  (display "╔══════════════════════════════════════════════════════════════╗\n")
  (display "║                    CAIChat for OpenCog                      ║\n")
  (display "║                                                              ║\n")
  (display "║  AI-powered chat interface integrated with AtomSpace        ║\n")
  (display "╚══════════════════════════════════════════════════════════════╝\n\n")
  
  (display "Quick Start:\n")
  (display "  (caichat-ask \"Hello!\")              ; Quick question\n")
  (display "  (caichat-repl)                      ; Start interactive chat\n")
  (display "  (caichat-help)                      ; Show help\n\n")
  
  (let ((providers (caichat-list-providers)))
    (if (null? providers)
        (begin
          (display "⚠️  No LLM providers configured!\n")
          (display "   Set up a provider first:\n")
          (display "   (caichat-setup-openai \"your-api-key\")\n")
          (display "   (caichat-setup-claude \"your-api-key\")\n")
          (display "   (caichat-setup-ollama \"http://localhost:11434\")\n\n"))
        (begin
          (display "✓ Configured providers: ")
          (display (string-join (map symbol->string providers) ", "))
          (display "\n\n")))))

;; Auto-initialize when module is loaded
(caichat-init)

;; Show welcome message
(caichat-welcome)