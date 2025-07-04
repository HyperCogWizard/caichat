/**
 * LLMClient.cc
 * 
 * Implementation of LLM client classes for OpenCog
 */

#include "LLMClient.h"

#ifdef HAVE_OPENCOG
#include <opencog/atomspace/AtomSpace.h>
#include <opencog/atoms/base/Node.h>
#include <opencog/atoms/base/Link.h>
#include <opencog/util/Logger.h>
#else
// Minimal logger for non-OpenCog builds
#include <iostream>
namespace opencog {
    struct logger {
        static void error(const char* fmt, ...) { 
            std::cerr << "ERROR: " << fmt << std::endl; 
        }
        static void warn(const char* fmt, ...) { 
            std::cerr << "WARN: " << fmt << std::endl; 
        }
        static void info(const char* fmt, ...) { 
            std::cout << "INFO: " << fmt << std::endl; 
        }
    };
    static logger get_logger() { static logger instance; return instance; }
}
// Minimal handle type
typedef void* Handle;
#define Handle_UNDEFINED nullptr
#endif

#include <curl/curl.h>
#include <json/json.h>
#include <sstream>
#include <iostream>
#include <thread>
#include <chrono>
#include <algorithm>

using namespace opencog;
using namespace opencog::caichat;

// Helper struct for curl response
struct CurlResponse {
    std::string data;
    
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, CurlResponse* response) {
        size_t total_size = size * nmemb;
        response->data.append((char*)contents, total_size);
        return total_size;
    }
};

LLMClient::LLMClient(const ClientConfig& config) : config_(config) {
    // Initialize curl globally (should be done once)
    static bool curl_initialized = false;
    if (!curl_initialized) {
        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl_initialized = true;
    }
}

// OpenAI Client Implementation
OpenAIClient::OpenAIClient(const ClientConfig& config) : LLMClient(config) {
    if (config_.api_base.empty()) {
        config_.api_base = "https://api.openai.com/v1";
    }
}

std::string OpenAIClient::chat_completion(const std::vector<Message>& messages) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        throw std::runtime_error("Failed to initialize curl");
    }
    
    // Build JSON payload
    Json::Value payload;
    payload["model"] = config_.model;
    payload["temperature"] = config_.temperature;
    payload["top_p"] = config_.top_p;
    if (config_.max_tokens > 0) {
        payload["max_tokens"] = config_.max_tokens;
    }
    
    Json::Value messages_json(Json::arrayValue);
    for (const auto& msg : messages) {
        Json::Value msg_json;
        msg_json["role"] = msg.role;
        msg_json["content"] = msg.content;
        messages_json.append(msg_json);
    }
    payload["messages"] = messages_json;
    
    Json::StreamWriterBuilder builder;
    std::string json_string = Json::writeString(builder, payload);
    
    // Set curl options
    std::string url = config_.api_base + "/chat/completions";
    CurlResponse response;
    
    struct curl_slist* headers = nullptr;
    std::string auth_header = "Authorization: Bearer " + config_.api_key;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, auth_header.c_str());
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_string.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlResponse::WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    
    CURLcode res = curl_easy_perform(curl);
    
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    
    if (res != CURLE_OK) {
        throw std::runtime_error("Curl request failed: " + std::string(curl_easy_strerror(res)));
    }
    
    // Parse response
    Json::CharReaderBuilder reader_builder;
    Json::Value response_json;
    std::istringstream response_stream(response.data);
    std::string parse_errors;
    
    if (!Json::parseFromStream(reader_builder, response_stream, &response_json, &parse_errors)) {
        throw std::runtime_error("Failed to parse JSON response: " + parse_errors);
    }
    
    if (response_json.isMember("error")) {
        throw std::runtime_error("API Error: " + response_json["error"]["message"].asString());
    }
    
    return response_json["choices"][0]["message"]["content"].asString();
}

