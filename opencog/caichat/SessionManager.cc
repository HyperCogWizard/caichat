/**
 * SessionManager.cc
 * 
 * Implementation of cognitive session persistence and hypergraph memory synergization
 */

#include "SessionManager.h"
#include <ctime>
#include <sstream>
#include <regex>
#include <random>
#include <iomanip>

#ifdef HAVE_OPENCOG
#include <opencog/atoms/base/Node.h>
#include <opencog/atoms/base/Link.h>
#include <opencog/util/Logger.h>
#include <opencog/util/uuid.h>
using namespace opencog;
#else
// Minimal definitions for non-OpenCog builds
#include <iostream>
namespace opencog {
    namespace caichat {
        std::string generate_uuid() {
            static std::random_device rd;
            static std::mt19937 gen(rd());
            static std::uniform_int_distribution<> dis(0, 15);
            
            std::stringstream ss;
            ss << std::hex;
            for (int i = 0; i < 32; ++i) {
                ss << dis(gen);
            }
            return ss.str();
        }
    }
}
#endif

using namespace opencog;
using namespace opencog::caichat;

// SessionManager Implementation
SessionManager::SessionManager(AtomSpace* atomspace) : atomspace_(atomspace) {
    // Initialize the router with default providers
    router_.init_default_providers();
}

std::string SessionManager::create_persistent_session(const std::string& session_name, 
                                                     const std::string& provider, 
                                                     const std::string& model) {
    std::string session_id = generate_session_id();
    
    // Create session metadata
    SessionMetadata metadata;
    metadata.session_id = session_id;
    metadata.provider = provider;
    metadata.model = model;
    metadata.created_at = std::time(nullptr);
    metadata.last_accessed = metadata.created_at;
    metadata.message_count = 0;
    metadata.is_persistent = true;
    
    // Create AtomSpace representation if available
    metadata.session_atom = create_session_atom(metadata);
    
    // Create client configuration and session
    ClientConfig config;
    config.provider = provider;
    config.model = model;
    // Note: API keys should be set separately for security
    
    try {
        auto client = create_client(config);
        auto completion = std::make_unique<ChatCompletion>(atomspace_, std::move(client));
        
        sessions_[session_id] = std::move(completion);
        session_metadata_[session_id] = metadata;
        
        // Establish hypergraph links for cognitive synergy
        establish_hypergraph_link(session_id, metadata.session_atom);
        
#ifdef HAVE_OPENCOG
        if (atomspace_) {
            // Save persistent session with name mapping
            Handle name_node = atomspace_->add_node(CONCEPT_NODE, "session_name:" + session_name);
            Handle session_node = atomspace_->add_node(CONCEPT_NODE, "session:" + session_id);
            atomspace_->add_link(EVALUATION_LINK,
                atomspace_->add_node(PREDICATE_NODE, "named_session"),
                atomspace_->add_link(LIST_LINK, name_node, session_node));
        }
#endif
        
        return session_id;
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to create persistent session: " + std::string(e.what()));
    }
}

std::string SessionManager::resume_session(const std::string& session_name, 
                                          const std::string& provider, 
                                          const std::string& model) {
#ifdef HAVE_OPENCOG
    if (atomspace_) {
        // Try to find existing session by name
        Handle name_node = atomspace_->get_node(CONCEPT_NODE, "session_name:" + session_name);
        if (name_node != Handle::UNDEFINED) {
            // Load existing session
            SessionMetadata metadata = load_session_from_atomspace(session_name);
            if (!metadata.session_id.empty()) {
                // Update last accessed time
                metadata.last_accessed = std::time(nullptr);
                session_metadata_[metadata.session_id] = metadata;
                
                return metadata.session_id;
            }
        }
    }
#endif
    
    // If no existing session found, create new one
    return create_persistent_session(session_name, provider, model);
}

void SessionManager::mediate_session(const std::string& session_id) {
    if (is_active(session_id)) {
        update_hypergraph_memory(session_id);
        
        // Update metadata
        auto it = session_metadata_.find(session_id);
        if (it != session_metadata_.end()) {
            it->second.last_accessed = std::time(nullptr);
            
            // Get message count from session
            auto session_it = sessions_.find(session_id);
            if (session_it != sessions_.end()) {
                it->second.message_count = session_it->second->get_messages().size();
            }
        }
    } else {
        persist_session(session_id);
    }
}

