#include "test.hpp"

#include "doctest/doctest.h" // Note: lwip overwrites stream functions
#include "log.hpp"

using namespace Log;
using enum Log::Level;

TEST_CASE("Testing Logging") {
    CHECK(logit(FATAL, "Testing %u", 1) != -1);
    CHECK(logit(CRITICAL, "Testing %u", 2) != -1);
    CHECK(logit(ERROR, "Testing %u", 3) != -1);
    CHECK(logit(WARNING, "Testing %u", 4) != -1);
    CHECK(logit(NOTICE, "Testing %u", 5) != -1);
    CHECK(logit(INFO, "Testing %u", 5) != -1);
    CHECK(logit(DEBUG, "Testing %u", 7) != -1);
    CHECK(logit(TRACE, "Testing %u", 8) != -1);
    CHECK(logme(FATAL, "doctest", "Testing %u", 1) != -1);
    CHECK(logme(CRITICAL, "doctest", "Testing %u", 2) != -1);
    CHECK(logme(ERROR, "doctest", "Testing %u", 3) != -1);
    CHECK(logme(WARNING, "doctest", "Testing %u", 4) != -1);
    CHECK(logme(NOTICE, "doctest", "Testing %u", 5) != -1);
    CHECK(logme(INFO, "doctest", "Testing %u", 5) != -1);
    CHECK(logme(DEBUG, "doctest", "Testing %u", 7) != -1);
    CHECK(logme(TRACE, "doctest", "Testing %u", 8) != -1);
}

namespace Test {

int32_t baseline_test() {
    doctest::Context context;
    context.setOption("abort-after", 5);   // stop test execution after 5 failed assertions
    context.setOption("order-by", "name"); // sort the test cases by their name
    return context.run();
}

} // namespace Test
