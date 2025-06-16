# CAIChat to OpenCog Transformation

This repository contains the transformation of the original CAIChat Rust-based LLM CLI tool into an OpenCog component implemented in Scheme and C++.

## Overview

CAIChat was originally a sophisticated all-in-one LLM CLI tool written in Rust. This project transforms it into a native OpenCog component that integrates Large Language Models with OpenCog's cognitive architecture and AtomSpace knowledge representation.

## Architecture

### Original (Rust)
```
┌─────────────────────────────────────────────────────────┐
│                    Rust CLI Tool                       │
│  ┌─────────────┐ ┌─────────────┐ ┌─────────────────────┐ │
│  │   Clients   │ │    REPL     │ │        RAG          │ │
│  └─────────────┘ └─────────────┘ └─────────────────────┘ │
│  ┌─────────────┐ ┌─────────────┐ ┌─────────────────────┐ │
│  │   Config    │ │  Rendering  │ │      Serving        │ │
│  └─────────────┘ └─────────────┘ └─────────────────────┘ │
└─────────────────────────────────────────────────────────┘
```

### Transformed (OpenCog)
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

## Project Structure

```
caichat/
├── src/                    # Original Rust implementation (preserved)
├── opencog/                # New OpenCog implementation
│   ├── caichat/           # C++ core library
│   │   ├── LLMClient.h    # LLM provider interfaces
│   │   ├── LLMClient.cc   # Client implementations
│   │   ├── ChatCompletion.h # Conversation management
│   │   ├── ChatCompletion.cc
│   │   └── SchemeBindings.cc # Guile Scheme bindings
│   ├── scm/caichat/       # Scheme modules
│   │   ├── caichat.scm    # Main interface
│   │   ├── config.scm     # Configuration management
│   │   ├── repl.scm       # Interactive REPL
│   │   ├── rag.scm        # Retrieval Augmented Generation
│   │   └── init.scm       # Module initialization
│   └── README.md          # OpenCog-specific documentation
├── CMakeLists.txt         # CMake build system
├── example.scm            # Usage examples
└── test-caichat.scm       # Basic tests
```

## Key Features Transformed

### ✅ Completed
- **OpenCog Module Structure**: CMake build system and module organization
- **C++ LLM Clients**: Abstract interfaces for OpenAI, Claude, and other providers
- **Scheme Interface**: Native Scheme API for OpenCog users
- **AtomSpace Integration**: Store conversations and knowledge as atoms
- **Configuration Management**: Scheme-based configuration with environment variable support
- **RAG System**: Document indexing and semantic search using AtomSpace
- **Interactive REPL**: OpenCog-integrated chat interface
- **Knowledge Representation**: Convert conversations to/from atom representations

### 🚧 In Progress
- **Full Provider Implementations**: Complete OpenAI and Claude client implementations
- **Streaming Support**: Server-sent events for real-time responses
- **Advanced AtomSpace Features**: Pattern matching and reasoning integration
- **Tool/Function Calling**: Integration with OpenCog's function system

### 📋 Planned
- **Cognitive Architecture Integration**: Connect LLMs with OpenCog reasoning
- **Learning Capabilities**: Learn from conversations and improve responses
- **Multi-modal Support**: Handle images, audio, and other media types
- **Distributed Processing**: Scale across multiple OpenCog instances

## Installation

### Prerequisites

```bash
# Ubuntu/Debian
sudo apt-get install cmake build-essential libcurl4-openssl-dev libjsoncpp-dev

# For full OpenCog integration
sudo apt-get install opencog-dev guile-3.0-dev

# Or minimal build (without OpenCog)
sudo apt-get install guile-3.0-dev
```

### Building

```bash
mkdir build
cd build
cmake ..
make
sudo make install
```

### Configuration

Set up your LLM providers:

```bash
export OPENAI_API_KEY="your-openai-key"
export ANTHROPIC_API_KEY="your-claude-key"
export OLLAMA_BASE_URL="http://localhost:11434"
```

## Usage

### OpenCog Integration

```scheme
;; Load the module in OpenCog
(use-modules (opencog caichat init))

;; Quick question
(caichat-ask "What is cognitive architecture?")

;; Start interactive chat
(caichat-repl)

;; Work with atoms
(define ai-concept (Concept "artificial-intelligence"))
(caichat-ask-about-atom ai-concept "What are the key challenges?")
```

### RAG with AtomSpace

```scheme
;; Create knowledge base
(caichat-create-knowledge-base "research" "AI research papers")

;; Add documents
(caichat-add-document "paper1" "Content of research paper..." 
                      '(("author" . "Smith et al.")
                        ("year" . "2024")))

;; Query with context
(caichat-rag-query "What are the latest developments in AGI?")
```

### Configuration Management

```scheme
;; Set up providers
(caichat-setup-openai "your-api-key")
(caichat-setup-claude "your-api-key")

;; Save configuration
(caichat-save-config "~/.caichat/config.scm")

;; Load configuration
(caichat-load-config "~/.caichat/config.scm")
```

## Migration from Original

The original Rust implementation is preserved in `src/` and serves as a reference. Key differences:

### API Changes
- **Rust**: Command-line interface with YAML configuration
- **OpenCog**: Scheme API with AtomSpace integration

### Data Storage
- **Rust**: File-based sessions and configuration
- **OpenCog**: AtomSpace-based knowledge representation

### Extensibility
- **Rust**: Rust plugins and extensions
- **OpenCog**: Scheme modules and cognitive architecture integration

## Compatibility

- **Build System**: CMake replaces Cargo
- **Dependencies**: C++/Scheme replace Rust ecosystem
- **Configuration**: Scheme S-expressions replace YAML
- **Extensions**: OpenCog modules replace Rust crates

## Development Status

This is an active transformation project. The original Rust functionality is being incrementally ported to OpenCog while adding new cognitive architecture capabilities.

### Current Phase: Foundation ✅
- Basic module structure
- Core C++ interfaces
- Scheme bindings framework
- Configuration system

### Next Phase: Integration 🚧
- Complete LLM client implementations
- Advanced AtomSpace features
- Reasoning integration
- Performance optimization

### Future Phase: Cognition 📋
- Learning from conversations
- Automated knowledge extraction
- Multi-agent coordination
- AGI research applications

## Contributing

This project bridges two major AI ecosystems:
1. **LLM Tools** (represented by the original CAIChat)
2. **Cognitive Architecture** (OpenCog)

Contributions are welcome in:
- Porting Rust functionality to C++/Scheme
- Enhancing OpenCog integration
- Adding new cognitive capabilities
- Improving documentation and examples

## License

Maintains the same dual license as the original:
- MIT License
- Apache License 2.0

## References

- [Original CAIChat](https://github.com/sigoden/aichat) - Rust-based LLM CLI tool
- [OpenCog](https://opencog.org/) - Cognitive architecture framework
- [AtomSpace](https://wiki.opencog.org/w/AtomSpace) - Knowledge representation system
- [Guile Scheme](https://www.gnu.org/software/guile/) - Scheme implementation for extensions