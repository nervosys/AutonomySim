#pragma once

#include "Commands.h"
#include <string>

class ConsoleBase {
  public:
    virtual Command *getCommand(const std::string &line) = 0;
    virtual int run() = 0;
};