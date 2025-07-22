/**
 * ChatCompletion.cc
 * 
 * Implementation of chat completion and conversation management
 */

#include "ChatCompletion.h"

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
    std::string uuid() {
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
#endif

using namespace opencog;
using namespace opencog::caichat;

ChatCompletion::ChatCompletion(AtomSpace* atomspace, std::unique_ptr<LLMClient> client)
    : atomspace_(atomspace), client_(std::move(client)) {
#ifdef HAVE_OPENCOG
    conversation_id_ = uuid();
#else
    conversation_id_ = opencog::uuid();
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
    }
#endif
}

std::string ChatCompletion::complete() {
    if (conversation_.empty()) {
        throw std::runtime_error("No messages in conversation");
    }
    
    try {
        std::string response = client_->chat_completion(conversation_);
        
        // Add assistant response to conversation
        add_message("assistant", response);
        
        return response;
    } catch (const std::exception& e) {
#ifdef HAVE_OPENCOG
        logger().error("Chat completion failed: %s", e.what());
#else
        std::cerr << "Chat completion failed: " << e.what() << std::endl;
#endif
        throw;
    }
}

void ChatCompletion::complete_stream(std::function<void(const std::string&)> callback) {
    if (conversation_.empty()) {
        throw std::runtime_error("No messages in conversation");
    }
    
    try {
        // Create a wrapper callback that also stores the response
        std::string full_response;
        auto wrapper_callback = [&](const std::string& chunk) {
            full_response += chunk;
            callback(chunk);
        };
        
        client_->chat_completion_stream(conversation_, wrapper_callback);
        
        // Add complete assistant response to conversation
        if (!full_response.empty()) {
            add_message("assistant", full_response);
        }
    } catch (const std::exception& e) {
#ifdef HAVE_OPENCOG
        logger().error("Streaming completion failed: %s", e.what());
#else
        std::cerr << "Streaming completion failed: " << e.what() << std::endl;
#endif
        throw;
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
    }
#endif
}

HandleSeq ChatCompletion::get_conversation_atoms() {
#ifdef HAVE_OPENCOG
    HandleSeq atoms;
    
    if (atomspace_) {
        for (const auto& message : conversation_) {
            atoms.push_back(message_to_atom(atomspace_, message));
        }
    }
    
    return atoms;
#else
    return HandleSeq(); // Empty in minimal mode
#endif
}

void ChatCompletion::load_conversation(const HandleSeq& conversation_atoms) {
    conversation_.clear();
    
#ifdef HAVE_OPENCOG
    if (atomspace_) {
        for (Handle atom : conversation_atoms) {
            try {
                Message msg = atom_to_message(atom);
                conversation_.push_back(msg);
            } catch (const std::exception& e) {
                logger().warn("Failed to convert atom to message: %s", e.what());
            }
        }
    }
#endif
}

Handle ChatCompletion::save_conversation(const std::string& conversation_id) {
    conversation_id_ = conversation_id;
    
#ifdef HAVE_OPENCOG
    if (atomspace_) {
        Handle conversation_node = atomspace_->add_node(CONCEPT_NODE, "conversation:" + conversation_id);
        
        // Add metadata
        Handle timestamp_node = atomspace_->add_node(CONCEPT_NODE, std::to_string(time(nullptr)));
        atomspace_->add_link(EVALUATION_LINK,
            atomspace_->add_node(PREDICATE_NODE, "timestamp"),
            atomspace_->add_link(LIST_LINK, conversation_node, timestamp_node));
        
        // Link all messages to this conversation
        for (const auto& message : conversation_) {
            Handle message_atom = message_to_atom(atomspace_, message);
            atomspace_->add_link(MEMBER_LINK, message_atom, conversation_node);
        }
        
        return conversation_node;
    }
#endif
    return Handle_UNDEFINED;
}

void ChatCompletion::load_conversation_by_id(const std::string& conversation_id) {
    conversation_id_ = conversation_id;
    conversation_.clear();
    
#ifdef HAVE_OPENCOG
    if (atomspace_) {
        Handle conversation_node = atomspace_->get_node(CONCEPT_NODE, "conversation:" + conversation_id);
        if (conversation_node == Handle::UNDEFINED) {
            logger().warn("Conversation not found: %s", conversation_id.c_str());
            return;
        }
        
        // Find all messages that are members of this conversation
        HandleSeq incoming = conversation_node->getIncomingSet();
        
        for (Handle link : incoming) {
            if (link->get_type() == MEMBER_LINK) {
                Handle message_atom = link->getOutgoingAtom(0);
                try {
                    Message msg = atom_to_message(message_atom);
                    conversation_.push_back(msg);
                } catch (const std::exception& e) {
                    logger().warn("Failed to load message from atom: %s", e.what());
                }
            }
        }
        
        logger().info("Loaded conversation with %zu messages", conversation_.size());
    }
#endif
}

std::vector<Message> ChatCompletion::get_messages() {
    return conversation_;
}

LLMClient* ChatCompletion::get_client() const {
    return client_.get();
}