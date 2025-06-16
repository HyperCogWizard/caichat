;;
;; rag.scm - Retrieval Augmented Generation for CAIChat
;;
;; Integrates document retrieval and embedding search with LLM conversations
;; using OpenCog's AtomSpace for knowledge storage
;;

(define-module (opencog caichat rag))

(use-modules (opencog)
             (opencog caichat)
             (opencog nlp)
             (opencog matrix)
             (ice-9 textual-ports)
             (ice-9 hash-table))

(export caichat-add-document
        caichat-search-documents
        caichat-rag-query
        caichat-embed-text
        caichat-create-knowledge-base
        caichat-index-documents
        caichat-semantic-search)

;; Knowledge base storage
(define rag-documents (make-hash-table))
(define document-embeddings (make-hash-table))

;;
;; Document management
;;

(define (caichat-add-document doc-id content metadata)
  "Add a document to the knowledge base"
  (let ((doc-atom (cog-new-node 'ConceptNode (string-append "doc:" doc-id))))
    
    ;; Store document content
    (cog-set-value! doc-atom (cog-new-node 'PredicateNode "content") 
                    (cog-new-string-value content))
    
    ;; Store metadata
    (when metadata
      (for-each 
       (lambda (key-value)
         (let ((key (car key-value))
               (value (cdr key-value)))
           (cog-set-value! doc-atom (cog-new-node 'PredicateNode key)
                          (cog-new-string-value (format #f "~a" value)))))
       metadata))
    
    ;; Add to document index
    (hash-set! rag-documents doc-id doc-atom)
    
    ;; Generate embeddings (placeholder - would use actual embedding model)
    (let ((embedding (caichat-embed-text content)))
      (when embedding
        (hash-set! document-embeddings doc-id embedding)))
    
    doc-atom))

(define (caichat-embed-text text)
  "Generate embeddings for text (placeholder implementation)"
  ;; In a real implementation, this would use the LLM client's embedding function
  ;; For now, return a simple hash-based pseudo-embedding
  (let ((hash-value (string-hash text)))
    (list (modulo hash-value 1000)
          (modulo (quotient hash-value 1000) 1000)
          (modulo (quotient hash-value 1000000) 1000))))

(define (caichat-search-documents query max-results)
  "Search documents using semantic similarity"
  (let ((query-embedding (caichat-embed-text query))
        (results '()))
    
    ;; Simple cosine similarity search (placeholder)
    (hash-for-each
     (lambda (doc-id doc-embedding)
       (let ((similarity (calculate-cosine-similarity query-embedding doc-embedding)))
         (set! results (cons (list doc-id similarity) results))))
     document-embeddings)
    
    ;; Sort by similarity and return top results
    (let ((sorted-results (sort results (lambda (a b) (> (cadr a) (cadr b))))))
      (take sorted-results (min max-results (length sorted-results))))))

(define (calculate-cosine-similarity vec1 vec2)
  "Calculate cosine similarity between two vectors"
  (if (and (list? vec1) (list? vec2) (= (length vec1) (length vec2)))
      (let ((dot-product (apply + (map * vec1 vec2)))
            (magnitude1 (sqrt (apply + (map (lambda (x) (* x x)) vec1))))
            (magnitude2 (sqrt (apply + (map (lambda (x) (* x x)) vec2)))))
        (if (and (> magnitude1 0) (> magnitude2 0))
            (/ dot-product (* magnitude1 magnitude2))
            0))
      0))

;;
;; RAG query processing
;;

(define* (caichat-rag-query question #:optional (provider "openai") (model "gpt-3.5-turbo") (max-docs 3))
  "Perform a RAG query: search documents and generate answer with context"
  
  ;; Search for relevant documents
  (let ((search-results (caichat-search-documents question max-docs)))
    
    (if (null? search-results)
        ;; No relevant documents found, answer without context
        (caichat-ask question provider model)
        
        ;; Build context from relevant documents
        (let ((context-parts '()))
          (for-each
           (lambda (result)
             (let* ((doc-id (car result))
                    (doc-atom (hash-ref rag-documents doc-id))
                    (content (cog-value-ref (cog-value doc-atom 
                                                       (cog-new-node 'PredicateNode "content")) 0)))
               (set! context-parts (cons content context-parts))))
           search-results)
          
          ;; Create augmented prompt
          (let ((augmented-question
                 (string-append
                  "Based on the following context information, please answer the question.\n\n"
                  "Context:\n"
                  (string-join (reverse context-parts) "\n\n")
                  "\n\nQuestion: " question
                  "\n\nAnswer based on the provided context:")))
            
            (caichat-ask augmented-question provider model))))))

;;
;; Knowledge base management
;;

(define (caichat-create-knowledge-base name description)
  "Create a new knowledge base"
  (let ((kb-atom (cog-new-node 'ConceptNode (string-append "kb:" name))))
    
    ;; Store description
    (cog-set-value! kb-atom (cog-new-node 'PredicateNode "description")
                    (cog-new-string-value description))
    
    ;; Initialize document list
    (cog-set-value! kb-atom (cog-new-node 'PredicateNode "documents")
                    (cog-new-link-value 'ListLink '()))
    
    kb-atom))

(define (caichat-index-documents file-paths knowledge-base)
  "Index multiple documents into a knowledge base"
  (let ((doc-count 0))
    (for-each
     (lambda (file-path)
       (when (file-exists? file-path)
         (let* ((content (call-with-input-file file-path get-string-all))
                (doc-id (string-append knowledge-base ":" (basename file-path)))
                (metadata `(("source" . ,file-path)
                           ("kb" . ,knowledge-base)
                           ("indexed-at" . ,(current-time)))))
           
           (caichat-add-document doc-id content metadata)
           (set! doc-count (+ doc-count 1))
           
           ;; Link document to knowledge base
           (let ((kb-atom (cog-node 'ConceptNode (string-append "kb:" knowledge-base)))
                 (doc-atom (hash-ref rag-documents doc-id)))
             (when (and kb-atom doc-atom)
               (cog-new-link 'MemberLink doc-atom kb-atom))))))
     file-paths)
    
    (format #t "Indexed ~a documents into knowledge base '~a'\n" 
            doc-count knowledge-base)
    doc-count))

;;
;; Advanced RAG features
;;

(define (caichat-semantic-search query knowledge-base max-results)
  "Perform semantic search within a specific knowledge base"
  (let ((kb-atom (cog-node 'ConceptNode (string-append "kb:" knowledge-base))))
    (if kb-atom
        ;; Find all documents in this knowledge base
        (let ((kb-docs '()))
          (cog-map-type
           (lambda (member-link)
             (let ((doc-atom (cog-outgoing-atom member-link 0)))
               (when (string-prefix? "doc:" (cog-name doc-atom))
                 (set! kb-docs (cons doc-atom kb-docs))))
             #f)
           'MemberLink)
          
          ;; Search among KB documents only
          (let ((filtered-results '()))
            (for-each
             (lambda (doc-atom)
               (let* ((doc-id (substring (cog-name doc-atom) 4)) ; Remove "doc:" prefix
                      (embedding (hash-ref document-embeddings doc-id)))
                 (when embedding
                   (let ((similarity (calculate-cosine-similarity 
                                     (caichat-embed-text query) embedding)))
                     (set! filtered-results 
                           (cons (list doc-id similarity) filtered-results))))))
             kb-docs)
            
            ;; Sort and return top results
            (let ((sorted-results (sort filtered-results 
                                       (lambda (a b) (> (cadr a) (cadr b))))))
              (take sorted-results (min max-results (length sorted-results))))))
        
        ;; Knowledge base not found
        '())))

(define (caichat-rag-conversation session-id knowledge-base)
  "Start a conversation with RAG support for a specific knowledge base"
  ;; This would wrap the normal chat completion to include RAG context
  ;; For now, return a helper function
  (lambda (question)
    (let ((rag-answer (caichat-rag-query question "openai" "gpt-3.5-turbo" 3)))
      (caichat-add-message session-id "user" question)
      (caichat-add-message session-id "assistant" rag-answer)
      rag-answer)))

;;
;; Document chunking and processing
;;

(define (caichat-chunk-document content chunk-size overlap)
  "Split a document into chunks for better retrieval"
  (let ((chunks '())
        (start 0)
        (content-length (string-length content)))
    
    (while (< start content-length)
      (let ((end (min (+ start chunk-size) content-length)))
        (set! chunks (cons (substring content start end) chunks))
        (set! start (- end overlap))))
    
    (reverse chunks)))

(define (caichat-add-chunked-document doc-id content metadata chunk-size overlap)
  "Add a document by splitting it into chunks"
  (let ((chunks (caichat-chunk-document content chunk-size overlap))
        (chunk-count 0))
    
    (for-each
     (lambda (chunk)
       (let ((chunk-id (string-append doc-id ":chunk:" (number->string chunk-count)))
             (chunk-metadata (append metadata `(("chunk-id" . ,chunk-count)
                                                ("parent-doc" . ,doc-id)))))
         (caichat-add-document chunk-id chunk chunk-metadata)
         (set! chunk-count (+ chunk-count 1))))
     chunks)
    
    chunk-count))

;; Initialize with some basic knowledge
(cog-logger-info "CAIChat RAG module loaded")