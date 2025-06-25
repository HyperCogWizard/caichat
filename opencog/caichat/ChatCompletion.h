/**
 * ChatCompletion.h
 * 
 * OpenCog chat completion and conversation management
 */

#ifndef _OPENCOG_CAICHAT_CHAT_COMPLETION_H
#define _OPENCOG_CAICHAT_CHAT_COMPLETION_H

#include "LLMClient.h"

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
 * Manages chat conversations and completion sessions
 */
class ChatCompletion {
public:
    ChatCompletion(AtomSpace* atomspace, std::unique_ptr<LLMClient> client);
    ~ChatCompletion() = default;
    
    /**
     * Add a message to the conversation
     */
    void add_message(const std::string& role, const std::string& content);
    
    /**
     * Get completion for the current conversation
     */
    std::string complete();
    
    /**
     * Stream completion for the current conversation
     */
    void complete_stream(std::function<void(const std::string&)> callback);
    
    /**
     * Clear conversation history
     */
    void clear_history();
    
    /**
     * Get conversation history as atoms
     */
    HandleSeq get_conversation_atoms();
    
    /**
     * Load conversation from atoms
     */
    void load_conversation(const HandleSeq& conversation_atoms);
    
    /**
     * Save current conversation to AtomSpace
     */
    Handle save_conversation(const std::string& conversation_id);
    
    /**
     * Load conversation from AtomSpace by ID
     */
    void load_conversation_by_id(const std::string& conversation_id);
    
    /**
     * Convert conversation to vector of messages
     */
    std::vector<Message> get_messages();

private:
    AtomSpace* atomspace_;
    std::unique_ptr<LLMClient> client_;
    std::vector<Message> conversation_;
    std::string conversation_id_;
};

} // namespace caichat
} // namespace opencog

#endif // _OPENCOG_CAICHAT_CHAT_COMPLETION_H