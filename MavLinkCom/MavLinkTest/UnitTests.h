// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once
#include "MavLinkFtpClient.hpp"
#include <functional>
#include <string>

typedef std::function<void()> TestHandler;
class ImageServer;

class UnitTests {
  public:
    void RunAll(std::string comPort, int boardRate);
    void SerialPx4Test();
    void UdpPingTest();
    void TcpPingTest();
    void SendImageTest();
    void FtpTest();
    void JSonLogTest();

  private:
    void RunTest(const std::string &name, TestHandler handler);
    void VerifyFile(mavlink_comm::MavLinkFtpClient &ftp, const std::string &dir, const std::string &name, bool exists,
                    bool isdir);
    ImageServer *server_;
    std::string com_port_;
    int baud_rate_;
};
