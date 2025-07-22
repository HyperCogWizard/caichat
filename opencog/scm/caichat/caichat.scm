;; 
;; caichat.scm - Main CAIChat OpenCog module
;;
;; This module provides Scheme bindings for the CAIChat LLM functionality
;; integrated with OpenCog's AtomSpace
;;

(define-module (opencog caichat))

(use-modules (opencog)
             (opencog exec)
             (opencog logger))

;; Load the C++ module
(load-extension "libcaichat-opencog" "opencog_caichat_init")

;; Export the main functions
(export caichat-create-client-config
        caichat-create-session
        caichat-add-message
        caichat-complete
        caichat-clear-history
        caichat-save-conversation
        caichat-load-conversation
        caichat-destroy-session
        caichat-chat
        caichat-ask
        caichat-set-system-prompt
        caichat-list-sessions
        caichat-get-session-info
        ;; GGML-specific functions
        caichat-ggml-load-model
        caichat-ggml-unload-model
        caichat-ggml-model-info
        caichat-ggml-cognitive-completion
        caichat-ggml-atomspace-to-prompt
        caichat-setup-ggml
        caichat-ggml-chat
        caichat-ggml-ask-with-atom
        ;; New functions from issue #3 requirements
        caichat-init-llm-provider
        caichat-route-llm-request
        caichat-propagate-patterns
        caichat-map-opencog-api)

;; Configuration storage
(define caichat-configs (make-hash-table))
(define caichat-sessions (make-hash-table))
(define caichat-system-prompt "You are a helpful AI assistant.")

;;
;; High-level convenience functions
;;

