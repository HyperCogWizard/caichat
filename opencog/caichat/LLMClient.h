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
 * Gemini (Google) client implementation  
 */
class GeminiClient : public LLMClient {
public:
    GeminiClient(const ClientConfig& config);
    
    std::string chat_completion(const std::vector<Message>& messages) override;
    void chat_completion_stream(
        const std::vector<Message>& messages,
        std::function<void(const std::string&)> callback) override;
    std::vector<double> embeddings(const std::string& text) override;
};

/**
 * Ollama client implementation  
 */
class OllamaClient : public LLMClient {
public:
    OllamaClient(const ClientConfig& config);
    
    std::string chat_completion(const std::vector<Message>& messages) override;
    void chat_completion_stream(
        const std::vector<Message>& messages,
        std::function<void(const std::string&)> callback) override;
    std::vector<double> embeddings(const std::string& text) override;
};

/**
 * Groq client implementation  
 */
class GroqClient : public LLMClient {
public:
    GroqClient(const ClientConfig& config);
    
    std::string chat_completion(const std::vector<Message>& messages) override;
    void chat_completion_stream(
        const std::vector<Message>& messages,
        std::function<void(const std::string&)> callback) override;
    std::vector<double> embeddings(const std::string& text) override;
};

/**
 * GGML client implementation for local model inference
 * Compatible with OpenCog cognitive architecture
 */
class GGMLClient : public LLMClient {
public:
    struct GGMLConfig {
        std::string model_path;
        int n_threads = 4;
        int n_ctx = 2048;
        int n_batch = 512;
        bool use_mmap = true;
        bool use_mlock = false;
        float temperature = 0.7f;
        int top_k = 40;
        float top_p = 0.95f;
        float repeat_penalty = 1.1f;
        int n_predict = 128;
    };
    
    GGMLClient(const ClientConfig& config);
    GGMLClient(const ClientConfig& config, const GGMLConfig& ggml_config);
    ~GGMLClient();
    
    std::string chat_completion(const std::vector<Message>& messages) override;
    void chat_completion_stream(
        const std::vector<Message>& messages,
        std::function<void(const std::string&)> callback) override;
    std::vector<double> embeddings(const std::string& text) override;
    
    /**
     * Load a GGML model from file
     */
    bool load_model(const std::string& model_path);
    
    /**
     * Unload current model
     */
    void unload_model();
    
    /**
     * Check if model is loaded
     */
    bool is_model_loaded() const;
    
    /**
     * Get model info
     */
    std::string get_model_info() const;
    
    /**
     * Neural-symbolic bridge integration
     * Convert AtomSpace patterns to prompts for better inference
     */
    std::string atomspace_to_prompt(Handle pattern_atom, const std::string& context = "");
    
    /**
     * Cognitive architecture integration
     * Generate responses that can be parsed into AtomSpace representations
     */
    std::string cognitive_completion(const std::vector<Message>& messages, Handle context_atom = Handle_UNDEFINED);

private:
    GGMLConfig ggml_config_;
    void* model_context_;  // Opaque pointer to GGML context
    bool model_loaded_;
    std::string model_info_;
    
    /**
     * Initialize GGML context
     */
    bool init_ggml_context();
    
    /**
     * Cleanup GGML context
     */
    void cleanup_ggml_context();
    
    /**
     * Format messages for GGML inference
     */
    std::string format_messages_for_ggml(const std::vector<Message>& messages);
    
    /**
     * Run GGML inference
     */
    std::string run_ggml_inference(const std::string& prompt);
};

/**
 * Factory function to create appropriate client
 */
std::unique_ptr<LLMClient> create_client(const ClientConfig& config);

/**
 * Provider Router - implements dynamic provider selection and routing logic
 */
class LLMProviderRouter {
public:
    struct ProviderCapabilities {
        bool supports_chat = true;
        bool supports_streaming = true;
        bool supports_embeddings = false;
        bool supports_functions = false;
        std::vector<std::string> supported_models;
        double cost_per_token = 0.0;
        int max_context_length = 4096;
    };
    
    /**
     * Register a provider with its capabilities
     */
    void register_provider(const std::string& provider_name, const ProviderCapabilities& caps);
    
    /**
     * Route LLM request to the best available provider
     */
    std::string route_llm_request(const std::vector<Message>& messages, 
                                  const std::string& preferred_provider = "",
                                  const std::string& task_type = "chat");
    
    /**
     * Get list of available providers for a specific task
     */
    std::vector<std::string> get_available_providers(const std::string& task_type = "chat");
    
    /**
     * Initialize with default provider configurations
     */
    void init_default_providers();

private:
    std::map<std::string, ProviderCapabilities> provider_capabilities_;
    std::map<std::string, ClientConfig> provider_configs_;
    
    /**
     * Select best provider based on criteria
     */
    std::string select_provider(const std::vector<Message>& messages, 
                               const std::string& task_type);
};

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