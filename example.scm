#!/usr/bin/env guile
!#

;;
;; example.scm - Example usage of CAIChat OpenCog module
;;

;; Add current directory to load path
(add-to-load-path ".")

;; Load the CAIChat modules
(use-modules (opencog caichat config)
             (opencog caichat)
             (opencog caichat rag)
             (opencog caichat repl))

(display "╔══════════════════════════════════════════════════════════════╗\n")
(display "║                 CAIChat OpenCog Example                     ║\n")
(display "╚══════════════════════════════════════════════════════════════╝\n\n")

;; Example 1: Basic configuration (without real API keys)
(display "Example 1: Configuration\n")
(display "========================\n")

;; Show how to configure providers (with dummy keys for example)
(display "Setting up OpenAI configuration (example):\n")
(display "  (caichat-setup-openai \"your-api-key-here\")\n\n")

(display "Setting up Claude configuration (example):\n")
(display "  (caichat-setup-claude \"your-api-key-here\")\n\n")

(display "Setting up Ollama configuration (example):\n")
(display "  (caichat-setup-ollama \"http://localhost:11434\")\n\n")

;; Example 2: Document RAG
(display "Example 2: Document RAG\n")
(display "=======================\n")

;; Create a sample knowledge base
(caichat-create-knowledge-base "opencog-docs" "OpenCog documentation")

;; Add some sample documents
(caichat-add-document "opencog-intro" 
                      "OpenCog is a cognitive architecture project aimed at creating beneficial artificial general intelligence. It combines symbolic and subsymbolic AI approaches."
                      '(("source" . "opencog-intro.txt")
                        ("type" . "documentation")))

(caichat-add-document "atomspace-intro"
                      "The AtomSpace is OpenCog's knowledge representation system. It stores knowledge as a hypergraph of typed nodes and links called atoms."
                      '(("source" . "atomspace-intro.txt")
                        ("type" . "documentation")))

(display "✓ Added sample documents to knowledge base\n")

;; Demonstrate search
(let ((search-results (caichat-search-documents "What is AtomSpace?" 2)))
  (display (format #f "✓ Search results for 'What is AtomSpace?': ~a matches\n" 
                   (length search-results)))
  (for-each 
   (lambda (result)
     (display (format #f "  - Document: ~a (similarity: ~a)\n" 
                      (car result) (cadr result))))
   search-results))

(display "\n")

;; Example 3: Embedding similarity
(display "Example 3: Text Embeddings\n")
(display "==========================\n")

(let ((text1 "artificial intelligence")
      (text2 "machine learning")
      (text3 "cooking recipes"))
  
  (let ((emb1 (caichat-embed-text text1))
        (emb2 (caichat-embed-text text2))
        (emb3 (caichat-embed-text text3)))
    
    (display (format #f "Text 1: '~a' -> ~a\n" text1 emb1))
    (display (format #f "Text 2: '~a' -> ~a\n" text2 emb2))
    (display (format #f "Text 3: '~a' -> ~a\n" text3 emb3))
    
    (let ((sim12 (calculate-cosine-similarity emb1 emb2))
          (sim13 (calculate-cosine-similarity emb1 emb3)))
      (display (format #f "Similarity between 1&2: ~a\n" sim12))
      (display (format #f "Similarity between 1&3: ~a\n" sim13))
      (display (format #f "Expected: 1&2 should be more similar than 1&3\n")))))

(display "\n")

;; Example 4: Configuration management
(display "Example 4: Configuration Management\n")
(display "===================================\n")

(display "Available providers: ")
(let ((providers (caichat-list-providers)))
  (display (if (null? providers) "None configured" providers)))
(display "\n")

(display "Default provider: ")
(let ((default (caichat-get-default-provider)))
  (display default))
(display "\n\n")

;; Example 5: Conversation simulation (without actual API calls)
(display "Example 5: Conversation Structure\n")
(display "=================================\n")

(display "This is how you would use CAIChat for conversations:\n\n")

(display "1. Configure a provider:\n")
(display "   (caichat-setup-openai \"your-api-key\")\n\n")

(display "2. Start a session:\n")
(display "   (define session (caichat-start-session \"openai\" \"gpt-3.5-turbo\"))\n\n")

(display "3. Chat:\n")
(display "   (caichat-chat session \"Hello, how are you?\")\n\n")

(display "4. Ask questions about atoms:\n")
(display "   (define my-atom (Concept \"artificial-intelligence\"))\n")
(display "   (caichat-ask-about-atom my-atom \"What are the applications?\")\n\n")

(display "5. Use RAG:\n")
(display "   (caichat-rag-query \"What is OpenCog?\" \"openai\" \"gpt-4\" 3)\n\n")

(display "6. Start interactive REPL:\n")
(display "   (caichat-repl)\n\n")

(display "Example complete! 🎉\n")
(display "\nTo actually use CAIChat with real LLMs:\n")
(display "1. Install OpenCog and required dependencies\n")
(display "2. Build the C++ components with CMake\n")
(display "3. Set up your API keys\n")
(display "4. Start using the functions above!\n")