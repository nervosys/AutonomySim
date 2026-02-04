#ifndef autonomylib_vehicles_simpleflight_firmware_IBoardInputPins_hpp
#define autonomylib_vehicles_simpleflight_firmware_IBoardInputPins_hpp

#include <cstdint>

namespace simple_flight {

class IBoardInputPins {

  public:
    virtual float readChannel(uint16_t index) const = 0; // output -1 to 1
    virtual bool isRcConnected() const = 0;
    virtual float getAvgMotorOutput() const = 0;

    virtual ~IBoardInputPins() = default;
};

} // namespace simple_flight

#endif
