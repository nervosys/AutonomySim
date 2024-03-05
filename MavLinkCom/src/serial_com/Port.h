// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#ifndef PORT_H
#define PORT_H

#include <cstdint>

class Port {
  public:
    // write to the port, return number of bytes written or -1 if error.
    virtual int write(const uint8_t *ptr, int count){};

    // read a given number of bytes from the port (blocking until the requested bytes are available).
    // return the number of bytes read or -1 if error.
    virtual int read(uint8_t *buffer, int bytesToRead){};

    // close the port.
    virtual void close(){};

    virtual bool isClosed(){};

    virtual int getRssi(const char *ifaceName){};

    Port() = default;
    virtual ~Port() = default;
};

#endif // !PORT_H
