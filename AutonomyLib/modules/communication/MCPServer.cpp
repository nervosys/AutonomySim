// Copyright (c) nervosys. All rights reserved.
// Licensed under the MIT License.

#include "modules/communication/MCPServer.hpp"
#include <algorithm>
#include <chrono>

namespace nervosys {
namespace autonomylib {
namespace modules {

MCPServer::MCPServer() : running_(false), initialized_(false) {}

MCPServer::MCPServer(const ServerConfig &config) : running_(false), initialized_(false) { initialize(config); }

MCPServer::~MCPServer() { stop(); }

void MCPServer::initialize(const ServerConfig &config) {
    std::lock_guard<std::mutex> lock_context(context_mutex_);
    std::lock_guard<std::mutex> lock_tools(tools_mutex_);
    std::lock_guard<std::mutex> lock_resources(resources_mutex_);
    std::lock_guard<std::mutex> lock_agents(agents_mutex_);

    config_ = config;
    context_history_.clear();
    tools_.clear();
    resources_.clear();
    agents_.clear();

    initialized_ = true;
}

void MCPServer::start() {
    if (!initialized_) {
        throw std::runtime_error("MCPServer not initialized");
    }
    running_ = true;
}

void MCPServer::stop() { running_ = false; }

void MCPServer::reset() {
    stop();
    std::lock_guard<std::mutex> lock_context(context_mutex_);
    std::lock_guard<std::mutex> lock_tools(tools_mutex_);
    std::lock_guard<std::mutex> lock_resources(resources_mutex_);
    std::lock_guard<std::mutex> lock_agents(agents_mutex_);

    context_history_.clear();
    tools_.clear();
    resources_.clear();
    agents_.clear();
}

// Context management
bool MCPServer::publishContext(const ContextData &context) {
    if (!running_ || !validateContext(context)) {
        return false;
    }

    std::lock_guard<std::mutex> lock(context_mutex_);

    auto &history = context_history_[context.agent_id];
    history.push_back(context);

    // Limit buffer size
    if (history.size() > config_.context_buffer_size) {
        history.erase(history.begin());
    }

    cleanupOldContext();
    return true;
}

std::vector<MCPServer::ContextData> MCPServer::queryContext(const std::string &agent_id) const {
    std::lock_guard<std::mutex> lock(context_mutex_);

    std::vector<ContextData> results;

    if (agent_id.empty()) {
        // Return latest context from all agents
        for (const auto &pair : context_history_) {
            if (!pair.second.empty()) {
                results.push_back(pair.second.back());
            }
        }
    } else {
        // Return context for specific agent
        auto it = context_history_.find(agent_id);
        if (it != context_history_.end()) {
            results = it->second;
        }
    }

    return results;
}

MCPServer::ContextData MCPServer::getLatestContext(const std::string &agent_id) const {
    std::lock_guard<std::mutex> lock(context_mutex_);

    auto it = context_history_.find(agent_id);
    if (it != context_history_.end() && !it->second.empty()) {
        return it->second.back();
    }

    return ContextData();
}

std::vector<MCPServer::ContextData> MCPServer::getContextHistory(const std::string &agent_id, uint32_t count) const {
    std::lock_guard<std::mutex> lock(context_mutex_);

    auto it = context_history_.find(agent_id);
    if (it == context_history_.end() || it->second.empty()) {
        return std::vector<ContextData>();
    }

    const auto &history = it->second;
    uint32_t start = history.size() > count ? history.size() - count : 0;

    return std::vector<ContextData>(history.begin() + start, history.end());
}

// Tool management
bool MCPServer::registerTool(const Tool &tool) {
    if (!running_ || !validateTool(tool)) {
        return false;
    }

    std::lock_guard<std::mutex> lock(tools_mutex_);

    std::string key = tool.agent_id + ":" + tool.name;
    tools_[key] = tool;
    return true;
}

bool MCPServer::unregisterTool(const std::string &tool_name, const std::string &agent_id) {
    std::lock_guard<std::mutex> lock(tools_mutex_);

    std::string key = agent_id + ":" + tool_name;
    return tools_.erase(key) > 0;
}

std::vector<MCPServer::Tool> MCPServer::discoverTools(const std::string &capability) const {
    std::lock_guard<std::mutex> lock(tools_mutex_);

    std::vector<Tool> results;

    for (const auto &pair : tools_) {
        const Tool &tool = pair.second;
        if (capability.empty() || tool.description.find(capability) != std::string::npos) {
            results.push_back(tool);
        }
    }

    return results;
}

std::string MCPServer::executeTool(const std::string &tool_name, const std::map<std::string, std::string> &params) {
    std::lock_guard<std::mutex> lock(tools_mutex_);

    // Find tool (any agent)
    for (const auto &pair : tools_) {
        const Tool &tool = pair.second;
        if (tool.name == tool_name) {
            if (tool.execute) {
                return tool.execute(params);
            }
        }
    }

    return "Error: Tool not found or not executable";
}

// Resource management
bool MCPServer::registerResource(const Resource &resource) {
    if (!running_ || !validateResource(resource)) {
        return false;
    }

    std::lock_guard<std::mutex> lock(resources_mutex_);
    resources_[resource.id] = resource;
    return true;
}

bool MCPServer::unregisterResource(const std::string &resource_id) {
    std::lock_guard<std::mutex> lock(resources_mutex_);
    return resources_.erase(resource_id) > 0;
}

std::vector<MCPServer::Resource> MCPServer::queryResources(const std::string &type) const {
    std::lock_guard<std::mutex> lock(resources_mutex_);

    std::vector<Resource> results;

    for (const auto &pair : resources_) {
        const Resource &resource = pair.second;
        if (type.empty() || resource.type == type) {
            results.push_back(resource);
        }
    }

    return results;
}

bool MCPServer::requestResource(const std::string &resource_id, const std::string &requester_id, real_T amount) {
    std::lock_guard<std::mutex> lock(resources_mutex_);

    auto it = resources_.find(resource_id);
    if (it == resources_.end()) {
        return false;
    }

    Resource &resource = it->second;

    if (!resource.available || resource.current_usage + amount > resource.capacity) {
        return false;
    }

    resource.current_usage += amount;
    return true;
}

bool MCPServer::releaseResource(const std::string &resource_id, const std::string &requester_id) {
    std::lock_guard<std::mutex> lock(resources_mutex_);

    auto it = resources_.find(resource_id);
    if (it == resources_.end()) {
        return false;
    }

    Resource &resource = it->second;
    resource.current_usage = 0;

    return true;
}

// Agent management
bool MCPServer::registerAgent(const std::string &agent_id, const std::map<std::string, std::string> &capabilities) {
    if (!running_ || agent_id.empty()) {
        return false;
    }

    std::lock_guard<std::mutex> lock(agents_mutex_);

    if (agents_.size() >= config_.max_agents) {
        return false;
    }

    agents_[agent_id] = capabilities;
    return true;
}

bool MCPServer::unregisterAgent(const std::string &agent_id) {
    std::lock_guard<std::mutex> lock(agents_mutex_);
    return agents_.erase(agent_id) > 0;
}

std::vector<std::string> MCPServer::getConnectedAgents() const {
    std::lock_guard<std::mutex> lock(agents_mutex_);

    std::vector<std::string> agent_ids;
    agent_ids.reserve(agents_.size());

    for (const auto &pair : agents_) {
        agent_ids.push_back(pair.first);
    }

    return agent_ids;
}

bool MCPServer::isAgentConnected(const std::string &agent_id) const {
    std::lock_guard<std::mutex> lock(agents_mutex_);
    return agents_.find(agent_id) != agents_.end();
}

// Internal methods
void MCPServer::cleanupOldContext() {
    uint64_t current_time = getCurrentTimestamp();
    uint64_t timeout_ms = static_cast<uint64_t>(config_.context_timeout_sec * 1000);

    for (auto &pair : context_history_) {
        auto &history = pair.second;
        history.erase(std::remove_if(history.begin(), history.end(),
                                     [current_time, timeout_ms](const ContextData &ctx) {
                                         return (current_time - ctx.timestamp) > timeout_ms;
                                     }),
                      history.end());
    }
}

bool MCPServer::validateContext(const ContextData &context) const {
    return !context.agent_id.empty() && context.timestamp > 0;
}

bool MCPServer::validateTool(const Tool &tool) const { return !tool.name.empty() && !tool.agent_id.empty(); }

bool MCPServer::validateResource(const Resource &resource) const {
    return !resource.id.empty() && !resource.type.empty() && resource.capacity > 0;
}

uint64_t MCPServer::getCurrentTimestamp() const {
    return static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
            .count());
}

} // namespace modules
} // namespace autonomylib
} // namespace nervosys