void OpenAIClient::chat_completion_stream(
    const std::vector<Message>& messages,
    std::function<void(const std::string&)> callback) {
    
    // For now, implement as non-streaming and call callback once
    // TODO: Implement proper SSE streaming
    try {
        std::string response = chat_completion(messages);
        callback(response);
    } catch (const std::exception& e) {
        logger().error("Streaming completion failed: %s", e.what());
        throw;
    }
}

std::vector<double> OpenAIClient::embeddings(const std::string& text) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        throw std::runtime_error("Failed to initialize curl");
    }
    
    // Build JSON payload
    Json::Value payload;
    payload["model"] = "text-embedding-ada-002"; // Default embedding model
    payload["input"] = text;
    
    Json::StreamWriterBuilder builder;
    std::string json_string = Json::writeString(builder, payload);
    
    // Set curl options
    std::string url = config_.api_base + "/embeddings";
    CurlResponse response;
    
    struct curl_slist* headers = nullptr;
    std::string auth_header = "Authorization: Bearer " + config_.api_key;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, auth_header.c_str());
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_string.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlResponse::WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    
    CURLcode res = curl_easy_perform(curl);
    
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    
    if (res != CURLE_OK) {
        throw std::runtime_error("Curl request failed: " + std::string(curl_easy_strerror(res)));
    }
    
    // Parse response
    Json::CharReaderBuilder reader_builder;
    Json::Value response_json;
    std::istringstream response_stream(response.data);
    std::string parse_errors;
    
    if (!Json::parseFromStream(reader_builder, response_stream, &response_json, &parse_errors)) {
        throw std::runtime_error("Failed to parse JSON response: " + parse_errors);
    }
    
    if (response_json.isMember("error")) {
        throw std::runtime_error("API Error: " + response_json["error"]["message"].asString());
    }
    
    // Extract embedding values
    std::vector<double> embedding;
    const Json::Value& embedding_array = response_json["data"][0]["embedding"];
    
    for (const auto& value : embedding_array) {
        embedding.push_back(value.asDouble());
    }
    
    return embedding;
}

// Claude Client Implementation
ClaudeClient::ClaudeClient(const ClientConfig& config) : LLMClient(config) {
    if (config_.api_base.empty()) {
        config_.api_base = "https://api.anthropic.com";
    }
}

std::string ClaudeClient::chat_completion(const std::vector<Message>& messages) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        throw std::runtime_error("Failed to initialize curl");
    }
    
    // Prepare Claude API request
    Json::Value payload;
    payload["model"] = config_.model;
    payload["max_tokens"] = (config_.max_tokens > 0) ? config_.max_tokens : 4096;
    payload["temperature"] = config_.temperature;
    payload["top_p"] = config_.top_p;
    
    // Convert messages to Claude format (system message separate)
    Json::Value messages_json(Json::arrayValue);
    std::string system_message = "";
    
    for (const auto& msg : messages) {
        if (msg.role == "system") {
            system_message = msg.content;
        } else {
            Json::Value msg_json;
            msg_json["role"] = msg.role;
            msg_json["content"] = msg.content;
            messages_json.append(msg_json);
        }
    }
    
    payload["messages"] = messages_json;
    if (!system_message.empty()) {
        payload["system"] = system_message;
    }
    
    Json::StreamWriterBuilder builder;
    std::string json_string = Json::writeString(builder, payload);
    
    // Set curl options for Claude API
    std::string url = config_.api_base + "/v1/messages";
    CurlResponse response;
    
    struct curl_slist* headers = nullptr;
    std::string auth_header = "x-api-key: " + config_.api_key;
    std::string version_header = "anthropic-version: 2023-06-01";
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, auth_header.c_str());
    headers = curl_slist_append(headers, version_header.c_str());
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_string.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlResponse::WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    
    CURLcode res = curl_easy_perform(curl);
    
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    
    if (res != CURLE_OK) {
        throw std::runtime_error("Curl request failed: " + std::string(curl_easy_strerror(res)));
    }
    
    // Parse response
    Json::CharReaderBuilder reader_builder;
    Json::Value response_json;
    std::istringstream response_stream(response.data);
    std::string parse_errors;
    
    if (!Json::parseFromStream(reader_builder, response_stream, &response_json, &parse_errors)) {
        throw std::runtime_error("Failed to parse JSON response: " + parse_errors);
    }
    
    if (response_json.isMember("error")) {
        throw std::runtime_error("Claude API Error: " + response_json["error"]["message"].asString());
    }
    
    // Extract message content from Claude response
    if (response_json.isMember("content") && response_json["content"].isArray() && 
        response_json["content"].size() > 0) {
        return response_json["content"][0]["text"].asString();
    }
    
    throw std::runtime_error("Unexpected Claude API response format");
}

