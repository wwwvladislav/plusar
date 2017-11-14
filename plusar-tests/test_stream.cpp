#include <core/environment.hpp>
#include <catch.hpp>

using namespace plusar;

TEST_CASE("Source stream reading", "[environment]" ) {
    REQUIRE(environment()
                .source([]() { return make_optional(1); })
                .take(1)
                .collect() == 1);
}
