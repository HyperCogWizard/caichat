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
}