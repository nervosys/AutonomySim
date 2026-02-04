// Copyright (c) nervosys. All rights reserved.
// Licensed under the MIT License.

#ifndef autonomylib_modules_communication_A2AProtocol_hpp
#define autonomylib_modules_communication_A2AProtocol_hpp

#include "common/Common.hpp"
#include "common/CommonStructs.hpp"
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace nervosys {
namespace autonomylib {
namespace modules {

/**
 * @brief Agent-to-Agent (A2A) Communication Protocol
 *
 * Enables direct vehicle-to-vehicle communication for:
 * - Task negotiation and allocation
 * - Consensus building
 * - Coordinated action execution
 * - Conflict resolution
 * - Peer discovery
 */
class A2AProtocol {
  public:
    enum class MessageType {
        Proposal,  // Propose a task or action
        Accept,    // Accept a proposal
        Reject,    // Reject a proposal
        Counter,   // Counter-proposal
        Request,   // Request information or action
        Response,  // Response to request
        Broadcast, // Broadcast message to all
        Heartbeat, // Keep-alive message
        Emergency  // Emergency notification
    };

    enum class TaskPriority { Low = 0, Medium = 1, High = 2, Critical = 3 };

    struct Message {
        std::string message_id;
        std::string sender_id;
        std::string receiver_id; // Empty for broadcast
        MessageType type;
        std::string content;
        std::map<std::string, std::string> data;
        TaskPriority priority;
        uint64_t timestamp;
        uint32_t ttl; // Time-to-live in seconds

        Message()
            : message_id(""), sender_id(""), receiver_id(""), type(MessageType::Request), content(""),
              priority(TaskPriority::Medium), timestamp(0), ttl(60) {}
    };

    struct Proposal {
        std::string proposal_id;
        std::string proposer_id;
        std::string task_description;
        std::vector<std::string> required_agents;
        real_T estimated_duration_sec;
        TaskPriority priority;
        std::map<std::string, std::string> parameters;
        std::vector<std::string> votes; // Agent IDs that voted
        uint64_t expiry_timestamp;

        Proposal()
            : proposal_id(""), proposer_id(""), task_description(""), estimated_duration_sec(0),
              priority(TaskPriority::Medium), expiry_timestamp(0) {}
    };

    struct Consensus {
        std::string consensus_id;
        std::string topic;
        std::map<std::string, std::string> votes; // agent_id -> vote
        uint32_t required_votes;
        bool achieved;
        uint64_t timestamp;

        Consensus() : consensus_id(""), topic(""), required_votes(0), achieved(false), timestamp(0) {}
    };

    struct ProtocolConfig {
        std::string agent_id;
        uint16_t port;
        uint32_t max_connections;
        uint32_t message_buffer_size;
        real_T heartbeat_interval_sec;
        real_T connection_timeout_sec;
        bool enable_encryption;

        ProtocolConfig()
            : agent_id("agent_0"), port(9100), max_connections(100), message_buffer_size(1000),
              heartbeat_interval_sec(1.0f), connection_timeout_sec(10.0f), enable_encryption(false) {}
    };

    using MessageCallback = std::function<void(const Message &)>;

  public:
    A2AProtocol();
    explicit A2AProtocol(const ProtocolConfig &config);
    ~A2AProtocol();

    void initialize(const ProtocolConfig &config);
    void start();
    void stop();
    void reset();

    // Message passing
    bool sendMessage(const Message &message);
    bool sendBroadcast(const Message &message);
    std::vector<Message> receiveMessages();
    Message receiveMessage(const std::string &sender_id = "");
    bool hasMessages() const;

    // Callbacks
    void registerMessageCallback(MessageType type, MessageCallback callback);
    void unregisterMessageCallback(MessageType type);

    // Task negotiation
    bool proposeTask(const Proposal &proposal);
    bool acceptProposal(const std::string &proposal_id, const std::string &agent_id);
    bool rejectProposal(const std::string &proposal_id, const std::string &agent_id, const std::string &reason);
    Proposal getProposal(const std::string &proposal_id) const;
    std::vector<Proposal> getActiveProposals() const;
    bool cancelProposal(const std::string &proposal_id);

    // Consensus building
    bool initiateConsensus(const std::string &topic, uint32_t required_votes);
    bool vote(const std::string &consensus_id, const std::string &agent_id, const std::string &vote);
    Consensus getConsensusStatus(const std::string &consensus_id) const;
    std::vector<Consensus> getActiveConsensus() const;

    // Peer management
    bool connectToPeer(const std::string &peer_id, const std::string &address, uint16_t port);
    bool disconnectFromPeer(const std::string &peer_id);
    std::vector<std::string> getConnectedPeers() const;
    bool isPeerConnected(const std::string &peer_id) const;

    // Discovery
    std::vector<std::string> discoverPeers(real_T radius_meters = 100.0f);
    bool announceSelf(const std::map<std::string, std::string> &capabilities);

    // State queries
    uint32_t getPeerCount() const { return static_cast<uint32_t>(peers_.size()); }
    uint32_t getMessageCount() const;
    bool isRunning() const { return running_; }

    const ProtocolConfig &getConfig() const { return config_; }

  private:
    // Internal methods
    void processIncomingMessages();
    void sendHeartbeats();
    void checkPeerTimeouts();
    void cleanupExpiredProposals();
    std::string generateMessageId() const;
    uint64_t getCurrentTimestamp() const;
    bool validateMessage(const Message &message) const;

  private:
    ProtocolConfig config_;
    bool running_;
    bool initialized_;

    // Data storage
    std::vector<Message> message_queue_;
    std::map<std::string, Proposal> proposals_;
    std::map<std::string, Consensus> consensus_map_;
    std::map<std::string, uint64_t> peers_; // peer_id -> last_heartbeat
    std::map<MessageType, MessageCallback> callbacks_;

    // Thread safety
    mutable std::mutex message_mutex_;
    mutable std::mutex proposal_mutex_;
    mutable std::mutex consensus_mutex_;
    mutable std::mutex peer_mutex_;
    mutable std::mutex callback_mutex_;
};

} // namespace modules
} // namespace autonomylib
} // namespace nervosys

#endif // autonomylib_modules_communication_A2AProtocol_hpp
