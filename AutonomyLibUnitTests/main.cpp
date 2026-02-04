#include "CelestialTests.hpp"
#include "PixhawkTest.hpp"
#include "QuaternionTest.hpp"
#include "SettingsTest.hpp"
// SimpleFlightTest requires full simulation environment
// #include "SimpleFlightTest.hpp"
#include "WorkerThreadTest.hpp"

#include <iostream>

using namespace nervosys::autonomylib;

int main() {
    std::cout << "Starting unit tests..." << std::endl;

    try {
        std::cout << "Running QuaternionTest..." << std::endl;
        QuaternionTest qt;
        qt.run();
        std::cout << "QuaternionTest PASSED" << std::endl;

        std::cout << "Running CelestialTest..." << std::endl;
        CelestialTest ct;
        ct.run();
        std::cout << "CelestialTest PASSED" << std::endl;

        std::cout << "Running SettingsTest..." << std::endl;
        SettingsTest st;
        st.run();
        std::cout << "SettingsTest PASSED" << std::endl;

        // SimpleFlightTest skipped - requires full simulation
        std::cout << "All tests PASSED!" << std::endl;
    } catch (const std::exception &e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
