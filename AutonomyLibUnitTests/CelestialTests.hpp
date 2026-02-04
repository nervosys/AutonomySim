#ifndef autonomylib_unittests_CelestialTest_hpp
#define autonomylib_unittests_CelestialTest_hpp

#include "TestBase.hpp"
#include "common/EarthCelestial.hpp"

#include <iostream>

namespace nervosys {
namespace autonomylib {

class CelestialTest : public TestBase {

  public:
    virtual void run() override {
        // Feb 22, 2018 15:24:00 UTC should be Unix timestamp 1519313040
        auto time_t_val = Utils::to_time_t("2018-02-22 15:24:00");
        auto t = static_cast<uint64_t>(time_t_val);

        std::cout << "Parsed time_t: " << time_t_val << std::endl;
        std::cout << "Expected Unix timestamp for Feb 22, 2018 15:24:00 UTC: 1519313040" << std::endl;

        auto c_sun = EarthCelestial::getSunCoordinates(t, 47.673988, -122.121513);
        auto c_moon = EarthCelestial::getMoonCoordinates(t, 47.673988, -122.121513);
        auto c_moon_phase = EarthCelestial::getMoonPhase(t);

        std::cout << "Sun altitude: " << c_sun.altitude << " (expected: 19.67)" << std::endl;
        std::cout << "Moon altitude: " << c_moon.altitude << " (expected: 45.02)" << std::endl;
        std::cout << "Moon phase fraction: " << c_moon_phase.fraction << " (expected: 0.47)" << std::endl;

        testAssert(Utils::isApproximatelyEqual(c_sun.altitude, 19.67, 0.5), "Sun altitude is not correct");
        testAssert(Utils::isApproximatelyEqual(c_moon.altitude, 45.02, 1.0), "Moon altitude is not correct");
        testAssert(Utils::isApproximatelyEqual(c_moon_phase.fraction, 0.47, 0.1), "Moon fraction is not correct");
    }
};

} // namespace autonomylib
} // namespace nervosys

#endif