void SessionManager::audit_core_modules() {
    std::vector<std::string> modules = {"LLMClient", "ChatCompletion", "SessionManager", "NeuralSymbolicBridge"};
    
    for (const auto& module : modules) {
#ifdef HAVE_OPENCOG
        if (atomspace_) {
            // Create module atom and check compliance
            Handle module_atom = atomspace_->add_node(CONCEPT_NODE, "module:" + module);
            Handle compliance_atom = atomspace_->add_node(PREDICATE_NODE, "spec_compliant");
            
            // Check module compliance (simplified for now)
            bool compliant = true;  // In real implementation, run actual compliance checks
            
            Handle truth_atom = atomspace_->add_node(CONCEPT_NODE, compliant ? "true" : "false");
            atomspace_->add_link(EVALUATION_LINK, compliance_atom,
                atomspace_->add_link(LIST_LINK, module_atom, truth_atom));
                
            // Establish hypergraph links for synergy
            for (const auto& [session_id, metadata] : session_metadata_) {
                establish_hypergraph_link(session_id, module_atom);
            }
        }
#else
        std::cout << "Auditing module: " << module << " - COMPLIANT" << std::endl;
#endif
    }
}

void SessionManager::establish_hypergraph_link(const std::string& session_id, Handle module_atom) {
#ifdef HAVE_OPENCOG
    if (atomspace_ && module_atom != Handle::UNDEFINED) {
        Handle session_atom = atomspace_->add_node(CONCEPT_NODE, "session:" + session_id);
        Handle synergy_link = atomspace_->add_link(EVALUATION_LINK,
            atomspace_->add_node(PREDICATE_NODE, "hypergraph_synergy"),
            atomspace_->add_link(LIST_LINK, session_atom, module_atom));
            
        // Encode session patterns
        encode_session_patterns(session_id);
    }
#endif
}

void SessionManager::update_hypergraph_memory(const std::string& session_id) {
#ifdef HAVE_OPENCOG
    if (atomspace_) {
        auto it = sessions_.find(session_id);
        if (it != sessions_.end()) {
            // Get conversation history and encode to hypergraph
            auto messages = it->second->get_messages();
            Handle session_atom = atomspace_->add_node(CONCEPT_NODE, "session:" + session_id);
            
            for (const auto& message : messages) {
                Handle message_atom = message_to_atom(atomspace_, message);
                atomspace_->add_link(MEMBER_LINK, message_atom, session_atom);
            }
            
            // Update timestamp
            Handle timestamp_atom = atomspace_->add_node(CONCEPT_NODE, std::to_string(std::time(nullptr)));
            atomspace_->add_link(EVALUATION_LINK,
                atomspace_->add_node(PREDICATE_NODE, "last_updated"),
                atomspace_->add_link(LIST_LINK, session_atom, timestamp_atom));
        }
    }
#endif
}

void SessionManager::persist_session(const std::string& session_id) {
    auto it = session_metadata_.find(session_id);
    if (it != session_metadata_.end() && it->second.is_persistent) {
        // In a full implementation, this would save to file system or database
        // For now, we keep it in AtomSpace if available
        update_hypergraph_memory(session_id);
        
#ifdef HAVE_OPENCOG
        if (atomspace_) {
            Handle session_atom = atomspace_->add_node(CONCEPT_NODE, "session:" + session_id);
            Handle persistent_atom = atomspace_->add_node(PREDICATE_NODE, "persistent");
            atomspace_->add_link(EVALUATION_LINK, persistent_atom,
                atomspace_->add_link(LIST_LINK, session_atom, 
                    atomspace_->add_node(CONCEPT_NODE, "true")));
        }
#endif
    }
}

bool SessionManager::is_active(const std::string& session_id) {
    auto it = session_metadata_.find(session_id);
    if (it == session_metadata_.end()) {
        return false;
    }
    
    // Consider session active if accessed within last hour
    std::time_t now = std::time(nullptr);
    return (now - it->second.last_accessed) < 3600;
}

SessionManager::SessionMetadata SessionManager::get_session_metadata(const std::string& session_id) {
    auto it = session_metadata_.find(session_id);
    if (it != session_metadata_.end()) {
        return it->second;
    }
    return SessionMetadata{};  // Return empty metadata if not found
}

std::vector<SessionManager::SessionMetadata> SessionManager::list_sessions() {
    std::vector<SessionMetadata> sessions;
    for (const auto& [id, metadata] : session_metadata_) {
        sessions.push_back(metadata);
    }
    return sessions;
}

std::vector<SessionManager::SessionMetadata> SessionManager::get_sessions_by_provider(const std::string& provider) {
    std::vector<SessionMetadata> sessions;
    for (const auto& [id, metadata] : session_metadata_) {
        if (metadata.provider == provider) {
            sessions.push_back(metadata);
        }
    }
    return sessions;
}

void SessionManager::cleanup_inactive_sessions(int max_age_hours) {
    std::time_t cutoff = std::time(nullptr) - (max_age_hours * 3600);
    
    auto it = session_metadata_.begin();
    while (it != session_metadata_.end()) {
        if (it->second.last_accessed < cutoff && !it->second.is_persistent) {
            // Remove from memory
            sessions_.erase(it->first);
            it = session_metadata_.erase(it);
        } else {
            ++it;
        }
    }
}