void ClaudeClient::chat_completion_stream(
    const std::vector<Message>& messages,
    std::function<void(const std::string&)> callback) {
    
    // For now, implement non-streaming version and call callback with chunks
    // TODO: Implement actual streaming using Server-Sent Events
    try {
        std::string response = chat_completion(messages);
        
        // Simulate streaming by sending response in chunks
        const size_t chunk_size = 50;
        for (size_t i = 0; i < response.length(); i += chunk_size) {
            std::string chunk = response.substr(i, chunk_size);
            callback(chunk);
            
            // Small delay to simulate streaming
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    } catch (const std::exception& e) {
        throw std::runtime_error("Claude streaming error: " + std::string(e.what()));
    }
}

std::vector<double> ClaudeClient::embeddings(const std::string& text) {
    // Claude (Anthropic) doesn't provide an embeddings API
    // Return an empty vector or throw a more informative error
    throw std::runtime_error("Claude (Anthropic) does not provide an embeddings API. Use OpenAI or other providers for embeddings.");
}

// Gemini Client Implementation
GeminiClient::GeminiClient(const ClientConfig& config) : LLMClient(config) {
    if (config_.api_base.empty()) {
        config_.api_base = "https://generativelanguage.googleapis.com";
    }
}

std::string GeminiClient::chat_completion(const std::vector<Message>& messages) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        throw std::runtime_error("Failed to initialize curl");
    }
    
    // Prepare Gemini API request
    Json::Value payload;
    Json::Value contents(Json::arrayValue);
    
    // Convert messages to Gemini format
    for (const auto& msg : messages) {
        Json::Value content;
        Json::Value parts(Json::arrayValue);
        Json::Value text_part;
        text_part["text"] = msg.content;
        parts.append(text_part);
        content["parts"] = parts;
        
        if (msg.role == "user") {
            content["role"] = "user";
        } else if (msg.role == "assistant") {
            content["role"] = "model";
        }
        // Note: Gemini handles system messages differently
        
        contents.append(content);
    }
    
    payload["contents"] = contents;
    
    Json::Value generation_config;
    generation_config["temperature"] = config_.temperature;
    generation_config["topP"] = config_.top_p;
    if (config_.max_tokens > 0) {
        generation_config["maxOutputTokens"] = config_.max_tokens;
    }
    payload["generationConfig"] = generation_config;
    
    Json::StreamWriterBuilder builder;
    std::string json_string = Json::writeString(builder, payload);
    
    // Set curl options for Gemini API
    std::string url = config_.api_base + "/v1beta/models/" + config_.model + ":generateContent?key=" + config_.api_key;
    CurlResponse response;
    
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_string.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlResponse::WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    
    CURLcode res = curl_easy_perform(curl);
    
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    
    if (res != CURLE_OK) {
        throw std::runtime_error("Curl request failed: " + std::string(curl_easy_strerror(res)));
    }
    
    // Parse response
    Json::CharReaderBuilder reader_builder;
    Json::Value response_json;
    std::istringstream response_stream(response.data);
    std::string parse_errors;
    
    if (!Json::parseFromStream(reader_builder, response_stream, &response_json, &parse_errors)) {
        throw std::runtime_error("Failed to parse JSON response: " + parse_errors);
    }
    
    if (response_json.isMember("error")) {
        throw std::runtime_error("Gemini API Error: " + response_json["error"]["message"].asString());
    }
    
    // Extract message content from Gemini response
    if (response_json.isMember("candidates") && response_json["candidates"].isArray() && 
        response_json["candidates"].size() > 0) {
        const Json::Value& candidate = response_json["candidates"][0];
        if (candidate.isMember("content") && candidate["content"].isMember("parts") &&
            candidate["content"]["parts"].isArray() && candidate["content"]["parts"].size() > 0) {
            return candidate["content"]["parts"][0]["text"].asString();
        }
    }
    
    throw std::runtime_error("Unexpected Gemini API response format");
}

