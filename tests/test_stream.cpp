#include <plusar/stream.hpp>
#include "catch.hpp"
#include <functional>
#include <vector>
#include <iterator>

using namespace plusar;
using namespace std;

TEST_CASE("Stream creation by initializer list", "[stream]" ) {
    REQUIRE(make_stream({ 42 })
                .collect() == 42);
}

TEST_CASE("Stream creation by functor", "[stream]" ) {
    REQUIRE(make_stream([]() { return make_optional(42); })
                .take(1)
                .collect() == 42);
}

TEST_CASE("Filter function", "[stream]" ) {
    REQUIRE(make_stream({ 1, 2, 3 })
                .filter([](int t) { return t % 2 == 0; })
                .collect() == 2);
}

TEST_CASE("Map function", "[stream]" ) {
    REQUIRE(make_stream({ 21 })
                .map([](int t) { return t * 2; })
                .collect() == 42);
}

TEST_CASE("Reduce function", "[stream]" ) {
    REQUIRE(make_stream({ 20, 22 })
                .reduce(0, std::plus<>())
                .collect() == 42);
}

TEST_CASE("Take N elements", "[stream]" ) {
    REQUIRE(make_stream([]() { return make_optional(5); })
                .take(3)
                .reduce(0, std::plus<>())
                .collect() == 15);
}

TEST_CASE("Skip N elements", "[stream]" ) {
    REQUIRE(make_stream([n = 0]() mutable { return make_optional(n++); })
                .skip(3)
                .take(3)
                .reduce(0, std::plus<>())
                .collect() == 12);

    REQUIRE(make_stream([n = 0]() mutable { return make_optional(n++); })
                .take(6)
                .skip(3)
                .reduce(0, std::plus<>())
                .collect() == 12);
}

TEST_CASE("Collect items from stream", "[stream]" ) {
    std::vector<int> v;

    make_stream([n = 0]() mutable { return make_optional(n++); })
        .take(3)
        .collect(std::back_inserter(v));

    int m = 0;
    for(auto n: v)
        REQUIRE(n == m++);
}

TEST_CASE("Read next item from stream", "[stream]" ) {

    auto s = make_stream([n = 0]() mutable { return make_optional(n++); });
    REQUIRE(s.next() == 0);
    REQUIRE(s.next() == 1);
    REQUIRE(s.next() == 2);
}
