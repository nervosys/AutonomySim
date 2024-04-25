// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#ifndef autonomylib_vehicles_SimpleFlightCommLink_hpp
#define autonomylib_vehicles_SimpleFlightCommLink_hpp

#include "common/Common.hpp"
#include "firmware/interfaces/ICommLink.hpp"

#include <exception>

namespace nervosys {
namespace autonomylib {

class SimpleFlightCommLink : public simple_flight::ICommLink {

  private:
    std::vector<std::string> messages_;

  public:
    // derived class specific methods
    void getStatusMessages(std::vector<std::string> &messages) {
        if (messages_.size() > 0) {
            messages.insert(messages.end(), messages_.begin(), messages_.end());
            messages_.clear();
        }
    }

    // implement CommLink interface
    virtual void reset() {
        simple_flight::ICommLink::reset();

        messages_.clear();
    }

    virtual void update() { simple_flight::ICommLink::update(); }

    virtual void log(const std::string &message, int32_t log_level = ICommLink::kLogLevelInfo) {
        unused(log_level);
        // if (log_level > 0)
        //     Utils::DebugBreak();
        messages_.push_back(std::string(message));
    }
};

} // namespace autonomylib
} // namespace nervosys

#endif