void GeminiClient::chat_completion_stream(
    const std::vector<Message>& messages,
    std::function<void(const std::string&)> callback) {
    
    // For now, implement non-streaming version and call callback with chunks
    // TODO: Implement actual streaming
    try {
        std::string response = chat_completion(messages);
        
        // Simulate streaming by sending response in chunks
        const size_t chunk_size = 50;
        for (size_t i = 0; i < response.length(); i += chunk_size) {
            std::string chunk = response.substr(i, chunk_size);
            callback(chunk);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    } catch (const std::exception& e) {
        throw std::runtime_error("Gemini streaming error: " + std::string(e.what()));
    }
}

std::vector<double> GeminiClient::embeddings(const std::string& text) {
    // Gemini does have an embeddings API, but it's different from the generative API
    // For now, throw an informative error
    throw std::runtime_error("Gemini embeddings not yet implemented. Use embedding-specific models.");
}

// Ollama Client Implementation  
OllamaClient::OllamaClient(const ClientConfig& config) : LLMClient(config) {
    if (config_.api_base.empty()) {
        config_.api_base = "http://localhost:11434";
    }
}

std::string OllamaClient::chat_completion(const std::vector<Message>& messages) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        throw std::runtime_error("Failed to initialize curl");
    }
    
    // Prepare Ollama API request (OpenAI-compatible format)
    Json::Value payload;
    payload["model"] = config_.model;
    payload["stream"] = false;
    
    Json::Value messages_json(Json::arrayValue);
    for (const auto& msg : messages) {
        Json::Value msg_json;
        msg_json["role"] = msg.role;
        msg_json["content"] = msg.content;
        messages_json.append(msg_json);
    }
    payload["messages"] = messages_json;
    
    Json::StreamWriterBuilder builder;
    std::string json_string = Json::writeString(builder, payload);
    
    // Set curl options for Ollama API
    std::string url = config_.api_base + "/v1/chat/completions";
    CurlResponse response;
    
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_string.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlResponse::WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    
    CURLcode res = curl_easy_perform(curl);
    
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    
    if (res != CURLE_OK) {
        throw std::runtime_error("Curl request failed: " + std::string(curl_easy_strerror(res)));
    }
    
    // Parse response (OpenAI-compatible format)
    Json::CharReaderBuilder reader_builder;
    Json::Value response_json;
    std::istringstream response_stream(response.data);
    std::string parse_errors;
    
    if (!Json::parseFromStream(reader_builder, response_stream, &response_json, &parse_errors)) {
        throw std::runtime_error("Failed to parse JSON response: " + parse_errors);
    }
    
    if (response_json.isMember("error")) {
        throw std::runtime_error("Ollama API Error: " + response_json["error"]["message"].asString());
    }
    
    // Extract message content from OpenAI-compatible response
    if (response_json.isMember("choices") && response_json["choices"].isArray() && 
        response_json["choices"].size() > 0) {
        return response_json["choices"][0]["message"]["content"].asString();
    }
    
    throw std::runtime_error("Unexpected Ollama API response format");
}

