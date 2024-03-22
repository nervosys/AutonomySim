// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#ifndef MavLinkCom_MavLinkLog_hpp
#define MavLinkCom_MavLinkLog_hpp

#include "MavLinkMessageBase.hpp"
#include <cstdint>
#include <mutex>
#include <stdio.h>
#include <string>

#define MAVLINK_STX_MAVLINK1 0xFE // marker for old protocol

namespace mavlink_comm {

// This abstract class defines the interface for logging MavLinkMessages.
class MavLinkLog {
  public:
    virtual void write(const mavlink_comm::MavLinkMessage &msg, uint64_t timestamp = 0) = 0;
    virtual ~MavLinkLog() = default;
};

// This implementation of MavLinkLog reads/writes MavLinkMessages to a local file.
class MavLinkFileLog : public MavLinkLog {
    std::string file_name_;
    FILE *ptr_;
    bool reading_;
    bool writing_;
    bool json_;
    std::mutex log_lock_;

  public:
    MavLinkFileLog();
    virtual ~MavLinkFileLog();
    bool isOpen();
    void openForReading(const std::string &filename);
    void openForWriting(const std::string &filename, bool json = false);
    void close();
    virtual void write(const mavlink_comm::MavLinkMessage &msg, uint64_t timestamp = 0) override;
    bool read(mavlink_comm::MavLinkMessage &msg, uint64_t &timestamp);
    static uint64_t getTimeStamp();
};
} // namespace mavlink_comm

#endif
