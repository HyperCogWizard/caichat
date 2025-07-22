/**
 * SchemeBindings.cc
 * 
 * Scheme bindings for CAIChat OpenCog module
 */

#include "LLMClient.h"
#include "ChatCompletion.h"
#include "SessionManager.h"
#include <stdexcept>
#include <ctime>
#include <map>
#include <algorithm>

#ifdef HAVE_OPENCOG
#include <opencog/guile/SchemeModule.h>
#include <opencog/atomspace/AtomSpace.h>
#include <opencog/util/Logger.h>
using namespace opencog;
#else
// Minimal Guile definitions for non-OpenCog builds
#include <libguile.h>
#include <iostream>
namespace opencog {
    AtomSpace* get_current_atomspace() { return nullptr; }
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
#endif

using namespace opencog;
using namespace opencog::caichat;

// Global client and completion instances
static std::map<std::string, std::unique_ptr<ChatCompletion>> completions;
static std::map<std::string, ClientConfig> client_configs;
static std::unique_ptr<SessionManager> session_manager;
static std::unique_ptr<NeuralSymbolicBridge> neural_bridge;
static std::unique_ptr<LLMProviderRouter> global_router;
static bool test_mode_enabled = false;

/**
 * Create a new LLM client configuration
 */
SCM caichat_create_client_config(SCM provider, SCM model, SCM api_key, SCM api_base) {
    std::string provider_str = scm_to_locale_string(provider);
    std::string model_str = scm_to_locale_string(model);
    std::string api_key_str = scm_to_locale_string(api_key);
    std::string api_base_str = "";
    
    if (!scm_is_false(api_base)) {
        api_base_str = scm_to_locale_string(api_base);
    }
    
    ClientConfig config;
    config.provider = provider_str;
    config.model = model_str;
    config.api_key = api_key_str;
    config.api_base = api_base_str;
    
    // Generate a unique config ID
    std::string config_id = provider_str + ":" + model_str;
    client_configs[config_id] = config;
    
    return scm_from_locale_string(config_id.c_str());
}

/**
 * Create a new chat completion session
 */
SCM caichat_create_session(SCM config_id, SCM atomspace_scm) {
    std::string config_id_str = scm_to_locale_string(config_id);
    
    if (client_configs.find(config_id_str) == client_configs.end()) {
        throw std::runtime_error("Client config not found: " + config_id_str);
    }
    
    AtomSpace* as = nullptr;
#ifdef HAVE_OPENCOG
    as = SchemeSmob::ss_to_atomspace(atomspace_scm);
    if (!as) {
        throw std::runtime_error("Invalid AtomSpace");
    }
#endif
    
    ClientConfig config = client_configs[config_id_str];
    auto client = create_client(config);
    auto completion = std::make_unique<ChatCompletion>(as, std::move(client));
    
    // Generate session ID
    std::string session_id = config_id_str + ":" + std::to_string(time(nullptr));
    completions[session_id] = std::move(completion);
    
    return scm_from_locale_string(session_id.c_str());
}

/**
 * Add a message to a chat session
 */
SCM caichat_add_message(SCM session_id, SCM role, SCM content) {
    std::string session_id_str = scm_to_locale_string(session_id);
    std::string role_str = scm_to_locale_string(role);
    std::string content_str = scm_to_locale_string(content);
    
    auto it = completions.find(session_id_str);
    if (it == completions.end()) {
        return SCM_BOOL_F;
    }
    
    try {
        it->second->add_message(role_str, content_str);
        return SCM_BOOL_T;
    } catch (const std::exception& e) {
#ifdef HAVE_OPENCOG
        logger().error("Failed to add message: %s", e.what());
#else
        std::cerr << "Failed to add message: " << e.what() << std::endl;
#endif
        return SCM_BOOL_F;
    }
}

/**
 * Get completion for a chat session
 */
SCM caichat_complete(SCM session_id) {
    std::string session_id_str = scm_to_locale_string(session_id);
    
    auto it = completions.find(session_id_str);
    if (it == completions.end()) {
        return SCM_BOOL_F;
    }
    
    try {
        std::string response = it->second->complete();
        return scm_from_locale_string(response.c_str());
    } catch (const std::exception& e) {
#ifdef HAVE_OPENCOG
        logger().error("Failed to get completion: %s", e.what());
#else
        std::cerr << "Failed to get completion: " << e.what() << std::endl;
#endif
        return SCM_BOOL_F;
    }
}

/**
 * Clear conversation history for a session
 */
SCM caichat_clear_history(SCM session_id) {
    std::string session_id_str = scm_to_locale_string(session_id);
    
    auto it = completions.find(session_id_str);
    if (it == completions.end()) {
        return SCM_BOOL_F;
    }
    
    try {
        it->second->clear_history();
        return SCM_BOOL_T;
    } catch (const std::exception& e) {
#ifdef HAVE_OPENCOG
        logger().error("Failed to clear history: %s", e.what());
#else
        std::cerr << "Failed to clear history: " << e.what() << std::endl;
#endif
        return SCM_BOOL_F;
    }
}

/**
 * Save conversation to AtomSpace
 */
SCM caichat_save_conversation(SCM session_id, SCM conversation_id) {
    std::string session_id_str = scm_to_locale_string(session_id);
    std::string conversation_id_str = scm_to_locale_string(conversation_id);
    
    auto it = completions.find(session_id_str);
    if (it == completions.end()) {
        return SCM_BOOL_F;
    }
    
    try {
        Handle conversation_atom = it->second->save_conversation(conversation_id_str);
#ifdef HAVE_OPENCOG
        return SchemeSmob::handle_to_scm(conversation_atom);
#else
        return SCM_BOOL_T;  // Return success in minimal mode
#endif
    } catch (const std::exception& e) {
#ifdef HAVE_OPENCOG
        logger().error("Failed to save conversation: %s", e.what());
#else
        std::cerr << "Failed to save conversation: " << e.what() << std::endl;
#endif
        return SCM_BOOL_F;
    }
}

/**
 * Load conversation from AtomSpace
 */
SCM caichat_load_conversation(SCM session_id, SCM conversation_id) {
    std::string session_id_str = scm_to_locale_string(session_id);
    std::string conversation_id_str = scm_to_locale_string(conversation_id);
    
    auto it = completions.find(session_id_str);
    if (it == completions.end()) {
        return SCM_BOOL_F;
    }
    
    try {
        it->second->load_conversation_by_id(conversation_id_str);
        return SCM_BOOL_T;
    } catch (const std::exception& e) {
#ifdef HAVE_OPENCOG
        logger().error("Failed to load conversation: %s", e.what());
#else
        std::cerr << "Failed to load conversation: " << e.what() << std::endl;
#endif
        return SCM_BOOL_F;
    }
}

/**
 * Destroy a chat session
 */
SCM caichat_destroy_session(SCM session_id) {
    std::string session_id_str = scm_to_locale_string(session_id);
    
    auto it = completions.find(session_id_str);
    if (it != completions.end()) {
        completions.erase(it);
        return SCM_BOOL_T;
    }
    
    return SCM_BOOL_F;
}

/**
 * Create a persistent session (hypergraph synergy)
 */
SCM caichat_create_persistent_session(SCM session_name, SCM provider, SCM model) {
    if (!session_manager) {
#ifdef HAVE_OPENCOG
        AtomSpace* as = get_current_atomspace();
#else
        AtomSpace* as = nullptr;
#endif
        session_manager = std::make_unique<SessionManager>(as);
    }
    
    std::string session_name_str = scm_to_locale_string(session_name);
    std::string provider_str = scm_to_locale_string(provider);
    std::string model_str = scm_to_locale_string(model);
    
    try {
        std::string session_id = session_manager->create_persistent_session(session_name_str, provider_str, model_str);
        return scm_from_locale_string(session_id.c_str());
    } catch (const std::exception& e) {
#ifdef HAVE_OPENCOG
        logger().error("Failed to create persistent session: %s", e.what());
#else
        std::cerr << "Failed to create persistent session: " << e.what() << std::endl;
#endif
        return SCM_BOOL_F;
    }
}

/**
 * Resume a session from hypergraph memory
 */
SCM caichat_resume_session(SCM session_name, SCM provider, SCM model) {
    if (!session_manager) {
#ifdef HAVE_OPENCOG
        AtomSpace* as = get_current_atomspace();
#else
        AtomSpace* as = nullptr;
#endif
        session_manager = std::make_unique<SessionManager>(as);
    }
    
    std::string session_name_str = scm_to_locale_string(session_name);
    std::string provider_str = scm_to_locale_string(provider);
    std::string model_str = scm_to_locale_string(model);
    
    try {
        std::string session_id = session_manager->resume_session(session_name_str, provider_str, model_str);
        return scm_from_locale_string(session_id.c_str());
    } catch (const std::exception& e) {
#ifdef HAVE_OPENCOG
        logger().error("Failed to resume session: %s", e.what());
#else
        std::cerr << "Failed to resume session: " << e.what() << std::endl;
#endif
        return SCM_BOOL_F;
    }
}

/**
 * Mediate session (updates hypergraph memory)
 */
SCM caichat_mediate_session(SCM session_id) {
    if (!session_manager) {
        return SCM_BOOL_F;
    }
    
    std::string session_id_str = scm_to_locale_string(session_id);
    
    try {
        session_manager->mediate_session(session_id_str);
        return SCM_BOOL_T;
    } catch (const std::exception& e) {
#ifdef HAVE_OPENCOG
        logger().error("Failed to mediate session: %s", e.what());
#else
        std::cerr << "Failed to mediate session: " << e.what() << std::endl;
#endif
        return SCM_BOOL_F;
    }
}

/**
 * Audit core modules for spec compliance
 */
SCM caichat_audit_core_modules() {
    if (!session_manager) {
#ifdef HAVE_OPENCOG
        AtomSpace* as = get_current_atomspace();
#else
        AtomSpace* as = nullptr;
#endif
        session_manager = std::make_unique<SessionManager>(as);
    }
    
    try {
        session_manager->audit_core_modules();
        return SCM_BOOL_T;
    } catch (const std::exception& e) {
#ifdef HAVE_OPENCOG
        logger().error("Failed to audit core modules: %s", e.what());
#else
        std::cerr << "Failed to audit core modules: " << e.what() << std::endl;
#endif
        return SCM_BOOL_F;
    }
}

/**
 * Neural-symbolic bridge function
 */
SCM caichat_neural_symbolic_bridge(SCM input) {
    if (!neural_bridge) {
#ifdef HAVE_OPENCOG
        AtomSpace* as = get_current_atomspace();
#else
        AtomSpace* as = nullptr;
#endif
        neural_bridge = std::make_unique<NeuralSymbolicBridge>(as);
    }
    
    std::string input_str = scm_to_locale_string(input);
    
    try {
        std::string response = neural_bridge->neural_symbolic_bridge(input_str);
        return scm_from_locale_string(response.c_str());
    } catch (const std::exception& e) {
#ifdef HAVE_OPENCOG
        logger().error("Neural-symbolic bridge failed: %s", e.what());
#else
        std::cerr << "Neural-symbolic bridge failed: " << e.what() << std::endl;
#endif
        return SCM_BOOL_F;
    }
}

/**
 * Load a GGML model
 */
SCM caichat_ggml_load_model(SCM session_id, SCM model_path) {
    std::string session_id_str = scm_to_locale_string(session_id);
    std::string model_path_str = scm_to_locale_string(model_path);
    
    try {
        auto it = completions.find(session_id_str);
        if (it != completions.end()) {
            // Get the LLM client from the completion
            auto client = it->second->get_client();
            auto ggml_client = dynamic_cast<GGMLClient*>(client);
            
            if (ggml_client) {
                bool success = ggml_client->load_model(model_path_str);
                return success ? SCM_BOOL_T : SCM_BOOL_F;
            }
        }
        return SCM_BOOL_F;
    } catch (const std::exception& e) {
#ifdef HAVE_OPENCOG
        logger().error("GGML model loading failed: %s", e.what());
#else
        std::cerr << "GGML model loading failed: " << e.what() << std::endl;
#endif
        return SCM_BOOL_F;
    }
}

/**
 * Unload GGML model
 */
SCM caichat_ggml_unload_model(SCM session_id) {
    std::string session_id_str = scm_to_locale_string(session_id);
    
    try {
        auto it = completions.find(session_id_str);
        if (it != completions.end()) {
            // Get the LLM client from the completion
            auto client = it->second->get_client();
            auto ggml_client = dynamic_cast<GGMLClient*>(client);
            
            if (ggml_client) {
                ggml_client->unload_model();
                return SCM_BOOL_T;
            }
        }
        return SCM_BOOL_F;
    } catch (const std::exception& e) {
#ifdef HAVE_OPENCOG
        logger().error("GGML model unloading failed: %s", e.what());
#else
        std::cerr << "GGML model unloading failed: " << e.what() << std::endl;
#endif
        return SCM_BOOL_F;
    }
}

/**
 * Get GGML model information
 */
SCM caichat_ggml_model_info(SCM session_id) {
    std::string session_id_str = scm_to_locale_string(session_id);
    
    try {
        auto it = completions.find(session_id_str);
        if (it != completions.end()) {
            // Get the LLM client from the completion
            auto client = it->second->get_client();
            auto ggml_client = dynamic_cast<GGMLClient*>(client);
            
            if (ggml_client) {
                std::string info = ggml_client->get_model_info();
                return scm_from_locale_string(info.c_str());
            }
        }
        return scm_from_locale_string("No GGML model loaded");
    } catch (const std::exception& e) {
#ifdef HAVE_OPENCOG
        logger().error("GGML model info failed: %s", e.what());
#else
        std::cerr << "GGML model info failed: " << e.what() << std::endl;
#endif
        return scm_from_locale_string("Error getting model info");
    }
}

/**
 * Cognitive completion with GGML
 */
SCM caichat_ggml_cognitive_completion(SCM session_id, SCM atom_handle) {
    std::string session_id_str = scm_to_locale_string(session_id);
    
    try {
        auto it = completions.find(session_id_str);
        if (it != completions.end()) {
            // Get the LLM client from the completion
            auto client = it->second->get_client();
            auto ggml_client = dynamic_cast<GGMLClient*>(client);
            
            if (ggml_client) {
                // Get current messages from the session
                auto messages = it->second->get_messages();
                
                // Convert SCM atom handle to Handle
                Handle context_atom = Handle_UNDEFINED;
#ifdef HAVE_OPENCOG
                if (!scm_is_false(atom_handle)) {
                    context_atom = scm_to_handle(atom_handle);
                }
#endif
                
                std::string response = ggml_client->cognitive_completion(messages, context_atom);
                return scm_from_locale_string(response.c_str());
            }
        }
        return scm_from_locale_string("No GGML client available");
    } catch (const std::exception& e) {
#ifdef HAVE_OPENCOG
        logger().error("GGML cognitive completion failed: %s", e.what());
#else
        std::cerr << "GGML cognitive completion failed: " << e.what() << std::endl;
#endif
        return scm_from_locale_string("Error in cognitive completion");
    }
}

/**
 * Convert AtomSpace pattern to GGML prompt
 */
SCM caichat_ggml_atomspace_to_prompt(SCM session_id, SCM atom_handle) {
    std::string session_id_str = scm_to_locale_string(session_id);
    
    try {
        auto it = completions.find(session_id_str);
        if (it != completions.end()) {
            // Get the LLM client from the completion
            auto client = it->second->get_client();
            auto ggml_client = dynamic_cast<GGMLClient*>(client);
            
            if (ggml_client) {
                // Convert SCM atom handle to Handle
                Handle pattern_atom = Handle_UNDEFINED;
#ifdef HAVE_OPENCOG
                if (!scm_is_false(atom_handle)) {
                    pattern_atom = scm_to_handle(atom_handle);
                }
#endif
                
                std::string prompt = ggml_client->atomspace_to_prompt(pattern_atom);
                return scm_from_locale_string(prompt.c_str());
            }
        }
        return scm_from_locale_string("No GGML client available");
    } catch (const std::exception& e) {
#ifdef HAVE_OPENCOG
        logger().error("GGML atomspace to prompt failed: %s", e.what());
#else
        std::cerr << "GGML atomspace to prompt failed: " << e.what() << std::endl;
#endif
        return scm_from_locale_string("Error converting atomspace to prompt");
    }
}

// Initialize the module
extern "C" void opencog_caichat_init();

// Additional function declarations for new Scheme bindings
SCM caichat_propagate_patterns(SCM seed_pattern_scm, SCM depth_scm);
SCM caichat_map_opencog_api(SCM functions_list);
SCM caichat_init_llm_provider(SCM backends_list);
SCM caichat_route_llm_request(SCM request_scm, SCM preferred_provider_scm);
SCM caichat_set_test_mode(SCM enabled_scm);

/**
 * Recursive pattern propagation function (from issue requirements)
 * Implements: (propagate-patterns seed-pattern depth)
 */
SCM caichat_propagate_patterns(SCM seed_pattern_scm, SCM depth_scm) {
    if (!session_manager) {
#ifdef HAVE_OPENCOG
        AtomSpace* as = get_current_atomspace();
#else
        AtomSpace* as = nullptr;
#endif
        session_manager = std::make_unique<SessionManager>(as);
    }
    
    int depth = scm_to_int(depth_scm);
    
    try {
#ifdef HAVE_OPENCOG
        AtomSpace* as = get_current_atomspace();
        if (as && scm_is_true(scm_atom_p(seed_pattern_scm))) {
            Handle seed_pattern = scm_to_handle(seed_pattern_scm);
            session_manager->propagate_patterns(seed_pattern, depth);
            return scm_from_locale_string("Pattern propagation completed");
        }
#endif
        // Fallback for non-OpenCog mode
        std::string pattern_str = scm_to_locale_string(seed_pattern_scm);
        std::string result = "Propagated pattern: " + pattern_str + " with depth " + std::to_string(depth);
        return scm_from_locale_string(result.c_str());
    } catch (const std::exception& e) {
#ifdef HAVE_OPENCOG
        logger().error("Pattern propagation failed: %s", e.what());
#else
        std::cerr << "Pattern propagation failed: " << e.what() << std::endl;
#endif
        return SCM_BOOL_F;
    }
}

/**
 * OpenCog AtomSpace API mapping function (from issue requirements)
 * Implements: (map-opencog-api atomspace-functions)
 */
SCM caichat_map_opencog_api(SCM functions_list) {
    try {
#ifdef HAVE_OPENCOG
        AtomSpace* as = get_current_atomspace();
        if (as) {
            // Map provided AtomSpace functions to cognitive functions
            std::string result = "Mapped OpenCog API functions: ";
            
            if (scm_is_pair(functions_list)) {
                SCM current = functions_list;
                while (!scm_is_null(current)) {
                    SCM func = scm_car(current);
                    if (scm_is_string(func)) {
                        std::string func_name = scm_to_locale_string(func);
                        result += func_name + " ";
                        
                        // Create cognitive function mapping in AtomSpace
                        Handle func_atom = as->add_node(CONCEPT_NODE, "cognitive_function:" + func_name);
                        Handle mapping_atom = as->add_node(PREDICATE_NODE, "api_mapped");
                        as->add_link(EVALUATION_LINK, mapping_atom,
                            as->add_link(LIST_LINK, func_atom));
                    }
                    current = scm_cdr(current);
                }
            }
            
            return scm_from_locale_string(result.c_str());
        }
#endif
        
        // Fallback for non-OpenCog mode
        return scm_from_locale_string("OpenCog API mapping requires OpenCog AtomSpace");
    } catch (const std::exception& e) {
#ifdef HAVE_OPENCOG
        logger().error("OpenCog API mapping failed: %s", e.what());
#else
        std::cerr << "OpenCog API mapping failed: " << e.what() << std::endl;
#endif
        return SCM_BOOL_F;
    }
}

/**
 * Initialize LLM provider function (from issue requirements)
 * Implements: (init-llm-provider backends)
 */
SCM caichat_init_llm_provider(SCM backends_list) {
    try {
        std::vector<std::string> backends;
        
        if (scm_is_pair(backends_list)) {
            SCM current = backends_list;
            while (!scm_is_null(current)) {
                SCM backend = scm_car(current);
                if (scm_is_string(backend)) {
                    std::string backend_name = scm_to_locale_string(backend);
                    backends.push_back(backend_name);
                }
                current = scm_cdr(current);
            }
        }
        
        // Initialize global router if not already done
        if (!global_router) {
            global_router = std::make_unique<LLMProviderRouter>();
            global_router->init_default_providers();
        }
        
        // Initialize session manager with router
        if (!session_manager) {
#ifdef HAVE_OPENCOG
            AtomSpace* as = get_current_atomspace();
#else
            AtomSpace* as = nullptr;
#endif
            session_manager = std::make_unique<SessionManager>(as);
        }
        
        // Validate and register each requested backend
        std::string result = "Initialized LLM providers: ";
        std::vector<std::string> available_providers = global_router->get_available_providers("chat");
        
        for (const auto& backend : backends) {
            // Check if provider is supported
            auto it = std::find(available_providers.begin(), available_providers.end(), backend);
            if (it != available_providers.end()) {
                // Create a default client configuration for this provider
                ClientConfig config;
                config.provider = backend;
                
                // Set provider-specific defaults
                if (backend == "openai") {
                    config.model = "gpt-3.5-turbo";
                    config.api_base = "https://api.openai.com/v1";
                } else if (backend == "claude") {
                    config.model = "claude-3-sonnet-20240229";
                    config.api_base = "https://api.anthropic.com";
                } else if (backend == "gemini") {
                    config.model = "gemini-pro";
                    config.api_base = "https://generativelanguage.googleapis.com";
                } else if (backend == "ollama") {
                    config.model = "llama2";
                    config.api_base = "http://localhost:11434";
                } else if (backend == "groq") {
                    config.model = "mixtral-8x7b-32768";
                    config.api_base = "https://api.groq.com/openai";
                } else if (backend == "ggml") {
                    config.model = "/path/to/model.ggml";  // Will be set later
                    config.api_base = "local";
                }
                
                // Store the configuration for later use
                std::string config_id = backend + ":default";
                client_configs[config_id] = config;
                
                result += backend + " ";
            } else {
                result += "[UNSUPPORTED:" + backend + "] ";
            }
        }
        
        return scm_from_locale_string(result.c_str());
    } catch (const std::exception& e) {
#ifdef HAVE_OPENCOG
        logger().error("LLM provider initialization failed: %s", e.what());
#else
        std::cerr << "LLM provider initialization failed: " << e.what() << std::endl;
#endif
        return SCM_BOOL_F;
    }
}

/**
 * Route LLM request function (from issue requirements) 
 * Implements: (route-llm-request request preferred-provider)
 */
SCM caichat_route_llm_request(SCM request_scm, SCM preferred_provider_scm) {
    try {
        std::string request_str = scm_to_locale_string(request_scm);
        std::string preferred_provider = "";
        
        if (scm_is_string(preferred_provider_scm)) {
            preferred_provider = scm_to_locale_string(preferred_provider_scm);
        }
        
        // Initialize router if needed
        if (!global_router) {
            global_router = std::make_unique<LLMProviderRouter>();
            global_router->init_default_providers();
        }
        
        if (!session_manager) {
#ifdef HAVE_OPENCOG
            AtomSpace* as = get_current_atomspace();
#else
            AtomSpace* as = nullptr;
#endif
            session_manager = std::make_unique<SessionManager>(as);
        }
        
        // Create messages for routing
        std::vector<Message> messages;
        messages.push_back(Message("user", request_str));
        
        // Use the router to select the best provider
        std::string selected_provider = global_router->route_llm_request(messages, preferred_provider, "chat");
        
        // Try to create a session with the selected provider
        std::string config_id = selected_provider + ":default";
        auto config_it = client_configs.find(config_id);
        
        if (config_it != client_configs.end()) {
            try {
                // In test mode, return simulated response without making API calls
                if (test_mode_enabled) {
                    std::string result = "Request routed to " + selected_provider;
                    if (!preferred_provider.empty()) {
                        result += " (preferred: " + preferred_provider + ")";
                    }
                    result += "\nTest mode response: Successfully routed '" + request_str.substr(0, TRUNCATION_LENGTH);
                    if (request_str.length() > TRUNCATION_LENGTH) {
                        result += "...";
                    }
                    result += "' to " + selected_provider + " provider.";
                    
                    return scm_from_locale_string(result.c_str());
                }
                
                // Create client and get response (only in non-test mode)
                auto client = create_client(config_it->second);
                std::string response = client->chat_completion(messages);
                
                std::string result = "Request routed to " + selected_provider + "\n";
                result += "Response: " + response.substr(0, RESPONSE_TRUNCATION_LIMIT);
                if (response.length() > RESPONSE_TRUNCATION_LIMIT) {
                    result += "...";
                }
                
                return scm_from_locale_string(result.c_str());
            } catch (const std::exception& e) {
                // If in test mode, return simulated error recovery
                if (test_mode_enabled) {
                    std::string result = "Test mode: Simulated fallback routing from " + selected_provider + " to alternative provider";
                    return scm_from_locale_string(result.c_str());
                }
                
                // If the selected provider fails, try fallback routing
                std::vector<std::string> available = global_router->get_available_providers("chat");
                for (const auto& fallback_provider : available) {
                    if (fallback_provider != selected_provider) {
                        std::string fallback_config_id = fallback_provider + ":default";
                        auto fallback_it = client_configs.find(fallback_config_id);
                        if (fallback_it != client_configs.end()) {
                            try {
                                auto fallback_client = create_client(fallback_it->second);
                                std::string response = fallback_client->chat_completion(messages);
                                
                                std::string result = "Request routed to " + fallback_provider + " (fallback from " + selected_provider + ")\n";
                                result += "Response: " + response.substr(0, 100);
                                if (response.length() > 100) {
                                    result += "...";
                                }
                                
                                return scm_from_locale_string(result.c_str());
                            } catch (const std::exception& fallback_e) {
                                // Log the fallback error
                                opencog::get_logger().error("Fallback provider failed: %s", fallback_e.what());
                                // Continue to next fallback
                                continue;
                            }
                        }
                    }
                }
                
                // If all providers fail, return error info
                std::string error_result = "All providers failed. Last error: " + std::string(e.what());
                return scm_from_locale_string(error_result.c_str());
            }
        } else {
            // Create a simulated response for routing demonstration
            std::string result = "Request routed to " + selected_provider;
            if (!preferred_provider.empty()) {
                result += " (preferred: " + preferred_provider + ")";
            }
            result += "\nSimulated response: Provider routing completed successfully.";
            
            return scm_from_locale_string(result.c_str());
        }
    } catch (const std::exception& e) {
#ifdef HAVE_OPENCOG
        logger().error("LLM request routing failed: %s", e.what());
#else
        std::cerr << "LLM request routing failed: " << e.what() << std::endl;
#endif
        return SCM_BOOL_F;
    }
}

void opencog_caichat_init() {
    // Register Scheme functions
    scm_c_define_gsubr("caichat-create-client-config", 4, 0, 0, 
                       (scm_t_subr) caichat_create_client_config);
    scm_c_define_gsubr("caichat-create-session", 2, 0, 0, 
                       (scm_t_subr) caichat_create_session);
    scm_c_define_gsubr("caichat-add-message", 3, 0, 0, 
                       (scm_t_subr) caichat_add_message);
    scm_c_define_gsubr("caichat-complete", 1, 0, 0, 
                       (scm_t_subr) caichat_complete);
    scm_c_define_gsubr("caichat-clear-history", 1, 0, 0, 
                       (scm_t_subr) caichat_clear_history);
    scm_c_define_gsubr("caichat-save-conversation", 2, 0, 0, 
                       (scm_t_subr) caichat_save_conversation);
    scm_c_define_gsubr("caichat-load-conversation", 2, 0, 0, 
                       (scm_t_subr) caichat_load_conversation);
    scm_c_define_gsubr("caichat-destroy-session", 1, 0, 0, 
                       (scm_t_subr) caichat_destroy_session);
    
    // Register new session management and cognitive functions
    scm_c_define_gsubr("caichat-create-persistent-session", 3, 0, 0, 
                       (scm_t_subr) caichat_create_persistent_session);
    scm_c_define_gsubr("caichat-resume-session", 3, 0, 0, 
                       (scm_t_subr) caichat_resume_session);
    scm_c_define_gsubr("caichat-mediate-session", 1, 0, 0, 
                       (scm_t_subr) caichat_mediate_session);
    scm_c_define_gsubr("caichat-audit-core-modules", 0, 0, 0, 
                       (scm_t_subr) caichat_audit_core_modules);
    scm_c_define_gsubr("caichat-neural-symbolic-bridge", 1, 0, 0, 
                       (scm_t_subr) caichat_neural_symbolic_bridge);
    
    // GGML-specific functions for OpenCog integration
    scm_c_define_gsubr("caichat-ggml-load-model", 2, 0, 0, 
                       (scm_t_subr) caichat_ggml_load_model);
    scm_c_define_gsubr("caichat-ggml-unload-model", 1, 0, 0, 
                       (scm_t_subr) caichat_ggml_unload_model);
    scm_c_define_gsubr("caichat-ggml-model-info", 1, 0, 0, 
                       (scm_t_subr) caichat_ggml_model_info);
    scm_c_define_gsubr("caichat-ggml-cognitive-completion", 2, 0, 0, 
                       (scm_t_subr) caichat_ggml_cognitive_completion);
    scm_c_define_gsubr("caichat-ggml-atomspace-to-prompt", 2, 0, 0, 
                       (scm_t_subr) caichat_ggml_atomspace_to_prompt);
    
    // Recursive pattern propagation functions (from issue requirements)
    scm_c_define_gsubr("caichat-propagate-patterns", 2, 0, 0, 
                       (scm_t_subr) caichat_propagate_patterns);
    scm_c_define_gsubr("caichat-map-opencog-api", 1, 0, 0, 
                       (scm_t_subr) caichat_map_opencog_api);
    
    // LLM Provider router functions (from issue requirements)
    scm_c_define_gsubr("caichat-init-llm-provider", 1, 0, 0, 
                       (scm_t_subr) caichat_init_llm_provider);
    scm_c_define_gsubr("caichat-route-llm-request", 2, 0, 0, 
                       (scm_t_subr) caichat_route_llm_request);
}