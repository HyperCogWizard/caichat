# Implementing AIChat in C++: A Comprehensive Guide

This comprehensive guide provides detailed instructions for implementing and extending AIChat functionality in C++, covering architecture, design patterns, integration with OpenCog, and best practices for building robust LLM-powered applications.

## Table of Contents

1. [Overview](#overview)
2. [Architecture](#architecture)
3. [Core Components](#core-components)
4. [Implementation Patterns](#implementation-patterns)
5. [Step-by-Step Implementation](#step-by-step-implementation)
6. [OpenCog Integration](#opencog-integration)
7. [Build System and Dependencies](#build-system-and-dependencies)
8. [Testing and Debugging](#testing-and-debugging)
9. [Performance Optimization](#performance-optimization)
10. [Extension and Customization](#extension-and-customization)
11. [Best Practices](#best-practices)
12. [Troubleshooting](#troubleshooting)

## Overview

AIChat's C++ implementation provides a high-performance, modular architecture for integrating Large Language Models (LLMs) with OpenCog's cognitive architecture. This implementation transforms the original Rust-based CLI tool into a native OpenCog component while maintaining compatibility and extending capabilities.

### Key Benefits

- **High Performance**: C++ implementation with optimized memory management
- **OpenCog Integration**: Native AtomSpace integration for knowledge representation
- **Modular Design**: Plugin-based architecture for easy extension
- **Multiple Providers**: Support for OpenAI, Claude, Gemini, Ollama, GGML, and more
- **Neural-Symbolic Bridge**: Advanced integration between neural and symbolic reasoning
- **Scheme Interface**: Complete Guile Scheme bindings for OpenCog users

### Architecture Goals

1. **Modularity**: Clear separation of concerns with well-defined interfaces
2. **Extensibility**: Easy addition of new LLM providers and features
3. **Performance**: Efficient memory usage and fast response times
4. **Integration**: Seamless OpenCog AtomSpace integration
5. **Flexibility**: Support for both OpenCog and standalone deployments

## Architecture

### High-Level Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    Scheme Interface Layer                   │
│  ┌─────────────┐ ┌─────────────┐ ┌─────────────────────────┐ │
│  │    REPL     │ │   Config    │ │         RAG             │ │
│  │  Interface  │ │ Management  │ │      System             │ │
│  └─────────────┘ └─────────────┘ └─────────────────────────┘ │
├─────────────────────────────────────────────────────────────┤
│                  C++ Core Library                          │
│  ┌─────────────┐ ┌─────────────┐ ┌─────────────────────────┐ │
│  │ LLM Client  │ │ Chat Engine │ │   Session Manager       │ │
│  │  Providers  │ │   Core      │ │   & N-S Bridge          │ │
│  └─────────────┘ └─────────────┘ └─────────────────────────┘ │
├─────────────────────────────────────────────────────────────┤
│                   OpenCog AtomSpace                        │
│  ┌─────────────┐ ┌─────────────┐ ┌─────────────────────────┐ │
│  │  Knowledge  │ │Conversations│ │     Embeddings &        │ │
│  │    Base     │ │   History   │ │    Relationships        │ │
│  └─────────────┘ └─────────────┘ └─────────────────────────┘ │
└─────────────────────────────────────────────────────────────┘
```

### Component Interaction

```cpp
// Simplified component interaction diagram
LLMClient <---> ChatCompletion <---> SessionManager
    |               |                      |
    |               |                      |
    v               v                      v
Provider       AtomSpace              Neural-Symbolic
Routing        Integration                Bridge
```

## Core Components

### 1. LLMClient Architecture

The `LLMClient` system provides a unified interface for multiple LLM providers through an abstract base class and concrete implementations.

#### Base Interface

```cpp
namespace opencog {
namespace caichat {

class LLMClient {
public:
    virtual ~LLMClient() = default;
    
    // Core completion interface
    virtual std::string complete(const std::vector<Message>& messages) = 0;
    virtual void complete_stream(const std::vector<Message>& messages, 
                               std::function<void(const std::string&)> callback) = 0;
    
    // Provider information
    virtual std::string get_provider_name() const = 0;
    virtual std::vector<std::string> get_supported_models() const = 0;
    virtual bool supports_streaming() const = 0;
    
    // Configuration
    virtual void set_model(const std::string& model) = 0;
    virtual void set_temperature(double temperature) = 0;
    virtual void set_max_tokens(int max_tokens) = 0;
};

} // namespace caichat
} // namespace opencog
```

#### Implementation Pattern

Each provider follows a consistent implementation pattern:

```cpp
class OpenAIClient : public LLMClient {
private:
    std::string api_key_;
    std::string model_;
    double temperature_;
    int max_tokens_;
    std::string base_url_;
    
public:
    OpenAIClient(const ClientConfig& config);
    
    std::string complete(const std::vector<Message>& messages) override;
    void complete_stream(const std::vector<Message>& messages, 
                        std::function<void(const std::string&)> callback) override;
    
    // Provider-specific methods
    void set_api_key(const std::string& api_key);
    void set_base_url(const std::string& base_url);
};
```

### 2. ChatCompletion Engine

The `ChatCompletion` class manages conversation state and coordinates between LLM providers and AtomSpace storage.

#### Core Functionality

```cpp
class ChatCompletion {
private:
    AtomSpace* atomspace_;
    std::unique_ptr<LLMClient> client_;
    std::vector<Message> conversation_;
    std::string conversation_id_;
    
public:
    ChatCompletion(AtomSpace* atomspace, std::unique_ptr<LLMClient> client);
    
    // Message management
    void add_message(const std::string& role, const std::string& content);
    std::string complete();
    void complete_stream(std::function<void(const std::string&)> callback);
    
    // Conversation management
    void clear_history();
    std::vector<Message> get_messages();
    
    // AtomSpace integration
    HandleSeq get_conversation_atoms();
    void load_conversation(const HandleSeq& conversation_atoms);
    Handle save_conversation(const std::string& conversation_id);
    void load_conversation_by_id(const std::string& conversation_id);
};
```

#### AtomSpace Integration Pattern

```cpp
void ChatCompletion::add_message(const std::string& role, const std::string& content) {
    conversation_.emplace_back(role, content);
    
#ifdef HAVE_OPENCOG
    if (atomspace_) {
        // Store in AtomSpace
        Handle message_atom = message_to_atom(atomspace_, Message(role, content));
        
        // Link to current conversation
        Handle conversation_node = atomspace_->add_node(CONCEPT_NODE, 
                                                       "conversation:" + conversation_id_);
        atomspace_->add_link(MEMBER_LINK, message_atom, conversation_node);
    }
#endif
}
```

### 3. SessionManager and Neural-Symbolic Bridge

The `SessionManager` provides advanced session management with hypergraph memory synergization, while the `NeuralSymbolicBridge` enables bidirectional conversion between neural (LLM) and symbolic (AtomSpace) representations.

#### SessionManager Architecture

```cpp
class SessionManager {
private:
    AtomSpace* atomspace_;
    std::map<std::string, std::unique_ptr<ChatCompletion>> sessions_;
    std::unique_ptr<NeuralSymbolicBridge> bridge_;
    
public:
    SessionManager(AtomSpace* atomspace = nullptr);
    
    // Session lifecycle
    std::string create_session(const std::string& provider, const std::string& model);
    void destroy_session(const std::string& session_id);
    bool session_exists(const std::string& session_id);
    
    // Session operations
    std::string chat(const std::string& session_id, const std::string& message);
    void stream_chat(const std::string& session_id, const std::string& message,
                    std::function<void(const std::string&)> callback);
    
    // Persistence
    void save_session(const std::string& session_id);
    void load_session(const std::string& session_id);
    void cleanup_inactive_sessions();
    
    // Provider routing
    std::vector<std::string> get_available_providers(const std::string& task_type = "chat");
    std::string route_request(const std::vector<Message>& messages,
                             const std::string& preferred_provider = "",
                             const std::string& task_type = "chat");
};
```

#### Neural-Symbolic Bridge

```cpp
class NeuralSymbolicBridge {
private:
    AtomSpace* atomspace_;
    
public:
    NeuralSymbolicBridge(AtomSpace* atomspace = nullptr);
    
    // Core bridge functionality
    Handle llm_to_atomspace(const std::string& response, const std::string& context = "");
    std::string atomspace_to_llm_query(Handle pattern_atom);
    std::string neural_symbolic_bridge(const std::string& input);
    
    // Concept extraction and analysis
    HandleSeq extract_concepts(const std::string& text);
    Handle create_concept_relationships(const HandleSeq& concepts, 
                                      const std::string& relation_type = "related_to");
    std::string build_cognitive_context(const HandleSeq& concepts);
    
private:
    std::vector<std::string> extract_entities(const std::string& text);
    std::string infer_relationship(const std::string& entity1, 
                                  const std::string& entity2, 
                                  const std::string& context);
};
```

## Implementation Patterns

### 1. RAII and Smart Pointers

```cpp
// Use RAII for resource management
class LLMSession {
private:
    std::unique_ptr<LLMClient> client_;
    std::unique_ptr<ChatCompletion> completion_;
    
public:
    LLMSession(std::unique_ptr<LLMClient> client, AtomSpace* atomspace)
        : client_(std::move(client))
        , completion_(std::make_unique<ChatCompletion>(atomspace, std::move(client_))) {
    }
    
    // Automatic cleanup through destructors
    ~LLMSession() = default;
};
```

### 2. Factory Pattern for Providers

```cpp
std::unique_ptr<LLMClient> create_client(const ClientConfig& config) {
    if (config.provider == "openai") {
        return std::make_unique<OpenAIClient>(config);
    } else if (config.provider == "claude") {
        return std::make_unique<ClaudeClient>(config);
    } else if (config.provider == "ollama") {
        return std::make_unique<OllamaClient>(config);
    } else if (config.provider == "ggml") {
        return std::make_unique<GGMLClient>(config);
    }
    
    throw std::invalid_argument("Unknown provider: " + config.provider);
}
```

### 3. Optional OpenCog Integration

```cpp
// Conditional compilation for OpenCog features
#ifdef HAVE_OPENCOG
    Handle message_atom = atomspace_->add_node(CONCEPT_NODE, content);
    return message_atom;
#else
    // Minimal implementation for non-OpenCog builds
    return Handle_UNDEFINED;
#endif
```

### 4. Error Handling Strategy

```cpp
class LLMException : public std::exception {
private:
    std::string message_;
    
public:
    explicit LLMException(const std::string& message) : message_(message) {}
    const char* what() const noexcept override { return message_.c_str(); }
};

// Usage in implementations
std::string OpenAIClient::complete(const std::vector<Message>& messages) {
    try {
        // Implementation
        return response;
    } catch (const std::exception& e) {
        throw LLMException("OpenAI completion failed: " + std::string(e.what()));
    }
}
```

## Step-by-Step Implementation

### 1. Setting Up the Development Environment

#### Prerequisites Installation

```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install -y \
    cmake \
    build-essential \
    libcurl4-openssl-dev \
    libjsoncpp-dev \
    pkg-config

# For OpenCog integration (optional)
sudo apt-get install -y \
    opencog-dev \
    guile-3.0-dev \
    libatomspace-dev \
    libcogutil-dev

# For minimal build (without OpenCog)
sudo apt-get install -y guile-3.0-dev
```

#### Project Structure Setup

```bash
mkdir caichat-cpp && cd caichat-cpp
mkdir -p opencog/caichat
mkdir -p opencog/scm/caichat
mkdir build
mkdir tests
```

### 2. Implementing the LLMClient Base Class

#### Create the Base Interface

```cpp
// opencog/caichat/LLMClient.h
#ifndef _OPENCOG_CAICHAT_LLM_CLIENT_H
#define _OPENCOG_CAICHAT_LLM_CLIENT_H

#include <string>
#include <vector>
#include <functional>
#include <memory>

// OpenCog includes (conditional)
#ifdef HAVE_OPENCOG
#include <opencog/atomspace/AtomSpace.h>
#include <opencog/atoms/base/Handle.h>
using namespace opencog;
#else
// Minimal definitions for non-OpenCog builds
class AtomSpace;
typedef void* Handle;
typedef std::vector<Handle> HandleSeq;
#define Handle_UNDEFINED nullptr
#endif

namespace opencog {
namespace caichat {

// Message structure
struct Message {
    std::string role;
    std::string content;
    
    Message(const std::string& r, const std::string& c) : role(r), content(c) {}
};

// Client configuration
struct ClientConfig {
    std::string provider;
    std::string model;
    std::string api_key;
    std::string base_url;
    double temperature = 0.7;
    int max_tokens = 1000;
    bool stream = false;
};

// Base LLM client interface
class LLMClient {
public:
    virtual ~LLMClient() = default;
    
    // Core functionality
    virtual std::string complete(const std::vector<Message>& messages) = 0;
    virtual void complete_stream(const std::vector<Message>& messages, 
                               std::function<void(const std::string&)> callback) = 0;
    
    // Provider information
    virtual std::string get_provider_name() const = 0;
    virtual std::vector<std::string> get_supported_models() const = 0;
    virtual bool supports_streaming() const = 0;
    virtual bool supports_embeddings() const { return false; }
    virtual bool supports_functions() const { return false; }
    
    // Configuration
    virtual void set_model(const std::string& model) = 0;
    virtual void set_temperature(double temperature) = 0;
    virtual void set_max_tokens(int max_tokens) = 0;
    virtual void set_api_key(const std::string& api_key) = 0;
    
    // Health check
    virtual bool is_healthy() const { return true; }
};

// Factory function
std::unique_ptr<LLMClient> create_client(const ClientConfig& config);

// Utility functions
Message atom_to_message(Handle message_atom);
Handle message_to_atom(AtomSpace* as, const Message& message);

} // namespace caichat
} // namespace opencog

#endif // _OPENCOG_CAICHAT_LLM_CLIENT_H
```

### 3. Implementing OpenAI Client

#### OpenAI Client Implementation

```cpp
// opencog/caichat/OpenAIClient.cc
#include "LLMClient.h"
#include <curl/curl.h>
#include <json/json.h>
#include <sstream>
#include <iostream>

namespace opencog {
namespace caichat {

class OpenAIClient : public LLMClient {
private:
    std::string api_key_;
    std::string model_;
    std::string base_url_;
    double temperature_;
    int max_tokens_;
    
    // HTTP response structure
    struct HTTPResponse {
        std::string data;
        long response_code;
    };
    
    // CURL callback for response data
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, HTTPResponse* response) {
        size_t totalSize = size * nmemb;
        response->data.append((char*)contents, totalSize);
        return totalSize;
    }
    
    // Build JSON request for OpenAI API
    Json::Value build_request_json(const std::vector<Message>& messages) {
        Json::Value request;
        request["model"] = model_;
        request["temperature"] = temperature_;
        request["max_tokens"] = max_tokens_;
        
        Json::Value messages_array(Json::arrayValue);
        for (const auto& msg : messages) {
            Json::Value message;
            message["role"] = msg.role;
            message["content"] = msg.content;
            messages_array.append(message);
        }
        request["messages"] = messages_array;
        
        return request;
    }
    
    // Make HTTP request to OpenAI API
    HTTPResponse make_request(const std::string& json_data, bool stream = false) {
        CURL* curl;
        CURLcode res;
        HTTPResponse response;
        
        curl = curl_easy_init();
        if (curl) {
            // Set URL
            std::string url = base_url_ + "/v1/chat/completions";
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            
            // Set headers
            struct curl_slist* headers = nullptr;
            std::string auth_header = "Authorization: Bearer " + api_key_;
            headers = curl_slist_append(headers, "Content-Type: application/json");
            headers = curl_slist_append(headers, auth_header.c_str());
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            
            // Set POST data
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data.c_str());
            
            // Set callback
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
            
            // Perform request
            res = curl_easy_perform(curl);
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response.response_code);
            
            // Cleanup
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
        }
        
        return response;
    }
    
public:
    OpenAIClient(const ClientConfig& config) 
        : api_key_(config.api_key)
        , model_(config.model)
        , base_url_(config.base_url.empty() ? "https://api.openai.com" : config.base_url)
        , temperature_(config.temperature)
        , max_tokens_(config.max_tokens) {
    }
    
    std::string complete(const std::vector<Message>& messages) override {
        Json::Value request = build_request_json(messages);
        
        Json::StreamWriterBuilder builder;
        std::string json_string = Json::writeString(builder, request);
        
        HTTPResponse response = make_request(json_string);
        
        if (response.response_code != 200) {
            throw LLMException("OpenAI API request failed with code: " + 
                             std::to_string(response.response_code));
        }
        
        // Parse response
        Json::Value root;
        Json::Reader reader;
        if (!reader.parse(response.data, root)) {
            throw LLMException("Failed to parse OpenAI response JSON");
        }
        
        if (root.isMember("error")) {
            throw LLMException("OpenAI API error: " + root["error"]["message"].asString());
        }
        
        if (!root.isMember("choices") || root["choices"].empty()) {
            throw LLMException("No choices in OpenAI response");
        }
        
        return root["choices"][0]["message"]["content"].asString();
    }
    
    void complete_stream(const std::vector<Message>& messages, 
                        std::function<void(const std::string&)> callback) override {
        Json::Value request = build_request_json(messages);
        request["stream"] = true;
        
        Json::StreamWriterBuilder builder;
        std::string json_string = Json::writeString(builder, request);
        
        // For simplicity, this implementation uses the non-streaming API
        // A full implementation would handle Server-Sent Events
        std::string response = complete(messages);
        callback(response);
    }
    
    std::string get_provider_name() const override {
        return "openai";
    }
    
    std::vector<std::string> get_supported_models() const override {
        return {
            "gpt-4", "gpt-4-turbo", "gpt-4o", "gpt-4o-mini",
            "gpt-3.5-turbo", "gpt-3.5-turbo-16k"
        };
    }
    
    bool supports_streaming() const override { return true; }
    bool supports_embeddings() const override { return true; }
    bool supports_functions() const override { return true; }
    
    void set_model(const std::string& model) override { model_ = model; }
    void set_temperature(double temperature) override { temperature_ = temperature; }
    void set_max_tokens(int max_tokens) override { max_tokens_ = max_tokens; }
    void set_api_key(const std::string& api_key) override { api_key_ = api_key; }
};

} // namespace caichat
} // namespace opencog
```

### 4. Implementing ChatCompletion

```cpp
// opencog/caichat/ChatCompletion.cc
#include "ChatCompletion.h"
#include "LLMClient.h"

#ifdef HAVE_OPENCOG
#include <opencog/atoms/base/Node.h>
#include <opencog/atoms/base/Link.h>
#include <opencog/util/Logger.h>
#include <opencog/util/uuid.h>
using namespace opencog;
#else
// Minimal definitions for non-OpenCog builds
#include <iostream>
#include <random>
#include <iomanip>
#include <sstream>
namespace opencog {
    // Minimal logger implementation
    struct Logger {
        void info(const char* fmt, ...) { /* stub */ }
        void error(const char* fmt, ...) { /* stub */ }
    };
    Logger& logger() {
        static Logger instance;
        return instance;
    }
    
    // UUID generation for non-OpenCog builds
    std::string uuid() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 15);
        
        std::stringstream ss;
        ss << std::hex;
        for (int i = 0; i < 8; i++) {
            ss << dis(gen);
        }
        ss << "-";
        for (int i = 0; i < 4; i++) {
            ss << dis(gen);
        }
        ss << "-4";
        for (int i = 0; i < 3; i++) {
            ss << dis(gen);
        }
        ss << "-";
        ss << (8 + (dis(gen) & 3));
        for (int i = 0; i < 3; i++) {
            ss << dis(gen);
        }
        ss << "-";
        for (int i = 0; i < 12; i++) {
            ss << dis(gen);
        }
        
        return ss.str();
    }
}
#endif

using namespace opencog;
using namespace opencog::caichat;

ChatCompletion::ChatCompletion(AtomSpace* atomspace, std::unique_ptr<LLMClient> client)
    : atomspace_(atomspace), client_(std::move(client)) {
    conversation_id_ = uuid();
    
#ifdef HAVE_OPENCOG
    if (atomspace_) {
        logger().info("Created ChatCompletion with AtomSpace integration, ID: %s", 
                     conversation_id_.c_str());
    } else {
        logger().info("Created ChatCompletion without AtomSpace, ID: %s", 
                     conversation_id_.c_str());
    }
#endif
}

void ChatCompletion::add_message(const std::string& role, const std::string& content) {
    conversation_.emplace_back(role, content);
    
#ifdef HAVE_OPENCOG
    if (atomspace_) {
        // Store in AtomSpace
        Handle message_atom = message_to_atom(atomspace_, Message(role, content));
        
        // Link to current conversation
        Handle conversation_node = atomspace_->add_node(CONCEPT_NODE, "conversation:" + conversation_id_);
        atomspace_->add_link(MEMBER_LINK, message_atom, conversation_node);
        
        logger().info("Added message to conversation %s", conversation_id_.c_str());
    }
#endif
}

std::string ChatCompletion::complete() {
    if (!client_) {
        throw LLMException("No LLM client configured");
    }
    
    if (conversation_.empty()) {
        throw LLMException("No messages in conversation");
    }
    
    try {
        std::string response = client_->complete(conversation_);
        
        // Add assistant response to conversation
        add_message("assistant", response);
        
        return response;
    } catch (const std::exception& e) {
        logger().error("Completion failed: %s", e.what());
        throw;
    }
}

void ChatCompletion::complete_stream(std::function<void(const std::string&)> callback) {
    if (!client_) {
        throw LLMException("No LLM client configured");
    }
    
    if (conversation_.empty()) {
        throw LLMException("No messages in conversation");
    }
    
    std::string full_response;
    
    client_->complete_stream(conversation_, [&](const std::string& chunk) {
        full_response += chunk;
        callback(chunk);
    });
    
    // Add complete response to conversation
    if (!full_response.empty()) {
        add_message("assistant", full_response);
    }
}

void ChatCompletion::clear_history() {
    conversation_.clear();
    
#ifdef HAVE_OPENCOG
    if (atomspace_) {
        // Remove from AtomSpace
        Handle conversation_node = atomspace_->add_node(CONCEPT_NODE, "conversation:" + conversation_id_);
        HandleSeq incoming = conversation_node->getIncomingSet();
        
        for (Handle link : incoming) {
            if (link->get_type() == MEMBER_LINK) {
                atomspace_->remove_atom(link);
            }
        }
        
        logger().info("Cleared conversation %s", conversation_id_.c_str());
    }
#endif
}
```

### 5. Build System Configuration

#### CMakeLists.txt Configuration

```cmake
cmake_minimum_required(VERSION 3.16)
project(caichat-opencog VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Configure build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Find required packages
find_package(PkgConfig REQUIRED)

# Try to find Guile
pkg_check_modules(GUILE guile-3.0)
if(NOT GUILE_FOUND)
    pkg_check_modules(GUILE REQUIRED guile-2.2)
endif()

# Find other dependencies
find_package(CURL REQUIRED)
pkg_check_modules(JSONCPP REQUIRED jsoncpp)

# Try to find OpenCog dependencies (optional)
find_package(AtomSpace QUIET)
find_package(CogUtil QUIET)

if(AtomSpace_FOUND AND CogUtil_FOUND)
    set(HAVE_OPENCOG TRUE)
    message(STATUS "OpenCog found - building with full integration")
else()
    set(HAVE_OPENCOG FALSE)
    message(WARNING "OpenCog not found - building minimal version")
endif()

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/opencog
    ${GUILE_INCLUDE_DIRS}
    ${CURL_INCLUDE_DIRS}
    ${JSONCPP_INCLUDE_DIRS}
)

if(HAVE_OPENCOG)
    include_directories(
        ${ATOMSPACE_INCLUDE_DIR}
        ${COGUTIL_INCLUDE_DIR}
    )
    add_definitions(-DHAVE_OPENCOG)
endif()

# Compile flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GUILE_CFLAGS_OTHER} ${CURL_CFLAGS_OTHER} ${JSONCPP_CFLAGS_OTHER}")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# Source files
set(CAICHAT_SOURCES
    opencog/caichat/LLMClient.cc
    opencog/caichat/ChatCompletion.cc
    opencog/caichat/SessionManager.cc
    opencog/caichat/SchemeBindings.cc
)

# Create the library
add_library(caichat-opencog SHARED ${CAICHAT_SOURCES})

# Link libraries
target_link_libraries(caichat-opencog
    ${CURL_LIBRARIES}
    ${JSONCPP_LIBRARIES}
    ${GUILE_LIBRARIES}
)

if(HAVE_OPENCOG)
    target_link_libraries(caichat-opencog
        ${ATOMSPACE_LIBRARIES}
        ${COGUTIL_LIBRARIES}
    )
endif()

# Install targets
install(TARGETS caichat-opencog
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# Install headers
install(DIRECTORY opencog/caichat/
    DESTINATION include/opencog/caichat
    FILES_MATCHING PATTERN "*.h"
)

# Install Scheme files
install(DIRECTORY opencog/scm/
    DESTINATION share/opencog/scm
    FILES_MATCHING PATTERN "*.scm"
)

# Testing (optional)
option(BUILD_TESTS "Build test programs" ON)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
```

## OpenCog Integration

### AtomSpace Message Representation

Messages are stored in AtomSpace using the following structure:

```scheme
;; Message representation
(EvaluationLink
    (PredicateNode "message")
    (ListLink
        (ConceptNode "role:user")
        (ConceptNode "content:Hello, how are you?")))

;; Conversation structure  
(MemberLink
    (EvaluationLink ...)  ; message
    (ConceptNode "conversation:uuid-123"))
```

### Implementing AtomSpace Utilities

```cpp
// Utility functions for AtomSpace integration
namespace opencog {
namespace caichat {

Handle message_to_atom(AtomSpace* as, const Message& message) {
#ifdef HAVE_OPENCOG
    if (!as) return Handle::UNDEFINED;
    
    Handle role_node = as->add_node(CONCEPT_NODE, "role:" + message.role);
    Handle content_node = as->add_node(CONCEPT_NODE, "content:" + message.content);
    Handle predicate = as->add_node(PREDICATE_NODE, "message");
    
    HandleSeq list_items = {role_node, content_node};
    Handle list_link = as->add_link(LIST_LINK, list_items);
    
    HandleSeq eval_items = {predicate, list_link};
    return as->add_link(EVALUATION_LINK, eval_items);
#else
    return Handle_UNDEFINED;
#endif
}

Message atom_to_message(Handle message_atom) {
#ifdef HAVE_OPENCOG
    if (message_atom == Handle::UNDEFINED) {
        return Message("", "");
    }
    
    if (message_atom->get_type() != EVALUATION_LINK) {
        return Message("", "");
    }
    
    HandleSeq outgoing = message_atom->getOutgoingSet();
    if (outgoing.size() != 2) {
        return Message("", "");
    }
    
    Handle list_link = outgoing[1];
    if (list_link->get_type() != LIST_LINK) {
        return Message("", "");
    }
    
    HandleSeq list_items = list_link->getOutgoingSet();
    if (list_items.size() != 2) {
        return Message("", "");
    }
    
    std::string role_str = list_items[0]->get_name();
    std::string content_str = list_items[1]->get_name();
    
    // Extract role and content from prefixed names
    std::string role = role_str.substr(5);  // Remove "role:"
    std::string content = content_str.substr(8);  // Remove "content:"
    
    return Message(role, content);
#else
    return Message("", "");
#endif
}

} // namespace caichat
} // namespace opencog
```

## Build System and Dependencies

### Installing Dependencies

#### Ubuntu/Debian

```bash
# Essential build tools
sudo apt-get install -y cmake build-essential pkg-config

# Core dependencies
sudo apt-get install -y libcurl4-openssl-dev libjsoncpp-dev

# Guile Scheme (required)
sudo apt-get install -y guile-3.0-dev

# OpenCog (optional, for full integration)
sudo apt-get install -y opencog-dev libatomspace-dev libcogutil-dev
```

#### Arch Linux

```bash
# Essential tools
sudo pacman -S cmake gcc pkgconf

# Core dependencies  
sudo pacman -S curl jsoncpp

# Guile Scheme
sudo pacman -S guile

# OpenCog (from AUR)
yay -S opencog-git
```

#### From Source (OpenCog)

```bash
# Build OpenCog from source if packages not available
git clone https://github.com/opencog/atomspace.git
cd atomspace
mkdir build && cd build
cmake ..
make -j$(nproc)
sudo make install

cd ../..
git clone https://github.com/opencog/cogutil.git
cd cogutil
mkdir build && cd build  
cmake ..
make -j$(nproc)
sudo make install
```

### Building AIChat C++

```bash
# Clone the repository
git clone https://github.com/HyperCogWizard/caichat.git
cd caichat

# Create build directory
mkdir build && cd build

# Configure with CMake
cmake ..

# Build the project
make -j$(nproc)

# Install (optional)
sudo make install

# Run tests
make test
```

### Build Options

```bash
# Build without OpenCog (minimal version)
cmake -DHAVE_OPENCOG=OFF ..

# Debug build
cmake -DCMAKE_BUILD_TYPE=Debug ..

# Release build with optimizations
cmake -DCMAKE_BUILD_TYPE=Release ..

# Build with specific Guile version
cmake -DGUILE_VERSION=3.0 ..

# Disable tests
cmake -DBUILD_TESTS=OFF ..
```

## Testing and Debugging

### Unit Testing Framework

Create a comprehensive testing framework using Google Test or a simple custom framework:

```cpp
// tests/test_llm_client.cc
#include <gtest/gtest.h>
#include "../opencog/caichat/LLMClient.h"

namespace opencog {
namespace caichat {

class MockLLMClient : public LLMClient {
public:
    std::string complete(const std::vector<Message>& messages) override {
        return "Mock response for: " + messages.back().content;
    }
    
    void complete_stream(const std::vector<Message>& messages, 
                        std::function<void(const std::string&)> callback) override {
        callback("Mock streaming response");
    }
    
    std::string get_provider_name() const override { return "mock"; }
    std::vector<std::string> get_supported_models() const override { return {"mock-model"}; }
    bool supports_streaming() const override { return true; }
    
    void set_model(const std::string& model) override {}
    void set_temperature(double temperature) override {}
    void set_max_tokens(int max_tokens) override {}
    void set_api_key(const std::string& api_key) override {}
};

class LLMClientTest : public ::testing::Test {
protected:
    void SetUp() override {
        client_ = std::make_unique<MockLLMClient>();
    }
    
    std::unique_ptr<LLMClient> client_;
};

TEST_F(LLMClientTest, BasicCompletion) {
    std::vector<Message> messages = {
        Message("user", "Hello, world!")
    };
    
    std::string response = client_->complete(messages);
    EXPECT_EQ(response, "Mock response for: Hello, world!");
}

TEST_F(LLMClientTest, StreamingCompletion) {
    std::vector<Message> messages = {
        Message("user", "Test streaming")
    };
    
    std::string received_response;
    client_->complete_stream(messages, [&](const std::string& chunk) {
        received_response += chunk;
    });
    
    EXPECT_EQ(received_response, "Mock streaming response");
}

TEST_F(LLMClientTest, ProviderInfo) {
    EXPECT_EQ(client_->get_provider_name(), "mock");
    EXPECT_TRUE(client_->supports_streaming());
    
    auto models = client_->get_supported_models();
    EXPECT_EQ(models.size(), 1);
    EXPECT_EQ(models[0], "mock-model");
}

} // namespace caichat
} // namespace opencog

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
```

### Integration Testing

```cpp
// tests/test_integration.cc
#include <gtest/gtest.h>
#include "../opencog/caichat/ChatCompletion.h"
#include "../opencog/caichat/SessionManager.h"

namespace opencog {
namespace caichat {

class IntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
#ifdef HAVE_OPENCOG
        atomspace_ = std::make_unique<AtomSpace>();
#else
        atomspace_ = nullptr;
#endif
        
        ClientConfig config;
        config.provider = "mock";
        config.model = "mock-model";
        
        auto client = std::make_unique<MockLLMClient>();
        completion_ = std::make_unique<ChatCompletion>(atomspace_.get(), std::move(client));
    }
    
#ifdef HAVE_OPENCOG
    std::unique_ptr<AtomSpace> atomspace_;
#else
    AtomSpace* atomspace_;
#endif
    std::unique_ptr<ChatCompletion> completion_;
};

TEST_F(IntegrationTest, ConversationFlow) {
    // Add user message
    completion_->add_message("user", "Hello");
    
    // Get completion
    std::string response = completion_->complete();
    EXPECT_FALSE(response.empty());
    
    // Check conversation history
    auto messages = completion_->get_messages();
    EXPECT_EQ(messages.size(), 2);  // user + assistant
    EXPECT_EQ(messages[0].role, "user");
    EXPECT_EQ(messages[0].content, "Hello");
    EXPECT_EQ(messages[1].role, "assistant");
}

#ifdef HAVE_OPENCOG
TEST_F(IntegrationTest, AtomSpaceIntegration) {
    completion_->add_message("user", "Test message");
    
    // Check atoms were created
    auto conversation_atoms = completion_->get_conversation_atoms();
    EXPECT_FALSE(conversation_atoms.empty());
    
    // Verify atom structure
    Handle first_atom = conversation_atoms[0];
    EXPECT_EQ(first_atom->get_type(), EVALUATION_LINK);
}
#endif

} // namespace caichat
} // namespace opencog
```

### Debugging Techniques

#### Enable Debug Logging

```cpp
// Add debug macros
#ifdef DEBUG
#define DEBUG_LOG(msg) std::cout << "[DEBUG] " << msg << std::endl
#else
#define DEBUG_LOG(msg)
#endif

// Usage in code
void ChatCompletion::add_message(const std::string& role, const std::string& content) {
    DEBUG_LOG("Adding message: " << role << " -> " << content);
    conversation_.emplace_back(role, content);
    // ... rest of implementation
}
```

#### Memory Debugging with Valgrind

```bash
# Build with debug symbols
cmake -DCMAKE_BUILD_TYPE=Debug ..
make

# Run with Valgrind
valgrind --leak-check=full --show-leak-kinds=all ./tests/test_integration

# Address Sanitizer (alternative)
cmake -DCMAKE_CXX_FLAGS="-fsanitize=address -g" ..
make
./tests/test_integration
```

#### GDB Debugging

```bash
# Compile with debug symbols
cmake -DCMAKE_BUILD_TYPE=Debug ..
make

# Start GDB session
gdb ./tests/test_integration

# Useful GDB commands
(gdb) break ChatCompletion::complete
(gdb) run
(gdb) backtrace
(gdb) print messages
(gdb) step
(gdb) continue
```

## Performance Optimization

### Memory Management

#### Object Pool for Messages

```cpp
// High-frequency message creation optimization
class MessagePool {
private:
    std::vector<std::unique_ptr<Message>> pool_;
    std::mutex pool_mutex_;
    
public:
    std::unique_ptr<Message> acquire(const std::string& role, const std::string& content) {
        std::lock_guard<std::mutex> lock(pool_mutex_);
        
        if (!pool_.empty()) {
            auto message = std::move(pool_.back());
            pool_.pop_back();
            message->role = role;
            message->content = content;
            return message;
        }
        
        return std::make_unique<Message>(role, content);
    }
    
    void release(std::unique_ptr<Message> message) {
        std::lock_guard<std::mutex> lock(pool_mutex_);
        if (pool_.size() < 100) {  // Limit pool size
            pool_.push_back(std::move(message));
        }
    }
};
```

#### String Optimization

```cpp
// Use string_view for read-only operations
class OptimizedLLMClient {
public:
    std::string complete(const std::vector<std::string_view>& message_contents) {
        // Process without copying strings
        std::string request_body;
        request_body.reserve(estimate_size(message_contents));
        
        for (const auto& content : message_contents) {
            request_body.append(content.data(), content.size());
        }
        
        return make_api_call(request_body);
    }
    
private:
    size_t estimate_size(const std::vector<std::string_view>& contents) {
        size_t total = 0;
        for (const auto& content : contents) {
            total += content.size();
        }
        return total + 1000;  // Add overhead for JSON structure
    }
};
```

### Caching Strategies

#### Response Caching

```cpp
class CachedLLMClient : public LLMClient {
private:
    struct CacheEntry {
        std::string response;
        std::chrono::steady_clock::time_point timestamp;
    };
    
    mutable std::unordered_map<std::string, CacheEntry> cache_;
    mutable std::mutex cache_mutex_;
    std::chrono::minutes cache_duration_{30};
    
    std::string hash_messages(const std::vector<Message>& messages) const {
        std::string combined;
        for (const auto& msg : messages) {
            combined += msg.role + ":" + msg.content + ";";
        }
        
        // Simple hash (use proper hash function in production)
        return std::to_string(std::hash<std::string>{}(combined));
    }
    
public:
    std::string complete(const std::vector<Message>& messages) override {
        std::string cache_key = hash_messages(messages);
        
        {
            std::lock_guard<std::mutex> lock(cache_mutex_);
            auto it = cache_.find(cache_key);
            if (it != cache_.end()) {
                auto age = std::chrono::steady_clock::now() - it->second.timestamp;
                if (age < cache_duration_) {
                    return it->second.response;  // Cache hit
                } else {
                    cache_.erase(it);  // Expired entry
                }
            }
        }
        
        // Cache miss - call underlying client
        std::string response = underlying_client_->complete(messages);
        
        {
            std::lock_guard<std::mutex> lock(cache_mutex_);
            cache_[cache_key] = {response, std::chrono::steady_clock::now()};
        }
        
        return response;
    }
};
```

### Async Processing

#### Asynchronous Completion

```cpp
#include <future>
#include <thread>

class AsyncLLMClient : public LLMClient {
public:
    std::future<std::string> complete_async(const std::vector<Message>& messages) {
        return std::async(std::launch::async, [this, messages]() {
            return this->complete(messages);
        });
    }
    
    // Batch processing
    std::vector<std::future<std::string>> complete_batch(
        const std::vector<std::vector<Message>>& batch_messages) {
        
        std::vector<std::future<std::string>> futures;
        futures.reserve(batch_messages.size());
        
        for (const auto& messages : batch_messages) {
            futures.push_back(complete_async(messages));
        }
        
        return futures;
    }
};
```

## Extension and Customization

### Adding New LLM Providers

#### Creating a Custom Provider

```cpp
// Example: Adding support for a hypothetical "CustomAI" provider
class CustomAIClient : public LLMClient {
private:
    std::string api_endpoint_;
    std::string auth_token_;
    
public:
    CustomAIClient(const ClientConfig& config) 
        : api_endpoint_(config.base_url)
        , auth_token_(config.api_key) {
    }
    
    std::string complete(const std::vector<Message>& messages) override {
        // Build custom API request format
        Json::Value request;
        request["prompt"] = build_prompt(messages);
        request["max_length"] = max_tokens_;
        request["temperature"] = temperature_;
        
        // Make HTTP request using custom API format
        HTTPResponse response = make_custom_request(request);
        
        // Parse custom response format
        return parse_custom_response(response);
    }
    
    std::string get_provider_name() const override {
        return "customai";
    }
    
    std::vector<std::string> get_supported_models() const override {
        return {"custom-large", "custom-small", "custom-instruct"};
    }
    
private:
    std::string build_prompt(const std::vector<Message>& messages) {
        std::string prompt;
        for (const auto& msg : messages) {
            prompt += msg.role + ": " + msg.content + "\n";
        }
        return prompt;
    }
    
    HTTPResponse make_custom_request(const Json::Value& request) {
        // Custom HTTP implementation
        // ...
    }
    
    std::string parse_custom_response(const HTTPResponse& response) {
        // Custom response parsing
        // ...
    }
};

// Register the new provider in the factory
std::unique_ptr<LLMClient> create_client(const ClientConfig& config) {
    if (config.provider == "openai") {
        return std::make_unique<OpenAIClient>(config);
    } else if (config.provider == "claude") {
        return std::make_unique<ClaudeClient>(config);
    } else if (config.provider == "customai") {  // Add new provider
        return std::make_unique<CustomAIClient>(config);
    }
    
    throw std::invalid_argument("Unknown provider: " + config.provider);
}
```

### Plugin Architecture

#### Plugin Interface

```cpp
// Plugin base class
class LLMPlugin {
public:
    virtual ~LLMPlugin() = default;
    virtual std::string get_name() const = 0;
    virtual std::string get_version() const = 0;
    virtual std::unique_ptr<LLMClient> create_client(const ClientConfig& config) = 0;
    virtual bool is_compatible(const std::string& provider) const = 0;
};

// Plugin manager
class PluginManager {
private:
    std::vector<std::unique_ptr<LLMPlugin>> plugins_;
    
public:
    void register_plugin(std::unique_ptr<LLMPlugin> plugin) {
        plugins_.push_back(std::move(plugin));
    }
    
    std::unique_ptr<LLMClient> create_client(const ClientConfig& config) {
        for (const auto& plugin : plugins_) {
            if (plugin->is_compatible(config.provider)) {
                return plugin->create_client(config);
            }
        }
        throw std::invalid_argument("No plugin found for provider: " + config.provider);
    }
    
    std::vector<std::string> list_providers() const {
        std::vector<std::string> providers;
        for (const auto& plugin : plugins_) {
            providers.push_back(plugin->get_name());
        }
        return providers;
    }
};

// Example plugin implementation
class OpenAIPlugin : public LLMPlugin {
public:
    std::string get_name() const override { return "OpenAI"; }
    std::string get_version() const override { return "1.0.0"; }
    
    std::unique_ptr<LLMClient> create_client(const ClientConfig& config) override {
        return std::make_unique<OpenAIClient>(config);
    }
    
    bool is_compatible(const std::string& provider) const override {
        return provider == "openai" || provider == "openai-compatible";
    }
};
```

### Custom Middleware

#### Request/Response Middleware

```cpp
class LLMMiddleware {
public:
    virtual ~LLMMiddleware() = default;
    
    virtual void before_request(std::vector<Message>& messages) {}
    virtual void after_response(const std::string& response, 
                              const std::vector<Message>& messages) {}
    virtual void on_error(const std::exception& error, 
                         const std::vector<Message>& messages) {}
};

class LoggingMiddleware : public LLMMiddleware {
public:
    void before_request(std::vector<Message>& messages) override {
        std::cout << "[LOG] Sending " << messages.size() << " messages" << std::endl;
    }
    
    void after_response(const std::string& response, 
                       const std::vector<Message>& messages) override {
        std::cout << "[LOG] Received response of " << response.length() 
                  << " characters" << std::endl;
    }
    
    void on_error(const std::exception& error, 
                 const std::vector<Message>& messages) override {
        std::cerr << "[ERROR] Request failed: " << error.what() << std::endl;
    }
};

class MiddlewareClient : public LLMClient {
private:
    std::unique_ptr<LLMClient> underlying_client_;
    std::vector<std::unique_ptr<LLMMiddleware>> middleware_;
    
public:
    MiddlewareClient(std::unique_ptr<LLMClient> client) 
        : underlying_client_(std::move(client)) {}
    
    void add_middleware(std::unique_ptr<LLMMiddleware> middleware) {
        middleware_.push_back(std::move(middleware));
    }
    
    std::string complete(const std::vector<Message>& messages) override {
        auto mutable_messages = messages;  // Copy for middleware modifications
        
        try {
            // Before request middleware
            for (auto& mw : middleware_) {
                mw->before_request(mutable_messages);
            }
            
            // Make actual request
            std::string response = underlying_client_->complete(mutable_messages);
            
            // After response middleware
            for (auto& mw : middleware_) {
                mw->after_response(response, mutable_messages);
            }
            
            return response;
            
        } catch (const std::exception& e) {
            // Error middleware
            for (auto& mw : middleware_) {
                mw->on_error(e, mutable_messages);
            }
            throw;
        }
    }
    
    // Delegate other methods to underlying client
    std::string get_provider_name() const override {
        return underlying_client_->get_provider_name();
    }
    // ... other delegated methods
};
```

## Best Practices

### 1. Error Handling

```cpp
// Use specific exception types
class NetworkException : public LLMException {
public:
    NetworkException(const std::string& msg) : LLMException("Network error: " + msg) {}
};

class AuthenticationException : public LLMException {
public:
    AuthenticationException() : LLMException("Authentication failed") {}
};

class RateLimitException : public LLMException {
public:
    RateLimitException(int retry_after) 
        : LLMException("Rate limit exceeded, retry after " + std::to_string(retry_after) + "s") {}
};

// Handle errors gracefully
std::string robust_complete(LLMClient* client, const std::vector<Message>& messages) {
    const int max_retries = 3;
    const int base_delay = 1000;  // milliseconds
    
    for (int attempt = 0; attempt < max_retries; ++attempt) {
        try {
            return client->complete(messages);
            
        } catch (const RateLimitException& e) {
            if (attempt == max_retries - 1) throw;
            
            int delay = base_delay * (1 << attempt);  // Exponential backoff
            std::this_thread::sleep_for(std::chrono::milliseconds(delay));
            
        } catch (const NetworkException& e) {
            if (attempt == max_retries - 1) throw;
            
            std::this_thread::sleep_for(std::chrono::milliseconds(base_delay));
            
        } catch (const AuthenticationException& e) {
            // Don't retry authentication errors
            throw;
        }
    }
    
    throw LLMException("Max retries exceeded");
}
```

### 2. Configuration Management

```cpp
class ConfigManager {
private:
    std::map<std::string, std::string> config_;
    std::string config_file_;
    
public:
    ConfigManager(const std::string& config_file = "caichat.conf") 
        : config_file_(config_file) {
        load_from_file();
        load_from_environment();
    }
    
    void set(const std::string& key, const std::string& value) {
        config_[key] = value;
    }
    
    std::string get(const std::string& key, const std::string& default_value = "") const {
        auto it = config_.find(key);
        return (it != config_.end()) ? it->second : default_value;
    }
    
    ClientConfig create_client_config(const std::string& provider) const {
        ClientConfig config;
        config.provider = provider;
        config.model = get(provider + ".model", "default");
        config.api_key = get(provider + ".api_key");
        config.base_url = get(provider + ".base_url");
        config.temperature = std::stod(get(provider + ".temperature", "0.7"));
        config.max_tokens = std::stoi(get(provider + ".max_tokens", "1000"));
        return config;
    }
    
private:
    void load_from_file() {
        std::ifstream file(config_file_);
        std::string line;
        while (std::getline(file, line)) {
            auto pos = line.find('=');
            if (pos != std::string::npos) {
                std::string key = line.substr(0, pos);
                std::string value = line.substr(pos + 1);
                config_[key] = value;
            }
        }
    }
    
    void load_from_environment() {
        const char* openai_key = std::getenv("OPENAI_API_KEY");
        if (openai_key) config_["openai.api_key"] = openai_key;
        
        const char* claude_key = std::getenv("ANTHROPIC_API_KEY");
        if (claude_key) config_["claude.api_key"] = claude_key;
        
        // ... other environment variables
    }
};
```

### 3. Thread Safety

```cpp
class ThreadSafeChatCompletion {
private:
    mutable std::shared_mutex conversation_mutex_;
    std::vector<Message> conversation_;
    std::unique_ptr<LLMClient> client_;
    
public:
    void add_message(const std::string& role, const std::string& content) {
        std::unique_lock<std::shared_mutex> lock(conversation_mutex_);
        conversation_.emplace_back(role, content);
    }
    
    std::vector<Message> get_messages() const {
        std::shared_lock<std::shared_mutex> lock(conversation_mutex_);
        return conversation_;  // Copy for thread safety
    }
    
    std::string complete() {
        std::shared_lock<std::shared_mutex> read_lock(conversation_mutex_);
        auto messages_copy = conversation_;
        read_lock.unlock();
        
        std::string response = client_->complete(messages_copy);
        
        std::unique_lock<std::shared_mutex> write_lock(conversation_mutex_);
        conversation_.emplace_back("assistant", response);
        
        return response;
    }
};
```

## Troubleshooting

### Common Issues and Solutions

#### 1. Build Issues

**Problem**: CMake can't find OpenCog libraries
```bash
CMake Error: Could not find a package configuration file provided by "AtomSpace"
```

**Solution**:
```bash
# Set CMAKE_PREFIX_PATH to OpenCog installation
export CMAKE_PREFIX_PATH=/usr/local:$CMAKE_PREFIX_PATH

# Or specify paths explicitly
cmake -DATOMSPACE_ROOT=/usr/local -DCOGUTIL_ROOT=/usr/local ..

# For custom OpenCog installation
cmake -DAtomSpace_DIR=/path/to/atomspace/lib/cmake/AtomSpace ..
```

**Problem**: Guile development headers not found
```bash
CMake Error: Could not find guile-3.0 using pkg-config
```

**Solution**:
```bash
# Install Guile development package
sudo apt-get install guile-3.0-dev

# Or specify Guile path manually
export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:$PKG_CONFIG_PATH
```

#### 2. Runtime Issues

**Problem**: Segmentation fault when accessing AtomSpace
```
Segmentation fault (core dumped)
```

**Solution**:
```cpp
// Always check for null pointers
void safe_atomspace_operation(AtomSpace* atomspace) {
    if (!atomspace) {
        throw std::runtime_error("AtomSpace is null");
    }
    
#ifdef HAVE_OPENCOG
    // Proceed with AtomSpace operations
    Handle node = atomspace->add_node(CONCEPT_NODE, "test");
#else
    throw std::runtime_error("OpenCog not available");
#endif
}
```

**Problem**: API authentication failures
```
OpenAI API request failed with code: 401
```

**Solution**:
```cpp
// Verify API key configuration
void validate_api_key(const std::string& api_key) {
    if (api_key.empty()) {
        throw AuthenticationException("API key not configured");
    }
    
    if (api_key.length() < 20) {
        throw AuthenticationException("API key appears to be invalid");
    }
    
    // Test API key with a simple request
    // ...
}
```

#### 3. Performance Issues

**Problem**: High memory usage with long conversations
```
Memory usage grows unbounded during long conversations
```

**Solution**:
```cpp
class ConversationManager {
private:
    static const size_t MAX_CONVERSATION_LENGTH = 100;
    
public:
    void add_message(const std::string& role, const std::string& content) {
        conversation_.emplace_back(role, content);
        
        // Trim conversation if too long
        if (conversation_.size() > MAX_CONVERSATION_LENGTH) {
            // Keep system message and recent messages
            auto system_msgs = extract_system_messages();
            auto recent_msgs = get_recent_messages(MAX_CONVERSATION_LENGTH - system_msgs.size());
            
            conversation_.clear();
            conversation_.insert(conversation_.end(), system_msgs.begin(), system_msgs.end());
            conversation_.insert(conversation_.end(), recent_msgs.begin(), recent_msgs.end());
        }
    }
};
```

**Problem**: Slow response times
```
Completion requests taking too long
```

**Solution**:
```cpp
// Implement timeout handling
class TimeoutLLMClient : public LLMClient {
private:
    std::chrono::seconds timeout_;
    
public:
    std::string complete(const std::vector<Message>& messages) override {
        auto future = std::async(std::launch::async, [this, &messages]() {
            return underlying_client_->complete(messages);
        });
        
        if (future.wait_for(timeout_) == std::future_status::timeout) {
            throw LLMException("Request timed out after " + 
                             std::to_string(timeout_.count()) + " seconds");
        }
        
        return future.get();
    }
};
```

### Debugging Checklist

1. **Build Issues**:
   - [ ] All dependencies installed
   - [ ] Correct CMake configuration
   - [ ] Environment variables set
   - [ ] Library paths configured

2. **Runtime Issues**:
   - [ ] API keys configured
   - [ ] Network connectivity
   - [ ] AtomSpace initialization
   - [ ] Memory limits checked

3. **Performance Issues**:
   - [ ] Conversation length management
   - [ ] Caching enabled
   - [ ] Timeout configuration
   - [ ] Resource monitoring

4. **Integration Issues**:
   - [ ] Scheme bindings working
   - [ ] OpenCog modules loaded
   - [ ] AtomSpace operations functional
   - [ ] Plugin registration correct

This comprehensive guide provides everything needed to implement, extend, and maintain AIChat's C++ functionality. For additional help, consult the existing documentation files and the project's issue tracker.