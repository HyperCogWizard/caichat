;;
;; repl.scm - REPL interface for CAIChat in OpenCog
;;
;; Provides an interactive chat interface that integrates with the 
;; OpenCog Scheme REPL
;;

(define-module (opencog caichat repl))

(use-modules (opencog)
             (opencog caichat)
             (opencog caichat config)
             (ice-9 readline)
             (ice-9 format))

(export caichat-repl
        caichat-interactive
        caichat-help
        caichat-set-provider
        caichat-show-history
        caichat-reset-session)

;; REPL state
(define current-session #f)
(define current-provider "openai")
(define current-model "gpt-3.5-turbo")
(define repl-history '())

;;
;; REPL commands
;;

(define (caichat-help)
  "Show help for CAIChat REPL commands"
  (display "CAIChat OpenCog REPL Commands:\n")
  (display "  (caichat-repl)           - Start interactive chat session\n")
  (display "  (caichat-set-provider p) - Set LLM provider (openai, claude, ollama)\n")
  (display "  (caichat-show-history)   - Show conversation history\n")
  (display "  (caichat-reset-session)  - Start a new session\n")
  (display "  (caichat-help)           - Show this help\n")
  (display "\nIn REPL mode:\n")
  (display "  /help     - Show help\n")
  (display "  /history  - Show history\n")
  (display "  /reset    - Reset session\n")
  (display "  /save <name> - Save conversation\n")
  (display "  /load <name> - Load conversation\n")
  (display "  /provider <name> - Change provider\n")
  (display "  /quit     - Exit REPL\n"))

(define (caichat-set-provider provider . model)
  "Set the current LLM provider and optionally model"
  (set! current-provider provider)
  (when (not (null? model))
    (set! current-model (car model)))
  (format #t "Set provider to ~a with model ~a\n" current-provider current-model)
  (caichat-reset-session))

(define (caichat-show-history)
  "Show the conversation history"
  (if (null? repl-history)
      (display "No conversation history.\n")
      (begin
        (display "Conversation History:\n")
        (display "====================\n")
        (for-each
         (lambda (entry)
           (let ((role (car entry))
                 (content (cadr entry)))
             (format #t "~a: ~a\n\n" 
                     (string-upcase role) content)))
         (reverse repl-history)))))

(define (caichat-reset-session)
  "Reset the current chat session"
  (when current-session
    (caichat-destroy-session current-session))
  (set! current-session (caichat-start-session current-provider current-model))
  (set! repl-history '())
  (format #t "Started new ~a session with model ~a\n" 
          current-provider current-model))

;;
;; REPL implementation
;;

(define (process-repl-command line)
  "Process a REPL command starting with /"
  (let ((parts (string-split line #\space)))
    (let ((command (car parts))
          (args (cdr parts)))
      (case (string->symbol (substring command 1))
        ((help)
         (caichat-help))
        ((history)
         (caichat-show-history))
        ((reset)
         (caichat-reset-session))
        ((save)
         (if (null? args)
             (display "Usage: /save <conversation-name>\n")
             (let ((name (car args)))
               (caichat-save-conversation current-session name)
               (format #t "Saved conversation as '~a'\n" name))))
        ((load)
         (if (null? args)
             (display "Usage: /load <conversation-name>\n")
             (let ((name (car args)))
               (caichat-load-conversation current-session name)
               (format #t "Loaded conversation '~a'\n" name))))
        ((provider)
         (if (null? args)
             (display "Usage: /provider <provider-name> [model-name]\n")
             (if (null? (cdr args))
                 (caichat-set-provider (car args))
                 (caichat-set-provider (car args) (cadr args)))))
        ((quit exit)
         'quit)
        (else
         (format #t "Unknown command: ~a\n" command))))))

(define (caichat-repl)
  "Start the interactive CAIChat REPL"
  
  ;; Initialize session if needed
  (unless current-session
    (caichat-reset-session))
  
  (display "Welcome to CAIChat OpenCog REPL!\n")
  (display "Type /help for commands, /quit to exit.\n")
  (format #t "Current provider: ~a (~a)\n\n" current-provider current-model)
  
  (let loop ()
    (display "caichat> ")
    (force-output)
    (let ((line (read-line)))
      (cond
       ((eof-object? line)
        (display "\nGoodbye!\n"))
       
       ((string-prefix? "/" line)
        (let ((result (process-repl-command line)))
          (unless (eq? result 'quit)
            (loop))))
       
       ((string=? (string-trim line) "")
        (loop))
       
       (else
        ;; Regular chat message
        (set! repl-history (cons (list "user" line) repl-history))
        (display "Thinking...\n")
        (force-output)
        
        (let ((response (caichat-chat current-session line)))
          (if response
              (begin
                (set! repl-history (cons (list "assistant" response) repl-history))
                (format #t "\nAssistant: ~a\n\n" response))
              (display "Error: Failed to get response from LLM\n")))
        
        (loop))))))

(define (caichat-interactive)
  "Alias for caichat-repl"
  (caichat-repl))

;;
;; Integration with OpenCog REPL
;;

(define (caichat-quick-ask question)
  "Quick ask without starting full REPL"
  (let ((response (caichat-ask question current-provider current-model)))
    (format #t "~a\n" response)
    response))

(define (caichat-context-ask question atom)
  "Ask a question with an atom as context"
  (caichat-ask-about-atom atom question))

;;
;; Session management helpers
;;

(define (caichat-list-saved-conversations)
  "List all saved conversations"
  (let ((conversations (caichat-find-conversations)))
    (if (null? conversations)
        (display "No saved conversations found.\n")
        (begin
          (display "Saved Conversations:\n")
          (for-each
           (lambda (conv-atom)
             (let ((name (substring (cog-name conv-atom) 13))) ; Remove "conversation:" prefix
               (format #t "  - ~a\n" name)))
           conversations)))))

(define (caichat-delete-conversation name)
  "Delete a saved conversation"
  (let ((conv-atom (cog-node 'ConceptNode (string-append "conversation:" name))))
    (when conv-atom
      (cog-delete conv-atom)
      (format #t "Deleted conversation '~a'\n" name))))

;; Auto-configure if needed
(when (null? (caichat-list-providers))
  (caichat-auto-configure))