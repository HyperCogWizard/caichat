/**
 * SchemeBindings.cc
 * 
 * Scheme bindings for CAIChat OpenCog module
 */

#include "LLMClient.h"
#include "ChatCompletion.h"
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
}