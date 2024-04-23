#ifndef autonomylib_vehicles_simpleflight_firmware_IFirmware_hpp
#define autonomylib_vehicles_simpleflight_firmware_IFirmware_hpp

#include "IOffboardApi.hpp"
#include "IStateEstimator.hpp"
#include "IUpdatable.hpp"

namespace simple_flight {

class IFirmware : public IUpdatable {
  public:
    virtual IOffboardApi &offboardApi() = 0;
};

} // namespace simple_flight

#endif
