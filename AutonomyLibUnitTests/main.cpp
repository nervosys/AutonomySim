#include "CelestialTests.hpp"
#include "PixhawkTest.hpp"
#include "QuaternionTest.hpp"
#include "SettingsTest.hpp"
#include "SimpleFlightTest.hpp"
#include "WorkerThreadTest.hpp"

using namespace nervosys::autonomylib;

int main() {

    std::unique_ptr<TestBase> tests[] = {
        std::unique_ptr<TestBase>(new QuaternionTest()), std::unique_ptr<TestBase>(new CelestialTest()),
        std::unique_ptr<TestBase>(new SettingsTest()), std::unique_ptr<TestBase>(new SimpleFlightTest())
        //,
        // std::unique_ptr<TestBase>(new PixhawkTest()),
        // std::unique_ptr<TestBase>(new WorkerThreadTest())
    };

    for (auto &test : tests) {
        test->run();
    };

    return 0;
}