(define* (caichat-configure provider model api-key #:optional (api-base #f))
  "Configure a CAIChat client with the given provider, model and API key"
  (let ((config-id (caichat-create-client-config provider model api-key api-base)))
    (hash-set! caichat-configs (string-append provider ":" model) config-id)
    config-id))

(define* (caichat-start-session provider model #:optional (atomspace (cog-atomspace)))
  "Start a new chat session with the given provider and model"
  (let* ((config-key (string-append provider ":" model))
         (config-id (hash-ref caichat-configs config-key)))
    (if config-id
        (let ((session-id (caichat-create-session config-id atomspace)))
          (hash-set! caichat-sessions session-id 
                     (list provider model (current-time)))
          ;; Add system prompt if set
          (when (> (string-length caichat-system-prompt) 0)
            (caichat-add-message session-id "system" caichat-system-prompt))
          session-id)
        (error "No configuration found for provider/model:" provider model))))

(define (caichat-chat session-id user-message)
  "Send a user message and get a response from the assistant"
  (caichat-add-message session-id "user" user-message)
  (caichat-complete session-id))

(define* (caichat-ask question #:optional (provider "openai") (model "gpt-3.5-turbo"))
  "Quick one-shot question to an LLM (creates temporary session)"
  (let ((session-id (caichat-start-session provider model)))
    (let ((response (caichat-chat session-id question)))
      (caichat-destroy-session session-id)
      response)))

(define (caichat-set-system-prompt prompt)
  "Set the default system prompt for new sessions"
  (set! caichat-system-prompt prompt))

(define (caichat-list-sessions)
  "List all active chat sessions"
  (hash-map->list (lambda (session-id info)
                    (list session-id info))
                  caichat-sessions))

(define (caichat-get-session-info session-id)
  "Get information about a specific session"
  (hash-ref caichat-sessions session-id))

;;
;; Utility functions for working with conversations in AtomSpace
;;

(define (caichat-conversation-to-atoms session-id)
  "Get the conversation history as a list of atoms"
  ;; This would need to be implemented in the C++ layer
  ;; For now, return empty list
  '())

(define (caichat-find-conversations)
  "Find all saved conversations in the AtomSpace"
  (let ((conversations '()))
    ;; Find all ConceptNodes that start with "conversation:"
    (cog-map-type 
     (lambda (atom)
       (let ((name (cog-name atom)))
         (when (string-prefix? "conversation:" name)
           (set! conversations (cons atom conversations))))
       #f)
     'ConceptNode)
    conversations))

;;
;; Advanced conversation management
;;

(define (caichat-create-persistent-session session-name provider model)
  "Create a session that will be saved to AtomSpace"
  (let ((session-id (caichat-start-session provider model)))
    (caichat-save-conversation session-id session-name)
    session-id))

(define (caichat-resume-session session-name provider model)
  "Resume a previously saved session"
  (let ((session-id (caichat-start-session provider model)))
    (caichat-load-conversation session-id session-name)
    session-id))

;;
;; Integration with OpenCog reasoning
;;

(define (caichat-ask-about-atom atom question)
  "Ask a question about a specific atom in the AtomSpace"
  (let* ((atom-str (atom->string atom))
         (full-question (string-append 
                        "Based on this OpenCog atom: " atom-str "\n"
                        "Question: " question)))
    (caichat-ask full-question)))

(define (caichat-explain-atom atom)
  "Get an explanation of what an atom represents"
  (caichat-ask-about-atom atom "What does this atom represent and what is its purpose?"))

(define (caichat-suggest-relations atom)
  "Get suggestions for relations this atom might have with others"
  (caichat-ask-about-atom atom 
    "What kinds of relationships or links might this atom have with other atoms?"))

;;
;; Pattern-based conversation helpers
;;

(define (caichat-match-and-ask pattern question)
  "Find atoms matching a pattern and ask about them"
  (let ((matches (cog-bind pattern)))
    (if (not (cog-link? matches))
        "No matches found for the pattern"
        (let ((match-strs (map atom->string (cog-outgoing-set matches))))
          (caichat-ask (string-append 
                       "Based on these OpenCog atoms:\n"
                       (string-join match-strs "\n")
                       "\nQuestion: " question))))))

;;
;; GGML Integration Functions
;;

(define* (caichat-setup-ggml model-path #:optional (n-threads 4) (n-ctx 2048))
  "Configure GGML client for local model inference"
  (let ((config-id (caichat-create-client-config "ggml" model-path "" "")))
    (hash-set! caichat-configs "ggml" config-id)
    config-id))

(define* (caichat-ggml-session #:optional (atomspace (cog-atomspace)))
  "Create a new GGML session for local inference"
  (let ((config-id (hash-ref caichat-configs "ggml")))
    (if config-id
        (let ((session-id (caichat-create-session config-id atomspace)))
          (hash-set! caichat-sessions "ggml" session-id)
          session-id)
        (error "GGML not configured. Run (caichat-setup-ggml model-path) first."))))

(define (caichat-ggml-chat message)
  "Chat with GGML model"
  (let ((session-id (hash-ref caichat-sessions "ggml")))
    (if session-id
        (begin
          (caichat-add-message session-id "user" message)
          (caichat-complete session-id))
        (error "No GGML session active. Run (caichat-ggml-session) first."))))

(define (caichat-ggml-ask-with-atom atom question)
  "Ask a question about an atom using GGML cognitive completion"
  (let ((session-id (hash-ref caichat-sessions "ggml")))
    (if session-id
        (begin
          (caichat-add-message session-id "user" 
                             (string-append "Question about atom: " question))
          (caichat-ggml-cognitive-completion session-id atom))
        (error "No GGML session active. Run (caichat-ggml-session) first."))))

(define (caichat-ggml-cognitive-chat message context-atom)
  "Chat with GGML using cognitive architecture context"
  (let ((session-id (hash-ref caichat-sessions "ggml")))
    (if session-id
        (begin
          (caichat-add-message session-id "user" message)
          (caichat-ggml-cognitive-completion session-id context-atom))
        (error "No GGML session active. Run (caichat-ggml-session) first."))))

(define (caichat-ggml-load-local-model model-path)
  "Load a local GGML model"
  (let ((session-id (hash-ref caichat-sessions "ggml")))
    (if session-id
        (caichat-ggml-load-model session-id model-path)
        (error "No GGML session active. Run (caichat-ggml-session) first."))))

(define (caichat-ggml-model-status)
  "Get status of loaded GGML model"
  (let ((session-id (hash-ref caichat-sessions "ggml")))
    (if session-id
        (caichat-ggml-model-info session-id)
        "No GGML session active")))

(define (caichat-neural-symbolic-analysis input)
  "Perform neural-symbolic analysis using the bridge"
  (caichat-neural-symbolic-bridge input))

;; Module initialization
(cog-logger-info "CAIChat OpenCog module loaded successfully")