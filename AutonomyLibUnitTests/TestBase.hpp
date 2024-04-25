#ifndef autonomylib_unittests_TestBase_hpp
#define autonomylib_unittests_TestBase_hpp

#include "common/utils/Utils.hpp"

#include <exception>
#include <string>

namespace nervosys {
namespace autonomylib {

class TestBase {

  public:
    virtual ~TestBase() = default;
    virtual void run() = 0;

    void testAssert(double lhs, double rhs, const std::string &message) { testAssert(lhs == rhs, message); }

    void testAssert(bool condition, const std::string &message) {
        if (!condition) {
            common_utils::Utils::DebugBreak();
            throw std::runtime_error(message.c_str());
        }
    }
};

} // namespace autonomylib
} // namespace nervosys

#endif