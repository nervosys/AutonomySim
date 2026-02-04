// Copyright (c) nervosys. All rights reserved.
// Licensed under the MIT License.

#include "modules/communication/A2AProtocol.hpp"
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace nervosys {
namespace autonomylib {
namespace modules {

A2AProtocol::A2AProtocol() : running_(false), initialized_(false) {}

A2AProtocol::A2AProtocol(const ProtocolConfig &config) : running_(false), initialized_(false) { initialize(config); }

A2AProtocol::~A2AProtocol() { stop(); }

void A2AProtocol::initialize(const ProtocolConfig &config) {
    std::lock_guard<std::mutex> lock_msg(message_mutex_);
    std::lock_guard<std::mutex> lock_prop(proposal_mutex_);
    std::lock_guard<std::mutex> lock_cons(consensus_mutex_);
    std::lock_guard<std::mutex> lock_peer(peer_mutex_);

    config_ = config;
    message_queue_.clear();
    proposals_.clear();
    consensus_map_.clear();
    peers_.clear();
    callbacks_.clear();

    initialized_ = true;
}

void A2AProtocol::start() {
    if (!initialized_) {
        throw std::runtime_error("A2AProtocol not initialized");
    }
    running_ = true;
}

void A2AProtocol::stop() { running_ = false; }

void A2AProtocol::reset() {
    stop();
    std::lock_guard<std::mutex> lock_msg(message_mutex_);
    std::lock_guard<std::mutex> lock_prop(proposal_mutex_);
    std::lock_guard<std::mutex> lock_cons(consensus_mutex_);
    std::lock_guard<std::mutex> lock_peer(peer_mutex_);

    message_queue_.clear();
    proposals_.clear();
    consensus_map_.clear();
    peers_.clear();
}

// Message passing
bool A2AProtocol::sendMessage(const Message &message) {
    if (!running_ || !validateMessage(message)) {
        return false;
    }

    std::lock_guard<std::mutex> lock(message_mutex_);

    // In a real implementation, this would send over network
    // For now, we simulate by adding to local queue
    message_queue_.push_back(message);

    if (message_queue_.size() > config_.message_buffer_size) {
        message_queue_.erase(message_queue_.begin());
    }

    // Trigger callback if registered
    std::lock_guard<std::mutex> cb_lock(callback_mutex_);
    auto it = callbacks_.find(message.type);
    if (it != callbacks_.end() && it->second) {
        it->second(message);
    }

    return true;
}

bool A2AProtocol::sendBroadcast(const Message &message) {
    if (!running_) {
        return false;
    }

    Message broadcast_msg = message;
    broadcast_msg.receiver_id = ""; // Empty means broadcast
    broadcast_msg.type = MessageType::Broadcast;

    return sendMessage(broadcast_msg);
}

std::vector<A2AProtocol::Message> A2AProtocol::receiveMessages() {
    std::lock_guard<std::mutex> lock(message_mutex_);

    std::vector<Message> messages = message_queue_;
    message_queue_.clear();

    return messages;
}

A2AProtocol::Message A2AProtocol::receiveMessage(const std::string &sender_id) {
    std::lock_guard<std::mutex> lock(message_mutex_);

    if (message_queue_.empty()) {
        return Message();
    }

    if (sender_id.empty()) {
        Message msg = message_queue_.front();
        message_queue_.erase(message_queue_.begin());
        return msg;
    }

    // Find message from specific sender
    for (auto it = message_queue_.begin(); it != message_queue_.end(); ++it) {
        if (it->sender_id == sender_id) {
            Message msg = *it;
            message_queue_.erase(it);
            return msg;
        }
    }

    return Message();
}

bool A2AProtocol::hasMessages() const {
    std::lock_guard<std::mutex> lock(message_mutex_);
    return !message_queue_.empty();
}

// Callbacks
void A2AProtocol::registerMessageCallback(MessageType type, MessageCallback callback) {
    std::lock_guard<std::mutex> lock(callback_mutex_);
    callbacks_[type] = callback;
}

void A2AProtocol::unregisterMessageCallback(MessageType type) {
    std::lock_guard<std::mutex> lock(callback_mutex_);
    callbacks_.erase(type);
}

// Task negotiation
bool A2AProtocol::proposeTask(const Proposal &proposal) {
    if (!running_ || proposal.proposal_id.empty()) {
        return false;
    }

    std::lock_guard<std::mutex> lock(proposal_mutex_);
    proposals_[proposal.proposal_id] = proposal;

    // Send proposal message to all required agents
    Message msg;
    msg.message_id = generateMessageId();
    msg.sender_id = config_.agent_id;
    msg.type = MessageType::Proposal;
    msg.content = proposal.task_description;
    msg.data["proposal_id"] = proposal.proposal_id;
    msg.priority = proposal.priority;
    msg.timestamp = getCurrentTimestamp();

    sendBroadcast(msg);

    return true;
}

bool A2AProtocol::acceptProposal(const std::string &proposal_id, const std::string &agent_id) {
    std::lock_guard<std::mutex> lock(proposal_mutex_);

    auto it = proposals_.find(proposal_id);
    if (it == proposals_.end()) {
        return false;
    }

    Proposal &proposal = it->second;

    // Add vote if not already present
    if (std::find(proposal.votes.begin(), proposal.votes.end(), agent_id) == proposal.votes.end()) {
        proposal.votes.push_back(agent_id);
    }

    // Send accept message
    Message msg;
    msg.message_id = generateMessageId();
    msg.sender_id = agent_id;
    msg.receiver_id = proposal.proposer_id;
    msg.type = MessageType::Accept;
    msg.data["proposal_id"] = proposal_id;
    msg.timestamp = getCurrentTimestamp();

    sendMessage(msg);

    return true;
}

bool A2AProtocol::rejectProposal(const std::string &proposal_id, const std::string &agent_id,
                                 const std::string &reason) {
    std::lock_guard<std::mutex> lock(proposal_mutex_);

    auto it = proposals_.find(proposal_id);
    if (it == proposals_.end()) {
        return false;
    }

    // Send reject message
    Message msg;
    msg.message_id = generateMessageId();
    msg.sender_id = agent_id;
    msg.receiver_id = it->second.proposer_id;
    msg.type = MessageType::Reject;
    msg.content = reason;
    msg.data["proposal_id"] = proposal_id;
    msg.timestamp = getCurrentTimestamp();

    sendMessage(msg);

    return true;
}

A2AProtocol::Proposal A2AProtocol::getProposal(const std::string &proposal_id) const {
    std::lock_guard<std::mutex> lock(proposal_mutex_);

    auto it = proposals_.find(proposal_id);
    if (it != proposals_.end()) {
        return it->second;
    }

    return Proposal();
}

std::vector<A2AProtocol::Proposal> A2AProtocol::getActiveProposals() const {
    std::lock_guard<std::mutex> lock(proposal_mutex_);

    std::vector<Proposal> active;
    uint64_t current_time = getCurrentTimestamp();

    for (const auto &pair : proposals_) {
        if (pair.second.expiry_timestamp > current_time) {
            active.push_back(pair.second);
        }
    }

    return active;
}

bool A2AProtocol::cancelProposal(const std::string &proposal_id) {
    std::lock_guard<std::mutex> lock(proposal_mutex_);
    return proposals_.erase(proposal_id) > 0;
}

// Consensus building
bool A2AProtocol::initiateConsensus(const std::string &topic, uint32_t required_votes) {
    if (!running_ || topic.empty()) {
        return false;
    }

    std::lock_guard<std::mutex> lock(consensus_mutex_);

    Consensus consensus;
    consensus.consensus_id = generateMessageId();
    consensus.topic = topic;
    consensus.required_votes = required_votes;
    consensus.achieved = false;
    consensus.timestamp = getCurrentTimestamp();

    consensus_map_[consensus.consensus_id] = consensus;

    // Broadcast consensus request
    Message msg;
    msg.message_id = generateMessageId();
    msg.sender_id = config_.agent_id;
    msg.type = MessageType::Request;
    msg.content = "Consensus: " + topic;
    msg.data["consensus_id"] = consensus.consensus_id;
    msg.timestamp = getCurrentTimestamp();

    sendBroadcast(msg);

    return true;
}

bool A2AProtocol::vote(const std::string &consensus_id, const std::string &agent_id, const std::string &vote) {
    std::lock_guard<std::mutex> lock(consensus_mutex_);

    auto it = consensus_map_.find(consensus_id);
    if (it == consensus_map_.end()) {
        return false;
    }

    Consensus &consensus = it->second;
    consensus.votes[agent_id] = vote;

    // Check if consensus achieved
    if (consensus.votes.size() >= consensus.required_votes) {
        consensus.achieved = true;
    }

    return true;
}

A2AProtocol::Consensus A2AProtocol::getConsensusStatus(const std::string &consensus_id) const {
    std::lock_guard<std::mutex> lock(consensus_mutex_);

    auto it = consensus_map_.find(consensus_id);
    if (it != consensus_map_.end()) {
        return it->second;
    }

    return Consensus();
}

std::vector<A2AProtocol::Consensus> A2AProtocol::getActiveConsensus() const {
    std::lock_guard<std::mutex> lock(consensus_mutex_);

    std::vector<Consensus> active;
    for (const auto &pair : consensus_map_) {
        if (!pair.second.achieved) {
            active.push_back(pair.second);
        }
    }

    return active;
}

// Peer management
bool A2AProtocol::connectToPeer(const std::string &peer_id, const std::string &address, uint16_t port) {
    if (!running_ || peer_id.empty()) {
        return false;
    }

    std::lock_guard<std::mutex> lock(peer_mutex_);

    if (peers_.size() >= config_.max_connections) {
        return false;
    }

    peers_[peer_id] = getCurrentTimestamp();
    return true;
}

bool A2AProtocol::disconnectFromPeer(const std::string &peer_id) {
    std::lock_guard<std::mutex> lock(peer_mutex_);
    return peers_.erase(peer_id) > 0;
}

std::vector<std::string> A2AProtocol::getConnectedPeers() const {
    std::lock_guard<std::mutex> lock(peer_mutex_);

    std::vector<std::string> peer_ids;
    peer_ids.reserve(peers_.size());

    for (const auto &pair : peers_) {
        peer_ids.push_back(pair.first);
    }

    return peer_ids;
}

bool A2AProtocol::isPeerConnected(const std::string &peer_id) const {
    std::lock_guard<std::mutex> lock(peer_mutex_);
    return peers_.find(peer_id) != peers_.end();
}

// Discovery
std::vector<std::string> A2AProtocol::discoverPeers(real_T radius_meters) {
    // In a real implementation, this would use network discovery
    // For now, return connected peers
    return getConnectedPeers();
}

bool A2AProtocol::announceSelf(const std::map<std::string, std::string> &capabilities) {
    Message msg;
    msg.message_id = generateMessageId();
    msg.sender_id = config_.agent_id;
    msg.type = MessageType::Broadcast;
    msg.content = "Agent announcement";
    msg.data = capabilities;
    msg.timestamp = getCurrentTimestamp();

    return sendBroadcast(msg);
}

uint32_t A2AProtocol::getMessageCount() const {
    std::lock_guard<std::mutex> lock(message_mutex_);
    return static_cast<uint32_t>(message_queue_.size());
}

// Internal methods
void A2AProtocol::cleanupExpiredProposals() {
    std::lock_guard<std::mutex> lock(proposal_mutex_);

    uint64_t current_time = getCurrentTimestamp();

    for (auto it = proposals_.begin(); it != proposals_.end();) {
        if (it->second.expiry_timestamp < current_time) {
            it = proposals_.erase(it);
        } else {
            ++it;
        }
    }
}

std::string A2AProtocol::generateMessageId() const {
    std::stringstream ss;
    ss << config_.agent_id << "_" << getCurrentTimestamp();
    return ss.str();
}

uint64_t A2AProtocol::getCurrentTimestamp() const {
    return static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
            .count());
}

bool A2AProtocol::validateMessage(const Message &message) const {
    return !message.sender_id.empty() && message.timestamp > 0;
}

} // namespace modules
} // namespace autonomylib
} // namespace nervosys
