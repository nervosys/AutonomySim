// Copyright (c) nervosys. All rights reserved.
// Licensed under the MIT License.

#ifndef autonomylib_modules_communication_MCPServer_hpp
#define autonomylib_modules_communication_MCPServer_hpp

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
 * @brief Model Context Protocol (MCP) Server for autonomous vehicles
 *
 * Enables vehicles to share context, register tools, and manage resources
 * in a collaborative multi-agent environment. Provides:
 * - Context sharing (perception, planning, execution)
 * - Tool registration and discovery
 * - Resource management
 * - State synchronization
 */
class MCPServer {
  public:
    struct ContextData {
        std::string agent_id;
        Vector3r position;
        Vector3r velocity;
        Quaternionr orientation;
        std::string mission_state;
        std::map<std::string, std::string> perception_data;
        std::map<std::string, std::string> planning_data;
        std::map<std::string, std::string> execution_data;
        uint64_t timestamp;

        ContextData()
            : agent_id(""), position(Vector3r::Zero()), velocity(Vector3r::Zero()),
              orientation(Quaternionr::Identity()), mission_state("idle"), timestamp(0) {}
    };

    struct Tool {
        std::string name;
        std::string description;
        std::vector<std::string> parameters;
        std::function<std::string(const std::map<std::string, std::string> &)> execute;
        std::string agent_id; // Agent that provides this tool

        Tool() : name(""), description(""), agent_id("") {}
    };

    struct Resource {
        std::string id;
        std::string type; // "sensor", "actuator", "computation", "energy"
        std::string owner_id;
        bool available;
        real_T capacity;
        real_T current_usage;
        std::map<std::string, std::string> metadata;

        Resource() : id(""), type(""), owner_id(""), available(true), capacity(1.0f), current_usage(0.0f) {}
    };

    struct ServerConfig {
        std::string server_id;
        uint16_t port;
        uint32_t max_agents;
        uint32_t context_buffer_size;
        real_T context_timeout_sec;
        bool enable_encryption;

        ServerConfig()
            : server_id("mcp_server_0"), port(9000), max_agents(100), context_buffer_size(1000),
              context_timeout_sec(5.0f), enable_encryption(false) {}
    };

  public:
    MCPServer();
    explicit MCPServer(const ServerConfig &config);
    ~MCPServer();

    void initialize(const ServerConfig &config);
    void start();
    void stop();
    void reset();

    // Context management
    bool publishContext(const ContextData &context);
    std::vector<ContextData> queryContext(const std::string &agent_id = "") const;
    ContextData getLatestContext(const std::string &agent_id) const;
    std::vector<ContextData> getContextHistory(const std::string &agent_id, uint32_t count) const;

    // Tool management
    bool registerTool(const Tool &tool);
    bool unregisterTool(const std::string &tool_name, const std::string &agent_id);
    std::vector<Tool> discoverTools(const std::string &capability = "") const;
    std::string executeTool(const std::string &tool_name, const std::map<std::string, std::string> &params);

    // Resource management
    bool registerResource(const Resource &resource);
    bool unregisterResource(const std::string &resource_id);
    std::vector<Resource> queryResources(const std::string &type = "") const;
    bool requestResource(const std::string &resource_id, const std::string &requester_id, real_T amount);
    bool releaseResource(const std::string &resource_id, const std::string &requester_id);

    // Agent management
    bool registerAgent(const std::string &agent_id, const std::map<std::string, std::string> &capabilities);
    bool unregisterAgent(const std::string &agent_id);
    std::vector<std::string> getConnectedAgents() const;
    bool isAgentConnected(const std::string &agent_id) const;

    // State queries
    uint32_t getAgentCount() const { return static_cast<uint32_t>(agents_.size()); }
    uint32_t getToolCount() const { return static_cast<uint32_t>(tools_.size()); }
    uint32_t getResourceCount() const { return static_cast<uint32_t>(resources_.size()); }
    bool isRunning() const { return running_; }

    const ServerConfig &getConfig() const { return config_; }

  private:
    // Internal methods
    void cleanupOldContext();
    bool validateContext(const ContextData &context) const;
    bool validateTool(const Tool &tool) const;
    bool validateResource(const Resource &resource) const;
    uint64_t getCurrentTimestamp() const;

  private:
    ServerConfig config_;
    bool running_;
    bool initialized_;

    // Data storage
    std::map<std::string, std::vector<ContextData>> context_history_;
    std::map<std::string, Tool> tools_;
    std::map<std::string, Resource> resources_;
    std::map<std::string, std::map<std::string, std::string>> agents_;

    // Thread safety
    mutable std::mutex context_mutex_;
    mutable std::mutex tools_mutex_;
    mutable std::mutex resources_mutex_;
    mutable std::mutex agents_mutex_;
};

} // namespace modules
} // namespace autonomylib
} // namespace nervosys

#endif // autonomylib_modules_communication_MCPServer_hpp
