/**
 * SessionManager.h
 * 
 * Cognitive session persistence and hypergraph memory synergization
 * Implements the mediate-session and audit-core-modules functionality
 */

#ifndef _OPENCOG_CAICHAT_SESSION_MANAGER_H
#define _OPENCOG_CAICHAT_SESSION_MANAGER_H

#include "LLMClient.h"
#include "ChatCompletion.h"
#include <string>
#include <memory>
#include <map>
#include <vector>
#include <ctime>

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
 * Cognitive Session Manager - implements hypergraph memory synergization
 * and persistent session mediation as described in the issue
 */
class SessionManager {
public:
    struct SessionMetadata {
        std::string session_id;
        std::string provider;
        std::string model;
        std::time_t created_at;
        std::time_t last_accessed;
        size_t message_count;
        bool is_persistent;
        Handle session_atom;  // AtomSpace handle if using OpenCog
    };
    
    SessionManager(AtomSpace* atomspace = nullptr);
    ~SessionManager() = default;
    
    /**
     * Create a persistent session that will be saved to hypergraph memory
     * Implements: caichat-create-persistent-session
     */
    std::string create_persistent_session(const std::string& session_name, 
                                         const std::string& provider, 
                                         const std::string& model);
    
    /**
     * Resume a previously saved session from hypergraph memory
     * Implements: caichat-resume-session
     */
    std::string resume_session(const std::string& session_name, 
                              const std::string& provider, 
                              const std::string& model);
    
    /**
     * Mediate session - updates hypergraph memory and manages persistence
     * Implements: mediate-session from the recursive scheme pseudocode
     */
    void mediate_session(const std::string& session_id);
    
    /**
     * Audit core modules for spec compliance and hypergraph synergy
     * Implements: audit-core-modules from the recursive scheme pseudocode
     */
    void audit_core_modules();
    
    /**
     * Establish hypergraph link between session and global memory
     */
    void establish_hypergraph_link(const std::string& session_id, Handle module_atom);
    
    /**
     * Update hypergraph memory with session data
     */
    void update_hypergraph_memory(const std::string& session_id);
    
    /**
     * Persist session data to long-term storage
     */
    void persist_session(const std::string& session_id);
    
    /**
     * Check if session is active
     */
    bool is_active(const std::string& session_id);
    
    /**
     * Get session metadata
     */
    SessionMetadata get_session_metadata(const std::string& session_id);
    
    /**
     * List all sessions with their metadata
     */
    std::vector<SessionMetadata> list_sessions();
    
    /**
     * Get sessions by provider or model
     */
    std::vector<SessionMetadata> get_sessions_by_provider(const std::string& provider);
    
    /**
     * Cleanup inactive sessions
     */
    void cleanup_inactive_sessions(int max_age_hours = 24);

private:
    AtomSpace* atomspace_;
    std::map<std::string, std::unique_ptr<ChatCompletion>> sessions_;
    std::map<std::string, SessionMetadata> session_metadata_;
    LLMProviderRouter router_;
    
    /**
     * Generate unique session ID
     */
    std::string generate_session_id();
    
    /**
     * Create session atom in AtomSpace (if available)
     */
    Handle create_session_atom(const SessionMetadata& metadata);
    
    /**
     * Load session from AtomSpace (if available)
     */
    SessionMetadata load_session_from_atomspace(const std::string& session_name);
    
    /**
     * Encode session patterns into hypergraph
     */
    void encode_session_patterns(const std::string& session_id);
    
    /**
     * Propagate cognitive patterns through the hypergraph
     */
    void propagate_patterns(Handle seed_pattern, int depth = 3);
};

/**
 * Neural-Symbolic Bridge - maps LLM responses to AtomSpace representations
 * Implements: neural-symbolic-bridge from the recursive scheme pseudocode
 */
class NeuralSymbolicBridge {
public:
    NeuralSymbolicBridge(AtomSpace* atomspace = nullptr);
    
    /**
     * Convert LLM response to AtomSpace representation
     */
    Handle llm_to_atomspace(const std::string& response, const std::string& context = "");
    
    /**
     * Generate LLM query from AtomSpace patterns
     */
    std::string atomspace_to_llm_query(Handle pattern_atom);
    
    /**
     * Bridge neural (LLM) and symbolic (AtomSpace) reasoning
     * Implements: (neural-symbolic-bridge input) from pseudocode
     */
    std::string neural_symbolic_bridge(const std::string& input);
    
    /**
     * Extract semantic concepts from text
     */
    HandleSeq extract_concepts(const std::string& text);
    
    /**
     * Create relationship links between concepts
     */
    Handle create_concept_relationships(const HandleSeq& concepts, const std::string& relation_type = "related_to");

private:
    AtomSpace* atomspace_;
    
    /**
     * Parse and extract entities from text
     */
    std::vector<std::string> extract_entities(const std::string& text);
    
    /**
     * Determine relationship types between entities
     */
    std::string infer_relationship(const std::string& entity1, const std::string& entity2, const std::string& context);
};

} // namespace caichat
} // namespace opencog

#endif // _OPENCOG_CAICHAT_SESSION_MANAGER_H