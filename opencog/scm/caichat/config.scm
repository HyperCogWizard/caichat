;;
;; config.scm - Configuration management for CAIChat
;;
;; Handles loading and managing LLM provider configurations
;;

(define-module (opencog caichat config))

(use-modules (opencog)
             (opencog caichat)
             (ice-9 textual-ports)
             (ice-9 hash-table))

(export caichat-load-config
        caichat-save-config
        caichat-get-config
        caichat-set-config
        caichat-list-providers
        caichat-get-default-provider)

;; Global configuration storage
(define caichat-config-table (make-hash-table))
(define caichat-default-provider "openai")
(define caichat-default-model "gpt-3.5-turbo")

;;
;; Configuration file format (S-expressions):
;; ((providers
;;   (openai 
;;     (api-key "your-key-here")
;;     (models ("gpt-3.5-turbo" "gpt-4" "gpt-4-turbo")))
;;   (claude
;;     (api-key "your-key-here") 
;;     (models ("claude-3-sonnet" "claude-3-opus")))
;;   (ollama
;;     (api-base "http://localhost:11434")
;;     (models ("llama2" "codellama")))))
;;

(define (caichat-load-config config-file)
  "Load configuration from a file"
  (if (file-exists? config-file)
      (let ((config-data (call-with-input-file config-file read)))
        (when (and (list? config-data) 
                   (eq? (car config-data) 'providers))
          (for-each 
           (lambda (provider-config)
             (let ((provider-name (car provider-config))
                   (provider-settings (cdr provider-config)))
               (hash-set! caichat-config-table provider-name provider-settings)))
           (cdr config-data))
          #t))
      (begin
        (cog-logger-warn "Configuration file not found: " config-file)
        #f)))

(define (caichat-save-config config-file)
  "Save current configuration to a file"
  (let ((config-data 
         (cons 'providers
               (hash-map->list 
                (lambda (provider settings)
                  (cons provider settings))
                caichat-config-table))))
    (call-with-output-file config-file
      (lambda (port)
        (write config-data port)
        (newline port)))
    #t))

(define (caichat-get-config provider)
  "Get configuration for a specific provider"
  (hash-ref caichat-config-table provider))

(define (caichat-set-config provider settings)
  "Set configuration for a provider"
  (hash-set! caichat-config-table provider settings))

(define (caichat-list-providers)
  "List all configured providers"
  (hash-map->list 
   (lambda (provider settings)
     provider)
   caichat-config-table))

(define (caichat-get-default-provider)
  "Get the default provider and model"
  (list caichat-default-provider caichat-default-model))

(define (caichat-set-default-provider provider model)
  "Set the default provider and model"
  (set! caichat-default-provider provider)
  (set! caichat-default-model model))

;;
;; Convenience functions for common configurations
;;

(define (caichat-setup-openai api-key)
  "Quick setup for OpenAI"
  (caichat-set-config 'openai 
                      `((api-key ,api-key)
                        (models ("gpt-3.5-turbo" "gpt-4" "gpt-4-turbo" "gpt-4o"))))
  (caichat-configure "openai" "gpt-3.5-turbo" api-key))

(define (caichat-setup-claude api-key)
  "Quick setup for Claude"
  (caichat-set-config 'claude
                      `((api-key ,api-key)
                        (models ("claude-3-sonnet-20240229" "claude-3-opus-20240229"))))
  (caichat-configure "claude" "claude-3-sonnet-20240229" api-key))

(define (caichat-setup-ollama base-url)
  "Quick setup for Ollama"
  (caichat-set-config 'ollama
                      `((api-base ,base-url)
                        (models ("llama2" "codellama" "mistral"))))
  (caichat-configure "ollama" "llama2" "" base-url))

;;
;; Auto-configuration from environment variables
;;

(define (caichat-auto-configure)
  "Auto-configure from environment variables"
  (let ((openai-key (getenv "OPENAI_API_KEY"))
        (claude-key (getenv "ANTHROPIC_API_KEY"))
        (ollama-url (getenv "OLLAMA_BASE_URL")))
    
    (when openai-key
      (caichat-setup-openai openai-key)
      (cog-logger-info "Configured OpenAI from environment"))
    
    (when claude-key
      (caichat-setup-claude claude-key)
      (cog-logger-info "Configured Claude from environment"))
    
    (when ollama-url
      (caichat-setup-ollama ollama-url)
      (cog-logger-info "Configured Ollama from environment"))
    
    ;; Set default if any provider was configured
    (cond 
     (openai-key (caichat-set-default-provider "openai" "gpt-3.5-turbo"))
     (claude-key (caichat-set-default-provider "claude" "claude-3-sonnet-20240229"))
     (ollama-url (caichat-set-default-provider "ollama" "llama2")))))

;;
;; Configuration validation
;;

(define (caichat-validate-config provider)
  "Validate configuration for a provider"
  (let ((config (caichat-get-config provider)))
    (if config
        (case provider
          ((openai claude)
           (assoc 'api-key config))
          ((ollama)
           (assoc 'api-base config))
          (else #f))
        #f)))

(define (caichat-test-provider provider model)
  "Test if a provider configuration works"
  (if (caichat-validate-config provider)
      (let ((test-session (caichat-start-session provider model)))
        (if test-session
            (let ((response (caichat-ask "Hello, can you hear me?" provider model)))
              (caichat-destroy-session test-session)
              (if (string? response)
                  #t
                  #f))
            #f))
      #f))

;; Try to auto-configure on module load
(caichat-auto-configure)