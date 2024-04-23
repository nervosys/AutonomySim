#ifndef autonomylib_vehicles_simpleflight_firmware_IBoardClock_hpp
#define autonomylib_vehicles_simpleflight_firmware_IBoardClock_hpp

#include <cstdint>

namespace simple_flight {

class IBoardClock {
  public:
    virtual uint64_t micros() const = 0;
    virtual uint64_t millis() const = 0;

    virtual ~IBoardClock() = default;
};

} // namespace simple_flight

#endif
