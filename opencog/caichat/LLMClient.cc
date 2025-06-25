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
    // Placeholder implementation - similar to OpenAI but with Claude API format
    // TODO: Implement Claude-specific API calls
    throw std::runtime_error("Claude client not yet implemented");
}

void ClaudeClient::chat_completion_stream(
    const std::vector<Message>& messages,
    std::function<void(const std::string&)> callback) {
    throw std::runtime_error("Claude streaming not yet implemented");
}

std::vector<double> ClaudeClient::embeddings(const std::string& text) {
    throw std::runtime_error("Claude embeddings not yet implemented");
}

// Factory function
std::unique_ptr<LLMClient> create_client(const ClientConfig& config) {
    if (config.provider == "openai") {
        return std::make_unique<OpenAIClient>(config);
    } else if (config.provider == "claude") {
        return std::make_unique<ClaudeClient>(config);
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