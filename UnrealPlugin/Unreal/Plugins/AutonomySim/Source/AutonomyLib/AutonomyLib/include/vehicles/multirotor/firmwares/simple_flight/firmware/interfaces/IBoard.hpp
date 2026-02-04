#ifndef autonomylib_vehicles_simpleflight_firmware_IBoard_hpp
#define autonomylib_vehicles_simpleflight_firmware_IBoard_hpp

#include "IBoardClock.hpp"
#include "IBoardInputPins.hpp"
#include "IBoardOutputPins.hpp"
#include "IBoardSensors.hpp"
#include "IUpdatable.hpp"

namespace simple_flight {

class IBoard : public IUpdatable,
               public IBoardClock,
               public IBoardInputPins,
               public IBoardOutputPins,
               public IBoardSensors {};

} // namespace simple_flight

#endif
