#ifndef autonomylib_unittests_CelestialTest_hpp
#define autonomylib_unittests_CelestialTest_hpp

#include "TestBase.hpp"
#include "common/EarthCelestial.hpp"

namespace nervosys {
namespace autonomylib {

class CelestialTest : public TestBase {
  public:
    virtual void run() override {
        auto t = static_cast<uint64_t>(Utils::to_time_t("2018-February-22 15:24:00"));
        auto c_sun = EarthCelestial::getSunCoordinates(t, 47.673988, -122.121513);
        auto c_moon = EarthCelestial::getMoonCoordinates(t, 47.673988, -122.121513);
        auto c_moon_phase = EarthCelestial::getMoonPhase(t);

        testAssert(Utils::isApproximatelyEqual(c_sun.altitude, 19.67, 0.1), "Sun azimuth is not correct");
        testAssert(Utils::isApproximatelyEqual(c_moon.altitude, 45.02, 0.1), "Monn azimuth is not correct");
        testAssert(Utils::isApproximatelyEqual(c_moon_phase.fraction, 0.47, 0.1), "Moon fraction is not correct");
    }
};

} // namespace autonomylib
} // namespace nervosys

#endif