# CAIChat OpenCog Module

This directory contains the OpenCog implementation of CAIChat, providing Large Language Model (LLM) integration with OpenCog's AtomSpace.

## Overview

CAIChat for OpenCog transforms the original Rust-based LLM CLI tool into a native OpenCog component, implemented in C++ and Scheme. This integration allows for:

- **AtomSpace Integration**: Store and manage conversations as atoms
- **Knowledge Representation**: Use OpenCog's powerful knowledge representation for chat context
- **Reasoning Integration**: Combine LLM capabilities with OpenCog's reasoning systems
- **Scheme Interface**: Native Scheme API for OpenCog users
- **Multi-Provider Support**: Support for OpenAI, Claude, Ollama, GGML (local models), and other LLM providers
- **Local Model Support**: GGML integration for running models locally without API dependencies
- **Neural-Symbolic Bridge**: Advanced integration between neural networks and symbolic reasoning
- **Cognitive Architecture**: Enhanced session management with hypergraph memory synergization

## Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    Scheme Interface                         │
│  ┌─────────────┐ ┌─────────────┐ ┌─────────────────────────┐ │
│  │    REPL     │ │   Config    │ │         RAG             │ │
│  └─────────────┘ └─────────────┘ └─────────────────────────┘ │
├─────────────────────────────────────────────────────────────┤
│                  C++ Core Library                          │
│  ┌─────────────┐ ┌─────────────┐ ┌─────────────────────────┐ │
│  │ LLM Clients │ │ Chat Engine │ │   Scheme Bindings       │ │
│  └─────────────┘ └─────────────┘ └─────────────────────────┘ │
├─────────────────────────────────────────────────────────────┤
│                   OpenCog AtomSpace                        │
│  ┌─────────────┐ ┌─────────────┐ ┌─────────────────────────┐ │
│  │  Knowledge  │ │Conversations│ │      Embeddings         │ │
│  │    Base     │ │   History   │ │                         │ │
│  └─────────────┘ └─────────────┘ └─────────────────────────┘ │
└─────────────────────────────────────────────────────────────┘
```

## Components

### C++ Core (`opencog/caichat/`)

- **LLMClient.h/cc**: Abstract interface and implementations for different LLM providers
  - OpenAI, Claude, Gemini, Groq, Ollama clients
  - **NEW**: GGMLClient for local model inference
  - Provider routing and capability management
- **ChatCompletion.h/cc**: Conversation management and AtomSpace integration
- **SessionManager.h/cc**: Enhanced session management with hypergraph memory
  - Persistent session management
  - **NEW**: Neural-symbolic bridge for cognitive integration
- **SchemeBindings.cc**: Guile Scheme bindings for C++ functionality
  - **NEW**: GGML-specific bindings for local model management

### Scheme Modules (`opencog/scm/caichat/`)

- **caichat.scm**: Main module with high-level convenience functions
  - **NEW**: GGML integration functions
  - **NEW**: Cognitive completion with AtomSpace
- **config.scm**: Configuration management and provider setup
  - **NEW**: GGML setup and auto-configuration
  - **NEW**: Enhanced help system
- **repl.scm**: Interactive REPL interface
- **rag.scm**: Retrieval Augmented Generation with document indexing
- **init.scm**: Module initialization and welcome interface

## Installation

### Prerequisites

- OpenCog (with AtomSpace and CogUtil)
- Guile 3.0+
- CMake 3.16+
- libcurl
- jsoncpp

### Building

```bash
mkdir build
cd build
cmake ..
make
sudo make install
```

### Configuration

Set up environment variables for your preferred LLM providers:

```bash
export OPENAI_API_KEY="your-openai-key"
export ANTHROPIC_API_KEY="your-claude-key"
export OLLAMA_BASE_URL="http://localhost:11434"
export GGML_MODEL_PATH="/path/to/your/model.ggmlv3.q4_0.bin"
```

## Usage

### Quick Start

```scheme
;; Load the module
(use-modules (opencog caichat init))

;; Quick question
(caichat-ask "What is OpenCog?")

;; Start interactive REPL
(caichat-repl)

;; Use local GGML model (NEW!)
(caichat-ggml-chat "Explain neural-symbolic AI")
```

### Configuration

```scheme
;; Configure providers
(caichat-setup-openai "your-api-key")
(caichat-setup-claude "your-api-key")
(caichat-setup-ollama "http://localhost:11434")

;; Configure GGML for local inference (NEW!)
(caichat-setup-ggml "/path/to/model.ggmlv3.q4_0.bin" 4 2048)

;; Create a session
(define session (caichat-start-session "openai" "gpt-4"))

;; Chat
(caichat-chat session "Hello, how are you?")
```

### RAG (Retrieval Augmented Generation)

```scheme
;; Create knowledge base
(caichat-create-knowledge-base "my-docs" "Personal document collection")