void OllamaClient::chat_completion_stream(
    const std::vector<Message>& messages,
    std::function<void(const std::string&)> callback) {
    
    // For now, implement non-streaming version and call callback with chunks
    // TODO: Implement actual streaming
    try {
        std::string response = chat_completion(messages);
        
        // Simulate streaming by sending response in chunks
        const size_t chunk_size = 50;
        for (size_t i = 0; i < response.length(); i += chunk_size) {
            std::string chunk = response.substr(i, chunk_size);
            callback(chunk);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    } catch (const std::exception& e) {
        throw std::runtime_error("Ollama streaming error: " + std::string(e.what()));
    }
}

std::vector<double> OllamaClient::embeddings(const std::string& text) {
    // Ollama supports embeddings through a different endpoint
    // For now, throw an informative error
    throw std::runtime_error("Ollama embeddings not yet implemented. Use the /api/embeddings endpoint.");
}

// Groq Client Implementation
GroqClient::GroqClient(const ClientConfig& config) : LLMClient(config) {
    if (config_.api_base.empty()) {
        config_.api_base = "https://api.groq.com/openai";
    }
}

std::string GroqClient::chat_completion(const std::vector<Message>& messages) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        throw std::runtime_error("Failed to initialize curl");
    }
    
    // Prepare Groq API request (OpenAI-compatible format)
    Json::Value payload;
    payload["model"] = config_.model;
    payload["temperature"] = config_.temperature;
    payload["top_p"] = config_.top_p;
    if (config_.max_tokens > 0) {
        payload["max_tokens"] = config_.max_tokens;
    }
    
    Json::Value messages_json(Json::arrayValue);
    for (const auto& msg : messages) {
        Json::Value msg_json;
        msg_json["role"] = msg.role;
        msg_json["content"] = msg.content;
        messages_json.append(msg_json);
    }
    payload["messages"] = messages_json;
    
    Json::StreamWriterBuilder builder;
    std::string json_string = Json::writeString(builder, payload);
    
    // Set curl options for Groq API
    std::string url = config_.api_base + "/v1/chat/completions";
    CurlResponse response;
    
    struct curl_slist* headers = nullptr;
    std::string auth_header = "Authorization: Bearer " + config_.api_key;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, auth_header.c_str());
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_string.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlResponse::WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    
    CURLcode res = curl_easy_perform(curl);
    
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    
    if (res != CURLE_OK) {
        throw std::runtime_error("Curl request failed: " + std::string(curl_easy_strerror(res)));
    }
    
    // Parse response (OpenAI-compatible format)
    Json::CharReaderBuilder reader_builder;
    Json::Value response_json;
    std::istringstream response_stream(response.data);
    std::string parse_errors;
    
    if (!Json::parseFromStream(reader_builder, response_stream, &response_json, &parse_errors)) {
        throw std::runtime_error("Failed to parse JSON response: " + parse_errors);
    }
    
    if (response_json.isMember("error")) {
        throw std::runtime_error("Groq API Error: " + response_json["error"]["message"].asString());
    }
    
    // Extract message content from OpenAI-compatible response
    if (response_json.isMember("choices") && response_json["choices"].isArray() && 
        response_json["choices"].size() > 0) {
        return response_json["choices"][0]["message"]["content"].asString();
    }
    
    throw std::runtime_error("Unexpected Groq API response format");
}

