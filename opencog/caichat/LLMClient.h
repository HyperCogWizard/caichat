/**
 * LLMClient.h
 * 
 * OpenCog C++ interface for Large Language Model clients
 * Provides bindings for various LLM providers (OpenAI, Claude, etc.)
 */

#ifndef _OPENCOG_CAICHAT_LLM_CLIENT_H
#define _OPENCOG_CAICHAT_LLM_CLIENT_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>

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

/**
 * Message structure for chat interactions
 */
struct Message {
    std::string role;
    std::string content;
    
    Message(const std::string& r, const std::string& c) : role(r), content(c) {}
};

/**
 * Configuration for LLM clients
 */
struct ClientConfig {
    std::string provider;
    std::string model;
    std::string api_key;
    std::string api_base;
    double temperature = 0.7;
    double top_p = 1.0;
    int max_tokens = -1;
};

/**
 * Abstract base class for LLM clients
 */
class LLMClient {
public:
    LLMClient(const ClientConfig& config);
    virtual ~LLMClient() = default;
    
    /**
     * Send chat completion request
     */
    virtual std::string chat_completion(const std::vector<Message>& messages) = 0;
    
    /**
     * Send streaming chat completion request
     */
    virtual void chat_completion_stream(
        const std::vector<Message>& messages,
        std::function<void(const std::string&)> callback) = 0;
    
    /**
     * Generate embeddings for text
     */
    virtual std::vector<double> embeddings(const std::string& text) = 0;
    
protected:
    ClientConfig config_;
};

/**
 * OpenAI client implementation
 */
class OpenAIClient : public LLMClient {
public:
    OpenAIClient(const ClientConfig& config);
    
    std::string chat_completion(const std::vector<Message>& messages) override;
    void chat_completion_stream(
        const std::vector<Message>& messages,
        std::function<void(const std::string&)> callback) override;
    std::vector<double> embeddings(const std::string& text) override;
};

/**
 * Claude client implementation  
 */
class ClaudeClient : public LLMClient {
public:
    ClaudeClient(const ClientConfig& config);
    
    std::string chat_completion(const std::vector<Message>& messages) override;
    void chat_completion_stream(
        const std::vector<Message>& messages,
        std::function<void(const std::string&)> callback) override;
    std::vector<double> embeddings(const std::string& text) override;
};

/**
 * Factory function to create appropriate client
 */
std::unique_ptr<LLMClient> create_client(const ClientConfig& config);

/**
 * Convert Atom representation to Message
 */
Message atom_to_message(Handle message_atom);

/**
 * Convert Message to Atom representation
 */
Handle message_to_atom(AtomSpace* as, const Message& message);

} // namespace caichat
} // namespace opencog

#endif // _OPENCOG_CAICHAT_LLM_CLIENT_H