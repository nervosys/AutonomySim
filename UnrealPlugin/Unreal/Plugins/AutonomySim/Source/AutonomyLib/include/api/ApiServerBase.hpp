// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#ifndef autonomylib_api_ApiServerBase_hpp
#define autonomylib_api_ApiServerBase_hpp

#include "common/Common.hpp"
#include <functional>

namespace nervosys {
namespace autonomylib {

class ApiServerBase {
  public:
    virtual void start(bool block, std::size_t thread_count) = 0;
    virtual void stop() = 0;

    virtual ~ApiServerBase() = default;
};

} // namespace autonomylib
} // namespace nervosys

#endif
