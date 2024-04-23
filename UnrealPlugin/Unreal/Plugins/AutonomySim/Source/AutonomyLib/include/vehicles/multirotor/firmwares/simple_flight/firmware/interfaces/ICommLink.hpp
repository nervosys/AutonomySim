#ifndef autonomylib_vehicles_simpleflight_firmware_ICommLink_hpp
#define autonomylib_vehicles_simpleflight_firmware_ICommLink_hpp

#include "IUpdatable.hpp"
#include <cstdint>

namespace simple_flight {

class ICommLink : public IUpdatable {
  public:
    static constexpr int kLogLevelInfo = 0;
    static constexpr int kLogLevelWarn = 1;
    static constexpr int kLogLevelError = 2;

    virtual void log(const std::string &message, int32_t log_level = ICommLink::kLogLevelInfo) = 0;
};

} // namespace simple_flight

#endif
