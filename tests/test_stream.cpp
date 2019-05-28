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

TEST_CASE("Flatten stream 1", "[stream]" ) {
    auto s1 = make_stream({ 1, 2, 3 });
    auto s2 = make_stream({ 4, 5, 6 });
    auto s3 = make_stream({ 7, 8, 9 });

    auto ss = make_stream({ s1, s2, s3 })
                .flatten();

    for(int i = 1; i < 10; ++i)
        REQUIRE(ss.next() == i);

    REQUIRE_THROWS(ss.collect());
}

TEST_CASE("Flatten stream 2", "[stream]" ) {
    int n = 0;
    auto ss = make_stream([&n]()
    {
        return std::make_optional(make_stream([&n] () mutable -> std::optional<int>
        {
            return std::make_optional(++n);
        })
        .take(3));
    })
    .take(3)
    .flatten();

    for(int i = 1; i < 10; ++i)
        REQUIRE(ss.next() == i);

    REQUIRE_THROWS(ss.collect());
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

TEST_CASE("Zip streams", "[stream]" ) {
    auto s1 = make_stream({ 1, 2, 3 });
    auto s2 = make_stream({ -1, -2, -3, -4 });
    auto ss = s1.zip(std::move(s2), std::plus<>());

    for(int i = 0; i < 3; ++i)
        REQUIRE(ss.next() == 0);

    REQUIRE_THROWS(ss.collect());
}

TEST_CASE("Slice stream", "[stream]" ) {
    size_t step = 2;

    for(size_t start = 0; start < 5; ++start)
    {
        for(size_t end = start; end < 5; ++end)
        {
            auto s = make_stream([n = 0]() mutable { return make_optional(n++); })
                        .slice(start, end, step);
            for(size_t i = start; i < end; i += step)
                REQUIRE(s.next() == i);
            REQUIRE_THROWS(s.collect());
        }
    }
}

TEST_CASE("Slice stream to end", "[stream]" ) {
    size_t step = 2;

    for(size_t start = 0; start < 5; ++start)
    {
        auto s = make_stream({ 0, 1, 2, 3, 4 })
                    .slice_to_end(start, step);
        for(size_t i = start; i < 5; i += step)
            REQUIRE(s.next() == i);
        REQUIRE_THROWS(s.collect());
    }
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