void GroqClient::chat_completion_stream(
    const std::vector<Message>& messages,
    std::function<void(const std::string&)> callback) {
    
    // For now, implement non-streaming version and call callback with chunks
    // TODO: Implement actual streaming
    try {
        std::string response = chat_completion(messages);
        
        // Simulate streaming by sending response in chunks
        const size_t chunk_size = 50;
        for (size_t i = 0; i < response.length(); i += chunk_size) {
            std::string chunk = response.substr(i, chunk_size);
            callback(chunk);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    } catch (const std::exception& e) {
        throw std::runtime_error("Groq streaming error: " + std::string(e.what()));
    }
}

std::vector<double> GroqClient::embeddings(const std::string& text) {
    // Groq doesn't provide embeddings API
    throw std::runtime_error("Groq does not provide an embeddings API. Use OpenAI or other providers for embeddings.");
}

// Factory function
std::unique_ptr<LLMClient> create_client(const ClientConfig& config) {
    if (config.provider == "openai") {
        return std::make_unique<OpenAIClient>(config);
    } else if (config.provider == "claude") {
        return std::make_unique<ClaudeClient>(config);
    } else if (config.provider == "gemini") {
        return std::make_unique<GeminiClient>(config);
    } else if (config.provider == "ollama") {
        return std::make_unique<OllamaClient>(config);
    } else if (config.provider == "groq") {
        return std::make_unique<GroqClient>(config);
    } else {
        throw std::runtime_error("Unknown provider: " + config.provider);
    }
}

// Atom conversion functions
Message atom_to_message(Handle message_atom) {
#ifdef HAVE_OPENCOG
    AtomSpace* as = message_atom->getAtomSpace();
    
    // Expected format: (MessageLink (ConceptNode "role") (ConceptNode "content"))
    if (message_atom->get_type() != MESSAGE_LINK) {
        throw std::runtime_error("Expected MessageLink atom");
    }
    
    const HandleSeq& outgoing = message_atom->getOutgoingSet();
    if (outgoing.size() != 2) {
        throw std::runtime_error("MessageLink must have exactly 2 outgoing atoms");
    }
    
    std::string role = outgoing[0]->get_name();
    std::string content = outgoing[1]->get_name();
    
    return Message(role, content);
#else
    // Placeholder for non-OpenCog builds
    throw std::runtime_error("atom_to_message not available without OpenCog");
#endif
}

Handle message_to_atom(AtomSpace* as, const Message& message) {
#ifdef HAVE_OPENCOG
    Handle role_node = as->add_node(CONCEPT_NODE, message.role);
    Handle content_node = as->add_node(CONCEPT_NODE, message.content);
    
    return as->add_link(MESSAGE_LINK, role_node, content_node);
#else
    // Placeholder for non-OpenCog builds
    return Handle_UNDEFINED;
#endif
}

// LLM Provider Router Implementation
void LLMProviderRouter::register_provider(const std::string& provider_name, const ProviderCapabilities& caps) {
    provider_capabilities_[provider_name] = caps;
}

std::string LLMProviderRouter::route_llm_request(const std::vector<Message>& messages, 
                                                const std::string& preferred_provider,
                                                const std::string& task_type) {
    // If preferred provider is specified and available, use it
    if (!preferred_provider.empty() && 
        provider_capabilities_.find(preferred_provider) != provider_capabilities_.end()) {
        
        const auto& caps = provider_capabilities_[preferred_provider];
        if ((task_type == "chat" && caps.supports_chat) ||
            (task_type == "embedding" && caps.supports_embeddings)) {
            return preferred_provider;
        }
    }
    
    // Otherwise, select the best provider based on criteria
    return select_provider(messages, task_type);
}

std::vector<std::string> LLMProviderRouter::get_available_providers(const std::string& task_type) {
    std::vector<std::string> available;
    
    for (const auto& [provider, caps] : provider_capabilities_) {
        if ((task_type == "chat" && caps.supports_chat) ||
            (task_type == "embedding" && caps.supports_embeddings) ||
            (task_type == "streaming" && caps.supports_streaming)) {
            available.push_back(provider);
        }
    }
    
    return available;
}

void LLMProviderRouter::init_default_providers() {
    // OpenAI capabilities
    ProviderCapabilities openai_caps;
    openai_caps.supports_chat = true;
    openai_caps.supports_streaming = true;
    openai_caps.supports_embeddings = true;
    openai_caps.supports_functions = true;
    openai_caps.supported_models = {"gpt-3.5-turbo", "gpt-4", "gpt-4-turbo", "gpt-4o"};
    openai_caps.cost_per_token = 0.000001;  // Rough estimate
    openai_caps.max_context_length = 8192;
    register_provider("openai", openai_caps);
    
    // Claude capabilities
    ProviderCapabilities claude_caps;
    claude_caps.supports_chat = true;
    claude_caps.supports_streaming = true;
    claude_caps.supports_embeddings = false;
    claude_caps.supports_functions = false;
    claude_caps.supported_models = {"claude-3-sonnet-20240229", "claude-3-opus-20240229", "claude-3-haiku-20240307"};
    claude_caps.cost_per_token = 0.000003;  // Rough estimate
    claude_caps.max_context_length = 200000;
    register_provider("claude", claude_caps);
    
    // Gemini capabilities
    ProviderCapabilities gemini_caps;
    gemini_caps.supports_chat = true;
    gemini_caps.supports_streaming = true;
    gemini_caps.supports_embeddings = false;  // Different API
    gemini_caps.supports_functions = true;
    gemini_caps.supported_models = {"gemini-pro", "gemini-pro-vision"};
    gemini_caps.cost_per_token = 0.0000005;  // Generally cheaper
    gemini_caps.max_context_length = 32768;
    register_provider("gemini", gemini_caps);
    
    // Ollama capabilities (local deployment)
    ProviderCapabilities ollama_caps;
    ollama_caps.supports_chat = true;
    ollama_caps.supports_streaming = true;
    ollama_caps.supports_embeddings = false;  // Different endpoint
    ollama_caps.supports_functions = false;
    ollama_caps.supported_models = {"llama2", "codellama", "mistral"};
    ollama_caps.cost_per_token = 0.0;  // Local deployment
    ollama_caps.max_context_length = 4096;
    register_provider("ollama", ollama_caps);
    
    // Groq capabilities (fast inference)
    ProviderCapabilities groq_caps;
    groq_caps.supports_chat = true;
    groq_caps.supports_streaming = true;
    groq_caps.supports_embeddings = false;
    groq_caps.supports_functions = false;
    groq_caps.supported_models = {"mixtral-8x7b-32768", "llama2-70b-4096"};
    groq_caps.cost_per_token = 0.0000002;  // Very fast and cheap
    groq_caps.max_context_length = 32768;
    register_provider("groq", groq_caps);
}

std::string LLMProviderRouter::select_provider(const std::vector<Message>& messages, 
                                              const std::string& task_type) {
    std::vector<std::pair<std::string, double>> provider_scores;
    
    // Calculate context length needed
    size_t total_context = 0;
    for (const auto& msg : messages) {
        total_context += msg.content.length();
    }
    
    for (const auto& [provider, caps] : provider_capabilities_) {
        double score = 0.0;
        
        // Check basic compatibility
        if (task_type == "chat" && !caps.supports_chat) continue;
        if (task_type == "embedding" && !caps.supports_embeddings) continue;
        
        // Score based on context length support
        if (total_context <= caps.max_context_length) {
            score += 10.0;  // Can handle the context
        } else {
            continue;  // Skip if can't handle context
        }
        
        // Score based on cost (lower cost = higher score)
        if (caps.cost_per_token == 0.0) {
            score += 5.0;  // Free (local) providers get bonus
        } else {
            score += (1.0 / caps.cost_per_token) * 0.000001;  // Inverse cost scoring
        }
        
        // Bonus for additional capabilities
        if (caps.supports_functions) score += 2.0;
        if (caps.supports_streaming) score += 1.0;
        
        provider_scores.push_back({provider, score});
    }
    
    if (provider_scores.empty()) {
        throw std::runtime_error("No suitable provider found for task: " + task_type);
    }
    
    // Sort by score (highest first)
    std::sort(provider_scores.begin(), provider_scores.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });
    
    return provider_scores[0].first;  // Return best provider
}