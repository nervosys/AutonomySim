#ifndef autonomylib_vehicles_simpleflight_firmware_IPidIntegrator_hpp
#define autonomylib_vehicles_simpleflight_firmware_IPidIntegrator_hpp

#include "CommonStructs.hpp"

#include <algorithm>

namespace simple_flight {

template <typename T> class IPidIntegrator {

  public:
    virtual void reset() = 0;
    virtual void set(T val) = 0;
    virtual void update(float dt, T error, uint64_t last_time) = 0;
    virtual T getOutput() = 0;

    virtual ~IPidIntegrator() {}
};

} // namespace simple_flight

#endif