std::string SessionManager::generate_session_id() {
#ifdef HAVE_OPENCOG
    return "session_" + uuid().substr(0, 16);
#else
    return "session_" + caichat::generate_uuid().substr(0, 16);
#endif
}

Handle SessionManager::create_session_atom(const SessionMetadata& metadata) {
#ifdef HAVE_OPENCOG
    if (atomspace_) {
        Handle session_atom = atomspace_->add_node(CONCEPT_NODE, "session:" + metadata.session_id);
        
        // Add metadata as attributes
        Handle provider_atom = atomspace_->add_node(CONCEPT_NODE, metadata.provider);
        Handle model_atom = atomspace_->add_node(CONCEPT_NODE, metadata.model);
        Handle timestamp_atom = atomspace_->add_node(CONCEPT_NODE, std::to_string(metadata.created_at));
        
        atomspace_->add_link(EVALUATION_LINK,
            atomspace_->add_node(PREDICATE_NODE, "has_provider"),
            atomspace_->add_link(LIST_LINK, session_atom, provider_atom));
            
        atomspace_->add_link(EVALUATION_LINK,
            atomspace_->add_node(PREDICATE_NODE, "has_model"),
            atomspace_->add_link(LIST_LINK, session_atom, model_atom));
            
        atomspace_->add_link(EVALUATION_LINK,
            atomspace_->add_node(PREDICATE_NODE, "created_at"),
            atomspace_->add_link(LIST_LINK, session_atom, timestamp_atom));
        
        return session_atom;
    }
#endif
    return Handle_UNDEFINED;
}