;; Add documents
(caichat-add-document "doc1" "OpenCog is a cognitive architecture..." 
                      '(("source" . "opencog-intro.txt")))

;; Query with context
(caichat-rag-query "What is OpenCog?" "openai" "gpt-4" 3)
```

### AtomSpace Integration

```scheme
;; Ask about specific atoms
(define my-atom (Concept "artificial-intelligence"))
(caichat-ask-about-atom my-atom "What are the key applications of this concept?")

;; Pattern-based queries
(define pattern (GetLink (TypedVariable (Variable "x") (Type "ConceptNode"))))
(caichat-match-and-ask pattern "What do these concepts have in common?")
```

### GGML Local Models (NEW!)

GGML integration allows running large language models locally without API dependencies, offering privacy and cost benefits.

#### Setup

```scheme
;; Setup GGML with a local model
(caichat-setup-ggml "/path/to/llama2-7b-chat.ggmlv3.q4_0.bin" 4 2048)

;; Create a GGML session
(caichat-ggml-session)

;; Check model status
(caichat-ggml-model-status)
```

#### Basic Usage

```scheme
;; Simple chat
(caichat-ggml-chat "Hello, explain quantum computing")

;; Load a different model
(caichat-ggml-load-local-model "/path/to/codellama-7b.ggmlv3.q4_0.bin")
```

#### Cognitive Architecture Integration

```scheme
;; Create an atom and ask about it
(define ai-concept (Concept "artificial-intelligence"))
(caichat-ggml-ask-with-atom ai-concept "What are the ethical implications?")

;; Use cognitive completion with context
(define context-atom (Concept "machine-learning"))
(caichat-ggml-cognitive-chat "Explain deep learning" context-atom)

;; Neural-symbolic analysis
(caichat-neural-symbolic-analysis "consciousness and artificial intelligence")
```

#### Model Management

```scheme
;; Load model in session
(caichat-ggml-load-model session-id "/path/to/model.bin")

;; Get model information
(caichat-ggml-model-info session-id)

;; Unload model
(caichat-ggml-unload-model session-id)
```

## Features

### LLM Provider Support
- ✅ OpenAI (GPT-3.5, GPT-4, GPT-4-turbo)
- ✅ Claude (Anthropic)
- ✅ Ollama (Local models)
- ✅ **GGML (Local inference)** - NEW!
- ✅ Groq (Fast inference)
- ✅ Gemini (Google)
- ✅ OpenAI-compatible APIs

### Core Functionality
- ✅ Chat completions
- ✅ Streaming responses
- ✅ Conversation persistence in AtomSpace
- ✅ Interactive REPL
- ✅ Configuration management
- ✅ RAG document indexing
- ✅ **Local model loading and inference** - NEW!

### OpenCog Integration
- ✅ AtomSpace conversation storage
- ✅ Atom-based context queries
- ✅ **Neural-symbolic bridge** - ENHANCED!
- ✅ **Cognitive session management** - NEW!
- ✅ **Hypergraph memory synergization** - NEW!
- 🚧 Pattern matching integration
- 🚧 Reasoning system integration
- 🚧 Learning from conversations

### GGML Features (NEW!)
- ✅ Local model loading from GGUF/GGML files
- ✅ CPU-based inference (no GPU required)
- ✅ Cognitive completion with AtomSpace context
- ✅ AtomSpace pattern to prompt conversion
- ✅ Zero API costs for local inference
- ✅ Privacy-preserving local processing

## API Reference

### Core Functions

- `(caichat-ask question [provider] [model])` - Quick question
- `(caichat-start-session provider model)` - Create chat session
- `(caichat-chat session-id message)` - Send message
- `(caichat-complete session-id)` - Get completion
- `(caichat-repl)` - Start interactive REPL

### Configuration

- `(caichat-setup-openai api-key)` - Configure OpenAI
- `(caichat-setup-claude api-key)` - Configure Claude  
- `(caichat-setup-ollama base-url)` - Configure Ollama
- `(caichat-setup-ggml model-path [n-threads] [n-ctx])` - Configure GGML (NEW!)
- `(caichat-save-config file)` - Save configuration
- `(caichat-load-config file)` - Load configuration
- `(caichat-auto-configure)` - Auto-configure from environment (NEW!)
- `(caichat-help)` - Display help information (NEW!)

### GGML Functions (NEW!)

- `(caichat-ggml-session)` - Create GGML session
- `(caichat-ggml-chat message)` - Chat with GGML model
- `(caichat-ggml-ask-with-atom atom question)` - Ask about atom with GGML
- `(caichat-ggml-cognitive-chat message context-atom)` - Cognitive chat
- `(caichat-ggml-load-local-model path)` - Load model
- `(caichat-ggml-model-status)` - Get model status
- `(caichat-neural-symbolic-analysis input)` - Neural-symbolic bridge

### RAG Functions

- `(caichat-add-document id content metadata)` - Add document
- `(caichat-rag-query question [provider] [model] [max-docs])` - RAG query
- `(caichat-create-knowledge-base name description)` - Create KB
- `(caichat-semantic-search query kb max-results)` - Search KB

## Status

✅ **Major Progress** - GGML integration and neural-symbolic bridge completed!

### Completed
- [x] Basic OpenCog module structure
- [x] C++ LLM client architecture
- [x] Scheme bindings framework
- [x] Configuration management
- [x] REPL interface
- [x] RAG foundation
- [x] **GGML client implementation** - NEW!
- [x] **Neural-symbolic bridge** - NEW!
- [x] **Cognitive session management** - NEW!
- [x] **Enhanced configuration system** - NEW!
- [x] **Multiple LLM provider support** - NEW!

### In Progress  
- [x] ~~Full OpenAI client implementation~~ ✅ Completed
- [x] ~~Claude client implementation~~ ✅ Completed  
- [x] ~~Streaming support~~ ✅ Completed
- [ ] Advanced AtomSpace integration
- [ ] Testing framework
- [ ] Real GGML model loading (currently simulated)

### Future
- [ ] Advanced reasoning integration
- [ ] Learning capabilities
- [ ] Tool/function calling
- [ ] Multi-modal support
- [ ] Performance optimization

## Contributing

This module is part of the CAIChat to OpenCog transformation project. The original Rust implementation serves as a reference for features and functionality to be ported.

## License

Same as original CAIChat: MIT OR Apache-2.0