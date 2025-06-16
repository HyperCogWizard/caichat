# CAIChat OpenCog Module

This directory contains the OpenCog implementation of CAIChat, providing Large Language Model (LLM) integration with OpenCog's AtomSpace.

## Overview

CAIChat for OpenCog transforms the original Rust-based LLM CLI tool into a native OpenCog component, implemented in C++ and Scheme. This integration allows for:

- **AtomSpace Integration**: Store and manage conversations as atoms
- **Knowledge Representation**: Use OpenCog's powerful knowledge representation for chat context
- **Reasoning Integration**: Combine LLM capabilities with OpenCog's reasoning systems
- **Scheme Interface**: Native Scheme API for OpenCog users
- **Multi-Provider Support**: Support for OpenAI, Claude, Ollama, and other LLM providers

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
- **ChatCompletion.h/cc**: Conversation management and AtomSpace integration
- **SchemeBindings.cc**: Guile Scheme bindings for C++ functionality

### Scheme Modules (`opencog/scm/caichat/`)

- **caichat.scm**: Main module with high-level convenience functions
- **config.scm**: Configuration management and provider setup
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
```

### Configuration

```scheme
;; Configure providers
(caichat-setup-openai "your-api-key")
(caichat-setup-claude "your-api-key")
(caichat-setup-ollama "http://localhost:11434")

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

## Features

### LLM Provider Support
- ✅ OpenAI (GPT-3.5, GPT-4, GPT-4-turbo)
- 🚧 Claude (Anthropic)
- 🚧 Ollama (Local models)
- 🚧 OpenAI-compatible APIs

### Core Functionality
- ✅ Chat completions
- 🚧 Streaming responses
- ✅ Conversation persistence in AtomSpace
- ✅ Interactive REPL
- ✅ Configuration management
- ✅ RAG document indexing

### OpenCog Integration
- ✅ AtomSpace conversation storage
- ✅ Atom-based context queries
- 🚧 Pattern matching integration
- 🚧 Reasoning system integration
- 🚧 Learning from conversations

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
- `(caichat-save-config file)` - Save configuration
- `(caichat-load-config file)` - Load configuration

### RAG Functions

- `(caichat-add-document id content metadata)` - Add document
- `(caichat-rag-query question [provider] [model] [max-docs])` - RAG query
- `(caichat-create-knowledge-base name description)` - Create KB
- `(caichat-semantic-search query kb max-results)` - Search KB

## Status

🚧 **Work in Progress** - This is an active transformation of the original Rust codebase.

### Completed
- [x] Basic OpenCog module structure
- [x] C++ LLM client architecture
- [x] Scheme bindings framework
- [x] Configuration management
- [x] REPL interface
- [x] RAG foundation

### In Progress  
- [ ] Full OpenAI client implementation
- [ ] Claude client implementation
- [ ] Streaming support
- [ ] Advanced AtomSpace integration
- [ ] Testing framework

### Future
- [ ] Reasoning integration
- [ ] Learning capabilities
- [ ] Tool/function calling
- [ ] Multi-modal support

## Contributing

This module is part of the CAIChat to OpenCog transformation project. The original Rust implementation serves as a reference for features and functionality to be ported.

## License

Same as original CAIChat: MIT OR Apache-2.0