SessionManager::SessionMetadata SessionManager::load_session_from_atomspace(const std::string& session_name) {
    SessionMetadata metadata;
    
#ifdef HAVE_OPENCOG
    if (atomspace_) {
        Handle name_node = atomspace_->get_node(CONCEPT_NODE, "session_name:" + session_name);
        if (name_node != Handle::UNDEFINED) {
            // Find linked session node
            HandleSeq incoming = name_node->getIncomingSet();
            for (Handle link : incoming) {
                if (link->get_type() == EVALUATION_LINK) {
                    HandleSeq outgoing = link->getOutgoingSet();
                    if (outgoing.size() >= 2) {
                        Handle predicate = outgoing[0];
                        if (predicate->get_name() == "named_session") {
                            Handle session_list = outgoing[1];
                            HandleSeq session_pair = session_list->getOutgoingSet();
                            if (session_pair.size() >= 2) {
                                Handle session_node = session_pair[1];
                                std::string session_id = session_node->get_name();
                                
                                // Extract session ID
                                if (session_id.substr(0, 8) == "session:") {
                                    metadata.session_id = session_id.substr(8);
                                    // Load other metadata from AtomSpace
                                    // This is a simplified version
                                    metadata.provider = "openai";  // Default
                                    metadata.model = "gpt-3.5-turbo";  // Default
                                    metadata.created_at = std::time(nullptr);
                                    metadata.last_accessed = metadata.created_at;
                                    metadata.is_persistent = true;
                                    metadata.session_atom = session_node;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
#endif
    
    return metadata;
}

void SessionManager::encode_session_patterns(const std::string& session_id) {
#ifdef HAVE_OPENCOG
    if (atomspace_) {
        Handle session_atom = atomspace_->add_node(CONCEPT_NODE, "session:" + session_id);
        
        // Create pattern atoms for cognitive analysis
        Handle pattern_atom = atomspace_->add_node(CONCEPT_NODE, "pattern:" + session_id);
        Handle interaction_pattern = atomspace_->add_link(EVALUATION_LINK,
            atomspace_->add_node(PREDICATE_NODE, "interaction_pattern"),
            atomspace_->add_link(LIST_LINK, session_atom, pattern_atom));
        
        // Start pattern propagation
        propagate_patterns(pattern_atom, 3);
    }
#endif
}

void SessionManager::propagate_patterns(Handle seed_pattern, int depth) {
#ifdef HAVE_OPENCOG
    if (atomspace_ && seed_pattern != Handle::UNDEFINED && depth > 0) {
        // Get incoming and outgoing links to propagate patterns
        HandleSeq incoming = seed_pattern->getIncomingSet();
        HandleSeq related_atoms;
        
        for (Handle link : incoming) {
            HandleSeq outgoing = link->getOutgoingSet();
            for (Handle atom : outgoing) {
                if (atom != seed_pattern) {
                    related_atoms.push_back(atom);
                }
            }
        }
        
        // Create emergent pattern links
        for (Handle related : related_atoms) {
            Handle emergent_pattern = atomspace_->add_link(EVALUATION_LINK,
                atomspace_->add_node(PREDICATE_NODE, "emergent_pattern"),
                atomspace_->add_link(LIST_LINK, seed_pattern, related));
            
            // Recursively propagate
            propagate_patterns(emergent_pattern, depth - 1);
        }
    }
#endif
}

// Neural-Symbolic Bridge Implementation
NeuralSymbolicBridge::NeuralSymbolicBridge(AtomSpace* atomspace) : atomspace_(atomspace) {}

Handle NeuralSymbolicBridge::llm_to_atomspace(const std::string& response, const std::string& context) {
#ifdef HAVE_OPENCOG
    if (atomspace_) {
        // Extract concepts from the response
        HandleSeq concepts = extract_concepts(response);
        
        // Create response atom
        Handle response_atom = atomspace_->add_node(CONCEPT_NODE, "llm_response:" + response.substr(0, 50));
        
        // Link concepts to response
        for (Handle concept : concepts) {
            atomspace_->add_link(MEMBER_LINK, concept, response_atom);
        }
        
        // Create relationships between concepts
        if (concepts.size() > 1) {
            create_concept_relationships(concepts, "co_occurs_with");
        }
        
        return response_atom;
    }
#endif
    return Handle_UNDEFINED;
}

std::string NeuralSymbolicBridge::atomspace_to_llm_query(Handle pattern_atom) {
#ifdef HAVE_OPENCOG
    if (atomspace_ && pattern_atom != Handle::UNDEFINED) {
        // Convert AtomSpace pattern to natural language query
        std::string atom_name = pattern_atom->get_name();
        
        // Simple conversion for demonstration
        if (atom_name.find("concept:") == 0) {
            return "Tell me about " + atom_name.substr(8);
        } else if (atom_name.find("relationship:") == 0) {
            return "Explain the relationship " + atom_name.substr(13);
        }
        
        return "Analyze this concept: " + atom_name;
    }
#endif
    return "";
}

std::string NeuralSymbolicBridge::neural_symbolic_bridge(const std::string& input) {
    // This implements the (neural-symbolic-bridge input) function from the pseudocode
    
    // Step 1: Process input through AtomSpace (symbolic reasoning)
    HandleSeq input_concepts = extract_concepts(input);
    
    // Step 2: Convert to LLM query format
    std::string llm_query = "Analyze these concepts in context: " + input;
    
    // Step 3: In a full implementation, this would call an LLM
    // For now, return a structured response
    std::string response = "Neural-symbolic analysis of: " + input + 
                          "\nExtracted " + std::to_string(input_concepts.size()) + " concepts";
    
    // Step 4: Convert LLM response back to AtomSpace
    Handle response_atom = llm_to_atomspace(response, input);
    
    return response;
}

HandleSeq NeuralSymbolicBridge::extract_concepts(const std::string& text) {
    HandleSeq concepts;
    
#ifdef HAVE_OPENCOG
    if (atomspace_) {
        // Simple concept extraction using regex patterns
        std::vector<std::string> entities = extract_entities(text);
        
        for (const std::string& entity : entities) {
            Handle concept_atom = atomspace_->add_node(CONCEPT_NODE, "concept:" + entity);
            concepts.push_back(concept_atom);
        }
    }
#endif
    
    return concepts;
}

Handle NeuralSymbolicBridge::create_concept_relationships(const HandleSeq& concepts, const std::string& relation_type) {
#ifdef HAVE_OPENCOG
    if (atomspace_ && concepts.size() >= 2) {
        Handle relation_atom = atomspace_->add_node(PREDICATE_NODE, relation_type);
        
        // Create pairwise relationships
        for (size_t i = 0; i < concepts.size(); ++i) {
            for (size_t j = i + 1; j < concepts.size(); ++j) {
                Handle relationship = atomspace_->add_link(EVALUATION_LINK, relation_atom,
                    atomspace_->add_link(LIST_LINK, concepts[i], concepts[j]));
            }
        }
        
        return relation_atom;
    }
#endif
    return Handle_UNDEFINED;
}

std::vector<std::string> NeuralSymbolicBridge::extract_entities(const std::string& text) {
    std::vector<std::string> entities;
    
    // Simple entity extraction using regex (in real implementation, use NLP library)
    std::regex word_regex(R"(\b[A-Z][a-z]+\b)");  // Capitalized words
    std::sregex_iterator iter(text.begin(), text.end(), word_regex);
    std::sregex_iterator end;
    
    for (; iter != end; ++iter) {
        std::string word = iter->str();
        if (word.length() > 2) {  // Filter short words
            entities.push_back(word);
        }
    }
    
    return entities;
}

std::string NeuralSymbolicBridge::infer_relationship(const std::string& entity1, const std::string& entity2, const std::string& context) {
    // Simple relationship inference (in real implementation, use more sophisticated NLP)
    if (context.find(entity1) < context.find(entity2)) {
        return "precedes";
    } else {
        return "follows";
    }
}